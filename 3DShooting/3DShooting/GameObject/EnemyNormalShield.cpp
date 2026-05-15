#include "EnemyNormal.h"
#include "DxLib.h"
#include "Effect.h"
#include "EffekseerWarningSuppress.h"
#include "Game.h"
#include "SceneMain.h"
#include "SphereCollider.h"
#include "CollisionGrid.h"
#include <algorithm>

void EnemyNormal::SetHasShield(bool hasShield)
{
    m_hasShieldConfigured = hasShield;
    if (m_hasShieldConfigured)
    {
        m_maxShieldHp = 50.0f; // ノーマルゾンビ用シールド耐久値
        m_shieldHp = m_maxShieldHp;
        m_isShieldBroken = false;

        // シールドコライダーの生成と初期化
        m_pShieldCollider = std::make_shared<SphereCollider>();
        m_pShieldCollider->SetRadius(80.0f);
        m_shieldRotation = 0.0f;
        m_shieldEffectTimer = 0.0f;
        m_shieldEffectHandles.clear();
        m_hasPlayedShieldBreakableEffect = false;
    }
}

void EnemyNormal::TriggerShieldChainBreak(int delayFrames)
{
    if (!m_hasShieldConfigured) return;
    // 既に破壊されていても、連鎖の起点になるためにタイマー設定を許可する
    m_shieldChainBreakTimer = delayFrames;
}

// シールド破壊 + 周囲への連鎖処理
void EnemyNormal::BreakShield(const EnemyUpdateContext* context)
{
    if (!m_hasShieldConfigured) return;

    if (!m_isShieldBroken)
    {
        m_isShieldBroken = true;
        m_pShieldCollider = nullptr;

        // 再生中のエフェクト停止
        for (int handle : m_shieldEffectHandles)
        {
            StopEffekseer3DEffect(handle);
        }
        m_shieldEffectHandles.clear();

        // 破壊エフェクト再生
        if (SceneMain::Instance() && SceneMain::Instance()->GetEffect())
        {
            VECTOR shieldPos = m_pos;
            shieldPos.y += EnemyNormalConstants::kBodyColliderHeight * 0.5f;
            SceneMain::Instance()->GetEffect()->PlayShieldBreakEffect(shieldPos);

            // プレイヤーとの距離に応じたカメラシェイク
            if (Game::m_pPlayer && Game::m_pPlayer->GetCamera())
            {
                float dist = VSize(VSub(m_pos, Game::m_pPlayer->GetPos()));
                float maxDist = 1000.0f;
                float intensityBase = 20.0f;

                float ratio = 1.0f - (dist / maxDist);
                if (ratio < 0.2f) ratio = 0.2f;
                if (ratio > 1.0f) ratio = 1.0f;

                Game::m_pPlayer->GetCamera()->Shake(intensityBase * ratio, 10);
            }
        }
    }

    // 周囲の敵への連鎖（contextがある場合のみ）
    if (context)
    {
        float chainRadius = 400.0f;
        std::vector<EnemyBase*> neighbors;
        if (context->collisionGrid)
        {
            context->collisionGrid->GetNeighbors(m_pos, neighbors);
        }
        else
        {
            neighbors = context->enemyList;
        }

        for (auto* enemyBase : neighbors)
        {
            auto* normalEnemy = dynamic_cast<EnemyNormal*>(enemyBase);
            if (normalEnemy && normalEnemy != this && normalEnemy->m_hasShieldConfigured && !normalEnemy->m_isShieldBroken)
            {
                float distSq = VSquareSize(VSub(m_pos, normalEnemy->GetPos()));
                if (distSq < chainRadius * chainRadius)
                {
                    // まだ連鎖タイマーが動いていない場合のみセット
                    if (normalEnemy->m_shieldChainBreakTimer <= 0)
                    {
                        normalEnemy->TriggerShieldChainBreak(12); // 12フレーム（約0.2秒）の遅延
                    }
                }
            }
        }
    }
}

// UpdateShield - UpdateAI から切り出した処理
void EnemyNormal::UpdateShield(const EnemyUpdateContext& context)
{
    // 連鎖破壊タイマーの更新
    if (m_hasShieldConfigured && m_shieldChainBreakTimer > 0)
    {
        m_shieldChainBreakTimer--;
        if (m_shieldChainBreakTimer <= 0)
        {
            BreakShield(&context);
        }
    }

    // シールドが存在する間だけ更新
    if (!m_hasShieldConfigured || m_isShieldBroken || !m_pShieldCollider) return;

    // コライダー位置を胸のあたりに設定
    VECTOR shieldPos = m_pos;
    shieldPos.y += EnemyNormalConstants::kBodyColliderHeight * 0.5f;
    m_pShieldCollider->SetCenter(shieldPos);

    // 生存中のみエフェクトを表示
    if (m_hp <= 0.0f) return;

    // シームレスループエフェクト生成
    const float kEffectDuration    = 240.0f;
    const float kOverlapSpawnTime  = kEffectDuration - 30.0f;
    m_shieldEffectTimer += 1.0f * Game::GetTimeScale();

    if (m_shieldEffectHandles.empty() || m_shieldEffectTimer >= kOverlapSpawnTime)
    {
        if (context.pEffect)
        {
            int handle = context.pEffect->PlayBossShieldEffect(shieldPos.x, shieldPos.y, shieldPos.z);
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

    // エフェクトのパラメータを更新（終了したものは除去）
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
        SetScalePlayingEffekseer3DEffect(handle, 0.3f, 0.3f, 0.3f);

        // シールドHP割合に応じた色変化（青 → 赤）
        if (m_maxShieldHp > 0.0f)
        {
            float ratio = m_shieldHp / m_maxShieldHp;
            int r = static_cast<int>(255.0f * (1.0f - (std::max)(0.0f, ratio)));
            int b = static_cast<int>(255.0f * (std::max)(0.0f, ratio));
            SetColorPlayingEffekseer3DEffect(handle, r, 0, b, 255);
        }
        ++it;
    }
}
