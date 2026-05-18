#include "PlayerEffectManager.h"
#include "EffekseerWarningSuppress.h"
#include <cmath>

namespace
{
    // 警告UI関連
    constexpr float kWarningBlinkSpeed       = 1.5f; // 警告UIの点滅速度
    constexpr float kLowHealthEffectMaxAlpha = 0.7f; // 体力低下UIの最大アルファ値

    // エフェクトフィードバック描画
    constexpr float kEdgeWidthRatio = 0.4f; // 画面端エフェクトの幅（画面対角距離に対する割合）
    constexpr int   kStepSize       = 8;    // エフェクト描画のステップサイズ（ピクセル）
    constexpr int   kEdgeAlphaMax   = 180;  // 画面端エフェクトの最大アルファ値
}

void PlayerEffectManager::Update(float deltaTime, bool isLowHealth, float lowHealthBlinkTimer)
{
    // ダメージエフェクト
    if (m_damageEffect.timer > 0)
    {
        m_damageEffect.timer -= 1.0f;
        m_damageEffect.alpha -= 1.0f / m_damageEffect.duration;
        if (m_damageEffect.alpha < 0) m_damageEffect.alpha = 0;
    }
    else if (isLowHealth)
    {
        float alpha = (sinf(lowHealthBlinkTimer * 2.0f * DX_PI_F / kWarningBlinkSpeed) + 1.0f) * 0.5f;
        m_damageEffect.alpha  = alpha * kLowHealthEffectMaxAlpha;
        m_damageEffect.colorR = 255;
        m_damageEffect.colorG = 0;
        m_damageEffect.colorB = 0;
    }
    else
    {
        m_damageEffect.alpha = 0.0f;
    }

    // 回復エフェクト
    if (m_healEffect.timer > 0)
    {
        m_healEffect.timer -= 1.0f;
        m_healEffect.alpha -= 1.0f / m_healEffect.duration;
        if (m_healEffect.alpha < 0) m_healEffect.alpha = 0;
    }

    // 弾薬エフェクト
    if (m_ammoEffect.timer > 0)
    {
        m_ammoEffect.timer -= 1.0f;
        m_ammoEffect.alpha -= 1.0f / m_ammoEffect.duration;
        if (m_ammoEffect.alpha < 0) m_ammoEffect.alpha = 0;
    }
}

void PlayerEffectManager::Draw()
{
    DrawEffectFeedback(m_damageEffect);
    DrawEffectFeedback(m_healEffect);
    DrawEffectFeedback(m_ammoEffect);
}

void PlayerEffectManager::TriggerDamageEffect(float duration, int r, int g, int b)
{
    m_damageEffect.Trigger(duration, r, g, b);
}

void PlayerEffectManager::TriggerHealEffect(float duration, int r, int g, int b)
{
    m_healEffect.Trigger(duration, r, g, b);
}

void PlayerEffectManager::TriggerAmmoEffect(float duration, int r, int g, int b)
{
    m_ammoEffect.Trigger(duration, r, g, b);
}

void PlayerEffectManager::DrawEffectFeedback(EffectFeedback& effect)
{
    if (effect.alpha <= 0.0f) return;

    int screenW, screenH;
    GetScreenState(&screenW, &screenH, nullptr);
    int   centerX     = static_cast<int>(screenW * 0.5f);
    int   centerY     = static_cast<int>(screenH * 0.5f);
    float maxDistance = sqrtf(static_cast<float>(screenW * screenW + screenH * screenH)) * 0.5f;
    float edgeWidth   = maxDistance * kEdgeWidthRatio;

    for (int y = 0; y < screenH; y += kStepSize)
    {
        for (int x = 0; x < screenW; x += kStepSize)
        {
            float distFromCenter = sqrtf(static_cast<float>((x - centerX) * (x - centerX) + (y - centerY) * (y - centerY)));
            float distFromEdge   = maxDistance - distFromCenter;
            float edgeIntensity  = 0.0f;
            if (distFromEdge < edgeWidth)
            {
                edgeIntensity = 1.0f - (distFromEdge / edgeWidth);
            }
            int alpha = static_cast<int>(effect.alpha * kEdgeAlphaMax * edgeIntensity);
            if (alpha > 0)
            {
                SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
                DrawBox(x, y, x + kStepSize, y + kStepSize, GetColor(effect.colorR, effect.colorG, effect.colorB), true);
            }
        }
    }
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}
