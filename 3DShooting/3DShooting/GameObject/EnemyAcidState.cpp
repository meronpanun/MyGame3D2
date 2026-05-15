#include "EnemyAcidState.h"
#include "EnemyAcid.h"
#include "Player.h"
#include "Game.h"
#include "DxLib.h"
#include "SphereCollider.h"
#include "CapsuleCollider.h"
#include "CollisionGrid.h"
#include "TaskTutorialManager.h"
#include "EffekseerWarningSuppress.h"
#include <memory>

void EnemyAcidStateWalk::Enter(EnemyAcid* enemy)
{
    enemy->ChangeAnimation(EnemyBase::AnimState::Walk, true);
}

void EnemyAcidStateWalk::Update(EnemyAcid* enemy, const EnemyUpdateContext& context)
{
    // 移動AIは UpdateMovementAI に委譲されているため、ここでは何もしない
}

void EnemyAcidStateBack::Enter(EnemyAcid* enemy)
{
    enemy->ChangeAnimation(EnemyBase::AnimState::Back, true);
}

void EnemyAcidStateBack::Update(EnemyAcid* enemy, const EnemyUpdateContext& context)
{
    // 移動AIは UpdateMovementAI に委譲
}

void EnemyAcidStateAttack::Enter(EnemyAcid* enemy)
{
    enemy->m_hasAttacked = false;
    enemy->m_attackCooldown = 160; // EnemyAcidConstants::kAttackCooldownMax;
    enemy->ChangeAnimation(EnemyBase::AnimState::Attack, false);
}

void EnemyAcidStateAttack::Update(EnemyAcid* enemy, const EnemyUpdateContext& context)
{
    // 攻撃ロジックは UpdateMovementAI に委譲
}

void EnemyAcidStateStunned::Enter(EnemyAcid* enemy)
{
    enemy->m_isStunned = true;
    enemy->m_stunTimer = 120; // EnemyAcidConstants::kStunDuration
    enemy->ChangeAnimation(EnemyBase::AnimState::Dead, false); // 死亡アニメーションを流用
}

void EnemyAcidStateStunned::Update(EnemyAcid* enemy, const EnemyUpdateContext& context)
{
    // 怯み処理はUpdateAIで優先的に処理される
}

void EnemyAcidStateDead::Enter(EnemyAcid* enemy)
{
    enemy->ChangeAnimation(EnemyBase::AnimState::Dead, false);
    enemy->m_isDeadAnimPlaying = true;
    enemy->m_animTime = 0.0f;
}

void EnemyAcidStateDead::Update(EnemyAcid* enemy, const EnemyUpdateContext& context)
{
    // 死亡時は何もしない
}

// UpdateAcidBalls - EnemyAcid.cpp の UpdateAI から分離
void EnemyAcid::UpdateAcidBalls(const EnemyUpdateContext& context)
{
    const Player& player = context.player;

    for (auto& ball : m_acidBalls)
    {
        if (!ball.active) continue;

        ball.Update(Game::GetTimeScale());

        if (ball.effectHandle != -1)
        {
            SetPosPlayingEffekseer3DEffect(ball.effectHandle, ball.pos.x, ball.pos.y, ball.pos.z);
        }

        VECTOR toPlayer = VSub(ball.pos, player.GetPos());
        float distanceToPlayer = VSize(toPlayer);

        // 距離が遠すぎたら非アクティブ化
        if (distanceToPlayer > EnemyAcidConstants::kAcidBallBoundaryDistance)
        {
            ball.active = false;
            if (ball.effectHandle != -1) { StopEffekseer3DEffect(ball.effectHandle); ball.effectHandle = -1; }
            continue;
        }

        // チュートリアル通知（パリィ可能弾が近い場合）
        if (ball.isParryable && !ball.isReflected && distanceToPlayer <= 100.0f)
        {
            TaskTutorialManager::GetInstance()->NotifyParryableAttack();
        }

        if (!ball.isReflected)
        {
            SphereCollider acidCol(ball.pos, ball.radius);
            bool hitDetected = false;

            // パリィ判定
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
                    const_cast<Player&>(player).PlayParrySE();
                    TaskTutorialManager::GetInstance()->NotifyParrySuccess();
                    const auto& playerCam = player.GetCamera();
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

            // プレイヤーへのヒット判定
            if (!hitDetected)
            {
                std::shared_ptr<CapsuleCollider> playerCol = player.GetBodyCollider();
                if (acidCol.IsIntersects(playerCol.get()))
                {
                    hitDetected = true;
                    const_cast<Player&>(player).TakeDamage(ball.damage, m_pos, ball.isParryable);
                    ball.active = false;
                    if (ball.effectHandle != -1) { StopEffekseer3DEffect(ball.effectHandle); ball.effectHandle = -1; }
                }
            }
        }
        else
        {
            // 反射弾の自分へのヒット判定
            SphereCollider reflectedAcidCol(ball.pos, ball.radius);
            if (reflectedAcidCol.IsIntersects(GetBodyCollider().get()))
            {
                TakeDamage(ball.damage, AttackType::Parry);
                OnParried();
                ball.active = false;
                if (ball.effectHandle != -1) { StopEffekseer3DEffect(ball.effectHandle); ball.effectHandle = -1; }
            }
        }

        // 地面に落ちたら消滅
        if (ball.pos.y < 0.0f)
        {
            ball.active = false;
            if (ball.effectHandle != -1) { StopEffekseer3DEffect(ball.effectHandle); ball.effectHandle = -1; }
        }
    }

    // 非アクティブな弾を削除
    m_acidBalls.erase(
        std::remove_if(m_acidBalls.begin(), m_acidBalls.end(), [](const AcidBall& b) { return !b.active; }),
        m_acidBalls.end());
}

