#include "EnemyAcid.h"
#include "Bullet.h"
#include "CapsuleCollider.h"
#include "Collision.h"
#include "CollisionGrid.h"
#include "DebugUtil.h"
#include "DxLib.h"
#include "Effect.h"
#include "EffekseerWarningSuppress.h"
#include "Game.h"
#include "Player.h"
#include "SceneMain.h"
#include "SphereCollider.h"
#include "Stage.h"
#include "TaskTutorialManager.h"
#include "TransformDataLoader.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>
#include "SoundManager.h"
#include "EnemyAcidState.h"


namespace EnemyAcidConstants 
{
    // アニメーション関連
    constexpr char kAttackAnimName[] = "Armature|ATK"; // 攻撃アニメーション
    constexpr char kWalkAnimName[] = "Armature|WALK";  // 歩くアニメーション
    constexpr char kBackAnimName[] = "Armature|BACK";  // 後退アニメーション
    constexpr char kDeadAnimName[] = "Armature|DEAD";  // 死亡アニメーション

    constexpr VECTOR kHeadShotPositionOffset = {0.0f, 0.0f, 0.0f}; // オフセット

    // コライダーのサイズを定義
    constexpr float kBodyColliderRadius = 40.0f; // 体のコライダー半径
    constexpr float kBodyColliderHeight = 50.0f; // 体のコライダー高さ
    constexpr float kHeadRadius = 18.0f;         // 頭のコライダー半径

    // 攻撃関連（遠距離攻撃に特化）
    constexpr int kAttackCooldownMax = 160;       // 攻撃クールダウン時間
    constexpr float kAttackRangeRadius = 1500.0f; // 攻撃範囲の半径
    constexpr float kAcidBulletSpeed = 5.0f;      // 酸弾の速度

    // 追跡関連（遠距離型なので、近づきすぎたら離れる）
    constexpr float kOptimalAttackDistanceMin = 500.0f; // 攻撃可能最小距離

    // スタン関連
    constexpr int kStunDuration = 120; // スタンの総持続時間
    constexpr float kStunAnimFrameLimit = 60.0f; // スタンアニメーションの再生上限フレーム

    // AcidBallの画面外判定距離
    constexpr float kAcidBallBoundaryDistance = 1500.0f;
 
    // 描画距離
    constexpr float kDrawDistanceSq = 5000.0f * 5000.0f;
    constexpr float kDrawNearDistanceSq = 300.0f * 300.0f;
    constexpr float kDrawDotThreshold = 0.4f;

    // 押し出し
    constexpr float kPushBackEpsilon = 0.0001f;
}

int EnemyAcid::s_modelHandle = -1;
bool EnemyAcid::s_shouldDrawCollision = false;

EnemyAcid::EnemyAcid()
    : m_headPosOffset{EnemyAcidConstants::kHeadShotPositionOffset}
    , m_animTime(0.0f)
    , m_currentAnimState(AnimState::Walk)
    , m_onDropItem(nullptr)
    , m_hasAttacked(false)
    , m_attackEndDelayTimer(0)
    , m_acidBulletSpawnOffset({0.0f, 0.0f, 0.0f})
    , m_backAnimCount(0)
    , m_hasDroppedItem(false)
    , m_isStunned(false)
    , m_stunTimer(0)
    , m_distToPlayer(0.0f)
    , m_isDeadAnimPlaying(false)
    , m_isNextAttackNormal(false)
    , m_shouldDrawParryCollider(false)
    , m_debugParryCapA(VGet(0,0,0))
    , m_debugParryCapB(VGet(0,0,0))
    , m_debugParryRadius(0.0f)

{
    // モデルの複製
    m_modelHandle = MV1DuplicateModel(s_modelHandle);

    // コライダーの初期化
    m_pBodyCollider = std::make_shared<CapsuleCollider>();
    m_pHeadCollider = std::make_shared<SphereCollider>();
    m_pAttackRangeCollider = std::make_shared<SphereCollider>();

  
    // AnimationManagerにアニメーション名を登録
    m_animationManager.SetAnimName(AnimState::Attack, EnemyAcidConstants::kAttackAnimName);
    m_animationManager.SetAnimName(AnimState::Walk, EnemyAcidConstants::kWalkAnimName);
    m_animationManager.SetAnimName(AnimState::Dead, EnemyAcidConstants::kDeadAnimName);
    m_animationManager.SetAnimName(AnimState::Back, EnemyAcidConstants::kBackAnimName);
}

