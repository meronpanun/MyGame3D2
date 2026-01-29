#include "EnemyBoss.h"
#include "Bullet.h"
#include "CapsuleCollider.h"
#include "Collision.h"
#include "DebugUtil.h"
#include "DxLib.h"
#include "Effect.h"
#include "EffekseerForDXLib.h"
#include "SceneMain.h"
#include "ScoreManager.h"
#include "SphereCollider.h"
#include "TaskTutorialManager.h"
#include "TransformDataLoader.h"
#include <algorithm>
#include <cassert>
#include <cmath>

namespace {
// アニメーション名
// constexpr char kIdleAnimName[]        = "IDLE";
constexpr char kWalkAnimName[] = "Armature|Run";
constexpr char kCloseAttackAnimName[] =
    "Armature|CloseRangeAttack"; // 近接範囲攻撃
constexpr char kLongRangeAttackAnimName[] =
    "Armature|LongRangeAttack"; // 遠距離攻撃
constexpr char kDeadAnimName[] = "Armature|Death";

constexpr float kLongRangeAttackMinDist = 400.0f; // 遠距離攻撃を行う最小距離
constexpr float kLongRangeAttackMaxDist =
    1000.0f; // 遠距離攻撃を行う最大距離（これより遠いと攻撃せず接近する）
constexpr int kLongRangeAttackCooldownMax = 120;
constexpr float kHomingBulletSpeed = 6.0f;
constexpr float kHomingTurnRate = 0.02f;        // 旋回性能
constexpr float kHomingBulletMaxDist = 1800.0f; // 弾の最大飛距離
constexpr float kHomingBulletDamage = 20.0f;
constexpr float kHomingBulletRadius = 15.0f;

// コライダーサイズ
constexpr float kBodyColliderRadius = 40.0f;
constexpr float kBodyColliderHeight = 200.0f;
constexpr float kHeadRadius = 20.0f;
constexpr float kAttackRangeRadius = 150.0f; // 指定された近接範囲
constexpr float kAttackHitRadius = 60.0f;    // 攻撃自体の当たり判定

constexpr int kAttackCooldownMax = 60;
constexpr int kAttackEndDelay = 30; // 攻撃後の硬直
} // namespace

int EnemyBoss::s_modelHandle = -1;

void EnemyBoss::LoadModel() {
  s_modelHandle = MV1LoadModel("data/model/Boss.mv1");
  assert(s_modelHandle != -1);
}

void EnemyBoss::DeleteModel() {
  MV1DeleteModel(s_modelHandle);
  s_modelHandle = -1;
}

EnemyBoss::EnemyBoss()
    : m_currentAnimState(AnimState::Idle), m_isDeadAnimPlaying(false),
      m_animTime(0.0f), m_chaseSpeed(0.0f), m_attackEndDelayTimer(0),
      m_isAttackHit(false), m_headNodeIndex(-1), m_headTopEndNodeIndex(-1),
      m_handRNodeIndex(-1), m_handLNodeIndex(-1) {
  m_modelHandle = MV1DuplicateModel(s_modelHandle);

  // コライダー初期化
  m_pBodyCollider = std::make_shared<CapsuleCollider>();
  m_pHeadCollider = std::make_shared<SphereCollider>();
  m_pAttackRangeCollider = std::make_shared<SphereCollider>();
  m_pAttackHitCollider = std::make_shared<CapsuleCollider>();
}

EnemyBoss::~EnemyBoss() { MV1DeleteModel(m_modelHandle); }

