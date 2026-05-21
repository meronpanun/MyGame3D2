#include "PlayerUI.h"
#include "DrawUtil.h"
#include "EffekseerWarningSuppress.h"
#include "EnemyBase.h"
#include "Game.h"
#include "Player.h"
#include <cmath>
#include <cstring>

namespace
{
    // アサルトライフルUI関連
    constexpr int kARImageWidth   = 300;
    constexpr int kARImageHeight  = 200;
    constexpr int kARImageMarginX =  60;
    constexpr int kARImageMarginY = -90;

    // ショットガンUI関連
    constexpr int kSGImageWidth   = 300;
    constexpr int kSGImageHeight  =  96;
    constexpr int kSGImageMarginX =  60;
    constexpr int kSGImageMarginY = -30;

    // 弾薬UI関連
    constexpr int          kAmmoTextHeight        =  48;
    constexpr char         kAmmoTextMaxWidthStr[] = "999";
    constexpr int          kAmmoTextGunOffsetX    =  30;
    constexpr int          kAmmoTextGunOffsetY    = -22;
    constexpr float        kFlashTimerMax         = 60.0f; // 弾薬テキストフラッシュタイマーの最大値
    constexpr float        kPulseScaleMax         =  0.2f; // 残弾少時のパルスアニメーション最大スケール
    constexpr unsigned int kColorAmmoFlash        = 0xffff00; // 弾薬フラッシュ色（黄色）

    // 警告UI関連
    constexpr int   kWarningImageSize    = 192;
    constexpr int   kWarningImageYOffset = 240;
    constexpr int   kWarningTextYOffset  =   8;
    constexpr int   kWarningImageSpacing =  30;
    constexpr float kWarningBlinkSpeed   =  1.5f; // 警告UIの点滅速度

    // HP UI関連
    constexpr int   kHpBarWidth              = 300;
    constexpr int   kHpBarHeight             =  36;
    constexpr int   kHpBarMargin             =  45;
    constexpr int   kHealthUiImageSize       =  96;
    constexpr int   kHealthUiImageBarSpacing =  15;
    constexpr float kMaxHp                   = 100.0f;
    constexpr int   kHpTextOffsetX           =  30;
    constexpr int   kHpTextOffsetY           =   3;
    constexpr int   kHpUiBottomCorrection    =  15; // HP UI下部領域の補正値

    // 盾UI関連
    constexpr int kShieldGaugeHeight = 230; // 盾ゲージの高さ（ピクセル）
    constexpr int kShieldBgAlpha     = 100; // 盾ゲージ背景のアルファ値

    // ロックオンUI関連
    constexpr float kLockOnUISize    = 64.0f; // ロックオンUIの1辺のサイズ
    constexpr float kLockOnUIYOffset = 90.0f; // UIを足元方向に移動させるためのオフセット

    // ガードUI関連
    constexpr int kGuardTextYOffset = 60; // ガードテキストの画面中心からのYオフセット

    // 色関連
    constexpr unsigned int kColorWhite           = 0xffffff;
    constexpr unsigned int kColorLowAmmo         = 0xd3381c;
    constexpr unsigned int kColorHpBarBg         = 0x505050;
    constexpr unsigned int kColorHpBarTop        = 0xff6060; // HPバー上部（明るい赤）
    constexpr unsigned int kColorHpBarBottom     = 0xa00000; // HPバー下部（暗い赤）
    constexpr unsigned int kColorHpBarDamageFlash= 0xffffff; // ダメージ時のフラッシュ色
    constexpr unsigned int kColorHpBarRecover    = 0x90EE90; // 回復時のバー色（ライトグリーン）
    constexpr unsigned int kColorHpBarBorder     = 0x000000;
    constexpr unsigned int kColorShadow          = 0x000000; // 影の色
    constexpr int          kShadowOffset         =         2; // 影のオフセット
    constexpr int          kAlphaMax             =       255; // アルファ最大値