EnemyAcid::~EnemyAcid()
{
  // モデルの解放
  MV1DeleteModel(m_modelHandle);

    for (auto& ball : m_acidBalls)
    {
        if (ball.effectHandle != -1)
        {
            StopEffekseer3DEffect(ball.effectHandle);
        }
    }
}

void EnemyAcid::LoadModel()
{
  s_modelHandle = MV1LoadModel("data/model/AcidZombie.mv1");
  assert(s_modelHandle != -1);
}

void EnemyAcid::DeleteModel()
{
  MV1DeleteModel(s_modelHandle);
}

// 初期化
void EnemyAcid::Init()
{
  m_attackCooldownMax = EnemyAcidConstants::kAttackCooldownMax;
  m_attackCooldown = 0; // 最初は攻撃可能にしておく

  m_isAlive = true;
  m_isDeadAnimPlaying = false;

  // CSVからAcidEnemyのTransform情報を取得
  LoadTransformData("AcidEnemy");

  // ここで一度「絶対にRunでない値」にリセット
  // 初期アニメーションを強制的に再生させるため
  m_currentAnimState = AnimState::Dead;

  // 初期ステートの設定
  ChangeState(std::make_shared<EnemyAcidStateWalk>());

  m_isNextAttackNormal = false; // 最初はパリィ弾から

  // ターゲットオフセットの初期化 (±400.0f)
  float offsetX = static_cast<float>(GetRand(800) - 400);
  float offsetZ = static_cast<float>(GetRand(800) - 400);
  m_targetOffset = VGet(offsetX, 0.0f, offsetZ);
}

// アニメーションを変更する
void EnemyAcid::ChangeAnimation(AnimState newAnimState, bool loop)
{
  // 後退アニメーションは同じ状態でも必ず再生し直す
    if (m_currentAnimState == newAnimState)
    {
        if (newAnimState == AnimState::Attack || newAnimState == AnimState::Back)
        {
            m_animationManager.PlayAnimation(m_modelHandle, (newAnimState == AnimState::Attack) ? EnemyAcidConstants::kAttackAnimName : EnemyAcidConstants::kBackAnimName, loop);
            m_animTime = 0.0f;
            if (newAnimState == AnimState::Attack)
            {
                m_hasAttacked = false;

                // 攻撃ボイスを再生
                // 距離に応じた音量計算 (2000以上で無音、最大音量を150に抑える)
                float maxDist = 2000.0f;
                float volRatio = 1.0f - (m_distToPlayer / maxDist);
                if (volRatio < 0.0f) volRatio = 0.0f;
                if (volRatio > 1.0f) volRatio = 1.0f;

                SoundManager::GetInstance()->Play("EnemyAcid", "Attack", (int)(150 * volRatio));
            }
            if (newAnimState == AnimState::Back) m_backAnimCount = 0;
        }
        return;
    }

    const char* animName = nullptr;
    switch (newAnimState)
    {
    case AnimState::Walk:
        animName = EnemyAcidConstants::kWalkAnimName;
        break;
      case AnimState::Attack:
        animName = EnemyAcidConstants::kAttackAnimName;
        break;
      case AnimState::Dead:
        animName = EnemyAcidConstants::kDeadAnimName;
        break;
    case AnimState::Back:
        animName = EnemyAcidConstants::kBackAnimName;
        break;
    default:
        return;
    }

    if (animName)
    {
        m_animationManager.PlayAnimation(m_modelHandle, animName, loop);
        m_animTime = 0.0f;
        if (newAnimState == AnimState::Attack)
        {
            m_hasAttacked = false;

            // 攻撃ボイスを再生
            // 距離に応じた音量計算
            float maxDist = 2000.0f;
            float volRatio = 1.0f - (m_distToPlayer / maxDist);
    
            if (volRatio < 0.0f) volRatio = 0.0f;
            if (volRatio > 1.0f) volRatio = 1.0f;

            SoundManager::GetInstance()->Play("EnemyAcid", "Attack", (int)(150 * volRatio));
        }
        if (newAnimState == AnimState::Back) m_backAnimCount = 0;
    }

  m_currentAnimState = newAnimState;
}