// UpdateMovementAI - EnemyAcid.cpp の UpdateAI から分離
void EnemyAcid::UpdateMovementAI(const EnemyUpdateContext& context)
{
    const Player& player = context.player;
    Effect* pEffect = context.pEffect;

    VECTOR playerPos = player.GetPos();

    // プレイヤーの方向を向く
    RotateTowards(playerPos, 0.05f * Game::GetTimeScale() * m_aiUpdateInterval);
    float disToPlayer = VSize(VSub(playerPos, m_pos));

    // 攻撃範囲判定
    std::shared_ptr<CapsuleCollider> playerBodyCollider = player.GetBodyCollider();
    bool inAttackRange = m_pAttackRangeCollider->IsIntersects(playerBodyCollider.get());

    // 攻撃中・硬直中は移動判断をスキップ
    if (m_currentAnimState != AnimState::Attack && m_attackEndDelayTimer == 0)
    {
        if (inAttackRange)
        {
            if (disToPlayer < EnemyAcidConstants::kOptimalAttackDistanceMin)
            {
                // 近すぎたら後退
                if (m_currentAnimState != AnimState::Back) ChangeState(std::make_shared<EnemyAcidStateBack>());
                VECTOR dirAway = VNorm(VSub(m_pos, playerPos));
                float scaledSpeed = m_chaseSpeed * Game::GetTimeScale() * m_aiUpdateInterval;
                m_pos.x += dirAway.x * scaledSpeed;
                m_pos.z += dirAway.z * scaledSpeed;
            }
            else
            {
                // 適切な距離なら攻撃開始
                if (m_attackCooldown == 0) ChangeState(std::make_shared<EnemyAcidStateAttack>());
            }
        }
        else
        {
            // 射程外なら接近
            if (m_currentAnimState != AnimState::Walk) ChangeState(std::make_shared<EnemyAcidStateWalk>());
            VECTOR targetPos = VAdd(playerPos, m_targetOffset);
            VECTOR dirTowards = VNorm(VSub(targetPos, m_pos));
            float scaledSpeed = m_chaseSpeed * Game::GetTimeScale() * m_aiUpdateInterval;
            m_pos.x += dirTowards.x * scaledSpeed;
            m_pos.z += dirTowards.z * scaledSpeed;
        }
    }

    // 攻撃アニメーション中の弾発射判定
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

    // クールダウンカウントダウン
    if (m_attackCooldown > 0) m_attackCooldown -= m_aiUpdateInterval;
    if (m_attackCooldown < 0) m_attackCooldown = 0;

    // 硬直タイマーカウントダウン
    if (m_attackEndDelayTimer > 0)
    {
        m_attackEndDelayTimer -= m_aiUpdateInterval;
        if (m_attackEndDelayTimer <= 0)
        {
            m_attackEndDelayTimer = 0;
            if (m_currentAnimState != AnimState::Walk) ChangeState(std::make_shared<EnemyAcidStateWalk>());
        }
    }
}