void EnemyBoss::Init() {
  m_isAlive = true;
  m_isDeadAnimPlaying = false;
  m_animTime = 0.0f;

  // 位置はWaveManagerでセットされるが、初期値として
  m_pos = VGet(0.0f, 0.0f, 1000.0f);
  MV1SetPosition(m_modelHandle, m_pos);

  // CSVからデータをロード
  auto dataList =
      TransformDataLoader::LoadDataCSV("data/CSV/CharacterTransfromData.csv");
  for (const auto &data : dataList) {
    if (data.name == "Boss") {
      MV1SetRotationXYZ(m_modelHandle, data.rot);
      MV1SetScale(m_modelHandle, data.scale);
      m_attackPower = data.attack;
      m_hp = data.hp;
      m_maxHp = data.hp;
      m_chaseSpeed = data.chaseSpeed;
      break;
    }
  }

  // フレームインデックスのキャッシュ
  m_headNodeIndex = MV1SearchFrame(m_modelHandle, "Head");
  m_headTopEndNodeIndex =
      MV1SearchFrame(m_modelHandle, "mixamorig:HeadTop_End");
  // 手は未使用なら検索しなくてもよいが、念のため
  m_handRNodeIndex = MV1SearchFrame(m_modelHandle, "Hand_R");
  m_handLNodeIndex = MV1SearchFrame(m_modelHandle, "Hand_L");

  // 攻撃範囲コライダー設定
  m_pAttackRangeCollider->SetRadius(kAttackRangeRadius);

  m_longRangeAttackCooldown = 0;
  m_homingBullets.clear();
  m_hasShotLongRange = false;
  m_isNextAttackNormal = false; // 最初はパリィ弾から

  ChangeAnimation(AnimState::Walk, true); // 最初は歩いて近づく
}

#include "Game.h"