void EnemyAcid::ChangeState(std::shared_ptr<EnemyState<EnemyAcid>> newState)
{
    if (m_pCurrentState)
    {
        m_pCurrentState->Exit(this);
    }
    m_pCurrentState = newState;
    if (m_pCurrentState)
    {
        m_pCurrentState->Enter(this);
    }
}

// プレイヤーに攻撃可能かどうかを判定する
bool EnemyAcid::CanAttackPlayer(const Player &player)
{
    VECTOR playerPos = player.GetPos();
    m_pAttackRangeCollider->SetCenter(m_pos);
    m_pAttackRangeCollider->SetRadius(EnemyAcidConstants::kAttackRangeRadius);

    // プレイヤーのボディコライダーを取得
    std::shared_ptr<CapsuleCollider> playerBodyCollider = player.GetBodyCollider();

    // プレイヤーが攻撃範囲内にいるかどうかのみ判定
    return m_pAttackRangeCollider->IsIntersects(playerBodyCollider.get());
}

// 酸弾を発射する
void EnemyAcid::ShootAcidBullet(std::vector<Bullet> &bullets, const Player &player, Effect *pEffect, const std::vector<Stage::StageCollisionData> &stageCollision, const CollisionGrid *pGrid) 
{
    // 発射位置
    int mouthIndex = MV1SearchFrame(m_modelHandle, "mixamorig:JawDowm");
    VECTOR spawnPos = m_pos;

    if (mouthIndex != -1) 
    {
        spawnPos = MV1GetFramePosition(m_modelHandle, mouthIndex);
    }
    spawnPos = VAdd(spawnPos, m_acidBulletSpawnOffset);
  
    // プレイヤーの位置
    VECTOR target = player.GetPos();

    // 直線的な攻撃処理
    VECTOR toTarget = VSub(target, spawnPos);

    AcidBall ball;
    ball.pos = spawnPos;

    // デフォルト設定
    ball.active = true;
    ball.damage = m_attackPower;
    ball.owner = this;
    ball.isReflected = false;
    ball.isParabolic = false;
    ball.gravity = 0.0f;
    ball.speed = EnemyAcidConstants::kAcidBulletSpeed;
    ball.dir = VNorm(toTarget);

    // 放物線攻撃判定
    std::string groundedObj = player.GetGroundedObjectName();

    // 発射基準位置（頭）
    int headIndex = MV1SearchFrame(m_modelHandle, "mixamorig:Head");
    VECTOR viewStartPos = (headIndex != -1) ? MV1GetFramePosition(m_modelHandle, headIndex) : VAdd(m_pos, m_headPosOffset);

    if ((groundedObj == "Rock3" || groundedObj == "Rock6") && !EnemyBase::IsTargetVisible(viewStartPos, player.GetPos(), stageCollision, pGrid)) 
    {
        ball.isParabolic = true;
        ball.gravity = 0.3f; // 重力設定
        ball.velocity = EnemyBase::CalculateParabolicVelocity(ball.pos, target, ball.gravity, EnemyAcidConstants::kAcidBulletSpeed);
    }

  
    if (m_isNextAttackNormal) 
    {
        // 通常弾 (パリィ不可)
        ball.isParryable = false;
        if (pEffect) 
        {
            ball.effectHandle = pEffect->PlayNormalBulletEffect(spawnPos.x, spawnPos.y, spawnPos.z);
        }
  
    }
    else 
    {
        // パリィ弾 (パリィ可能)
        ball.isParryable = true;
        if (pEffect) 
        {
            ball.effectHandle = pEffect->PlayAcidEffect(spawnPos.x, spawnPos.y, spawnPos.z);
        }
    }
    // 次回のためにフラグ反転
    m_isNextAttackNormal = !m_isNextAttackNormal;

    m_acidBalls.push_back(ball);
}

// 更新処理
void EnemyAcid::Update(const EnemyUpdateContext &context) 
{
    UpdateStandard(context);
}

