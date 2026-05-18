#include "EnemyRunner.h"
#include "Bullet.h"
#include "CapsuleCollider.h"
#include "CollisionGrid.h"
#include "DebugUtil.h"
#include "DxLib.h"
#include "EffekseerWarningSuppress.h"
#include "Player.h"
#include "SceneMain.h"
#include "SphereCollider.h"
#include "TransformDataLoader.h"
#include "ScoreManager.h"
#include "Game.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>
#include <string>
#include <vector>
#include <memory>
#include "SoundManager.h"
#include "EnemyRunnerState.h"

int EnemyRunner::s_modelHandle = -1;
bool EnemyRunner::s_shouldDrawCollision = false;

EnemyRunner::EnemyRunner()
    : m_headPosOffset{ EnemyRunnerConstants::kHeadShotPositionOffset }
    , m_hasTakenTackleDamage(false)
    , m_animTime(0.0f)
    , m_hasAttackHit(false)
    , m_onDropItem(nullptr)
    , m_currentAnimState(AnimState::Run)
    , m_attackEndDelayTimer(0)
    , m_isDeadAnimPlaying(false)
    , m_hasDroppedItem(false)
    , m_wanderTimer(0)
    , m_wanderOffset(VGet(0.0f, 0.0f, 0.0f))
    , m_evadeSwitchTimer(0.0f)
    , m_isEvadingRight(false)
    , m_distToPlayer(0.0f)
    , m_damageSECooldown(0.0f)
{
    m_modelHandle = MV1DuplicateModel(s_modelHandle);

    m_pBodyCollider = std::make_shared<CapsuleCollider>();
    m_pHeadCollider = std::make_shared<SphereCollider>();
    m_pAttackRangeCollider = std::make_shared<SphereCollider>();
    m_pAttackHitCollider = std::make_shared<CapsuleCollider>();
}

EnemyRunner::~EnemyRunner()
{
    MV1DeleteModel(m_modelHandle);
}

void EnemyRunner::LoadModel()
{
    s_modelHandle = MV1LoadModel("data/model/RunnerZombie.mv1");
    assert(s_modelHandle != -1);
}

void EnemyRunner::DeleteModel()
{
    MV1DeleteModel(s_modelHandle);
}

void EnemyRunner::Init()
{
    m_attackCooldownMax = EnemyRunnerConstants::kAttackCooldownMax;
    m_isAlive = true;
    m_isDeadAnimPlaying = false;
    m_hasDroppedItem = false;
    m_lastHitPart = HitPart::None;
    m_hitDisplayTimer = 0;

    LoadTransformData("RunnerEnemy");

    // ChangeAnimation で初期アニメーションを強制再生させるため、現在状態を非Run値でリセット
    m_currentAnimState = AnimState::Dead;

    ChangeState(std::make_shared<EnemyRunnerStateRun>());

    // 追跡目標にランダムオフセットを付与して各敵が同一点に集中しないようにする
    float offsetX = static_cast<float>(GetRand(EnemyRunnerConstants::kTargetOffsetRange * 2) - EnemyRunnerConstants::kTargetOffsetRange);
    float offsetZ = static_cast<float>(GetRand(EnemyRunnerConstants::kTargetOffsetRange * 2) - EnemyRunnerConstants::kTargetOffsetRange);
    m_targetOffset = VGet(offsetX, 0.0f, offsetZ);

    m_evadeSwitchTimer = 0.0f;
    m_isEvadingRight = (GetRand(1) == 0);

    m_wanderTimer = 0;
    m_wanderOffset = VGet(0.0f, 0.0f, 0.0f);
}

void EnemyRunner::ChangeAnimation(AnimState newAnimState, bool loop)
{
    if (m_currentAnimState == newAnimState)
    {
        // 同じ状態でも必ずリセット再生する
        switch (newAnimState)
        {
        case AnimState::Attack:
            m_animationManager.PlayAnimation(m_modelHandle, EnemyRunnerConstants::kAttackAnimName, loop);
            break;
        case AnimState::Run:
            m_animationManager.PlayAnimation(m_modelHandle, EnemyRunnerConstants::kRunAnimName, loop);
            break;
        case AnimState::Dead:
            m_animationManager.PlayAnimation(m_modelHandle, EnemyRunnerConstants::kDeadAnimName, loop);
            break;
        }
        m_animTime = 0.0f;
        m_currentAnimState = newAnimState;
        return;
    }

    const char* animName = nullptr;

    switch (newAnimState)
    {
    case AnimState::Run:
        animName = EnemyRunnerConstants::kRunAnimName;
        break;
    case AnimState::Attack:
        animName = EnemyRunnerConstants::kAttackAnimName;
        break;
    case AnimState::Dead:
        animName = EnemyRunnerConstants::kDeadAnimName;
        break;
    }

    if (animName)
    {
        m_animationManager.PlayAnimation(m_modelHandle, animName, loop);
        m_animTime = 0.0f;

        // 攻撃ボイスの再生
        if (newAnimState == AnimState::Attack)
        {
            float volRatio = 1.0f - (m_distToPlayer / EnemyRunnerConstants::kVoiceMaxDist);
            if (volRatio < 0.0f) volRatio = 0.0f;
            if (volRatio > 1.0f) volRatio = 1.0f;
            SoundManager::GetInstance()->Play("EnemyRunner", "Attack", static_cast<int>(EnemyRunnerConstants::kAttackVoiceVolMax * volRatio));
        }
    }

    m_currentAnimState = newAnimState;
}