    // フォント関連
    constexpr int  kDefaultFontThickness = 4;
    constexpr int  kAmmoFont             = 48;
    constexpr int  kHpFont               = 30;
    constexpr int  kWarningFont          = 36;
    constexpr char kDefaultFontName[]    = "Arial Black";
    constexpr char kWarningFontName[]    = "HGPゴシックE";
    constexpr int  kDefaultFontType      = DX_FONTTYPE_ANTIALIASING_EDGE_8X8;

    // スケール変更検知
    constexpr float kScaleChangeTolerance = 0.001f; // スケール変更検知のしきい値
}

PlayerUI::PlayerUI(Player* player)
    : m_pPlayer(player)
    , m_noAmmoImage("data/image/NoAmmo.png")
    , m_noHealthImage("data/image/NoHealthUI.png")
    , m_arImage("data/image/ARUI.png")
    , m_noAmmoARImage("data/image/NoAmmoARUI.png")
    , m_sgImage("data/image/SGUI.png")
    , m_noAmmoSGImage("data/image/NoAmmoSGUI.png")
    , m_healthUiImage("data/image/HealthUI.png")
    , m_shieldImage("data/image/ShieldUI.png")
    , m_lockOnUI("data/image/LockOnUI.png")
    , m_font(kDefaultFontName, kAmmoFont, kDefaultFontThickness, kDefaultFontType)
    , m_hpFont(kDefaultFontName, kHpFont, kDefaultFontThickness, kDefaultFontType)
    , m_warningFont(kWarningFontName, kWarningFont, kDefaultFontThickness, kDefaultFontType)
    , m_prevScale(1.0f)
{
}

void PlayerUI::Init()
{
    ReloadFonts(1.0f);
}

void PlayerUI::Update(float deltaTime)
{
    // スケール変更検知
    float currentScale = Game::GetUIScale();
    if (fabsf(currentScale - m_prevScale) > kScaleChangeTolerance)
    {
        ReloadFonts(currentScale);
        m_prevScale = currentScale;
    }
}

void PlayerUI::Draw()
{
    if (!m_pPlayer) return;

    float baseAlpha = 1.0f;

    // ガード中のテキスト表示
    DrawGuardText(baseAlpha);

    // ロックオンUIの描画
    DrawLockOnUI(baseAlpha);

    if (!m_pPlayer->IsDead())
    {
        // 武器UIの描画
        DrawWeaponUI(baseAlpha);

        // 盾UIの描画
        DrawShieldUI(baseAlpha);

        // 警告UIの描画
        DrawWarningUI(baseAlpha);

        // HPバーの描画
        DrawHPBar(baseAlpha);
    }
}

