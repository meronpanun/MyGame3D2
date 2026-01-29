#include "EnemyRunner.h"
#include "Bullet.h"
#include "CapsuleCollider.h"
#include "CollisionGrid.h"
#include "DebugUtil.h"
#include "DxLib.h"
#include "EffekseerForDXLib.h"
#include "Player.h"
#include "SceneMain.h"
#include "SphereCollider.h"
#include "TransformDataLoader.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>

namespace {
// アニメーション関連
constexpr char kAttackAnimName[] = "Armature|Attack"; // 攻撃アニメーション
constexpr char kRunAnimName[] = "Armature|Run";       // 走るアニメーション
constexpr char kDeadAnimName[] = "Armature|Death";    // 死亡アニメーション

// カプセルコライダーのサイズを定義
constexpr float kBodyColliderRadius = 20.0f;  // 体のコライダー半径
constexpr float kBodyColliderHeight = 110.0f; // 体のコライダー高さ
constexpr float kHeadRadius = 15.0f;          // 頭のコライダー半径

// 攻撃関連
constexpr int kAttackCooldownMax = 30;    // 攻撃クールダウン時間
constexpr float kAttackHitRadius = 60.0f; // 攻撃の当たり判定半径
constexpr float kAttackTriggerRadius =
    30.0f;                                   // 攻撃開始判定半径(通常より小さめ)
constexpr float kAttackRangeRadius = 100.0f; // 攻撃範囲の半径

// 追跡関連
constexpr int kAttackEndDelay = 10;

constexpr VECTOR kHeadShotPositionOffset = {0.0f, 0.0f, 0.0f};
} // namespace

int EnemyRunner::s_modelHandle = -1;

EnemyRunner::EnemyRunner()
    : m_headPosOffset{kHeadShotPositionOffset}, m_isTackleHit(false),
      m_animTime(0.0f), m_isAttackHit(false), m_onDropItem(nullptr),
      m_currentAnimState(AnimState::Run), m_attackEndDelayTimer(0),
      m_isDeadAnimPlaying(false), m_chaseSpeed(0.0f), m_isItemDropped(false) {
  // モデルの複製
  m_modelHandle = MV1DuplicateModel(s_modelHandle);

  // コライダーの初期化
  m_pBodyCollider = std::make_shared<CapsuleCollider>();
  m_pHeadCollider = std::make_shared<SphereCollider>();
  m_pAttackRangeCollider = std::make_shared<SphereCollider>();
  m_pAttackHitCollider = std::make_shared<CapsuleCollider>();
}

EnemyRunner::~EnemyRunner() {
  // モデルの解放
  MV1DeleteModel(m_modelHandle);
}

void EnemyRunner::LoadModel() {
  s_modelHandle = MV1LoadModel("data/model/RunnerZombie.mv1");
  assert(s_modelHandle != -1);
}

void EnemyRunner::DeleteModel() { MV1DeleteModel(s_modelHandle); }

void EnemyRunner::Init() {
  m_attackCooldownMax = kAttackCooldownMax;
  m_isAlive = true;

  // CSVからRunnerEnemyのTransform情報を取得
  auto dataList =
      TransformDataLoader::LoadDataCSV("data/CSV/CharacterTransfromData.csv");
  for (const auto &data : dataList) {
    if (data.name == "RunnerEnemy") {
      MV1SetRotationXYZ(m_modelHandle, data.rot);
      MV1SetScale(m_modelHandle, data.scale);
      m_attackPower = data.attack;
      m_hp = data.hp;
      m_chaseSpeed = data.chaseSpeed;
      break;
    }
  }

  // ここで一度「絶対にWalkでない値」にリセット
  // 初期アニメーションを強制的に再生させるため
  m_currentAnimState = AnimState::Dead;

  // 初期化時に走行アニメーションを開始
  ChangeAnimation(AnimState::Run, true);

  // ターゲットオフセットの初期化 (±40.0f)
  float offsetX = static_cast<float>(GetRand(80) - 40);
  float offsetZ = static_cast<float>(GetRand(80) - 40);
  m_targetOffset = VGet(offsetX, 0.0f, offsetZ);

  // 回避用パラメータ初期化
  m_evadeSwitchTimer = 0.0f;
  m_isEvadingRight = (GetRand(1) == 0);

  // 徘徊用パラメータ初期化
  m_wanderTimer = 0;
  m_wanderOffset = VGet(0.0f, 0.0f, 0.0f);
}