void EnemyRunner::ChangeState(std::shared_ptr<EnemyState<EnemyRunner>> newState)
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

bool EnemyRunner::CanAttackPlayer(const Player& player, float checkRadius)
{
    int handRIndex = MV1SearchFrame(m_modelHandle, "mixamorig:RightHand");
    int handLIndex = MV1SearchFrame(m_modelHandle, "mixamorig:LeftHand");
    if (handRIndex == -1 || handLIndex == -1) return false;

    VECTOR handRPos = MV1GetFramePosition(m_modelHandle, handRIndex);
    VECTOR handLPos = MV1GetFramePosition(m_modelHandle, handLIndex);

    m_pAttackHitCollider->SetSegment(handRPos, handLPos);

    float radius = (checkRadius > 0.0f) ? checkRadius : EnemyRunnerConstants::kAttackHitRadius;
    m_pAttackHitCollider->SetRadius(radius);

    std::shared_ptr<CapsuleCollider> playerBodyCollider = player.GetBodyCollider();
    return m_pAttackHitCollider->IsIntersects(playerBodyCollider.get());
}

void EnemyRunner::Update(const EnemyUpdateContext& context)
{
    UpdateStandard(context);
}

void EnemyRunner::UpdateAI(const EnemyUpdateContext& context)
{
    const Player& player = context.player;

    // ダメージSEのクールタイム更新
    if (m_damageSECooldown > 0.0f)
    {
        m_damageSECooldown -= 1.0f * Game::GetTimeScale() * m_aiUpdateInterval;
    }

    if (!m_shouldUpdateAI) return;

    // プレイヤーの方向を向く（追跡中のみ）
    if (m_currentAnimState == AnimState::Run)
    {
        VECTOR playerPos = player.GetPos();
        VECTOR targetPos;

        std::string groundObj = player.GetGroundedObjectName();
        if (groundObj == "Rock3" || groundObj == "Rock6")
        {
            m_wanderTimer -= m_aiUpdateInterval;
            if (m_wanderTimer <= 0)
            {
                m_wanderTimer = EnemyRunnerConstants::kWanderTimerInterval;
                float angle = static_cast<float>(GetRand(360)) * DX_PI_F / 180.0f;
                float dist = EnemyRunnerConstants::kWanderMinDist + static_cast<float>(GetRand(EnemyRunnerConstants::kWanderDistRange));
                m_wanderOffset = VGet(cosf(angle) * dist, 0.0f, sinf(angle) * dist);
            }
            targetPos = VAdd(playerPos, m_wanderOffset);
        }
        else
        {
            targetPos = VAdd(playerPos, m_targetOffset);
        }

        RotateTowards(targetPos, EnemyRunnerConstants::kRotateSpeedPerFrame * Game::GetTimeScale() * m_aiUpdateInterval);

        // 移動と回避
        {
            VECTOR toPlayer = VSub(targetPos, m_pos);
            toPlayer.y = 0.0f;
            VECTOR dir = VNorm(toPlayer);
            VECTOR moveVec = VScale(dir, m_chaseSpeed * Game::GetTimeScale() * m_aiUpdateInterval);

            // 回避判定
            VECTOR camPos = GetCameraPosition();
            VECTOR camDir = VNorm(VSub(GetCameraTarget(), camPos));
            VECTOR rayEnd = VAdd(camPos, VScale(camDir, 5000.0f));

            VECTOR hitPos;
            float hitDistSq;
            HitPart hitPart = CheckHitPart(camPos, rayEnd, hitPos, hitDistSq);
            bool shouldEvade = (hitPart != HitPart::None);

            if (!shouldEvade)
            {
                VECTOR enemyCenter = VAdd(m_pos, VGet(0.0f, EnemyRunnerConstants::kBodyColliderHeight * 0.5f, 0.0f));
                VECTOR toEnemy = VSub(enemyCenter, camPos);
                float t = VDot(toEnemy, camDir);
                if (t > 0.0f)
                {
                    VECTOR nearestPoint = VAdd(camPos, VScale(camDir, t));
                    float distSq = VSquareSize(VSub(enemyCenter, nearestPoint));
                    if (distSq < EnemyRunnerConstants::kEvasionMarginRadius * EnemyRunnerConstants::kEvasionMarginRadius) shouldEvade = true;
                }
            }

            if (shouldEvade)
            {
                m_evadeSwitchTimer += Game::GetTimeScale();
                if (m_evadeSwitchTimer > EnemyRunnerConstants::kEvasionSwitchTime)
                {
                    m_evadeSwitchTimer = 0.0f;
                    m_isEvadingRight = !m_isEvadingRight;
                }
                VECTOR right = VCross(dir, VGet(0.0f, 1.0f, 0.0f));
                moveVec = VAdd(moveVec, VScale(right, (m_isEvadingRight ? 1.0f : -1.0f) * (m_chaseSpeed * 0.5f) * Game::GetTimeScale() * m_aiUpdateInterval));
            }
            m_pos = VAdd(m_pos, moveVec);
        }
    }

    // AIステートの更新
    if (m_pCurrentState)
    {
        m_pCurrentState->Update(this, context);
    }

    // 攻撃判定
    if (m_currentAnimState == AnimState::Attack)
    {
        float currentAnimTotalTime = m_animationManager.GetAnimationTotalTime(m_modelHandle, EnemyRunnerConstants::kAttackAnimName);
        float attackStart = currentAnimTotalTime * EnemyRunnerConstants::kAttackHitStartRatio;
        float attackEnd   = currentAnimTotalTime * EnemyRunnerConstants::kAttackHitEndRatio;

        if (!m_hasAttackHit && m_animTime >= attackStart && m_animTime <= attackEnd)
        {
            if (CanAttackPlayer(player))
            {
                const_cast<Player&>(player).TakeDamage(m_attackPower, m_pos);
                m_hasAttackHit = true;
            }
        }
    }
}

