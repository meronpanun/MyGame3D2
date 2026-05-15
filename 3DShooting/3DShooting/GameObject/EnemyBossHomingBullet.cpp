#include "EnemyBoss.h"
#include "DxLib.h"
#include "EffekseerWarningSuppress.h"
#include "Game.h"
#include "Player.h"
#include "SphereCollider.h"
#include "CapsuleCollider.h"
#include "TaskTutorialManager.h"

void EnemyBoss::UpdateHomingBullets(const EnemyUpdateContext& context)
{
    const Player& player = context.player;

    for (auto& bullet : m_homingBullets)
    {
        if (!bullet.active) continue;

        // --- 弾の移動 ---
        float scale = Game::GetTimeScale();
        if (bullet.isParabolic)
        {
            // 放物線運動（重力あり）
            bullet.velocity.y -= bullet.gravity * scale;
            bullet.pos = VAdd(bullet.pos, VScale(bullet.velocity, scale));
            if (VSquareSize(bullet.velocity) > 0.0001f)
            {
                bullet.dir = VNorm(bullet.velocity);
            }
        }
        else
        {
            // ホーミング運動
            VECTOR toPlayer = VSub(player.GetPos(), bullet.pos);
            VECTOR targetDir = VNorm(toPlayer);
            bullet.dir = VAdd(bullet.dir, VScale(targetDir, EnemyBossConstants::kHomingTurnRate * scale));
            bullet.dir = VNorm(bullet.dir);
            bullet.pos = VAdd(bullet.pos, VScale(bullet.dir, bullet.speed * scale));
            bullet.distTraveled += bullet.speed * scale;
        }

        // エフェクト位置更新
        if (bullet.effectHandle != -1)
        {
            SetPosPlayingEffekseer3DEffect(bullet.effectHandle, bullet.pos.x, bullet.pos.y, bullet.pos.z);
        }

        // --- ヒット判定 ---
        if (!bullet.isReflected)
        {
            SphereCollider bulletCol(bullet.pos, EnemyBossConstants::kHomingBulletRadius);
            bool hitDetected = false;

            // パリィ判定
            if (bullet.isParryable && player.IsJustGuarded())
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
                if (bulletCol.IsIntersects(&parryCollider))
                {
                    hitDetected = true;
                    bullet.isReflected = true;
                    const_cast<Player&>(player).PlayParrySE();
                    TaskTutorialManager::GetInstance()->NotifyParrySuccess();

                    // 弾を自分（ボス）に向けて反射
                    if (bullet.owner)
                    {
                        VECTOR targetPos = bullet.owner->GetPos();
                        targetPos.y += EnemyBossConstants::kBodyColliderHeight * 0.5f;
                        bullet.dir = VNorm(VSub(targetPos, bullet.pos));
                    }
                    else
                    {
                        bullet.dir = VScale(bullet.dir, -1.0f);
                    }
                    bullet.speed *= 1.5f;
                    bullet.turnRate = 0.0f;
                    Game::SetTimeScale(0.1f, 1.0f);
                }
            }

            // プレイヤーへのヒット判定
            if (!hitDetected)
            {
                std::shared_ptr<CapsuleCollider> pCol = player.GetBodyCollider();
                if (bulletCol.IsIntersects(pCol.get()))
                {
                    hitDetected = true;
                    const_cast<Player&>(player).TakeDamage(bullet.damage, m_pos, bullet.isParryable);
                    bullet.active = false;
                    if (bullet.effectHandle != -1) { StopEffekseer3DEffect(bullet.effectHandle); bullet.effectHandle = -1; }
                }
            }
        }
        else
        {
            // 反射弾がボス自身に当たった場合
            SphereCollider bulletCol(bullet.pos, EnemyBossConstants::kHomingBulletRadius);
            if (m_pBodyCollider && m_pBodyCollider->IsIntersects(&bulletCol))
            {
                this->TakeDamage(bullet.damage * 2.0f, AttackType::Parry); // 反射弾はダメージ大きめ
                OnParried();
                bullet.active = false;
                if (bullet.effectHandle != -1) { StopEffekseer3DEffect(bullet.effectHandle); bullet.effectHandle = -1; }
            }
        }

        // --- 廃棄判定（地面 or 最大飛距離）---
        if (bullet.pos.y < 0.0f || bullet.distTraveled > EnemyBossConstants::kHomingBulletMaxDist)
        {
            bullet.active = false;
            if (bullet.effectHandle != -1) { StopEffekseer3DEffect(bullet.effectHandle); bullet.effectHandle = -1; }
        }
    }

    // 非アクティブな弾を削除
    m_homingBullets.erase(
        std::remove_if(m_homingBullets.begin(), m_homingBullets.end(),
                       [](const HomingBullet& b) { return !b.active; }),
        m_homingBullets.end());
}