#include "Game.h"

void EnemyRunner::ChangeAnimation(AnimState newAnimState, bool loop) {
  // Attackだけはリセット再生
  if (m_currentAnimState == newAnimState) {
    // どの状態でも必ず再生し直す
    switch (newAnimState) {
    case AnimState::Attack:
      m_animationManager.PlayAnimation(m_modelHandle, kAttackAnimName, loop);
      break;
    case AnimState::Run:
      m_animationManager.PlayAnimation(m_modelHandle, kRunAnimName, loop);
      break;
    case AnimState::Dead:
      m_animationManager.PlayAnimation(m_modelHandle, kDeadAnimName, loop);
      break;
    }
    m_animTime = 0.0f;
    m_currentAnimState = newAnimState;
    return;
  }

  const char *animName = nullptr;

  switch (newAnimState) {
  case AnimState::Run:
    animName = kRunAnimName;
    break;
  case AnimState::Attack:
    animName = kAttackAnimName;
    break;
  case AnimState::Dead:
    animName = kDeadAnimName;
    break;
  }

  if (animName) {
    m_animationManager.PlayAnimation(m_modelHandle, animName, loop);
    m_animTime = 0.0f;
  }

  m_currentAnimState = newAnimState;
}

bool EnemyRunner::CanAttackPlayer(const Player &player, float checkRadius) {
  // 手の位置を取得
  int handRIndex = MV1SearchFrame(m_modelHandle, "mixamorig:RightHand");
  int handLIndex = MV1SearchFrame(m_modelHandle, "mixamorig:LeftHand");
  if (handRIndex == -1 || handLIndex == -1)
    return false;

  VECTOR handRPos = MV1GetFramePosition(m_modelHandle, handRIndex);
  VECTOR handLPos = MV1GetFramePosition(m_modelHandle, handLIndex);

  m_pAttackHitCollider->SetSegment(handRPos, handLPos);

  // 半径の設定
  float radius = (checkRadius > 0.0f) ? checkRadius : kAttackHitRadius;
  m_pAttackHitCollider->SetRadius(radius);

  std::shared_ptr<CapsuleCollider> playerBodyCollider =
      player.GetBodyCollider();
  return m_pAttackHitCollider->IsIntersects(playerBodyCollider.get());
}