void EnemyAcid::UpdateAI(const EnemyUpdateContext &context) 
{
    const Player &player = context.player;
    Effect *pEffect = context.pEffect;

#ifdef _DEBUG
    m_shouldDrawParryCollider = false;
#endif

    // AcidBallの更新と当たり判定
    for (auto &ball : m_acidBalls) 
    {
        if (!ball.active) continue;
        ball.Update(Game::GetTimeScale());

        if (ball.effectHandle != -1) 
        {
            SetPosPlayingEffekseer3DEffect(ball.effectHandle, ball.pos.x, ball.pos.y, ball.pos.z);
        }

        VECTOR toPlayer = VSub(ball.pos, player.GetPos());
        float distanceToPlayer = VSize(toPlayer);

        if (distanceToPlayer > EnemyAcidConstants::kAcidBallBoundaryDistance) 
        {
            ball.active = false;
            if (ball.effectHandle != -1) { StopEffekseer3DEffect(ball.effectHandle); ball.effectHandle = -1; }
            continue;
        }

        if (ball.isParryable && !ball.isReflected && distanceToPlayer <= 100.0f) 
        {
            TaskTutorialManager::GetInstance()->NotifyParryableAttack();
        }

        if (!ball.isReflected) 
        {
            SphereCollider acidCol(ball.pos, ball.radius);
            bool hitDetected = false;

            if (ball.isParryable && player.IsJustGuarded()) 
            {
                VECTOR playerCapA, playerCapB;
                float playerRadius;
                player.GetCapsuleInfo(playerCapA, playerCapB, playerRadius);
                float parryRadius = playerRadius * 1.5f;
                CapsuleCollider parryCollider(playerCapA, playerCapB, parryRadius);

#ifdef _DEBUG
                m_shouldDrawParryCollider = true;
                m_debugParryCapA = playerCapA;
                m_debugParryCapB = playerCapB;
                m_debugParryRadius = parryRadius;
#endif
                if (acidCol.IsIntersects(&parryCollider))
                {
                    hitDetected = true;
                    ball.isReflected = true;
                    const_cast<Player &>(player).PlayParrySE();
                    TaskTutorialManager::GetInstance()->NotifyParrySuccess();
                    const auto &playerCam = player.GetCamera();
                    if (playerCam) 
                    {
                        VECTOR enemyBodyCenter = m_pos;
                        enemyBodyCenter.y += 50.0f;
                        ball.dir = VNorm(VSub(enemyBodyCenter, ball.pos));
                        ball.speed *= 1.5f;
                        Game::SetTimeScale(0.1f, 1.0f);
                    }
                }
            }
      
            if (!hitDetected) 
            {
                std::shared_ptr<CapsuleCollider> playerCol = player.GetBodyCollider();
                if (acidCol.IsIntersects(playerCol.get())) 
                {
                    hitDetected = true;
                    const_cast<Player &>(player).TakeDamage(ball.damage, m_pos, ball.isParryable);
                    ball.active = false;
                    if (ball.effectHandle != -1) { StopEffekseer3DEffect(ball.effectHandle); ball.effectHandle = -1; }
                }
            }
        }
        else 
        {
            SphereCollider reflectedAcidCol(ball.pos, ball.radius);
            if (reflectedAcidCol.IsIntersects(GetBodyCollider().get())) 
            {
                TakeDamage(ball.damage, AttackType::Parry);
                OnParried();
                ball.active = false;
                if (ball.effectHandle != -1) { StopEffekseer3DEffect(ball.effectHandle); ball.effectHandle = -1; }
            }
        }

        if (ball.pos.y < 0.0f) 
        {
            ball.active = false;
            if (ball.effectHandle != -1) { StopEffekseer3DEffect(ball.effectHandle); ball.effectHandle = -1; }
        }
    }
    m_acidBalls.erase(std::remove_if(m_acidBalls.begin(), m_acidBalls.end(), [](const AcidBall &b) { return !b.active; }), m_acidBalls.end());

    // ステート更新
    if (m_isStunned) 
    {
        m_stunTimer--;
        if (m_stunTimer <= 0) 
        {
            m_isStunned = false;
            if (m_hp > 0.0f)
            {
                if (CanAttackPlayer(player)) ChangeState(std::make_shared<EnemyAcidStateAttack>());
                else ChangeState(std::make_shared<EnemyAcidStateWalk>());
            }
        }
        return;
    }

    if (m_pCurrentState)
    {
        m_pCurrentState->Update(this, context);
    }

    VECTOR playerPos = player.GetPos();
    RotateTowards(playerPos, 0.05f * Game::GetTimeScale());
    float disToPlayer = VSize(VSub(playerPos, m_pos));

    std::shared_ptr<CapsuleCollider> playerBodyCollider = player.GetBodyCollider();
    bool inAttackRange = m_pAttackRangeCollider->IsIntersects(playerBodyCollider.get());

    if (m_currentAnimState == AnimState::Attack || m_attackEndDelayTimer > 0) {}
    else if (inAttackRange) 
    {
        if (disToPlayer < EnemyAcidConstants::kOptimalAttackDistanceMin) 
        {
            if (m_currentAnimState != AnimState::Back) ChangeState(std::make_shared<EnemyAcidStateBack>());
            VECTOR dirAway = VNorm(VSub(m_pos, playerPos));
            float scaledSpeed = m_chaseSpeed * Game::GetTimeScale();
            m_pos.x += dirAway.x * scaledSpeed;
            m_pos.z += dirAway.z * scaledSpeed;
        }
        else 
        {
            if (m_attackCooldown == 0) ChangeState(std::make_shared<EnemyAcidStateAttack>());
        }
    }
    else 
    {
        if (m_currentAnimState != AnimState::Walk) ChangeState(std::make_shared<EnemyAcidStateWalk>());
        VECTOR targetPos = VAdd(playerPos, m_targetOffset);
        VECTOR dirTowards = VNorm(VSub(targetPos, m_pos));
        float scaledSpeed = m_chaseSpeed * Game::GetTimeScale();
        m_pos.x += dirTowards.x * scaledSpeed;
        m_pos.z += dirTowards.z * scaledSpeed;
    }

    if (m_currentAnimState == AnimState::Attack) 
    {
        float totalAttackAnimTime = m_animationManager.GetAnimationTotalTime(m_modelHandle, EnemyAcidConstants::kAttackAnimName);
        if (!m_hasAttacked && m_animTime >= totalAttackAnimTime * 0.3f) 
        {
            ShootAcidBullet(const_cast<std::vector<Bullet>&>(context.bullets), player, pEffect, context.collisionData, context.collisionGrid);
            m_hasAttacked = true;
        }
        if (m_animTime >= totalAttackAnimTime) 
        {
            m_attackEndDelayTimer = 20;
            m_animTime = 0.0f;
            if (m_animationManager.GetCurrentAttachedAnimHandle(m_modelHandle) != -1) 
            {
                MV1DetachAnim(m_modelHandle, 0);
                m_animationManager.ResetAttachedAnimHandle(m_modelHandle);
            }
        }
    }

    if (m_attackCooldown > 0) m_attackCooldown--;
    if (m_attackEndDelayTimer > 0) 
    {
        m_attackEndDelayTimer--;
        if (m_attackEndDelayTimer == 0 && m_currentAnimState != AnimState::Walk) ChangeState(std::make_shared<EnemyAcidStateWalk>());
    }
}