void EnemyRunner::UpdateAnimation(const EnemyUpdateContext& context)
{
    // コライダーの更新
    VECTOR bodyCapA = VAdd(m_pos, VGet(0, EnemyRunnerConstants::kBodyColliderRadius, 0));
    VECTOR bodyCapB = VAdd(m_pos, VGet(0, EnemyRunnerConstants::kBodyColliderHeight - EnemyRunnerConstants::kBodyColliderRadius, 0));
    m_pBodyCollider->SetSegment(bodyCapA, bodyCapB);
    m_pBodyCollider->SetRadius(EnemyRunnerConstants::kBodyColliderRadius);

    int headIndex = MV1SearchFrame(m_modelHandle, "mixamorig:Head");
    VECTOR headModelPos = (headIndex != -1) ? MV1GetFramePosition(m_modelHandle, headIndex) : VGet(0, 0, 0);
    m_pHeadCollider->SetCenter(VAdd(headModelPos, m_headPosOffset));
    m_pHeadCollider->SetRadius(EnemyRunnerConstants::kHeadRadius);

    VECTOR attackRangeCenter = m_pos;
    attackRangeCenter.y += (EnemyRunnerConstants::kBodyColliderHeight * 0.5f);
    m_pAttackRangeCollider->SetCenter(attackRangeCenter);
    m_pAttackRangeCollider->SetRadius(EnemyRunnerConstants::kAttackRangeRadius);

    // アニメーション時間の更新
    if (m_animationManager.GetCurrentAttachedAnimHandle(m_modelHandle) != -1)
    {
        m_animTime += (1.0f * m_aiUpdateInterval) * Game::GetTimeScale();
        const char* animName = (m_currentAnimState == AnimState::Attack) ? EnemyRunnerConstants::kAttackAnimName :
                               ((m_currentAnimState == AnimState::Dead) ? EnemyRunnerConstants::kDeadAnimName : EnemyRunnerConstants::kRunAnimName);

        float totalTime = m_animationManager.GetAnimationTotalTime(m_modelHandle, animName);
        if (m_currentAnimState == AnimState::Run) { if (m_animTime >= totalTime) m_animTime = fmodf(m_animTime, totalTime); }
        else { if (m_animTime >= totalTime) m_animTime = totalTime; }
        
        m_animationManager.UpdateAnimationTime(m_modelHandle, m_animTime);
    }

    m_distToPlayer = VSize(VSub(context.player.GetPos(), m_pos));
}