// EnemyRunner.cpp: Update implementation
void EnemyRunner::Update(const EnemyUpdateContext &context) {
  // コンテキストから展開
  std::vector<Bullet> &bullets = context.bullets;
  const Player::TackleInfo &tackleInfo = context.tackleInfo;
  const Player &player = context.player;
  const std::vector<EnemyBase *> &enemyList = context.enemyList;
  const std::vector<Stage::StageCollisionData> &collisionData =
      context.collisionData;
  Effect *pEffect = context.pEffect;

  // AI間引き処理の更新
  UpdateThrottling(player.GetPos());

  // 視界外の単純動作モード
  if (m_isSimpleMode) {
    // ステージとの当たり判定のみ簡易に行う
    UpdateStageCollision(collisionData);
    return;
  }

  // ステージとの当たり判定
  UpdateStageCollision(collisionData);

  if (m_hp <= 0.0f) {
    if (!m_isDeadAnimPlaying) {
      // スコア加算処理はTakeDamageで行うのでここでは不要
      ChangeAnimation(AnimState::Dead, false);
      m_isDeadAnimPlaying = true;
      m_animTime = 0.0f; // アニメーション時間をリセット
      m_isAlive = true;  // 死亡アニメーション中はtrueのまま
    }

    // 死亡アニメーション中もアニメーション時間を更新
    if (m_animationManager.GetCurrentAttachedAnimHandle(m_modelHandle) != -1) {
      if (m_shouldUpdateAI) {
        m_animTime += (1.0f * m_aiUpdateInterval) * Game::GetTimeScale();
        m_animationManager.UpdateAnimationTime(m_modelHandle, m_animTime);
      }
    }

    float currentAnimTotalTime =
        m_animationManager.GetAnimationTotalTime(m_modelHandle, kDeadAnimName);
    if (m_animTime >= currentAnimTotalTime) {
      if (m_animationManager.GetCurrentAttachedAnimHandle(m_modelHandle) !=
          -1) {
        MV1DetachAnim(m_modelHandle, 0);
        m_animationManager.ResetAttachedAnimHandle(m_modelHandle);
      }
      // アイテムドロップと死亡コールバックを呼び出し
      if (!m_isItemDropped && m_onDropItem) {
        m_onDropItem(m_pos);
        m_onDropItem = nullptr;
        m_isItemDropped = true;
      }
      if (m_onDeathCallback) {
        m_onDeathCallback(m_pos);
        m_onDeathCallback = nullptr; // 一度だけ呼び出す
      }
      m_isAlive = false; // 死亡アニメーション終了時のみfalseにする
      SetActive(false);  // プールに戻す
    }
    return;
  }

  MV1SetPosition(m_modelHandle, m_pos); // モデルの位置は常に反映する

  // プレイヤーの方向を常に向く（追跡中のみ）
  // 攻撃アニメーション中は回転しない
  if (m_currentAnimState == AnimState::Run) {
    VECTOR playerPos = player.GetPos();
    VECTOR targetPos;

    // プレイヤーが岩の上にいる場合は、プレイヤーの周囲をうろうろする
    std::string groundObj = player.GetGroundedObjectName();
    if (groundObj == "rock_3_br" || groundObj == "rock_6_br") {
      m_wanderTimer--;
      if (m_wanderTimer <= 0) {
        // 2秒ごとに新しい目標位置を設定 (距離300~700) - 範囲拡大
        m_wanderTimer = 120;
        float angle = static_cast<float>(GetRand(360)) * DX_PI_F / 180.0f;
        float dist = static_cast<float>(300 + GetRand(400));
        m_wanderOffset = VGet(cosf(angle) * dist, 0.0f, sinf(angle) * dist);
      }
      targetPos = VAdd(playerPos, m_wanderOffset);
    } else {
      // 通常時はプレイヤーに向かう（オフセット付き）
      targetPos = VAdd(playerPos, m_targetOffset);
    }

    VECTOR toPlayer = VSub(targetPos, m_pos);
    toPlayer.y = 0.0f; // Y成分を無視して水平距離を計算

    // 距離計算
    float disToPlayer = VSize(toPlayer);

    float yaw = 0.0f;
    if (toPlayer.x != 0.0f || toPlayer.z != 0.0f) {
      yaw = atan2f(toPlayer.x, toPlayer.z);
      yaw += DX_PI_F; // モデルの向きに合わせて調整
    }
    // 補間速度(大きいほど速く向く)
    // 0.2f -> 0.05f に変更して滑らかにする
    float rotSpeed = 0.05f * Game::GetTimeScale();
    float currentYaw = MV1GetRotationXYZ(m_modelHandle).y;

    // 角度差を計算して滑らかに回転
    float diffYaw = yaw - currentYaw;
    while (diffYaw <= -DX_PI_F)
      diffYaw += DX_TWO_PI_F;
    while (diffYaw > DX_PI_F)
      diffYaw -= DX_TWO_PI_F;

    if (fabs(diffYaw) > rotSpeed) {
      currentYaw += (diffYaw > 0 ? rotSpeed : -rotSpeed);
    } else {
      currentYaw = yaw;
    }

    MV1SetRotationXYZ(m_modelHandle, VGet(0.0f, currentYaw, 0.0f));

    // 回避行動 (AI間引き対象にする？
    // 移動計算は毎フレームの方が滑らかだが、決定は間引ける)
    // ここでは簡易的に移動は毎フレーム、決定ロジックは間引く
    bool isEvading = false;
    if (m_updateFrameCount % m_aiUpdateInterval ==
        0) // AI更新フレームのみ判定更新
    {
      // 回避切り替え判定などがあればここに記述
      // Runnerの実装では特に複雑な回避ロジックはUpdate内にベタ書きされていないが、
      // 左右移動の決定などはここに含まれる
    }

    // 既存の移動ロジック
    {
      VECTOR dir = VNorm(toPlayer);

      // 直進成分
      VECTOR moveVec = VScale(dir, m_chaseSpeed * Game::GetTimeScale());

      // 回避成分（左右移動）
      VECTOR evadeVec = VGet(0.0f, 0.0f, 0.0f);

      // プレイヤー（カメラ）の射線がコライダーに当たっているかチェック
      VECTOR camPos = GetCameraPosition();
      VECTOR camTarget = GetCameraTarget();
      VECTOR camDir = VNorm(VSub(camTarget, camPos)); // Normalize direction

      // 5000.0f は Player::Update で使用されている射程距離
      VECTOR rayEnd = VAdd(camPos, VScale(camDir, 5000.0f));

      VECTOR hitPos;
      float hitDistSq;
      HitPart hitPart = CheckHitPart(camPos, rayEnd, hitPos, hitDistSq);

      // 射線が当たっている場合は回避行動（左右移動）を行う
      bool shouldEvade = (hitPart != HitPart::None);

      // 直接当たっていなくても、少し余裕を持たせる（レイと敵の位置の距離で判定）
      if (!shouldEvade) {
        // 敵の中心位置（足元 + 高さ/2）
        VECTOR enemyCenter = VAdd(m_pos, VGet(0.0f, 55.0f, 0.0f));

        VECTOR toEnemy = VSub(enemyCenter, camPos);
        float t = VDot(toEnemy, camDir);

        if (t > 0.0f) {
          // レイ上の最も近い点
          VECTOR nearestPoint = VAdd(camPos, VScale(camDir, t));
          VECTOR distVec = VSub(enemyCenter, nearestPoint);
          float distSq = VDot(distVec, distVec);

          // 判定半径 (体の半径20.0f * 4.0倍 = 80.0f 程度余裕を持たせる)
          constexpr float kEvasionMarginRadius = 80.0f;
          if (distSq < kEvasionMarginRadius * kEvasionMarginRadius) {
            shouldEvade = true;
          }
        }
      }

      if (shouldEvade) {
        m_evadeSwitchTimer += Game::GetTimeScale();
        if (m_evadeSwitchTimer > 60.0f) { // 1秒ごとに切り替え
          m_evadeSwitchTimer = 0.0f;
          m_isEvadingRight = !m_isEvadingRight;
        }

        VECTOR right = VCross(dir, VGet(0.0f, 1.0f, 0.0f));
        evadeVec =
            VScale(right, (m_isEvadingRight ? 1.0f : -1.0f) *
                              (m_chaseSpeed * 0.5f) * Game::GetTimeScale());
      }

      // 合成
      moveVec = VAdd(moveVec, evadeVec);
      m_pos = VAdd(m_pos, moveVec);
    }
  }

  // プレイヤーのカプセルコライダー情報を取得
  std::shared_ptr<CapsuleCollider> playerBodyCollider =
      player.GetBodyCollider();
  bool isPlayerInAttackRange =
      m_pAttackRangeCollider->IsIntersects(playerBodyCollider.get());

  // アニメーションの状態管理 (AI間引き対象)
  if (m_shouldUpdateAI) {
    if (m_currentAnimState == AnimState::Attack) {
      // 攻撃アニメーションの終了判定
      float currentAnimTotalTime = m_animationManager.GetAnimationTotalTime(
          m_modelHandle, kAttackAnimName);
      if (m_animTime > currentAnimTotalTime) {
        ChangeAnimation(AnimState::Run, true); // 走り直す
        m_attackEndDelayTimer = kAttackEndDelay;
      }
    } else if (m_currentAnimState == AnimState::Dead) {
      // 何もしない
    } else // Run
    {
      if (m_attackEndDelayTimer > 0) {
        m_attackEndDelayTimer -= m_aiUpdateInterval;
      } else {
        // 攻撃トリガー範囲内なら攻撃開始
        // 少し狭めの判定を用いる
        if (CanAttackPlayer(player, kAttackTriggerRadius)) {
          // 攻撃中フラグをリセットしてから遷移
          m_isAttackHit = false;
          ChangeAnimation(AnimState::Attack, false);
        }
      }
    }
  }

  // アニメーション更新 (間引き)
  if (m_shouldUpdateAI &&
      m_animationManager.GetCurrentAttachedAnimHandle(m_modelHandle) != -1) {
    m_animTime += (1.0f * m_aiUpdateInterval) * Game::GetTimeScale();

    float currentAnimTotalTime = m_animationManager.GetAnimationTotalTime(
        m_modelHandle,
        (m_currentAnimState == AnimState::Run
             ? kRunAnimName
             : (m_currentAnimState == AnimState::Attack ? kAttackAnimName
                                                        : kDeadAnimName)));

    if (m_currentAnimState == AnimState::Run) {
      if (m_animTime >= currentAnimTotalTime) {
        m_animTime = fmodf(m_animTime, currentAnimTotalTime);
      }
    } else if (m_currentAnimState == AnimState::Dead) {
      if (m_animTime >= currentAnimTotalTime) {
        m_animTime = currentAnimTotalTime;
      }
    }
    // Attackはループ制御を上でやってるので、ここでは単に更新

    m_animationManager.UpdateAnimationTime(m_modelHandle, m_animTime);
  }

  // コライダーの更新
  // 体のコライダーの位置を設定
  VECTOR bodyCapA = VAdd(m_pos, VGet(0, kBodyColliderRadius, 0));
  VECTOR bodyCapB =
      VAdd(m_pos, VGet(0, kBodyColliderHeight - kBodyColliderRadius, 0));
  m_pBodyCollider->SetSegment(bodyCapA, bodyCapB);
  m_pBodyCollider->SetRadius(kBodyColliderRadius);

  // 頭のコライダーの位置を取得
  int headIndex = MV1SearchFrame(m_modelHandle, "mixamorig:Head");
  VECTOR headModelPos = (headIndex != -1)
                            ? MV1GetFramePosition(m_modelHandle, headIndex)
                            : VGet(0, 0, 0);
  VECTOR headCenter = VAdd(headModelPos, m_headPosOffset);
  m_pHeadCollider->SetCenter(headCenter);
  m_pHeadCollider->SetRadius(kHeadRadius);

  // 攻撃範囲のコライダーの位置と半径を設定
  VECTOR attackRangeCenter = m_pos;
  attackRangeCenter.y += (kBodyColliderHeight * 0.5f);
  m_pAttackRangeCollider->SetCenter(attackRangeCenter);
  m_pAttackRangeCollider->SetRadius(kAttackRangeRadius);

  // 敵とプレイヤーの押し出し処理
  if (m_pBodyCollider->IsIntersects(playerBodyCollider.get())) {
    VECTOR enemyCenter = VScale(
        VAdd(m_pBodyCollider->GetSegmentA(), m_pBodyCollider->GetSegmentB()),
        0.5f);
    VECTOR playerCenter = VScale(VAdd(playerBodyCollider->GetSegmentA(),
                                      playerBodyCollider->GetSegmentB()),
                                 0.5f);
    VECTOR diff = VSub(enemyCenter, playerCenter);
    float distSq = VDot(diff, diff);
    float minDist = kBodyColliderRadius + playerBodyCollider->GetRadius();

    if (distSq < minDist * minDist && distSq > 0.0001f) {
      float dist = std::sqrt(distSq);
      float pushBack = minDist - dist;
      if (dist > 0) {
        VECTOR pushDir = VSub(enemyCenter, playerCenter);
        pushDir.y = 0.0f;
        if (VDot(pushDir, pushDir) > 0.0001f) {
          pushDir = VNorm(pushDir);
          m_pos = VAdd(m_pos, VScale(pushDir, pushBack * 0.5f));
        }
      }
    }
  }

  // 敵同士の押し出し処理
  std::vector<EnemyBase *> neighbors;
  if (context.collisionGrid) {
    context.collisionGrid->GetNeighbors(m_pos, neighbors);
  }
  const std::vector<EnemyBase *> &targets =
      (context.collisionGrid) ? neighbors : enemyList;

  for (EnemyBase *other : targets) {
    if (!other || other == this)
      continue;

    VECTOR otherPos = other->GetPos();
    VECTOR diff = VSub(m_pos, otherPos);
    diff.y = 0.0f; // Y軸は無視
    float distSq = VDot(diff, diff);
    float minDist = kBodyColliderRadius * 2.0f;

    if (distSq < minDist * minDist && distSq > 0.0001f) {
      float dist = std::sqrt(distSq);
      float pushBack = (minDist - dist) * 0.5f; // 押し出す量を半分にする
      VECTOR pushDir = VNorm(diff);

      m_pos = VAdd(m_pos, VScale(pushDir, pushBack));
    }
  }

  // 攻撃判定 (間引き)
  if (m_currentAnimState == AnimState::Attack) {
    if (m_shouldUpdateAI) {
      float currentAnimTotalTime = m_animationManager.GetAnimationTotalTime(
          m_modelHandle, kAttackAnimName);
      // Runnerの攻撃発生タイミング調整
      float attackStart = currentAnimTotalTime * 0.3f;
      float attackEnd = currentAnimTotalTime * 0.6f;

      if (!m_isAttackHit && m_animTime >= attackStart &&
          m_animTime <= attackEnd) {
        if (CanAttackPlayer(player)) {
          const_cast<Player &>(player).TakeDamage(m_attackPower, m_pos);
          m_isAttackHit = true;
        }
      }
    }
  }

  CheckHitAndDamage(const_cast<std::vector<Bullet> &>(bullets), pEffect);

  // タックル判定
  if (tackleInfo.isTackling && m_hp > 0.0f &&
      tackleInfo.tackleId != m_lastTackleId) {
    CapsuleCollider playerTackleCollider(tackleInfo.capA, tackleInfo.capB,
                                         tackleInfo.radius);
    if (m_pBodyCollider->IsIntersects(&playerTackleCollider)) {
      TakeTackleDamage(tackleInfo.damage);
      m_lastTackleId = tackleInfo.tackleId;
    }
  } else if (!tackleInfo.isTackling) {
    m_lastTackleId = -1;
  }

  if (m_hitDisplayTimer > 0) {
    --m_hitDisplayTimer;
    if (m_hitDisplayTimer == 0)
      m_lastHitPart = HitPart::None;
  }
}