void EnemyAcid::UpdateAnimation(const EnemyUpdateContext &context) 
{
    // コライダー更新
    int hipsIndex = MV1SearchFrame(m_modelHandle, "mixamorig:Hips");
    VECTOR hipsPos = (hipsIndex != -1) ? MV1GetFramePosition(m_modelHandle, hipsIndex) : m_pos;
    VECTOR bodySegmentP1 = VAdd(hipsPos, VGet(0, EnemyAcidConstants::kBodyColliderHeight * 0.5f, 0));
    VECTOR bodySegmentP2 = VAdd(hipsPos, VGet(0, -EnemyAcidConstants::kBodyColliderHeight * 0.5f, 0));
    m_pBodyCollider->SetSegment(bodySegmentP1, bodySegmentP2);
    m_pBodyCollider->SetRadius(EnemyAcidConstants::kBodyColliderRadius);

    int headIndex = MV1SearchFrame(m_modelHandle, "mixamorig:Head");
    VECTOR headModelPos = (headIndex != -1) ? MV1GetFramePosition(m_modelHandle, headIndex) : VAdd(m_pos, m_headPosOffset);
    m_pHeadCollider->SetCenter(VAdd(headModelPos, m_headPosOffset));
    m_pHeadCollider->SetRadius(EnemyAcidConstants::kHeadRadius);

    VECTOR attackRangeCenter = m_pos;
    attackRangeCenter.y += (EnemyAcidConstants::kBodyColliderHeight * 0.5f);
    m_pAttackRangeCollider->SetCenter(attackRangeCenter);
    m_pAttackRangeCollider->SetRadius(EnemyAcidConstants::kAttackRangeRadius);

    // アニメ更新
    if (m_attackEndDelayTimer == 0 || m_isStunned) 
    {
        m_animTime += 1.0f * Game::GetTimeScale();
        if (m_isStunned)
        {
            if (m_animTime > EnemyAcidConstants::kStunAnimFrameLimit) m_animTime = EnemyAcidConstants::kStunAnimFrameLimit;
        }
        else
        {
            const char* animName = nullptr;
            if (m_currentAnimState == AnimState::Back) animName = EnemyAcidConstants::kBackAnimName;
            else if (m_currentAnimState == AnimState::Walk) animName = EnemyAcidConstants::kWalkAnimName;
            else if (m_currentAnimState == AnimState::Attack) animName = EnemyAcidConstants::kAttackAnimName;
            else if (m_currentAnimState == AnimState::Dead) animName = EnemyAcidConstants::kDeadAnimName;

            float animTotal = m_animationManager.GetAnimationTotalTime(m_modelHandle, animName);
            if ((m_currentAnimState == AnimState::Back || m_currentAnimState == AnimState::Walk) && animTotal > 0.0f) 
            {
                if (m_animTime >= animTotal) m_animTime = fmodf(m_animTime, animTotal);
            }
            else if (m_animTime >= animTotal) m_animTime = animTotal;
        }
        m_animationManager.UpdateAnimationTime(m_modelHandle, m_animTime);
    }

    m_distToPlayer = VSize(VSub(context.player.GetPos(), m_pos));
}