void PlayerUI::DrawHPBar(float baseAlpha)
{
    float health       = m_pPlayer->GetHealth();
    float healthBarAnim= m_pPlayer->GetHealthBarAnim();
    float maxHealth    = m_pPlayer->GetMaxHealth();

    int   screenW = Game::GetScreenWidth();
    int   screenH = Game::GetScreenHeight();
    float scale   = Game::GetUIScale();

    // スケール適用後のサイズとマージン
    int scaledHpBarHeight = static_cast<int>(kHpBarHeight             * scale);
    int scaledHpBarMargin = static_cast<int>(kHpBarMargin             * scale);
    int scaledHealthUiSize= static_cast<int>(kHealthUiImageSize       * scale);
    int scaledBarSpacing  = static_cast<int>(kHealthUiImageBarSpacing * scale);
    int scaledHpBarWidth  = static_cast<int>(kHpBarWidth              * scale);

    // HPバーのY座標算出
    const int barY = screenH - scaledHpBarHeight - scaledHpBarMargin;

    // ヘルスアイコン描画
    const int healthUiImageX = scaledHpBarMargin;
    const int healthUiImageY = screenH - scaledHpBarHeight - scaledHpBarMargin
                               + static_cast<int>((scaledHpBarHeight - scaledHealthUiSize) * 0.5f); // 縦方向センタリング

    int baseAlphaInt = static_cast<int>(baseAlpha * kAlphaMax);
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, baseAlphaInt);
    DrawExtendGraph(healthUiImageX, healthUiImageY,
                    healthUiImageX + scaledHealthUiSize, healthUiImageY + scaledHealthUiSize,
                    m_healthUiImage, true);
    const int barX = healthUiImageX + scaledHealthUiSize + scaledBarSpacing;

    // HP値をクランプ
    float hp     = (health       < 0.0f) ? 0.0f : (health       > kMaxHp ? kMaxHp : health);
    float hpAnim = (healthBarAnim < 0.0f) ? 0.0f : (healthBarAnim > kMaxHp ? kMaxHp : healthBarAnim);

    float hpRate     = hp     / kMaxHp;
    float hpAnimRate = hpAnim / kMaxHp;

    // 背景
    DrawBox(barX, barY, barX + scaledHpBarWidth, barY + scaledHpBarHeight, kColorHpBarBg, true);

    // HPバー本体（グラデーション）
    if (hpRate > 0.0f)
    {
        int currentBarWidth = static_cast<int>(scaledHpBarWidth * hpRate);
        DrawUtil::DrawGradientBox(barX, barY, barX + currentBarWidth, barY + scaledHpBarHeight,
                        kColorHpBarTop, kColorHpBarBottom);
    }

    // アニメーションバー（ゴーストバー）
    if (healthBarAnim > health)
    {
        // ダメージ後：白フラッシュバー
        int animStart = barX + static_cast<int>(scaledHpBarWidth * hpRate);
        int animEnd   = barX + static_cast<int>(scaledHpBarWidth * hpAnimRate);
        DrawBox(animStart, barY, animEnd, barY + scaledHpBarHeight, kColorHpBarDamageFlash, true);
    }
    else if (healthBarAnim < health)
    {
        // 回復時：ライトグリーンのバー
        int animStart = barX + static_cast<int>(scaledHpBarWidth * hpAnimRate);
        int animEnd   = barX + static_cast<int>(scaledHpBarWidth * hpRate);
        DrawBox(animStart, barY, animEnd, barY + scaledHpBarHeight, kColorHpBarRecover, true);
    }

    // 枠
    DrawBox(barX, barY, barX + scaledHpBarWidth, barY + scaledHpBarHeight, kColorHpBarBorder, false);

    // HP数値テキスト（影付き）
    int textAlpha = static_cast<int>(baseAlpha * kAlphaMax);
    int textX = barX + static_cast<int>(kHpTextOffsetX * scale);
    int textY = barY + static_cast<int>(kHpTextOffsetY * scale);
    DrawFormatStringToHandle(textX + kShadowOffset, textY + kShadowOffset,
        (textAlpha << 24) | kColorShadow, m_hpFont, "%.0f", healthBarAnim);
    DrawFormatStringToHandle(textX, textY,
        (textAlpha << 24) | kColorWhite, m_hpFont, "%.0f", healthBarAnim);

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void PlayerUI::DrawWeaponUI(float baseAlpha)
{
    const PlayerWeaponManager& weaponManager  = m_pPlayer->GetWeaponManager();
    float ammoTextFlashTimer = m_pPlayer->GetAmmoTextFlashTimer();

    int   screenW = Game::GetScreenWidth();
    int   screenH = Game::GetScreenHeight();
    float scale   = Game::GetUIScale();

    // HP バーのY座標（武器UIはこれに合わせる）
    int scaledHpBarHeight = static_cast<int>(kHpBarHeight * scale);
    int scaledHpBarMargin = static_cast<int>(kHpBarMargin * scale);
    const int barY      = screenH - scaledHpBarHeight - scaledHpBarMargin;
    const int tackleUIY = barY;

    // 銃画像パラメータ
    int gunHandle       = -1;
    int gunImageWidth   = 0;
    int gunImageHeight  = 0;
    int gunImageMarginX = 0;
    int gunImageMarginY = 0;

    WeaponType currentWeaponType = weaponManager.GetCurrentWeaponType();
    bool isLowAmmo     = weaponManager.IsLowAmmo();
    bool isInfiniteAmmo= weaponManager.IsInfiniteAmmo();
    int  currentAmmo   = weaponManager.GetCurrentAmmo();

    int baseAlphaInt = static_cast<int>(baseAlpha * kAlphaMax);

    switch (currentWeaponType)
    {
    case WeaponType::AssaultRifle:
        gunImageWidth   = static_cast<int>(kARImageWidth   * scale);
        gunImageHeight  = static_cast<int>(kARImageHeight  * scale);
        gunImageMarginX = static_cast<int>(kARImageMarginX * scale);
        gunImageMarginY = static_cast<int>(kARImageMarginY * scale);
        if (currentAmmo == 0 && !isInfiniteAmmo)
        {
            // 弾切れ時は点滅なし、そのまま
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, baseAlphaInt);
            gunHandle = static_cast<int>(m_noAmmoARImage);
        }
        else if (isLowAmmo)
        {
            // 残弾少時は点滅
            float blinkAlpha = (sinf(weaponManager.GetLowAmmoBlinkTimer() * 2.0f * DX_PI_F / kWarningBlinkSpeed) + 1.0f) * 0.5f;
            int alphaInt = static_cast<int>(blinkAlpha * baseAlpha * kAlphaMax);
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, alphaInt);
            gunHandle = static_cast<int>(m_noAmmoARImage);
        }
        else
        {
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, baseAlphaInt);
            gunHandle = static_cast<int>(m_arImage);
        }
        break;
    case WeaponType::Shotgun:
        gunImageWidth   = static_cast<int>(kSGImageWidth   * scale);
        gunImageHeight  = static_cast<int>(kSGImageHeight  * scale);
        gunImageMarginX = static_cast<int>(kSGImageMarginX * scale);
        gunImageMarginY = static_cast<int>(kSGImageMarginY * scale);
        if (currentAmmo == 0 && !isInfiniteAmmo)
        {
            // 弾切れ時は点滅なし、そのまま
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, baseAlphaInt);
            gunHandle = static_cast<int>(m_noAmmoSGImage);
        }
        else if (isLowAmmo)
        {
            // 残弾少時は点滅
            float blinkAlpha = (sinf(weaponManager.GetLowAmmoBlinkTimer() * 2.0f * DX_PI_F / kWarningBlinkSpeed) + 1.0f) * 0.5f;
            int alphaInt = static_cast<int>(blinkAlpha * baseAlpha * kAlphaMax);
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, alphaInt);
            gunHandle = static_cast<int>(m_noAmmoSGImage);
        }
        else
        {
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, baseAlphaInt);
            gunHandle = static_cast<int>(m_sgImage);
        }
        break;
    default:
        break;
    }

    int gunImageY = tackleUIY - gunImageHeight - gunImageMarginY;
    int gunImageX = screenW   - gunImageWidth  - gunImageMarginX;

    DrawExtendGraph(gunImageX, gunImageY, gunImageX + gunImageWidth, gunImageY + gunImageHeight, gunHandle, true);

    // 残弾数の描画に向けたブレンドモード設定（ベースの不透明度を適用）
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, baseAlphaInt);

    int ammoTextWidth = GetDrawStringWidthToHandle(kAmmoTextMaxWidthStr,
                            static_cast<int>(strlen(kAmmoTextMaxWidthStr)), m_font);

    // 弾薬UIの位置をARの位置で固定計算（スケール済み値を使用）
    int scaledARWidth        = static_cast<int>(kARImageWidth   * scale);
    int scaledARMarginX      = static_cast<int>(kARImageMarginX * scale);
    int scaledARHeight       = static_cast<int>(kARImageHeight  * scale);
    int scaledARMarginY      = static_cast<int>(kARImageMarginY * scale);
    int scaledAmmoTextHeight = static_cast<int>(kAmmoTextHeight * scale);

    int arGunImageX = screenW - scaledARWidth  - scaledARMarginX;
    int arGunImageY = tackleUIY - scaledARHeight - scaledARMarginY;

    int scaledAmmoOffsetX = static_cast<int>(kAmmoTextGunOffsetX * scale);
    int scaledAmmoOffsetY = static_cast<int>(kAmmoTextGunOffsetY * scale);

    int ammoTextX = arGunImageX - scaledAmmoOffsetX - ammoTextWidth;
    int ammoTextY = arGunImageY + static_cast<int>((scaledARHeight - scaledAmmoTextHeight) * 0.5f) + scaledAmmoOffsetY;

    // 弾薬無限モードの場合は「∞」を表示
    if (isInfiniteAmmo)
    {
        int alpha = static_cast<int>(baseAlpha * kAlphaMax);
        DrawFormatStringToHandle(ammoTextX + kShadowOffset, ammoTextY + kShadowOffset,
            (alpha << 24) | kColorShadow, m_font, "∞");
        DrawFormatStringToHandle(ammoTextX, ammoTextY, (alpha << 24) | kColorWhite, m_font, "∞");
    }
    else
    {
        // デフォルトの色から始める
        int textColor = isLowAmmo ? kColorLowAmmo : kColorWhite;
        float textScale = 1.0f;

        // フラッシュタイマーが動作中なら色とサイズを変更
        if (ammoTextFlashTimer > 0.0f)
        {
            float flashProgress = ammoTextFlashTimer / kFlashTimerMax;

            // ターゲットの色のRGB成分
            int targetR = (textColor >> 16) & 0xFF;
            int targetG = (textColor >> 8)  & 0xFF;
            int targetB =  textColor         & 0xFF;

            // フラッシュ色（黄色）のRGB成分
            int flashR = (kColorAmmoFlash >> 16) & 0xFF;
            int flashG = (kColorAmmoFlash >> 8)  & 0xFF;
            int flashB =  kColorAmmoFlash         & 0xFF;

            // 線形補間
            int currentR = static_cast<int>(flashR * flashProgress + targetR * (1.0f - flashProgress));
            int currentG = static_cast<int>(flashG * flashProgress + targetG * (1.0f - flashProgress));
            int currentB = static_cast<int>(flashB * flashProgress + targetB * (1.0f - flashProgress));

            textColor = GetColor(currentR, currentG, currentB);
        }
        else if (isLowAmmo)
        {
            // 残弾少時のパルスアニメーション
            float pulse = (sinf(weaponManager.GetLowAmmoBlinkTimer() * 5.0f * DX_PI_F / kWarningBlinkSpeed) + 1.0f) * 0.5f;
            textScale = 1.0f + pulse * kPulseScaleMax;
        }

        // テキスト描画（影付き・拡縮対応）
        char ammoStr[16];
        sprintf_s(ammoStr, "%d", currentAmmo);

        int strW = GetDrawStringWidthToHandle(ammoStr, static_cast<int>(strlen(ammoStr)), m_font);
        int strH = GetFontSizeToHandle(m_font);

        // 中心基準で拡縮
        int drawW   = static_cast<int>(strW * textScale);
        int drawH   = static_cast<int>(strH * textScale);
        int offsetX = static_cast<int>((drawW - strW) * 0.5f);
        int offsetY = static_cast<int>((drawH - strH) * 0.5f);

        int alpha = static_cast<int>(baseAlpha * kAlphaMax);
        DrawExtendStringToHandle(ammoTextX - offsetX + kShadowOffset, ammoTextY - offsetY + kShadowOffset,
            textScale, textScale, ammoStr, (alpha << 24) | kColorShadow, m_font);
        DrawExtendStringToHandle(ammoTextX - offsetX, ammoTextY - offsetY,
            textScale, textScale, ammoStr, (alpha << 24) | textColor, m_font);
    }

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void PlayerUI::DrawShieldUI(float baseAlpha)
{
    const PlayerShieldSystem& shieldSystem = m_pPlayer->GetShieldSystem();
    int   screenW = Game::GetScreenWidth();
    int   screenH = Game::GetScreenHeight();
    float scale   = Game::GetUIScale();

    // 耐久値の割合
    float shieldBarAnim         = shieldSystem.GetBarAnim();
    float maxShieldDurability   = shieldSystem.GetMaxDurability();
    float shieldDurabilityRate  = shieldBarAnim / maxShieldDurability;

    if (shieldDurabilityRate < 0.0f) shieldDurabilityRate = 0.0f;
    if (shieldDurabilityRate > 1.0f) shieldDurabilityRate = 1.0f;

    // 盾テクスチャサイズを取得
    int shieldTexW, shieldTexH;
    GetGraphSize(m_shieldImage, &shieldTexW, &shieldTexH);

    // 盾ゲージのサイズと位置
    const int shieldGaugeHeight = static_cast<int>(kShieldGaugeHeight * scale);
    const int shieldGaugeWidth  = static_cast<int>(static_cast<float>(shieldGaugeHeight) * shieldTexW / shieldTexH); // 縦長のゲージの幅
    float drawScale = static_cast<float>(shieldGaugeHeight) / shieldTexH;

    int scaledHpBarMargin = static_cast<int>(kHpBarMargin * scale);

    // HP UIの占める高さを計算してゲージのY座標を決める
    int hpUITopFromBottom = static_cast<int>((kHpBarMargin + kHealthUiImageSize - kHpUiBottomCorrection) * scale);
    int shieldGaugeY = screenH - hpUITopFromBottom - shieldGaugeHeight;

    // HPバーのマージンに揃える
    int shieldGaugeX = scaledHpBarMargin;

    // ゲージの背景（半透明で表示）
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(kShieldBgAlpha * baseAlpha));
    DrawRotaGraph3F(shieldGaugeX + shieldGaugeWidth * 0.5f,
                    shieldGaugeY + shieldGaugeHeight * 0.5f,
                    shieldTexW * 0.5f, shieldTexH * 0.5f,
                    drawScale, drawScale, 0.0f, m_shieldImage, true);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    // ゲージ本体（耐久値分だけクリッピングして表示）
    if (shieldDurabilityRate > 0.0f)
    {
        int filledWidth = static_cast<int>(shieldGaugeWidth * shieldDurabilityRate);
        SetDrawArea(shieldGaugeX, shieldGaugeY, shieldGaugeX + filledWidth, shieldGaugeY + shieldGaugeHeight);

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(kAlphaMax * baseAlpha));
        DrawRotaGraph3F(shieldGaugeX + shieldGaugeWidth * 0.5f,
                        shieldGaugeY + shieldGaugeHeight * 0.5f,
                        shieldTexW * 0.5f, shieldTexH * 0.5f,
                        drawScale, drawScale, 0.0f, m_shieldImage, true);

        SetDrawArea(0, 0, screenW, screenH);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }
}