void EnemyRunner::Draw() {
  if (m_hp <= 0.0f &&
      m_animationManager.GetCurrentAttachedAnimHandle(m_modelHandle) == -1)
    return;

  // 視錐台カリング (描画最適化)
  // CheckCameraViewClip系の関数が環境によって不安定なため、
  // 手動で「カメラ前方への内積チェック(簡易コーン判定)」を行う
  VECTOR camPos = GetCameraPosition();
  VECTOR camTarget = GetCameraTarget();
  VECTOR camDir = VNorm(VSub(camTarget, camPos));
  VECTOR toEnemy = VSub(m_pos, camPos);
  float distSq = VSquareSize(toEnemy);

  // 1. 距離チェック (Farクリップ + マージン)
  if (distSq > 16000.0f * 16000.0f)
    return;

  // 2. 画角チェック (内積)
  // 近距離(300.0f以内)なら無条件で描画
  if (distSq > 300.0f * 300.0f) {
    VECTOR dirToEnemy = VNorm(toEnemy);
    float dot = VDot(camDir, dirToEnemy);
    // 視野角90度に対し余裕を持って判定
    if (dot < 0.4f)
      return;
  }

  EnemyBase::IncrementDrawCount();
  MV1DrawModel(m_modelHandle);

#ifdef _DEBUG
  DrawCollisionDebug();

  const char *hitMsg = "";

  switch (m_lastHitPart) {
  case HitPart::Head:
    hitMsg = "HeadShot!";
    break;
  case HitPart::Body:
    hitMsg = "BodyHit!";
    break;
  default:
    break;
  }

  if (*hitMsg) {
    DrawFormatString(20, 100, 0xff0000, "%s", hitMsg);
  }

  DebugUtil::DrawFormat(20, 80, 0x000000, "Runner HP: %.1f", m_hp);
#endif
}