void EnemyAcid::UpdateSimpleMode(const EnemyUpdateContext &context) 
{
    if (m_hp > 0.0f) 
    {
        VECTOR playerPos = context.player.GetPos();
        VECTOR targetPos = VAdd(playerPos, m_targetOffset);
        VECTOR dirTowards = VNorm(VSub(targetPos, m_pos));
        float scaledSpeed = m_chaseSpeed * Game::GetTimeScale();
        m_pos.x += dirTowards.x * scaledSpeed;
        m_pos.z += dirTowards.z * scaledSpeed;
        RotateTowards(playerPos, 0.05f * Game::GetTimeScale());
    }
    UpdateStageCollision(context.collisionData, context.collisionGrid);
}

void EnemyAcid::Draw() 
{
    DrawStandard(EnemyAcidConstants::kDrawDistanceSq, EnemyAcidConstants::kDrawNearDistanceSq, EnemyAcidConstants::kDrawDotThreshold);
}

void EnemyAcid::UpdateDeath(const EnemyUpdateContext &context) 
{
    if (!m_isDeadAnimPlaying)
    {
        ChangeState(std::make_shared<EnemyAcidStateDead>());
        m_isDeadAnimPlaying = true;
        m_animTime = 0.0f;
        m_isAlive = true;
    }

    if (m_animationManager.GetCurrentAttachedAnimHandle(m_modelHandle) != -1)
    {
        if (m_shouldUpdateAI)
        {
            m_animTime += (1.0f * m_aiUpdateInterval) * Game::GetTimeScale();
            m_animationManager.UpdateAnimationTime(m_modelHandle, m_animTime);
        }
    }

    float currentAnimTotalTime = m_animationManager.GetAnimationTotalTime(m_modelHandle, EnemyAcidConstants::kDeadAnimName);
    if (m_animTime >= currentAnimTotalTime)
    {
        if (m_animationManager.GetCurrentAttachedAnimHandle(m_modelHandle) != -1)
        {
            MV1DetachAnim(m_modelHandle, 0);
            m_animationManager.ResetAttachedAnimHandle(m_modelHandle);
        }

        if (!m_hasDroppedItem && m_onDropItem)
        {
            m_onDropItem(m_pos);
            m_onDropItem = nullptr;
            m_hasDroppedItem = true;
        }
        if (m_onDeathCallback)
        {
            m_onDeathCallback(m_pos);
            m_onDeathCallback = nullptr;
        }
        m_isAlive = false;
        SetActive(false);
    }
}