void PlayerUI::DrawWarningUI(float baseAlpha)
{
    bool isLowHealth         = m_pPlayer->IsLowHealth();
    float lowHealthBlinkTimer= m_pPlayer->GetLowHealthBlinkTimer();
    const PlayerWeaponManager& weaponManager = m_pPlayer->GetWeaponManager();

    int   screenW = Game::GetScreenWidth();
    int   screenH = Game::GetScreenHeight();
    float scale   = Game::GetUIScale();

    int scaledWarningSize      = static_cast<int>(kWarningImageSize    * scale);
    int scaledWarningYOffset   = static_cast<int>(kWarningImageYOffset * scale);
    int scaledWarningTextYOffset = static_cast<int>(kWarningTextYOffset * scale);
    int scaledWarningSpacing   = static_cast<int>(kWarningImageSpacing * scale);

    // 体力低下の警告
    if (isLowHealth)
    {
        float alpha = (sinf(lowHealthBlinkTimer * 2.0f * DX_PI_F / kWarningBlinkSpeed) + 1.0f) * 0.5f;
        int drawX   = static_cast<int>((screenW - scaledWarningSize) * 0.5f);
        int drawY   = static_cast<int>((screenH - scaledWarningSize) * 0.5f) + scaledWarningYOffset;

        // 弾薬警告も表示する場合は左に寄せる
        bool isLowAmmoForHealth    = weaponManager.IsLowAmmo();
        bool isNoAmmoWarningForHealth = weaponManager.IsNoAmmoWarning();
        bool isSwitchingWeaponForHealth = weaponManager.IsSwitchingWeapon();
        bool prevWeaponHadLowAmmoForHealth = weaponManager.GetPrevWeaponHadLowAmmo();
        bool prevWeaponHadNoAmmoForHealth  = weaponManager.GetPrevWeaponHadNoAmmo();
        if (isLowAmmoForHealth || isNoAmmoWarningForHealth
            || (isSwitchingWeaponForHealth && (prevWeaponHadLowAmmoForHealth || prevWeaponHadNoAmmoForHealth)))
        {
            drawX = static_cast<int>((screenW * 0.5f) - scaledWarningSize - (scaledWarningSpacing * 0.5f));
        }

        int finalAlpha = static_cast<int>(alpha * baseAlpha * kAlphaMax);
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, finalAlpha);
        DrawExtendGraph(drawX, drawY, drawX + scaledWarningSize, drawY + scaledWarningSize, m_noHealthImage, true);

        const char* text = "体力低下";
        int textWidth = GetDrawStringWidthToHandle(text, static_cast<int>(strlen(text)), m_warningFont);
        int textX = drawX + static_cast<int>((scaledWarningSize - textWidth) * 0.5f);
        int textY = drawY + scaledWarningSize + scaledWarningTextYOffset;
        unsigned int textColor = (finalAlpha << 24) | kColorWhite;
        DrawStringToHandle(textX, textY, text, textColor, m_warningFont);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }

    // 弾薬低下の警告
    bool isLowAmmo          = weaponManager.IsLowAmmo();
    bool isSwitchingWeapon  = weaponManager.IsSwitchingWeapon();
    bool prevWeaponHadLowAmmo = weaponManager.GetPrevWeaponHadLowAmmo();
    bool prevWeaponHadNoAmmo  = weaponManager.GetPrevWeaponHadNoAmmo();
    float weaponSwitchTimer   = weaponManager.GetWeaponSwitchTimer();
    float weaponSwitchDuration= weaponManager.GetWeaponSwitchDuration();

    bool isNoAmmoWarning   = weaponManager.IsNoAmmoWarning();
    bool currentNeedsWarning = isLowAmmo || isNoAmmoWarning;
    bool prevNeedsWarning    = prevWeaponHadLowAmmo || prevWeaponHadNoAmmo;

    bool shouldDraw  = false;
    float fadeAlpha  = 1.0f;

    if (isSwitchingWeapon)
    {
        float halfDuration = weaponSwitchDuration * 0.5f;
        if (weaponSwitchTimer < halfDuration)
        {
            // フェードアウト
            if (prevNeedsWarning)
            {
                shouldDraw = true;
                fadeAlpha  = 1.0f - (weaponSwitchTimer / halfDuration);
            }
        }
        else
        {
            // フェードイン
            if (currentNeedsWarning)
            {
                shouldDraw = true;
                fadeAlpha  = (weaponSwitchTimer - halfDuration) / halfDuration;
            }
        }
    }
    else if (currentNeedsWarning)
    {
        shouldDraw = true;
    }

    if (shouldDraw)
    {
        bool isFadingOut = isSwitchingWeapon && (weaponSwitchTimer < weaponSwitchDuration * 0.5f);
        bool isNoAmmo    = isFadingOut ? prevWeaponHadNoAmmo : isNoAmmoWarning;
        const char* text = isNoAmmo ? "残弾なし" : "残弾少";

        float lowAmmoBlinkTimer = weaponManager.GetLowAmmoBlinkTimer();
        float blinkAlpha = (sinf(lowAmmoBlinkTimer * 2.0f * DX_PI_F / kWarningBlinkSpeed) + 1.0f) * 0.5f;
        int alphaInt = static_cast<int>(blinkAlpha * fadeAlpha * baseAlpha * kAlphaMax);

        int drawX = static_cast<int>((screenW - scaledWarningSize) * 0.5f);
        int drawY = static_cast<int>((screenH - scaledWarningSize) * 0.5f) + scaledWarningYOffset;

        // 体力警告も表示する場合は右に寄せる
        if (isLowHealth)
        {
            drawX = static_cast<int>((screenW * 0.5f) + (scaledWarningSpacing * 0.5f));
        }

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, alphaInt);
        DrawExtendGraph(drawX, drawY, drawX + scaledWarningSize, drawY + scaledWarningSize, m_noAmmoImage, true);

        int textWidth = GetDrawStringWidthToHandle(text, static_cast<int>(strlen(text)), m_warningFont);
        int textX = drawX + static_cast<int>((scaledWarningSize - textWidth) * 0.5f);
        int textY = drawY + scaledWarningSize + scaledWarningTextYOffset;
        unsigned int textColor = (alphaInt << 24) | kColorWhite;
        DrawStringToHandle(textX, textY, text, textColor, m_warningFont);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }
}