void EnemyRunner::DrawCollisionDebug() const {
  // 体のコライダーデバッグ描画
  DebugUtil::DrawCapsule(m_pBodyCollider->GetSegmentA(),
                         m_pBodyCollider->GetSegmentB(),
                         m_pBodyCollider->GetRadius(), 16, 0xff0000);

  // 頭のコライダーデバッグ描画
  DebugUtil::DrawSphere(m_pHeadCollider->GetCenter(),
                        m_pHeadCollider->GetRadius(), 16, 0x00ff00);

  // 攻撃範囲のデバッグ描画
  DebugUtil::DrawSphere(m_pAttackRangeCollider->GetCenter(),
                        m_pAttackRangeCollider->GetRadius(), 24, 0xff8000);

  // ここをLeftHandとRightHandに変更
  int handRIndex = MV1SearchFrame(m_modelHandle, "mixamorig:RightHand");
  int handLIndex = MV1SearchFrame(m_modelHandle, "mixamorig:LeftHand");

  if (handRIndex != -1 && handLIndex != -1) {
    VECTOR handRPos = MV1GetFramePosition(m_modelHandle, handRIndex);
    VECTOR handLPos = MV1GetFramePosition(m_modelHandle, handLIndex);

    // 攻撃ヒット用コライダーのデバッグ描画
    DebugUtil::DrawCapsule(handRPos, handLPos, kAttackHitRadius, 16, 0x0000ff);
  }
}

