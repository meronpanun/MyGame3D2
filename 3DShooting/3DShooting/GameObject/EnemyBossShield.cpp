#include "EnemyBoss.h"
#include "DxLib.h"
#include "Effect.h"
#include "EffekseerWarningSuppress.h"
#include "Game.h"
#include "SphereCollider.h"
#include <algorithm>
#include <cmath>

// UpdateShieldEffect - UpdateAnimation から切り出した処理
void EnemyBoss::UpdateShieldEffect(const EnemyUpdateContext& context)
{
    Effect* pEffect = context.pEffect;

    VECTOR shieldPos = m_pos;
    shieldPos.y += EnemyBossConstants::kBodyColliderHeight * 0.6f;

    // シールドコライダー位置更新
    if (m_pShieldCollider)
    {
        m_pShieldCollider->SetCenter(shieldPos);
    }

    if (!m_isShieldBroken)
    {
        // シームレスループエフェクト生成
        // フェードイン30F、総再生240F を想定 → 210Fで次を生成して重ねる
        const float kEffectDuration   = 240.0f;
        const float kFadeInDuration   = 30.0f;
        const float kOverlapSpawnTime = kEffectDuration - kFadeInDuration;

        m_shieldEffectTimer += 1.0f * Game::GetTimeScale();

        if (m_shieldEffectHandles.empty() || m_shieldEffectTimer >= kOverlapSpawnTime)
        {
            if (pEffect)
            {
                int handle = pEffect->PlayBossShieldEffect(shieldPos.x, shieldPos.y, shieldPos.z);
                if (handle != -1)
                {
                    m_shieldEffectHandles.push_back(handle);
                    m_shieldEffectTimer = 0.0f;
                }
            }
        }

        // シールド回転更新
        m_shieldRotation += 0.3f * Game::GetTimeScale();
        while (m_shieldRotation >= 360.0f) m_shieldRotation -= 360.0f;

        // 有効なエフェクト全てのパラメータを更新（終了したものは除去）
        auto it = m_shieldEffectHandles.begin();
        while (it != m_shieldEffectHandles.end())
        {
            int handle = *it;
            if (IsEffekseer3DEffectPlaying(handle) == -1)
            {
                it = m_shieldEffectHandles.erase(it);
                continue;
            }

            SetPosPlayingEffekseer3DEffect(handle, shieldPos.x, shieldPos.y, shieldPos.z);
            SetRotationPlayingEffekseer3DEffect(handle, 0.0f, (m_shieldRotation * DX_PI_F / 180.0f), 0.0f);

            // シールドHP割合に応じた色変化（青 → 赤）
            if (m_maxShieldHp > 0.0f)
            {
                float ratio = m_shieldHp / m_maxShieldHp;
                if (ratio < 0.0f) ratio = 0.0f;
                if (ratio > 1.0f) ratio = 1.0f;

                int r = static_cast<int>(255.0f * (1.0f - ratio));
                int b = static_cast<int>(255.0f * ratio);
                SetColorPlayingEffekseer3DEffect(handle, r, 0, b, 255);
            }
            ++it;
        }
    }
    else
    {
        // 破壊されているなら全エフェクト停止
        for (int handle : m_shieldEffectHandles)
        {
            StopEffekseer3DEffect(handle);
        }
        m_shieldEffectHandles.clear();
    }
}

// UpdateShieldPushout - UpdateAnimation から切り出した処理
void EnemyBoss::UpdateShieldPushout()
{
    if (m_isShieldBroken || !m_pShieldCollider) return;
    if (!Game::m_pPlayer) return;

    VECTOR shieldPos = m_pos;
    shieldPos.y += EnemyBossConstants::kBodyColliderHeight * 0.6f;
    float shieldRadius = m_pShieldCollider->GetRadius();

    VECTOR playerPos = Game::m_pPlayer->GetPos();
    VECTOR playerCapA, playerCapB;
    float playerRadius;
    Game::m_pPlayer->GetCapsuleInfo(playerCapA, playerCapB, playerRadius);

    // カプセル（Player）と球（Shield）の最近接点を求める
    VECTOR segVec   = VSub(playerCapB, playerCapA);
    VECTOR ptToA    = VSub(shieldPos, playerCapA);
    float segLenSq  = VSquareSize(segVec);
    float t = 0.0f;
    if (segLenSq > 0.0001f)
    {
        t = VDot(ptToA, segVec) / segLenSq;
        t = (std::max)(0.0f, (std::min)(1.0f, t));
    }
    VECTOR closestPoint = VAdd(playerCapA, VScale(segVec, t));

    VECTOR pushDir = VSub(closestPoint, shieldPos);
    float distSq   = VSquareSize(pushDir);
    float minDist  = shieldRadius + playerRadius;

    // 完全重なり対策
    if (distSq <= 0.0001f)
    {
        pushDir = VSub(playerPos, m_pos);
        pushDir.y = 0.0f;
        if (VSquareSize(pushDir) > 0.0001f)
        {
            pushDir = VNorm(pushDir);
        }
        else
        {
            pushDir = VGet(0.0f, 0.0f, -1.0f);
        }
        distSq = 0.0f;
    }

    if (distSq < minDist * minDist)
    {
        float dist    = sqrtf(distSq);
        float pushLen = minDist - dist + 1.0f; // マージン込み

        if (dist > 0.0001f)
        {
            pushDir = VScale(pushDir, 1.0f / dist);
        }

        VECTOR newPos = VAdd(playerPos, VScale(pushDir, pushLen));
        Game::m_pPlayer->SetPos(newPos);

        // 上方向に押し出された場合（シールドに乗ったとき）は垂直速度をリセット
        if (pushDir.y > 0.5f)
        {
            Game::m_pPlayer->ResetVerticalVelocity();
        }
    }
}
