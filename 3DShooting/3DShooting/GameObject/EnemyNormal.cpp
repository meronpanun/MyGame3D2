#include "EnemyNormal.h"
#include "Bullet.h"
#include "CapsuleCollider.h"
#include "CollisionGrid.h"
#include "DebugUtil.h"
#include "DxLib.h"
#include "EffekseerWarningSuppress.h"
#include "Game.h"
#include "Player.h"
#include "SceneMain.h"
#include "ScoreManager.h"
#include "SphereCollider.h"
#include "TransformDataLoader.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>
#include <string>
#include <vector>
#include "SoundManager.h"
#include "EnemyState.h"
#include "EnemyNormalState.h"

int EnemyNormal::s_modelHandle = -1;
bool EnemyNormal::s_shouldDrawCollision = false;
bool EnemyNormal::s_shouldDrawShieldCollision = false;

EnemyNormal::EnemyNormal()
    : m_headPosOffset(EnemyNormalConstants::kHeadShotPositionOffset)
    , m_hasTakenTackleDamage(false)
    , m_damageTimer(0)
    , m_animTime(0.0f)
    , m_hasAttackHit(false)
    , m_onDropItem(nullptr)
    , m_currentAnimState(AnimState::Walk)
    , m_attackEndDelayTimer(0)
    , m_isDeadAnimPlaying(false)
    , m_hasDroppedItem(false)
    , m_wanderTimer(0)
    , m_wanderOffset(VGet(0.0f, 0.0f, 0.0f))
    , m_isBlownAway(false)
    , m_deathKnockbackDir(VGet(0.0f, 0.0f, 0.0f))
    , m_deathKnockbackSpeed(0.0f)
    , m_hasShieldConfigured(false)
    , m_isShieldBroken(false)
    , m_shieldHp(0.0f)
    , m_maxShieldHp(0.0f)
    , m_shieldRotation(0.0f)
    , m_shieldEffectTimer(0.0f)
    , m_hasPlayedShieldBreakableEffect(false)
    , m_shieldChainBreakTimer(0)
    , m_voiceTimer(EnemyNormalConstants::kVoiceTimerMin + GetRand(EnemyNormalConstants::kVoiceTimerRand))
    , m_distToPlayer(0.0f)
{
    m_modelHandle = MV1DuplicateModel(s_modelHandle);

    m_pBodyCollider = std::make_shared<CapsuleCollider>();
    m_pHeadCollider = std::make_shared<SphereCollider>();
    m_pAttackRangeCollider = std::make_shared<SphereCollider>();
    m_pAttackHitCollider = std::make_shared<CapsuleCollider>();
}

EnemyNormal::~EnemyNormal()
{
    MV1DeleteModel(m_modelHandle);
}

void EnemyNormal::LoadModel()
{
    s_modelHandle = MV1LoadModel("data/model/NormalZombie.mv1");
    assert(s_modelHandle != -1);
}

void EnemyNormal::DeleteModel()
{
    MV1DeleteModel(s_modelHandle);
}

void EnemyNormal::Init()
{
    m_attackCooldownMax = EnemyNormalConstants::kAttackCooldownMax;

    m_isAlive = true;
    m_isDeadAnimPlaying = false;
    m_hasDroppedItem = false;
    m_voiceTimer = EnemyNormalConstants::kVoiceTimerMin + GetRand(EnemyNormalConstants::kVoiceTimerRand);
    m_lastHitPart = HitPart::None;
    m_hitDisplayTimer = 0;
    m_damageTimer = 0;
    m_isBlownAway = false;
    m_deathKnockbackSpeed = 0.0f;

    LoadTransformData("NormalEnemy");

    // ChangeAnimation で初期アニメーションを強制再生させるため、現在状態を非Walk値でリセット
    m_currentAnimState = AnimState::Dead;

    ChangeState(std::make_shared<EnemyNormalStateWalk>());

    // 追跡目標にランダムオフセットを付与して各敵が同一点に集中しないようにする
    float offsetX = static_cast<float>(GetRand(EnemyNormalConstants::kTargetOffsetRange * 2) - EnemyNormalConstants::kTargetOffsetRange);
    float offsetZ = static_cast<float>(GetRand(EnemyNormalConstants::kTargetOffsetRange * 2) - EnemyNormalConstants::kTargetOffsetRange);
    m_targetOffset = VGet(offsetX, 0.0f, offsetZ);

    // 徘徊用パラメータ初期化
    m_wanderTimer = 0;
    m_wanderOffset = VGet(0.0f, 0.0f, 0.0f);

    // シールド関連の初期化フラグ
    m_hasShieldConfigured = false;
    m_isShieldBroken = false;
    m_shieldHp = 0.0f;
    m_maxShieldHp = 0.0f;
    m_shieldRotation = 0.0f;
    m_shieldEffectTimer = 0.0f;
    m_pShieldCollider = nullptr;
    m_shieldEffectHandles.clear();
    m_shieldChainBreakTimer = 0;
}