// パリィされた時のコールバック
void EnemyAcid::OnParried() 
{
    // 既に怯んでいるか、死んでいる場合は何もしない
    if (m_isStunned || m_hp <= 0.0f) return;

    m_isStunned = true;
    m_stunTimer = EnemyAcidConstants::kStunDuration; // 怯み時間
    ChangeState(std::make_shared<EnemyAcidStateStunned>());

    // パリィボイスを再生
    float maxDist = 2000.0f;
    float volRatio = 1.0f - (m_distToPlayer / maxDist);
    if (volRatio < 0.0f) volRatio = 0.0f;
    if (volRatio > 1.0f) volRatio = 1.0f;

    SoundManager::GetInstance()->Play("EnemyAcid", "ParryHit", (int)(150 * volRatio));
}

// 描画処理
void EnemyAcid::Draw() 
{
    // 死亡時も死亡アニメーションが終わるまでは描画する
    if (!m_isAlive) 
    {
        float currentAnimTotalTime = m_animationManager.GetAnimationTotalTime(m_modelHandle, EnemyAcidConstants::kDeadAnimName);
        if (m_animTime < currentAnimTotalTime) 
        {
            MV1DrawModel(m_modelHandle);
        }
        return;
    }

    // 死亡アニメーションが完全に終了したらモデルを描画しない
    if (m_hp <= 0.0f && m_animationManager.IsAnimationFinished(m_modelHandle)) return;

    // 視錐台カリング (描画最適化)
    // 距離チェック: 5000^2, 近距離: 300^2, 内積閾値: 0.4
    if (!ShouldDraw(5000.0f * 5000.0f, 300.0f * 300.0f, 0.4f)) return;

    EnemyBase::IncrementDrawCount();
    MV1DrawModel(m_modelHandle);

    // デバッグ用の当たり判定描画
    if (s_shouldDrawCollision) 
    {
        DrawCollisionDebug();
    }

#ifdef _DEBUG
    // 体力デバッグ表示
    DebugUtil::DrawFormat(20, 100, 0xffffff, "EnemyAcid HP: %.1f", m_hp);
#endif
}

void EnemyAcid::DrawCollisionDebug() const
{

    if (!s_shouldDrawCollision) return;
  
    if (m_pBodyCollider) 
    {
        DebugUtil::DrawCapsule(m_pBodyCollider->GetSegmentA(), m_pBodyCollider->GetSegmentB(), m_pBodyCollider->GetRadius(), 16, 0xff00ff);
    }
    if (m_pHeadCollider) 
    {
        DebugUtil::DrawSphere(m_pHeadCollider->GetCenter(), m_pHeadCollider->GetRadius(), 16, 0xffff00);
    }
    if (m_pAttackRangeCollider) 
    {
        DebugUtil::DrawSphere(m_pAttackRangeCollider->GetCenter(), m_pAttackRangeCollider->GetRadius(), 16, 0x00ffff);
    }
    if (m_shouldDrawParryCollider) 
    {
        DebugUtil::DrawCapsule(m_debugParryCapA, m_debugParryCapB, m_debugParryRadius, 16, 0x0000ff);
    }
}

// どこに当たったのか判定する
EnemyBase::HitPart EnemyAcid::CheckHitPart(const VECTOR &rayStart, const VECTOR &rayEnd, VECTOR &outHtPos, float &outHtDistSq) const
{
    if (m_isDeadAnimPlaying) return HitPart::None;

    VECTOR hitPosHead, hitPosBody; // 当たった位置
    float hitDistSqHead = FLT_MAX;
    float hitDistSqBody = FLT_MAX;

    // 頭のフレーム位置を取得してコライダー中心に設定
    int headIndex = MV1SearchFrame(m_modelHandle, "mixamorig:Head");
    VECTOR headCenter = (headIndex != -1) ? MV1GetFramePosition(m_modelHandle, headIndex) : VAdd(m_pos, m_headPosOffset);
  
    m_pHeadCollider->SetCenter(headCenter);
    m_pHeadCollider->SetRadius(EnemyAcidConstants::kHeadRadius);

    bool headHit = m_pHeadCollider->IsIsIntersectsRay(rayStart, rayEnd, hitPosHead, hitDistSqHead);
    bool bodyHit = m_pBodyCollider->IsIsIntersectsRay(rayStart, rayEnd, hitPosBody, hitDistSqBody);

    if (headHit && bodyHit) 
    {
        if (hitDistSqHead <= hitDistSqBody) 
        {
            outHtPos = hitPosHead;
            outHtDistSq = hitDistSqHead;
            return HitPart::Head;
        }
        else 
        {
            outHtPos = hitPosBody;
            outHtDistSq = hitDistSqBody;
            return HitPart::Body;
        }
    } 
    else if (headHit) 
    {
        outHtPos = hitPosHead;
        outHtDistSq = hitDistSqHead;
        return HitPart::Head;
    }
    else if (bodyHit) 
    {
        outHtPos = hitPosBody;
        outHtDistSq = hitDistSqBody;
        return HitPart::Body;
    }

    outHtPos = VGet(0, 0, 0);
    outHtDistSq = FLT_MAX;
    return HitPart::None;
}