void PlayerUI::DrawLockOnUI(float baseAlpha)
{
    EnemyBase* lockedOnEnemy = m_pPlayer->GetLockedOnEnemy();
    if (!lockedOnEnemy) return;

    float scale      = Game::GetUIScale();
    float scaledSize = kLockOnUISize * scale;

    VECTOR enemyPos = lockedOnEnemy->GetPos();
    enemyPos.y += kLockOnUIYOffset; // Y座標を調整して体の中心に近づける
    VECTOR screenPos = ConvWorldPosToScreenPos(enemyPos);

    if (screenPos.z > 0.0f) // 画面内にあるか
    {
        float halfSize = scaledSize * 0.5f;
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(baseAlpha * kAlphaMax));
        DrawExtendGraph(static_cast<int>(screenPos.x - halfSize), static_cast<int>(screenPos.y - halfSize),
                        static_cast<int>(screenPos.x + halfSize), static_cast<int>(screenPos.y + halfSize),
                        m_lockOnUI, true);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }
}

void PlayerUI::DrawGuardText(float baseAlpha)
{
    bool isGuarding      = m_pPlayer->IsGuarding();
    EnemyBase* lockedOnEnemy = m_pPlayer->GetLockedOnEnemy();

    if (!isGuarding || lockedOnEnemy) return;

    int screenW = Game::GetScreenWidth();
    int screenH = Game::GetScreenHeight();
    int alpha   = static_cast<int>(baseAlpha * kAlphaMax);
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

    const char* text = "ターゲットなし";
    int textWidth = GetDrawStringWidthToHandle(text, static_cast<int>(strlen(text)), m_warningFont);
    int textX = static_cast<int>((screenW - textWidth) * 0.5f);
    int textY = static_cast<int>(screenH * 0.5f) + kGuardTextYOffset; // 画面中央の下方向に表示

    DrawStringToHandle(textX + kShadowOffset, textY + kShadowOffset,
        text, (alpha << 24) | kColorShadow, m_warningFont);
    DrawStringToHandle(textX, textY,
        text, (alpha << 24) | kColorWhite, m_warningFont);

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void PlayerUI::ReloadFonts(float scale)
{
    m_font.Reload(scale);
    m_hpFont.Reload(scale);
    m_warningFont.Reload(scale);
}