void EnemyNormal::ChangeAnimation(AnimState newAnimState, bool loop)
{
    if (m_currentAnimState == newAnimState)
    {
        if (newAnimState == AnimState::Attack)
        {
            // 同じ攻撃アニメーションをリセットして再開
            m_animationManager.PlayAnimation(m_modelHandle, EnemyNormalConstants::kAttackAnimName, loop);
        }
        else return;
    }

    const char* animName = nullptr;

    switch (newAnimState)
    {
    case AnimState::Walk:
        animName = EnemyNormalConstants::kWalkAnimName;
        break;
    case AnimState::Attack:
        animName = EnemyNormalConstants::kAttackAnimName;
        break;
    case AnimState::Damage: // 怯み時は歩行モーションを流用
        animName = EnemyNormalConstants::kWalkAnimName;
        break;
    case AnimState::Dead:
        animName = EnemyNormalConstants::kDeadAnimName;
        break;
    }

    if (animName)
    {
        m_animationManager.PlayAnimation(m_modelHandle, animName, loop);
        m_animTime = 0.0f;
    }

    m_currentAnimState = newAnimState;
}

void EnemyNormal::ChangeState(std::shared_ptr<EnemyState<EnemyNormal>> newState)
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

bool EnemyNormal::CanAttackPlayer(const Player& player)
{
    int handRIndex = MV1SearchFrame(m_modelHandle, "Hand_R");
    int handLIndex = MV1SearchFrame(m_modelHandle, "Hand_L");
    if (handRIndex == -1 || handLIndex == -1) return false;

    VECTOR handRPos = MV1GetFramePosition(m_modelHandle, handRIndex);
    VECTOR handLPos = MV1GetFramePosition(m_modelHandle, handLIndex);

    m_pAttackHitCollider->SetSegment(handRPos, handLPos);
    m_pAttackHitCollider->SetRadius(EnemyNormalConstants::kAttackHitRadius);

    std::shared_ptr<CapsuleCollider> playerBodyCollider = player.GetBodyCollider();
    return m_pAttackHitCollider->IsIntersects(playerBodyCollider.get());
}

void EnemyNormal::Update(const EnemyUpdateContext& context)
{
    UpdateStandard(context);
}

void EnemyNormal::UpdateAI(const EnemyUpdateContext& context)
{
    // シールドの更新
    UpdateShield(context);

    // 環境ボイスの更新
    if (m_isAlive && !m_isDeadAnimPlaying)
    {
        m_voiceTimer--;
        if (m_voiceTimer <= 0)
        {
            int randomIndex = 1 + GetRand(3);
            SoundManager::GetInstance()->PlayAtDistance("EnemyNormal", "Voice" + std::to_string(randomIndex), m_distToPlayer, EnemyNormalConstants::kVoiceMaxDist);
            m_voiceTimer = EnemyNormalConstants::kVoiceTimerMin + GetRand(EnemyNormalConstants::kVoiceTimerRand);
        }
    }

    // AIステートの更新
    if (m_pCurrentState)
    {
        m_pCurrentState->Update(this, context);
    }
}

