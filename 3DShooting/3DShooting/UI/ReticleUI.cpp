#include "ReticleUI.h"
#include "Player.h"
#include "Game.h"
#include "PlayerWeaponManager.h"
#include <cmath>

namespace
{
    constexpr float kRecoilScaleMultiplier = 0.5f; // 反動によるレティクル拡大率（最大 1 + kRecoilScaleMultiplier 倍）
    constexpr int   kAlphaMax              = 255;  // アルファ最大値
    constexpr int   kAimingTintGB          = 100;  // 狙い中の赤みティント（G・B チャンネル値）
}

ReticleUI::ReticleUI(Player* player)
    : m_pPlayer(player)
    , m_reticleDefault("data/image/SGDefaultReticl.png")
    , m_reticleOnTarget("data/image/SGOnTargetReticle.png")
    , m_dotDefault("data/image/DotDefault.png")
    , m_dotOnTarget("data/image/DotOnTarget.png")
{
}

void ReticleUI::Init()
{
}

void ReticleUI::Update(float deltaTime)
{
}

void ReticleUI::Draw()
{
    if (!m_pPlayer) return;

    int centerX = static_cast<int>(Game::GetScreenWidth() * 0.5f);
    int centerY = static_cast<int>(Game::GetScreenHeight() * 0.5f);
    float scale = Game::GetUIScale();

    // 拡大描画時のジャギー対策
    SetDrawMode(DX_DRAWMODE_BILINEAR);

    // 反動によるスケール計算（最大 1 + kRecoilScaleMultiplier 倍）
    float recoil      = m_pPlayer->GetWeaponManager().GetRecoilScale();
    float recoilScale = 1.0f + (recoil * kRecoilScaleMultiplier);

    bool isAiming = m_pPlayer->IsAimingAtEnemy();
    WeaponType currentWeapon = m_pPlayer->GetWeaponManager().GetCurrentWeaponType();

    // ショットガンの場合のみレティクルの線を描画
    if (currentWeapon == WeaponType::Shotgun)
    {
        int currentReticleHandle = isAiming ? static_cast<int>(m_reticleOnTarget) : static_cast<int>(m_reticleDefault);

        if (currentReticleHandle != -1)
        {
            int reticleWidth, reticleHeight;
            GetGraphSize(currentReticleHandle, &reticleWidth, &reticleHeight);

            int scaledReticleW = static_cast<int>(reticleWidth * scale * recoilScale);
            int scaledReticleH = static_cast<int>(reticleHeight * scale * recoilScale);

            if (isAiming)
            {
                SetDrawBlendMode(DX_BLENDMODE_ADD, kAlphaMax);
                SetDrawBright(kAlphaMax, kAimingTintGB, kAimingTintGB);
            }

            DrawExtendGraph(static_cast<int>(centerX - scaledReticleW * 0.5f), static_cast<int>(centerY - scaledReticleH * 0.5f),
                static_cast<int>(centerX + scaledReticleW * 0.5f), static_cast<int>(centerY + scaledReticleH * 0.5f), currentReticleHandle, true);

            if (isAiming)
            {
                SetDrawBright(kAlphaMax, kAlphaMax, kAlphaMax);
                SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
            }
        }
    }

    // ドットレティクル
    int dotReticleHandle = isAiming ? static_cast<int>(m_dotOnTarget) : static_cast<int>(m_dotDefault);
    if (dotReticleHandle != -1)
    {
        int dotW, dotH;
        GetGraphSize(dotReticleHandle, &dotW, &dotH);
        int scaledDotW = static_cast<int>(dotW * scale);
        int scaledDotH = static_cast<int>(dotH * scale);

        DrawExtendGraph(static_cast<int>(centerX - scaledDotW * 0.5f), static_cast<int>(centerY - scaledDotH * 0.5f),
            static_cast<int>(centerX + scaledDotW * 0.5f), static_cast<int>(centerY + scaledDotH * 0.5f), dotReticleHandle, true);
    }

    SetDrawMode(DX_DRAWMODE_NEAREST);
}