// どこに当たったのか判定する
EnemyBase::HitPart EnemyRunner::CheckHitPart(const VECTOR &rayStart,
                                             const VECTOR &rayEnd,
                                             VECTOR &outHtPos,
                                             float &outHtDistSq) const {
  VECTOR hitPosHead, hitPosBody;
  float hitDistSqHead = FLT_MAX;
  float hitDistSqBody = FLT_MAX;

  bool headHit = m_pHeadCollider->IsIsIntersectsRay(rayStart, rayEnd,
                                                    hitPosHead, hitDistSqHead);
  bool bodyHit = m_pBodyCollider->IsIsIntersectsRay(rayStart, rayEnd,
                                                    hitPosBody, hitDistSqBody);

  if (headHit && bodyHit) {
    if (hitDistSqHead <= hitDistSqBody) {
      outHtPos = hitPosHead;
      outHtDistSq = hitDistSqHead;
      return HitPart::Head;
    } else {
      outHtPos = hitPosBody;
      outHtDistSq = hitDistSqBody;
      return HitPart::Body;
    }
  } else if (headHit) {
    outHtPos = hitPosHead;
    outHtDistSq = hitDistSqHead;
    return HitPart::Head;
  } else if (bodyHit) {
    outHtPos = hitPosBody;
    outHtDistSq = hitDistSqBody;
    return HitPart::Body;
  }

  outHtPos = VGet(0, 0, 0);
  outHtDistSq = FLT_MAX;
  return HitPart::None;
}