void EnemyBoss::ChangeAnimation(AnimState newAnimState, bool loop) {
  // 遠距離攻撃など、同じ状態でも再度再生したいケースがある場合は調整
  if (m_currentAnimState == newAnimState && newAnimState != AnimState::Attack &&
      newAnimState != AnimState::LongRangeAttack) {
    return;
  }

  const char *animName = nullptr;
  switch (newAnimState) {
  case AnimState::Idle:
    animName = kWalkAnimName;
    break;
  case AnimState::Walk:
    animName = kWalkAnimName;
    break;
  case AnimState::Attack:
    animName = kCloseAttackAnimName;
    break;
  case AnimState::LongRangeAttack:
    animName = kLongRangeAttackAnimName;
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

void EnemyBoss::Update(const EnemyUpdateContext &context) {
  // コンテキストから展開
  std::vector<Bullet> &bullets = context.bullets;
  const Player::TackleInfo &tackleInfo = context.tackleInfo;
  const Player &player = context.player;
  // const std::vector<EnemyBase*>& enemyList = context.enemyList; //
  // EnemyBossでは使っていない
  const std::vector<Stage::StageCollisionData> &collisionData =
      context.collisionData;
  Effect *pEffect = context.pEffect;

  UpdateStageCollision(collisionData);

#ifdef _DEBUG
  m_shouldDrawParryCollider = false;
#endif

  // ホーミング弾の更新
  for (auto &bullet : m_homingBullets) {
    if (!bullet.active)
      continue;

    // プレイヤーへの方向
    VECTOR toPlayer = VSub(player.GetPos(), bullet.pos);
    float distToPlayer = VSize(toPlayer);
    VECTOR targetDir = VNorm(toPlayer);

    // パラボリックかホーミングかで分岐
    float scale = Game::GetTimeScale();
    if (bullet.isParabolic) {
      // 放物線運動
      bullet.velocity.y -= bullet.gravity * scale;
      bullet.pos = VAdd(bullet.pos, VScale(bullet.velocity, scale));
      // 進行方向を速度ベクトルに合わせる（見た目のため）
      if (VSquareSize(bullet.velocity) > 0.0001f) {
        bullet.dir = VNorm(bullet.velocity);
      }
    } else {
      // ホーミング処理 (現在の向きからターゲット向きへ徐々に補間)
      // 戻ってくる動きを応用 -> プレイヤーが動いても追従
      // シンプルにターンレートで補間
      bullet.dir = VAdd(bullet.dir, VScale(targetDir, kHomingTurnRate * scale));
      bullet.dir = VNorm(bullet.dir);

      // 移動
      VECTOR moveVec = VScale(bullet.dir, bullet.speed * scale);
      bullet.pos = VAdd(bullet.pos, moveVec);
      bullet.distTraveled += bullet.speed * scale;
    }

    // エフェクト更新(あれば)
    if (bullet.effectHandle != -1) {
      SetPosPlayingEffekseer3DEffect(bullet.effectHandle, bullet.pos.x,
                                     bullet.pos.y, bullet.pos.z);
    }

    // まだ反射されていない弾の処理
    if (!bullet.isReflected) {
      SphereCollider bulletCol(bullet.pos, kHomingBulletRadius);
      bool hitDetected = false;

      // パリィ判定
      if (bullet.isParryable && player.IsJustGuarded()) {
        VECTOR playerCapA, playerCapB;
        float playerRadius;
        player.GetCapsuleInfo(playerCapA, playerCapB, playerRadius);

        // パリィしやすくするために判定を広くする
        float parryRadius = playerRadius * 1.5f;
        CapsuleCollider parryCollider(playerCapA, playerCapB, parryRadius);

#ifdef _DEBUG
        m_shouldDrawParryCollider = true;
        m_debugParryCapA = playerCapA;
        m_debugParryCapB = playerCapB;
        m_debugParryRadius = parryRadius;
#endif

        if (bulletCol.IsIntersects(&parryCollider)) {
          // パリィ成功
          hitDetected = true;
          bullet.isReflected = true;

          // チュートリアルマネージャーに通知
          TaskTutorialManager::GetInstance()->NotifyParrySuccess();

          // 反射方向計算 (プレイヤーカメラの前方へ、あるいはボスへ)
          // ここではボス（発射主）へ跳ね返す
          if (bullet.owner) {
            VECTOR targetPos = bullet.owner->GetPos();
            targetPos.y += kBodyColliderHeight * 0.5f; // 中心付近へ
            bullet.dir = VNorm(VSub(targetPos, bullet.pos));
          } else {
            bullet.dir = VScale(bullet.dir, -1.0f); // 単純反転
          }

          bullet.speed *= 1.5f;
          bullet.turnRate = 0.0f; // 反射後はホーミング切る
          Game::SetTimeScale(0.1f, 1.0f);
        }
      }

      if (!hitDetected) {
        std::shared_ptr<CapsuleCollider> pCol = player.GetBodyCollider();
        if (bulletCol.IsIntersects(pCol.get())) {
          hitDetected = true;
          // ダメージ処理
          const_cast<Player &>(player).TakeDamage(bullet.damage, m_pos,
                                                  bullet.isParryable);
          bullet.active = false;
          if (bullet.effectHandle != -1) {
            StopEffekseer3DEffect(bullet.effectHandle);
            bullet.effectHandle = -1;
          }
        }
      }
    }
    // 反射された弾の処理
    else {
      // 弾の所有者(Boss自身)と当たり判定
      if (bullet.owner) {
        SphereCollider reflectedCol(bullet.pos, kHomingBulletRadius);
        // ボスの当たり判定を使用
        if (reflectedCol.IsIntersects(this->GetBodyCollider().get())) {
          this->TakeDamage(bullet.damage, AttackType::Shoot);
          this->OnParried(); // 怯み処理
          bullet.active = false;
          if (bullet.effectHandle != -1) {
            StopEffekseer3DEffect(bullet.effectHandle);
            bullet.effectHandle = -1;
          }
        }
      }
    }

    // 最大飛距離チェック
    if (bullet.distTraveled > kHomingBulletMaxDist) {
      bullet.active = false;
      if (bullet.effectHandle != -1) {
        StopEffekseer3DEffect(bullet.effectHandle);
        bullet.effectHandle = -1;
      }
    }

    // 地面接触で消滅
    if (bullet.pos.y < 0) {
      bullet.active = false;
      if (bullet.effectHandle != -1) {
        StopEffekseer3DEffect(bullet.effectHandle);
        bullet.effectHandle = -1;
      }
    }
  }

  // 不要な弾を削除
  m_homingBullets.erase(
      std::remove_if(m_homingBullets.begin(), m_homingBullets.end(),
                     [](const HomingBullet &b) { return !b.active; }),
      m_homingBullets.end());

  // 怯み状態の処理
  if (m_isStunned) {
    m_stunTimer--;
    if (m_stunTimer <= 0) {
      m_isStunned = false;
      ChangeAnimation(AnimState::Walk, true);
    } else {
      // アニメーション更新（Deadモーションなどを途中まで再生するなど）
      // AcidではAnimationManagerの更新を手動で制御していたが、
      // ここでは単純にタイマー待機とする
      // 一旦アニメーションを進める
      if (m_animationManager.GetCurrentAttachedAnimHandle(m_modelHandle) !=
          -1) {
        // ある程度進んだら止める等の制御が必要ならここで行う
        m_animTime += 1.0f * Game::GetTimeScale();
        m_animationManager.UpdateAnimationTime(m_modelHandle, m_animTime);
      }
    }
    MV1SetPosition(m_modelHandle, m_pos);
    return; // 他のAIロジックをスキップ
  }

  if (m_hp <= 0.0f) {
    if (!m_isDeadAnimPlaying) {
      ChangeAnimation(AnimState::Dead, false);
      m_isDeadAnimPlaying = true;
      m_animTime = 0.0f;
      m_isAlive = true;
    }

    if (m_animationManager.GetCurrentAttachedAnimHandle(m_modelHandle) != -1) {
      m_animTime += 1.0f * Game::GetTimeScale();
      m_animationManager.UpdateAnimationTime(m_modelHandle, m_animTime);
    }

    float currentAnimTotalTime =
        m_animationManager.GetAnimationTotalTime(m_modelHandle, kDeadAnimName);
    if (m_animTime >= currentAnimTotalTime) {
      if (m_animationManager.GetCurrentAttachedAnimHandle(m_modelHandle) !=
          -1) {
        MV1DetachAnim(m_modelHandle, 0);
        m_animationManager.ResetAttachedAnimHandle(m_modelHandle);
      }
      if (m_onDeathCallback) {
        m_onDeathCallback(m_pos);
        m_onDeathCallback = nullptr;
      }
      m_isAlive = false;
      SetActive(false);
    }
    return;
  }

  MV1SetPosition(m_modelHandle, m_pos);

  // プレイヤーの位置・コライダー
  VECTOR playerPos = player.GetPos();
  std::shared_ptr<CapsuleCollider> playerBodyCollider =
      player.GetBodyCollider();

  // 攻撃範囲コライダー更新
  VECTOR attackRangeCenter = m_pos;
  attackRangeCenter.y += (kBodyColliderHeight * 0.5f);
  m_pAttackRangeCollider->SetCenter(attackRangeCenter);

  // 状態遷移ロジック
  if (m_currentAnimState == AnimState::Attack) {
    // 攻撃中
    float currentAnimTotalTime = m_animationManager.GetAnimationTotalTime(
        m_modelHandle, kCloseAttackAnimName);

    // 攻撃アニメーション終了判定
    if (m_animTime > currentAnimTotalTime) {
      if (m_attackEndDelayTimer <= 0) {
        m_attackEndDelayTimer = kAttackEndDelay;
      }
    }

    if (m_attackEndDelayTimer > 0) {
      --m_attackEndDelayTimer;
      if (m_attackEndDelayTimer == 0) {
        m_isAttackHit = false;
        // 攻撃終了後、範囲内にいれば再度攻撃、いなければ移動へ
        if (CanAttackPlayer(player)) {
          ChangeAnimation(AnimState::Attack, false);
        } else {
          ChangeAnimation(AnimState::Walk, true);
        }
      }
    }
  } else if (m_currentAnimState == AnimState::LongRangeAttack) {
    float totalTime = m_animationManager.GetAnimationTotalTime(
        m_modelHandle, kLongRangeAttackAnimName);

    // 弾生成タイミング
    if (!m_hasShotLongRange && m_animTime > totalTime * 0.3f) {
      // 弾生成
      VECTOR spawnPos = m_pos;
      if (m_headTopEndNodeIndex != -1) {
        spawnPos = MV1GetFramePosition(m_modelHandle, m_headTopEndNodeIndex);
      } else {
        // 見つからない場合は頭付近オフセット
        spawnPos = VAdd(m_pos, VGet(0, kBodyColliderHeight, 0));
      }

      HomingBullet bullet;
      bullet.pos = spawnPos;
      bullet.active = true;
      bullet.damage = kHomingBulletDamage; // ダメージ20
      bullet.distTraveled = 0.0f;
      bullet.distTraveled = 0.0f;
      // プレイヤー方向へ発射
      VECTOR toTarget = VSub(playerPos, spawnPos);
      bullet.dir = VNorm(toTarget);
      bullet.speed = kHomingBulletSpeed;
      bullet.owner = this;
      bullet.isReflected = false;

      // 放物線攻撃判定
      std::string groundedObj = player.GetGroundedObjectName();
      if ((groundedObj == "rock_3_br" || groundedObj == "rock_6_br") &&
          !EnemyBase::IsTargetVisible(spawnPos, player.GetPos(),
                                      collisionData)) {
        bullet.isParabolic = true;
        bullet.gravity = 0.3f; // 重力設定
        bullet.velocity = EnemyBase::CalculateParabolicVelocity(
            bullet.pos, playerPos, bullet.gravity, kHomingBulletSpeed);
      } else {
        bullet.isParabolic = false;
      }

      // エフェクトがあればここで再生しハンドル保持
      if (pEffect) {
        // マズルフラッシュ（射撃時の一瞬のエフェクト）
        pEffect->PlayMuzzleFlash(spawnPos.x, spawnPos.y, spawnPos.z, 0, 0, 0);

        if (m_isNextAttackNormal) {
          // 通常弾 (パリィ不可)
          bullet.isParryable = false;
          bullet.effectHandle = pEffect->PlayNormalBulletEffect(
              spawnPos.x, spawnPos.y, spawnPos.z);
        } else {
          // パリィ弾 (パリィ可能)
          bullet.isParryable = true;
          // EnemyAcidと同様のエフェクトを使用（または専用エフェクト）
          bullet.effectHandle =
              pEffect->PlayAcidEffect(spawnPos.x, spawnPos.y, spawnPos.z);
        }
      }

      // 次回のためにフラグ反転
      m_isNextAttackNormal = !m_isNextAttackNormal;

      m_homingBullets.push_back(bullet);
      m_hasShotLongRange = true;
    }

    if (m_animTime >= totalTime) {
      // アニメーション終了
      if (m_attackEndDelayTimer <= 0)
        m_attackEndDelayTimer = 30; // 硬直
    }

    if (m_attackEndDelayTimer > 0) {
      m_attackEndDelayTimer--;
      if (m_attackEndDelayTimer == 0) {
        ChangeAnimation(AnimState::Walk, true);
        m_longRangeAttackCooldown = kLongRangeAttackCooldownMax;
      }
    }
  } else if (m_currentAnimState == AnimState::Idle) {
    // 待機状態 (Idle)
    if (m_longRangeAttackCooldown > 0)
      m_longRangeAttackCooldown--;

    VECTOR toPlayer = VSub(playerPos, m_pos);
    toPlayer.y = 0.0f;
    float disToPlayer = VSize(toPlayer);

    // 向き変更
    if (disToPlayer > 0.1f) {
      float yaw = atan2f(toPlayer.x, toPlayer.z);
      yaw += DX_PI_F;
      MV1SetRotationXYZ(m_modelHandle, VGet(0.0f, yaw, 0.0f));
    }

    // 攻撃判定
    if (CanAttackPlayer(player)) {
      m_isAttackHit = false;
      ChangeAnimation(AnimState::Attack, false);
    } else if (disToPlayer > kLongRangeAttackMinDist &&
               disToPlayer < kLongRangeAttackMaxDist &&
               m_longRangeAttackCooldown <= 0) {
      m_hasShotLongRange = false;
      ChangeAnimation(AnimState::LongRangeAttack, false);
    } else {
      // 移動が必要かチェック
      if (disToPlayer > kLongRangeAttackMaxDist ||
          disToPlayer < kLongRangeAttackMinDist) {
        ChangeAnimation(AnimState::Walk, true);
      }
    }
  } else // Walk
  {
    // 移動処理 (Walk)
    if (m_currentAnimState == AnimState::Walk) {
      // クールダウン減少
      if (m_longRangeAttackCooldown > 0)
        m_longRangeAttackCooldown--;

      VECTOR toPlayer = VSub(playerPos, m_pos);
      toPlayer.y = 0.0f;
      float disToPlayer = VSize(toPlayer);

      // 向き変更
      if (disToPlayer > 0.1f) {
        float yaw = atan2f(toPlayer.x, toPlayer.z);
        yaw += DX_PI_F; // モデルの向き補正が必要なら調整
        MV1SetRotationXYZ(m_modelHandle, VGet(0.0f, yaw, 0.0f));
      }

      // 攻撃判定
      if (CanAttackPlayer(player)) {
        m_isAttackHit = false;
        ChangeAnimation(AnimState::Attack, false);
      } else if (disToPlayer > kLongRangeAttackMinDist &&
                 disToPlayer < kLongRangeAttackMaxDist &&
                 m_longRangeAttackCooldown <= 0) {
        // 遠距離攻撃へ遷移（プレイヤーが遠距離攻撃の有効範囲内にいる場合のみ）
        m_hasShotLongRange = false;
        ChangeAnimation(AnimState::LongRangeAttack, false);
      } else {
        // 範囲外なら近づく
        if (disToPlayer > kLongRangeAttackMaxDist) {
          VECTOR dir = VNorm(toPlayer);
          m_pos = VAdd(m_pos, VScale(dir, m_chaseSpeed * Game::GetTimeScale()));
        } else if (disToPlayer < kLongRangeAttackMinDist) {
          VECTOR dir = VNorm(toPlayer);
          m_pos = VAdd(m_pos, VScale(dir, m_chaseSpeed * Game::GetTimeScale()));
        } else {
          // 範囲内で移動不要ならIdleへ
          ChangeAnimation(AnimState::Idle, true);
        }
      }
    }
  }

  // アニメーション更新
  if (m_animationManager.GetCurrentAttachedAnimHandle(m_modelHandle) != -1) {
    const char *animName = nullptr;
    float animSpeed = 1.0f;

    if (m_currentAnimState == AnimState::Walk) {
      animName = kWalkAnimName;
      animSpeed = 0.6f; // 走りが速すぎるので調整
    } else if (m_currentAnimState == AnimState::Idle) {
      animName = kWalkAnimName;
      animSpeed = 0.0f; // 待機中はアニメーション停止
    } else if (m_currentAnimState == AnimState::Attack)
      animName = kCloseAttackAnimName;
    else if (m_currentAnimState == AnimState::Dead)
      animName = kDeadAnimName;
    else if (m_currentAnimState == AnimState::LongRangeAttack)
      animName = kLongRangeAttackAnimName;

    animSpeed *= Game::GetTimeScale();

    if (animName) {
      m_animTime += animSpeed;

      float totalTime =
          m_animationManager.GetAnimationTotalTime(m_modelHandle, animName);
      // ループ処理
      if (m_currentAnimState ==
          AnimState::Walk /*|| m_currentAnimState == AnimState::Idle*/) {
        m_animTime = fmodf(m_animTime, totalTime);
      }
      m_animationManager.UpdateAnimationTime(m_modelHandle, m_animTime);
    }
  }

  // コライダー更新(Body)
  VECTOR bodyCapA = VAdd(m_pos, VGet(0, kBodyColliderRadius, 0));
  VECTOR bodyCapB =
      VAdd(m_pos, VGet(0, kBodyColliderHeight - kBodyColliderRadius, 0));
  m_pBodyCollider->SetSegment(bodyCapA, bodyCapB);
  m_pBodyCollider->SetRadius(kBodyColliderRadius);

  // コライダー更新(Head)
  if (m_headTopEndNodeIndex != -1) {
    VECTOR headPos = MV1GetFramePosition(m_modelHandle, m_headTopEndNodeIndex);
    m_pHeadCollider->SetCenter(headPos);
    m_pHeadCollider->SetRadius(kHeadRadius);
  } else if (m_headNodeIndex != -1) {
    VECTOR headPos = MV1GetFramePosition(m_modelHandle, m_headNodeIndex);
    m_pHeadCollider->SetCenter(headPos);
    m_pHeadCollider->SetRadius(kHeadRadius);
  } else {
    // 頭が見つからない場合のフォールバック（体の上の方）
    m_pHeadCollider->SetCenter(VAdd(m_pos, VGet(0, kBodyColliderHeight, 0)));
    m_pHeadCollider->SetRadius(kHeadRadius);
  }

  // プレイヤーとの押し出しなど
  if (m_pBodyCollider->IsIntersects(playerBodyCollider.get())) {
    // 簡易押し出し
    VECTOR diff = VSub(m_pos, player.GetPos());
    diff.y = 0.0f;
    if (VSize(diff) > 0.001f) {
      VECTOR pushDir = VNorm(diff);
      float pushDist = 2.0f; // 適当な押し出し係数
      m_pos = VAdd(m_pos, VScale(pushDir, pushDist));
    }
  }

  // 攻撃判定発生
  if (m_currentAnimState == AnimState::Attack && !m_isAttackHit) {
    // アニメーションの特定タイミングでのみヒット判定を出す
    float totalTime = m_animationManager.GetAnimationTotalTime(
        m_modelHandle, kCloseAttackAnimName);
    // 判定期間を少し広げる or 調整
    // 攻撃の出始めから終わり際まで判定があっても良いならそうするが、
    // 以前のロジック(20%~70%)を維持する
    if (m_animTime > totalTime * 0.4f && m_animTime < totalTime * 0.6f) {
      // ユーザー要望：デバッグで表示している範囲内（m_pAttackRangeCollider）はすべてダメージが入るように
      // 範囲攻撃なので、攻撃範囲コライダーとプレイヤーが接触していればヒットとする
      if (m_pAttackRangeCollider->IsIntersects(playerBodyCollider.get())) {
        // ダメージを与える (CSVから読み込んだ攻撃力を使用)
        const_cast<Player &>(player).TakeDamage(
            static_cast<float>(m_attackPower), m_pos);
        m_isAttackHit = true;

        // デバッグ用ヒット表示も更新しておく（青色表示用）
        // カプセルとして設定（球として扱う）
        m_pAttackHitCollider->SetSegment(m_pAttackRangeCollider->GetCenter(),
                                         m_pAttackRangeCollider->GetCenter());
        m_pAttackHitCollider->SetRadius(m_pAttackRangeCollider->GetRadius());
      }
    }
  }

  // カメラ位置を取得
  VECTOR cameraPos = VGet(0,0,0);
  if(player.GetCamera()) {
      cameraPos = player.GetCamera()->GetPos();
  }
  CheckHitAndDamage(bullets, collisionData, pEffect, cameraPos);

  // タックル判定
  if (tackleInfo.isTackling && m_hp > 0.0f &&
      tackleInfo.tackleId != m_lastTackleId) {
    CapsuleCollider tackleCol(tackleInfo.capA, tackleInfo.capB,
                              tackleInfo.radius);
    if (m_pBodyCollider->IsIntersects(&tackleCol)) {
      TakeTackleDamage(tackleInfo.damage);
      m_lastTackleId = tackleInfo.tackleId;
    }
  } else if (!tackleInfo.isTackling) {
    m_lastTackleId = -1;
  }
}

void EnemyBoss::Draw() {
  if (!m_isAlive &&
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
  // ボスは巨大なので、近距離判定を広めにとる
  if (distSq > 600.0f * 600.0f) {
    VECTOR dirToEnemy = VNorm(toEnemy);
    float dot = VDot(camDir, dirToEnemy);
    // ボスは横幅もあるので、かなり広めに判定 (視野前面180度近くを許可)
    // 0.0f = 90度(左右) -> 180度視野
    if (dot < 0.0f)
      return;
  }

  EnemyBase::IncrementDrawCount();
  MV1DrawModel(m_modelHandle);

#ifdef _DEBUG
  DrawCollisionDebug();
#endif

  // ホーミング弾の描画
  // エフェクトのみ表示するため、デバッグ用の球体描画は削除

  // for (const auto& bullet : m_homingBullets)
  //{
  //     if (bullet.active)
  //     {
  //          // DebugUtilではなくDxLibの標準関数を使用
  //          DrawSphere3D(bullet.pos, kHomingBulletRadius, 8, 0xffff00,
  //          0xffff00, TRUE);
  //     }
  // }
}

void EnemyBoss::TakeDamage(float damage, AttackType type) {
  EnemyBase::TakeDamage(damage, type);

  // UIへのスコア加算などの演出はEnemyNormal同様にあれば追加
  if (m_hp <= 0.0f) {
    bool isHeadShot = (m_lastHitPart == HitPart::Head);
    int addScore = ScoreManager::Instance().AddScore(isHeadShot) *
                   10; // ボスなのでスコア高め
    if (SceneMain::Instance()) {
      SceneMain::Instance()->AddScorePopup(addScore, isHeadShot,
                                           ScoreManager::Instance().GetCombo());
    }
  }
}

void EnemyBoss::TakeTackleDamage(float damage) {
  EnemyBase::TakeTackleDamage(damage);
}

std::shared_ptr<CapsuleCollider> EnemyBoss::GetBodyCollider() const {
  return m_pBodyCollider;
}

float EnemyBoss::CalcDamage(float bulletDamage, HitPart part) const {
  // ボスは硬い、あるいは弱点だけ効くなどの調整が可能
  if (part == HitPart::Head) {
    return bulletDamage * 1.5f;
  }
  return bulletDamage * 0.8f; // ボディは少し硬い
}

void EnemyBoss::DrawCollisionDebug() const {
  // 体
  if (m_pBodyCollider)
    DebugUtil::DrawCapsule(m_pBodyCollider->GetSegmentA(),
                           m_pBodyCollider->GetSegmentB(),
                           m_pBodyCollider->GetRadius(), 16, 0xff0000); // 赤

  // 頭
  if (m_pHeadCollider)
    DebugUtil::DrawSphere(m_pHeadCollider->GetCenter(),
                          m_pHeadCollider->GetRadius(), 16, 0x00ff00); // 緑

  // 攻撃範囲（指定されたデバッグ表示）
  // 黄色やオレンジでわかりやすく
  if (m_pAttackRangeCollider)
    DebugUtil::DrawSphere(m_pAttackRangeCollider->GetCenter(),
                          m_pAttackRangeCollider->GetRadius(), 32, 0xffaa00);

  // 攻撃判定（攻撃中のみ青などで）
  if (m_currentAnimState == AnimState::Attack && m_pAttackHitCollider) {
    DebugUtil::DrawCapsule(m_pAttackHitCollider->GetSegmentA(),
                           m_pAttackHitCollider->GetSegmentB(),
                           m_pAttackHitCollider->GetRadius(), 16, 0x0000ff);
  }

  // 遠距離攻撃有効範囲（紫色）
  // プレイヤーとの距離がこの範囲外なら遠距離攻撃を行う
  DebugUtil::DrawSphere(m_pos, kLongRangeAttackMinDist, 32, 0xff00ff);
}

bool EnemyBoss::CanAttackPlayer(const Player &player) {
  // 攻撃範囲コライダー内にプレイヤーがいるか
  auto playerCol = player.GetBodyCollider();
  return m_pAttackRangeCollider->IsIntersects(playerCol.get());
}

EnemyBase::HitPart EnemyBoss::CheckHitPart(const VECTOR &rayStart,
                                           const VECTOR &rayEnd,
                                           VECTOR &outHtPos,
                                           float &outHtDistSq) const {
  HitPart part = HitPart::None;
  float minDistSq = FLT_MAX;
  VECTOR hitPos = VGet(0, 0, 0);

  // 頭との判定
  if (m_pHeadCollider) {
    VECTOR tmpHitPos;
    float tmpDistSq;
    if (m_pHeadCollider->IsIsIntersectsRay(rayStart, rayEnd, tmpHitPos,
                                           tmpDistSq)) {
      if (tmpDistSq < minDistSq) {
        minDistSq = tmpDistSq;
        hitPos = tmpHitPos;
        part = HitPart::Head;
      }
    }
  }

  // 体との判定
  if (m_pBodyCollider) {
    VECTOR tmpHitPos;
    float tmpDistSq;
    if (m_pBodyCollider->IsIsIntersectsRay(rayStart, rayEnd, tmpHitPos,
                                           tmpDistSq)) {
      if (tmpDistSq < minDistSq) {
        minDistSq = tmpDistSq;
        hitPos = tmpHitPos;
        part = HitPart::Body;
      }
    }
  }

  if (part != HitPart::None) {
    outHtPos = hitPos;
    outHtDistSq = minDistSq;
  }

  return part;
}

void EnemyBoss::OnParried() {
  if (m_isStunned || m_hp <= 0.0f)
    return;

  m_isStunned = true;
  m_stunTimer = 120; // 怯み時間
  // 怯みとして死亡モーションなどを流用、あるいはIdleで止めるなど
  // ここではAcid同様Deadモーションを使ってみる（あるいは専用モーションがあればそちら）
  ChangeAnimation(AnimState::Dead, false);
  // ※Deadモーションを使う場合は死亡判定と競合しないよう注意が必要だが、
  // UpdateStateでm_isStunnedを見て制御すればOK。
  // ただしEnemyBoss::Updateにはm_isStunnedの分岐がまだないので追加が必要。
}