void EnemyNormal::UpdateAnimation(const EnemyUpdateContext& context)
{
    // コライダーの更新
    VECTOR bodyCapA = VAdd(m_pos, VGet(0, EnemyNormalConstants::kBodyColliderRadius, 0));
    VECTOR bodyCapB = VAdd(m_pos, VGet(0, EnemyNormalConstants::kBodyColliderHeight - EnemyNormalConstants::kBodyColliderRadius, 0));
    m_pBodyCollider->SetSegment(bodyCapA, bodyCapB);
    m_pBodyCollider->SetRadius(EnemyNormalConstants::kBodyColliderRadius);

    int headIndex = MV1SearchFrame(m_modelHandle, "Head");
    VECTOR headModelPos = (headIndex != -1) ? MV1GetFramePosition(m_modelHandle, headIndex) : VGet(0, 0, 0);
    m_pHeadCollider->SetCenter(VAdd(headModelPos, m_headPosOffset));
    m_pHeadCollider->SetRadius(EnemyNormalConstants::kHeadRadius);

    VECTOR attackRangeCenter = m_pos;
    attackRangeCenter.y += (EnemyNormalConstants::kBodyColliderHeight * 0.5f);
    m_pAttackRangeCollider->SetCenter(attackRangeCenter);
    m_pAttackRangeCollider->SetRadius(EnemyNormalConstants::kAttackRangeRadius);

    // アニメーション時間の更新
    if (m_animationManager.GetCurrentAttachedAnimHandle(m_modelHandle) != -1)
    {
        m_animTime += (1.0f * m_aiUpdateInterval) * Game::GetTimeScale();
        const char* animName = (m_currentAnimState == AnimState::Attack) ? EnemyNormalConstants::kAttackAnimName :
                               ((m_currentAnimState == AnimState::Dead) ? EnemyNormalConstants::kDeadAnimName : EnemyNormalConstants::kWalkAnimName);

        float totalTime = m_animationManager.GetAnimationTotalTime(m_modelHandle, animName);
        if (m_currentAnimState == AnimState::Dead) { if (m_animTime >= totalTime) m_animTime = totalTime; }
        else if (m_currentAnimState != AnimState::Attack) { if (m_animTime >= totalTime) m_animTime = fmodf(m_animTime, totalTime); }
        
        m_animationManager.UpdateAnimationTime(m_modelHandle, m_animTime);
    }

    m_distToPlayer = VSize(VSub(context.player.GetPos(), m_pos));
}

void EnemyNormal::UpdateSimpleMode(const EnemyUpdateContext& context)
{
    if (m_hp > 0.0f)
    {
        VECTOR playerPos = context.player.GetPos();
        VECTOR targetPos = VAdd(playerPos, m_targetOffset);
        VECTOR toTarget = VSub(targetPos, m_pos);
        toTarget.y = 0.0f;
        float dist = VSize(toTarget);

        if (dist > EnemyNormalConstants::kChaseStopDistance)
        {
            VECTOR dir = VNorm(toTarget);
            float step = (std::min)(dist - EnemyNormalConstants::kChaseStopDistance, m_chaseSpeed * Game::GetTimeScale());
            m_pos.x += dir.x * step;
            m_pos.z += dir.z * step;
            RotateTowards(targetPos, EnemyNormalConstants::kRotateSpeedPerFrame * Game::GetTimeScale());
        }
    }
    UpdateStageCollision(context.collisionData, context.collisionGrid);
}

void EnemyNormal::Draw()
{
    DrawStandard(EnemyNormalConstants::kDrawDistanceSq, EnemyNormalConstants::kDrawNearDistanceSq, EnemyNormalConstants::kDrawDotThreshold);
}

void EnemyNormal::DrawCollisionDebug() const
{
    if (s_shouldDrawShieldCollision && m_hasShieldConfigured && !m_isShieldBroken && m_pShieldCollider)
    {
        DebugUtil::DrawSphere(m_pShieldCollider->GetCenter(), m_pShieldCollider->GetRadius(), 16, 0x00ffff);
    }

    if (!s_shouldDrawCollision) return;

    // 体のコライダーデバッグ描画
    DebugUtil::DrawCapsule(m_pBodyCollider->GetSegmentA(), m_pBodyCollider->GetSegmentB(), m_pBodyCollider->GetRadius(), 16, 0xff0000);

    // 頭のコライダーデバッグ描画
    DebugUtil::DrawSphere(m_pHeadCollider->GetCenter(), m_pHeadCollider->GetRadius(), 16, 0x00ff00);

    // 攻撃範囲のデバッグ描画
    DebugUtil::DrawSphere(m_pAttackRangeCollider->GetCenter(), m_pAttackRangeCollider->GetRadius(), 24, 0xff8000);

    int handRIndex = MV1SearchFrame(m_modelHandle, "Hand_R");
    int handLIndex = MV1SearchFrame(m_modelHandle, "Hand_L");

    if (handRIndex != -1 && handLIndex != -1)
    {
        VECTOR handRPos = MV1GetFramePosition(m_modelHandle, handRIndex);
        VECTOR handLPos = MV1GetFramePosition(m_modelHandle, handLIndex);

        // 攻撃ヒット用コライダーのデバッグ描画
        DebugUtil::DrawCapsule(handRPos, handLPos, EnemyNormalConstants::kAttackHitRadius, 16, 0x0000ff);
    }
}