void EnemyRunner::UpdateSimpleMode(const EnemyUpdateContext& context)
{
    if (m_hp > 0.0f)
    {
        VECTOR playerPos = context.player.GetPos();
        VECTOR targetPos = VAdd(playerPos, m_targetOffset);
        VECTOR toTarget = VSub(targetPos, m_pos);
        toTarget.y = 0.0f;
        float dist = VSize(toTarget);

        VECTOR dir = VNorm(toTarget);
        m_pos.x += dir.x * m_chaseSpeed * Game::GetTimeScale();
        m_pos.z += dir.z * m_chaseSpeed * Game::GetTimeScale();
        RotateTowards(targetPos, EnemyRunnerConstants::kRotateSpeedPerFrame * Game::GetTimeScale());
    }
    UpdateStageCollision(context.collisionData, context.collisionGrid);
}

void EnemyRunner::Draw()
{
    DrawStandard(EnemyRunnerConstants::kDrawDistanceSq, EnemyRunnerConstants::kDrawNearDistanceSq, EnemyRunnerConstants::kDrawDotThreshold);
}

void EnemyRunner::DrawCollisionDebug() const
{
    if (!s_shouldDrawCollision) return;

    // 体のコライダーデバッグ描画
    DebugUtil::DrawCapsule(m_pBodyCollider->GetSegmentA(), m_pBodyCollider->GetSegmentB(), m_pBodyCollider->GetRadius(), 16, 0xff0000);

    // 頭のコライダーデバッグ描画
    DebugUtil::DrawSphere(m_pHeadCollider->GetCenter(), m_pHeadCollider->GetRadius(), 16, 0x00ff00);

    // 攻撃範囲のデバッグ描画
    DebugUtil::DrawSphere(m_pAttackRangeCollider->GetCenter(), m_pAttackRangeCollider->GetRadius(), 24, 0xff8000);

    int handRIndex = MV1SearchFrame(m_modelHandle, "mixamorig:RightHand");
    int handLIndex = MV1SearchFrame(m_modelHandle, "mixamorig:LeftHand");

    if (handRIndex != -1 && handLIndex != -1)
    {
        VECTOR handRPos = MV1GetFramePosition(m_modelHandle, handRIndex);
        VECTOR handLPos = MV1GetFramePosition(m_modelHandle, handLIndex);

        // 攻撃ヒット用コライダーのデバッグ描画
        DebugUtil::DrawCapsule(handRPos, handLPos, EnemyRunnerConstants::kAttackHitRadius, 16, 0x0000ff);
    }
}

EnemyBase::HitPart EnemyRunner::CheckHitPart(const VECTOR& rayStart, const VECTOR& rayEnd, VECTOR& outHtPos, float& outHtDistSq) const
{
    if (m_isDeadAnimPlaying) return HitPart::None;

    VECTOR hitPosHead, hitPosBody;
    float hitDistSqHead = FLT_MAX;
    float hitDistSqBody = FLT_MAX;

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

void EnemyRunner::SetOnDropItemCallback(std::function<void(const VECTOR&)> cb)
{
    m_onDropItem = cb;
}
void EnemyRunner::TakeDamage(float damage, AttackType type)
{
    EnemyBase::TakeDamage(damage, type);

    // ダメージSEの再生（クールタイム中ならスキップ）
    if (m_damageSECooldown <= 0.0f && m_isAlive)
    {
        float volRatio = 1.0f - (m_distToPlayer / EnemyRunnerConstants::kVoiceMaxDist);
        if (volRatio < 0.0f) volRatio = 0.0f;
        if (volRatio > 1.0f) volRatio = 1.0f;

        SoundManager::GetInstance()->Play("EnemyRunner", "Damage", static_cast<int>(EnemyRunnerConstants::kDamageSEVolMax * volRatio));
        m_damageSECooldown = EnemyRunnerConstants::kDamageSECooldownMax;
    }
}

void EnemyRunner::OnDeath()
{
    if (m_lastHitPart == HitPart::None)
        m_lastHitPart = HitPart::Body;
    bool isHeadShot = (m_lastHitPart == HitPart::Head);
    int addScore = ScoreManager::Instance().AddScore(isHeadShot);
    if (SceneMain::Instance())
    {
        SceneMain::Instance()->AddScorePopup(addScore, isHeadShot, ScoreManager::Instance().GetCombo());
    }
}

void EnemyRunner::TakeTackleDamage(float damage)
{
    if (m_isDeadAnimPlaying) return;

    EnemyBase::TakeTackleDamage(damage);
}

std::shared_ptr<CapsuleCollider> EnemyRunner::GetBodyCollider() const
{
    return m_pBodyCollider;
}

void EnemyRunner::UpdateDeath(const EnemyUpdateContext& context)
{
    if (!m_isDeadAnimPlaying)
    {
        ChangeState(std::make_shared<EnemyRunnerStateDead>());
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

    float currentAnimTotalTime = m_animationManager.GetAnimationTotalTime(m_modelHandle, EnemyRunnerConstants::kDeadAnimName);
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