// アイテムドロップ時のコールバック関数
void EnemyAcid::SetOnDropItemCallback(std::function<void(const VECTOR &)> cb) 
{
    m_onDropItem = cb;
}

// ダメージ処理
void EnemyAcid::TakeDamage(float damage, AttackType type) 
{
    if (m_isDeadAnimPlaying) return;

    EnemyBase::TakeDamage(damage, type);
  
}
  
void EnemyAcid::OnDeath()
{
    // スコア加算
    if (m_lastHitPart == HitPart::None) m_lastHitPart = HitPart::Body;
    bool isHeadShot = (m_lastHitPart == HitPart::Head);
    int addScore = ScoreManager::Instance().AddScore(isHeadShot);

    if (SceneMain::Instance()) 
    {
        SceneMain::Instance()->AddScorePopup(addScore, isHeadShot, ScoreManager::Instance().GetCombo());
    }

    // 死亡時に残っているAcidBallを全て停止
    for (auto& ball : m_acidBalls)
    {
        if (ball.effectHandle != -1)
        {
            StopEffekseer3DEffect(ball.effectHandle);
            ball.effectHandle = -1;
        }
    }
    m_acidBalls.clear(); // 全てのAcidBallをクリア
}

// タックル攻撃のダメージ処理
void EnemyAcid::TakeTackleDamage(float damage) 
{
    if (m_isDeadAnimPlaying) return;
  
    EnemyBase::TakeTackleDamage(damage);
}

std::shared_ptr<CapsuleCollider> EnemyAcid::GetBodyCollider() const 
{
    return m_pBodyCollider;
}

void EnemyAcid::AcidBall::Update(float timeScale)
{
    if (!active) return;

    if (isParabolic) 
    {
        // 放物線運動
        velocity.y -= gravity * timeScale;
        pos = VAdd(pos, VScale(velocity, timeScale));
    }
    else 
    {
        // 通常の直線運動
        // タイムスケールを適用
        pos = VAdd(pos, VScale(dir, speed * timeScale));
    }

  if (pos.y < 0.0f) active = false; // 地面で消滅
}

// 死亡時の更新処理
void EnemyAcid::UpdateDeath()
{
    if (!m_isDeadAnimPlaying) 
    {
        ChangeAnimation(AnimState::Dead, false);
        m_isDeadAnimPlaying = true;
        m_animTime = 0.0f; // アニメーション時間をリセット
        m_isAlive = true;  // 死亡アニメーション中はtrueのまま
    }

    // 死亡アニメーション中もアニメーション時間を更新
    if (m_animationManager.GetCurrentAttachedAnimHandle(m_modelHandle) != -1) 
    {
        m_animTime += 1.0f * Game::GetTimeScale();
        m_animationManager.UpdateAnimationTime(m_modelHandle, m_animTime);
    }

    float currentAnimTotalTime = m_animationManager.GetAnimationTotalTime(m_modelHandle, EnemyAcidConstants::kDeadAnimName);
    if (m_animTime >= currentAnimTotalTime) 
    {
        if (m_animationManager.GetCurrentAttachedAnimHandle(m_modelHandle) != -1) 
        {
            MV1DetachAnim(m_modelHandle, 0);
            m_animationManager.ResetAttachedAnimHandle(m_modelHandle);
        }

        // アイテムドロップと死亡コールバックを呼び出し
        if (!m_hasDroppedItem && m_onDropItem) 
        {
            m_onDropItem(m_pos);
            m_onDropItem = nullptr;
            m_hasDroppedItem = true;
    
        }
        if (m_onDeathCallback) 
        {
            m_onDeathCallback(m_pos);
            m_onDeathCallback = nullptr; // 一度だけ呼び出す
        }
        m_isAlive = false; // 死亡アニメーション終了時のみfalseにする
    }
}