EnemyBase::HitPart EnemyNormal::CheckHitPart(const VECTOR& rayStart, const VECTOR& rayEnd, VECTOR& outHtPos, float& outHtDistSq) const
{
    if (m_isDeadAnimPlaying) return HitPart::None;

    // シールドヒット判定
    if (m_hasShieldConfigured && !m_isShieldBroken && m_pShieldCollider)
    {
        VECTOR hitPosShield;
        float hitDistSqShield;
        if (m_pShieldCollider->IsIsIntersectsRay(rayStart, rayEnd, hitPosShield, hitDistSqShield))
        {
            outHtPos = hitPosShield;
            outHtDistSq = hitDistSqShield;
            return HitPart::Shield;
        }
    }

    // ヘッドとボディのコライダーをそれぞれチェック
    VECTOR hitPosHead, hitPosBody;
    float hitDistSqHead = FLT_MAX;
    float hitDistSqBody = FLT_MAX;

    // ヘッドとボディのコライダーに対してRayをチェック
    bool headHit = m_pHeadCollider->IsIsIntersectsRay(rayStart, rayEnd, hitPosHead, hitDistSqHead);
    bool bodyHit = m_pBodyCollider->IsIsIntersectsRay(rayStart, rayEnd, hitPosBody, hitDistSqBody);

    // ヒットした部位を判定
    if (headHit && bodyHit)
    {
        // 両方にヒットした場合、より近い方を優先
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

    outHtPos = VGet(0, 0, 0); // ヒットしない場合は適当な値を入れておく
    outHtDistSq = FLT_MAX;
    return HitPart::None;
}

void EnemyNormal::SetOnDropItemCallback(std::function<void(const VECTOR&)> cb)
{
    m_onDropItem = cb;
}

void EnemyNormal::TakeDamage(float damage, AttackType type)
{
    if (m_isDeadAnimPlaying) return;

    // シールド処理
    if (m_hasShieldConfigured && !m_isShieldBroken)
    {
        // シールドHPが0以下の状態で盾投げを食らったら破壊
        if (m_shieldHp <= 0.0f && type == AttackType::ShieldThrow)
        {
            BreakShield(); 
            TriggerShieldChainBreak(1); // 次のUpdateで連鎖を開始させる
        }
        else if (type != AttackType::ShieldThrow) // 盾投げ以外はシールドHPを減らす
        {
            m_shieldHp -= damage;
            if (m_shieldHp < 0.0f)
            {
                m_shieldHp = 0.0f;
            }

            // シールドHPが0になった瞬間に、破壊可能エフェクトを再生
            if (m_shieldHp <= 0.0f && !m_hasPlayedShieldBreakableEffect)
            {
                if (m_pShieldCollider)
                {
                    // シールドの位置で再生
                    SceneMain::Instance()->GetEffect()->PlayShieldBreakEffect(m_pShieldCollider->GetCenter());
                }
                m_hasPlayedShieldBreakableEffect = true;
            }
        }
        
        // シールドがある間は本体にダメージを与えない
        return;
    }

    EnemyBase::TakeDamage(damage, type);

    // 怯み処理
    if (m_hp > 0.0f)
    {
        if (type == AttackType::Shotgun)
        {
            // ショットガンは上書きしてでも大きく怯む
            ChangeState(std::make_shared<EnemyNormalStateDamage>());
            m_damageTimer = EnemyNormalConstants::kDamageDuration;
        }
        else if (type == AttackType::Shoot)
        {
            // アサルトライフルは短い怯み（連射によるスタンロック防止のため、既に怯み中なら上書きしない）
            if (m_currentAnimState != AnimState::Damage)
            {
                ChangeState(std::make_shared<EnemyNormalStateDamage>());
                m_damageTimer = EnemyNormalConstants::kShortDamageDuration; // 連射によるスタンロック防止のため短い怯み時間
            }
        }
    }
}

void EnemyNormal::OnDeath()
{
    if (m_lastAttackType == AttackType::Shotgun)
    {
        m_isBlownAway = true;
        // プレイヤーと逆方向に吹き飛ぶ
        if (SceneMain::Instance())
        {
            VECTOR playerPos = SceneMain::Instance()->GetPlayer().GetPos();
            VECTOR toEnemy = VSub(m_pos, playerPos);
            toEnemy.y = 0.0f; // 水平方向のみ
            if (VSquareSize(toEnemy) > EnemyNormalConstants::kPushBackEpsilon)
            {
                m_deathKnockbackDir = VNorm(toEnemy);
                m_deathKnockbackSpeed = EnemyNormalConstants::kKnockbackInitialSpeed;
            }
        }

        // フォールバック: もし速度が設定されなかった場合（プレイヤー位置取得失敗 or 重なり）
        if (m_deathKnockbackSpeed <= 0.0f)
        {
            // 敵の向いている方向の逆（後ろ）へ飛ばす
            MATRIX worldMat = MV1GetLocalWorldMatrix(m_modelHandle);
            VECTOR forward = VGet(worldMat.m[2][0], worldMat.m[2][1], worldMat.m[2][2]);
            forward.y = 0.0f;
            if (VSquareSize(forward) > EnemyNormalConstants::kPushBackEpsilon)
            {
                m_deathKnockbackDir = VScale(VNorm(forward), -1.0f);
            }
            else
            {
                m_deathKnockbackDir = VGet(0, 0, 1); // 完全なフォールバック
            }
            m_deathKnockbackSpeed = EnemyNormalConstants::kKnockbackInitialSpeed;
        }
    }

    if (m_lastHitPart == HitPart::None)
        m_lastHitPart = HitPart::Body;
    bool isHeadShot = (m_lastHitPart == HitPart::Head);
    int addScore = ScoreManager::Instance().AddScore(isHeadShot);
    if (SceneMain::Instance())
    {
        SceneMain::Instance()->AddScorePopup(addScore, isHeadShot, ScoreManager::Instance().GetCombo());
    }
}

void EnemyNormal::TakeTackleDamage(float damage)
{
    if (m_isDeadAnimPlaying) return;

    EnemyBase::TakeTackleDamage(damage);
}

std::shared_ptr<CapsuleCollider> EnemyNormal::GetBodyCollider() const
{
    return m_pBodyCollider;
}

void EnemyNormal::CheckHitAndDamage(std::vector<Bullet>& bullets, Effect* pEffect)
{
    for (auto& bullet : bullets)
    {
        if (!bullet.IsActive()) continue;

        VECTOR rayStart = bullet.GetPrevPos();
        VECTOR rayEnd = bullet.GetPos();

        // 共通の距離チェック（ブロードフェーズ）
        VECTOR enemyCenter = VAdd(m_pos, VGet(0, EnemyNormalConstants::kBodyColliderHeight * 0.5f, 0));
        float d1 = VSquareSize(VSub(rayStart, enemyCenter));
        float d2 = VSquareSize(VSub(rayEnd, enemyCenter));
        if (d1 > EnemyNormalConstants::kBroadPhaseDistSq && d2 > EnemyNormalConstants::kBroadPhaseDistSq) continue;

        // ① シールド判定 (シールドがある場合)
        if (m_hasShieldConfigured && !m_isShieldBroken && m_pShieldCollider)
        {
            VECTOR hitPosShield;
            float hitDistSqShield;
            if (m_pShieldCollider->IsIsIntersectsRay(rayStart, rayEnd, hitPosShield, hitDistSqShield))
            {
                ApplyBulletDamage(bullet, HitPart::Shield, hitDistSqShield, pEffect);
                continue; // シールドに当たったら本体判定は行わない
            }
        }

        // ② 本体判定
        VECTOR hitPos;
        float hitDistSq;
        HitPart part = CheckHitPart(rayStart, rayEnd, hitPos, hitDistSq);
        if (part != HitPart::None)
        {
            ApplyBulletDamage(bullet, part, hitDistSq, pEffect);
        }
    }
}

float EnemyNormal::CalcDamage(float bulletDamage, HitPart part) const
{
    if (m_isDeadAnimPlaying) return 0.0f;

    // シールドヒット時
    if (part == HitPart::Shield && m_hasShieldConfigured && !m_isShieldBroken)
    {
        return bulletDamage; // 等倍ダメージとする
    }

    // ヘッドショット時
    if (part == HitPart::Head)
    {
        return bulletDamage * EnemyNormalConstants::kHeadshotMultiplier;
    }

    // それ以外（Bodyなど）
    return bulletDamage * 1.0f;
}

void EnemyNormal::ApplyBulletDamage(Bullet& bullet, HitPart part, float distSq, Effect* pEffect)
{
    // シールドヒット時
    if (part == HitPart::Shield && m_hasShieldConfigured && !m_isShieldBroken)
    {
        // シールド専用エフェクト (HitBurst)
        if (pEffect)
        {
            VECTOR hitPos = bullet.GetPos();
            VECTOR normal = VGet(0.0f, 0.0f, -1.0f); // デフォルト

            if (m_pShieldCollider)
            {
                VECTOR center = m_pShieldCollider->GetCenter();
                VECTOR diff = VSub(hitPos, center);
                if (VSquareSize(diff) > EnemyNormalConstants::kPushBackEpsilon)
                {
                    normal = VNorm(diff);
                    // 弾の現在位置(内側にめり込んでいる可能性がある)ではなく、シールドの表面でエフェクトを発生させる
                    hitPos = VAdd(center, VScale(normal, m_pShieldCollider->GetRadius()));
                }
            }

            pEffect->PlayShieldHitEffect(hitPos, normal);
        }

        // ダメージ計算と適用 (シールドHP減少)
        float damage = CalcDamage(bullet.GetDamage(), part);
        TakeDamage(damage, bullet.GetAttackType()); // TakeDamage側で減算処理を行う

        // デバッグ表示用
        if (s_shouldShowDamage)
        {
            s_debugLastDamage = damage;
            s_debugDamageTimer = EnemyConstants::kDebugDamageDisplayTimer;
            s_debugHitInfo = "(Shield)";
        }

        // 弾を非アクティブ化する
        bullet.Deactivate();
    }
    else
    {
        // それ以外は基底クラス（出血エフェクトあり）
        EnemyBase::ApplyBulletDamage(bullet, part, distSq, pEffect);
    }
}

void EnemyNormal::UpdateDeath(const EnemyUpdateContext& context)
{
    if (!m_isDeadAnimPlaying)
    {
        // 死亡時にシールドエフェクトが残存している場合は破棄する
        if (!m_shieldEffectHandles.empty())
        {
            for (int handle : m_shieldEffectHandles)
            {
                StopEffekseer3DEffect(handle);
            }
            m_shieldEffectHandles.clear();
            m_pShieldCollider = nullptr;
        }

        ChangeState(std::make_shared<EnemyNormalStateDead>());
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

    // 死亡吹き飛び処理
    if (m_isBlownAway && m_deathKnockbackSpeed > 0.0f)
    {
        m_pos = VAdd(m_pos, VScale(m_deathKnockbackDir, m_deathKnockbackSpeed * Game::GetTimeScale()));
        m_deathKnockbackSpeed -= EnemyNormalConstants::kKnockbackDeceleration * Game::GetTimeScale();
        if (m_deathKnockbackSpeed < 0.0f) m_deathKnockbackSpeed = 0.0f;

        UpdateStageCollision(context.collisionData, context.collisionGrid);
    }

    MV1SetPosition(m_modelHandle, m_pos);

    float currentAnimTotalTime = m_animationManager.GetAnimationTotalTime(m_modelHandle, EnemyNormalConstants::kDeadAnimName);
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