// 部位ごとのダメージ計算
float EnemyRunner::CalcDamage(float bulletDamage, HitPart part) const {
  if (part == HitPart::Head) {
    return bulletDamage * 2.0f;
  } else if (part == HitPart::Body) {
    return bulletDamage;
  }
  return 0.0f;
}

// アイテムドロップ時のコールバック関数
void EnemyRunner::SetOnDropItemCallback(
    std::function<void(const VECTOR &)> cb) {
  m_onDropItem = cb;
}

// ダメージ処理
void EnemyRunner::TakeDamage(float damage, AttackType type) {
  EnemyBase::TakeDamage(damage, type);
  // HP減算・死亡判定は基底クラスで行う
  if (m_hp <= 0.0f) // 死亡時一度だけ
  {
    if (m_lastHitPart == HitPart::None)
      m_lastHitPart = HitPart::Body;
    bool isHeadShot = (m_lastHitPart == HitPart::Head);
    int addScore = ScoreManager::Instance().AddScore(isHeadShot);
    if (SceneMain::Instance()) {
      SceneMain::Instance()->AddScorePopup(addScore, isHeadShot,
                                           ScoreManager::Instance().GetCombo());
    }
  }
}

// タックル攻撃のダメージ処理
void EnemyRunner::TakeTackleDamage(float damage) {
  EnemyBase::TakeTackleDamage(damage);
}

std::shared_ptr<CapsuleCollider> EnemyRunner::GetBodyCollider() const {
  return m_pBodyCollider;
}