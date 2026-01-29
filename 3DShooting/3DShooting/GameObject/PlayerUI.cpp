#include "PlayerUI.h"
#include "EffekseerForDXLib.h"
#include "EnemyBase.h"
#include "Game.h"
#include <cmath>
#include <cstring>


namespace {
// アサルトライフルUI関連
constexpr int kARImageWidth = 300;
constexpr int kARImageHeight = 200;
constexpr int kARImageMarginX = 60;
constexpr int kARImageMarginY = -90;

// ショットガンUI関連
constexpr int kSGImageWidth = 300;
constexpr int kSGImageHeight = 96;
constexpr int kSGImageMarginX = 60;
constexpr int kSGImageMarginY = -30;

// 弾薬UI関連
constexpr int kAmmoTextHeight = 48;
constexpr char kAmmoTextMaxWidthStr[] = "999";
constexpr int kAmmoTextGunOffsetX = 30;
constexpr int kAmmoTextGunOffsetY = -22;

// 警告UI関連
constexpr int kWarningImageSize = 192;
constexpr int kWarningImageYOffset = 240;
constexpr int kWarningTextYOffset = 8;
constexpr int kWarningImageSpacing = 30;
constexpr float kWarningBlinkSpeed = 1.5f; // 警告UIの点滅速度

// HpUI関連
constexpr int kHpBarWidth = 300;
constexpr int kHpBarHeight = 36;
constexpr int kHpBarMargin = 45;
constexpr int kHealthUiImageSize = 96;
constexpr int kHealthUiImageBarSpacing = 15;
constexpr float kMaxHp = 100.0f;
constexpr int kHpTextOffsetX = 12;
constexpr int kHpTextOffsetY = 3;

// 色関連
constexpr unsigned int kColorWhite = 0xffffff;
constexpr unsigned int kColorLowAmmo = 0xd3381c;
constexpr unsigned int kColorHpBarBg = 0x505050;
constexpr unsigned int kColorHpBarDamage = 0xFFD700;
constexpr unsigned int kColorHpBarFill = 0xff4040;
constexpr unsigned int kColorHpBarBorder = 0x000000;

// 盾UI関連
constexpr int kShieldUIYPosition = 630;
constexpr int kShieldUIYOffset = 45; // 盾UIのY軸調整オフセット

// フォント関連
constexpr int kDefaultFontThickness = 4; // フォントの太さ
constexpr int kAmmoFont = 48;            // 弾薬フォントサイズ
constexpr int kHpFont = 30;              // HPフォントサイズ
constexpr int kWarningFont = 36;         // 警告フォントサイズ
constexpr char kDefaultFontName[] = "Arial Black";
constexpr char kWarningFontName[] = "HGPｺﾞｼｯｸE";
constexpr int kDefaultFontType = DX_FONTTYPE_ANTIALIASING_EDGE_8X8;
} // namespace

PlayerUI::PlayerUI()
    : m_noAmmoImageHandle(-1), m_noHealthImageHandle(-1), m_arImageHandle(-1),
      m_noAmmoARImageHandle(-1), m_sgImageHandle(-1), m_noAmmoSGImageHandle(-1),
      m_healthUiImageHandle(-1), m_shieldImageHandle(-1), m_lockOnUIHandle(-1),
      m_fontHandle(-1), m_hpFontHandle(-1), m_warningFontHandle(-1) {
  // 弾薬切れ画像の読み込み
  m_noAmmoImageHandle = LoadGraph("data/image/NoAmmo.png");
  assert(m_noAmmoImageHandle != -1);

  // 体力低下画像の読み込み
  m_noHealthImageHandle = LoadGraph("data/image/NoHealthUI.png");
  assert(m_noHealthImageHandle != -1);

  // アサルトライフルUI画像の読み込み
  m_arImageHandle = LoadGraph("data/image/ARUI.png");
  assert(m_arImageHandle != -1);
  m_noAmmoARImageHandle = LoadGraph("data/image/NoAmmoARUI.png");
  assert(m_noAmmoARImageHandle != -1);

  // ショットガンUI画像の読み込み
  m_sgImageHandle = LoadGraph("data/image/SGUI.png");
  assert(m_sgImageHandle != -1);
  m_noAmmoSGImageHandle = LoadGraph("data/image/NoAmmoSGUI.png");
  assert(m_noAmmoSGImageHandle != -1);

  // HPUI画像の読み込み
  m_healthUiImageHandle = LoadGraph("data/image/HealthUI.png");
  assert(m_healthUiImageHandle != -1);

  // 盾UI画像の読み込み
  m_shieldImageHandle = LoadGraph("data/image/ShieldUI.png");
  assert(m_shieldImageHandle != -1);

  // ロックオンUI画像の読み込み
  m_lockOnUIHandle = LoadGraph("data/image/LockOnUI.png");
  assert(m_lockOnUIHandle != -1);

  // フォントの作成
  m_fontHandle = CreateFontToHandle(kDefaultFontName, kAmmoFont,
                                    kDefaultFontThickness, kDefaultFontType);
  assert(m_fontHandle != -1);
  m_hpFontHandle = CreateFontToHandle(kDefaultFontName, kHpFont,
                                      kDefaultFontThickness, kDefaultFontType);
  assert(m_hpFontHandle != -1);
  m_warningFontHandle = CreateFontToHandle(
      kWarningFontName, kWarningFont, kDefaultFontThickness, kDefaultFontType);
  assert(m_warningFontHandle != -1);
}

PlayerUI::~PlayerUI() {
  // 画像の解放
  DeleteGraph(m_noAmmoImageHandle);
  DeleteGraph(m_noHealthImageHandle);
  DeleteGraph(m_arImageHandle);
  DeleteGraph(m_noAmmoARImageHandle);
  DeleteGraph(m_sgImageHandle);
  DeleteGraph(m_noAmmoSGImageHandle);
  DeleteGraph(m_healthUiImageHandle);
  DeleteGraph(m_shieldImageHandle);
  DeleteGraph(m_lockOnUIHandle);

  // フォントの解放
  DeleteFontToHandle(m_fontHandle);
  DeleteFontToHandle(m_hpFontHandle);
  DeleteFontToHandle(m_warningFontHandle);
}

void PlayerUI::Draw(bool isDead, bool isGuarding, EnemyBase *lockedOnEnemy,
                    bool isTargetAvailable, float health, float healthBarAnim,
                    float maxHealth, bool isLowHealth,
                    float lowHealthBlinkTimer, float ammoTextFlashTimer,
                    const PlayerWeaponManager &weaponManager,
                    const PlayerShieldSystem &shieldSystem) {
  // ガード中のテキスト表示
  DrawGuardText(isGuarding, lockedOnEnemy, isTargetAvailable);

  // ロックオンUIの描画
  DrawLockOnUI(lockedOnEnemy);

  if (!isDead) {
    // 武器UIの描画
    DrawWeaponUI(weaponManager, ammoTextFlashTimer);

    // 盾UIの描画
    DrawShieldUI(shieldSystem);

    // 警告UIの描画
    DrawWarningUI(isLowHealth, lowHealthBlinkTimer, weaponManager);

    // HPバーの描画
    DrawHPBar(health, healthBarAnim, maxHealth);
  }
}

void PlayerUI::DrawHPBar(float health, float healthBarAnim, float maxHealth) {
  int screenW = Game::kScreenWidth;
  int screenH = Game::kScreenHeigth;
  GetScreenState(&screenW, &screenH, NULL);

  // HPバーのY座標を計算
  const int barY = screenH - kHpBarHeight - kHpBarMargin;

  // HPバーのパラメータ
  const int healthUiImageX = kHpBarMargin;
  const int healthUiImageY = screenH - kHpBarHeight - kHpBarMargin +
                             (kHpBarHeight - kHealthUiImageSize) * 0.5f;
  DrawExtendGraph(
      healthUiImageX, healthUiImageY, healthUiImageX + kHealthUiImageSize,
      healthUiImageY + kHealthUiImageSize, m_healthUiImageHandle, true);
  const int barX =
      healthUiImageX + kHealthUiImageSize + kHealthUiImageBarSpacing;

  // 最大HP
  float hp = health;
  if (hp < 0)
    hp = 0;
  if (hp > kMaxHp)
    hp = kMaxHp;

  float hpAnim = healthBarAnim;
  if (hpAnim < 0)
    hpAnim = 0;
  if (hpAnim > kMaxHp)
    hpAnim = kMaxHp;

  // HP割合
  float hpRate = hp / kMaxHp;
  float hpAnimRate = hpAnim / kMaxHp;

  // 背景
  DrawBox(barX, barY, barX + kHpBarWidth, barY + kHpBarHeight, kColorHpBarBg,
          true);

  // HPバー本体（実際の体力を反映）
  DrawBox(barX, barY, barX + static_cast<int>(kHpBarWidth * hpRate),
          barY + kHpBarHeight, kColorHpBarFill, true);

  // アニメーションバー（ゴーストバー）
  if (healthBarAnim > health) {
    // ダメージ時（黄色いバー）
    int animStart = barX + static_cast<int>(kHpBarWidth * hpRate);
    int animEnd = barX + static_cast<int>(kHpBarWidth * hpAnimRate);
    DrawBox(animStart, barY, animEnd, barY + kHpBarHeight, kColorHpBarDamage,
            true);
  } else if (healthBarAnim < health) {
    // 回復時（明るい緑のバー）
    int animStart = barX + static_cast<int>(kHpBarWidth * hpAnimRate);
    int animEnd = barX + static_cast<int>(kHpBarWidth * hpRate);
    DrawBox(animStart, barY, animEnd, barY + kHpBarHeight, 0x90EE90, true);
  }

  // 枠
  DrawBox(barX, barY, barX + kHpBarWidth, barY + kHpBarHeight,
          kColorHpBarBorder, false);

  // HP数値
  DrawFormatStringToHandle(barX + kHpTextOffsetX, barY + kHpTextOffsetY,
                           kColorWhite, m_hpFontHandle, "%.0f", healthBarAnim);
}

void PlayerUI::DrawWeaponUI(const PlayerWeaponManager &weaponManager,
                            float ammoTextFlashTimer) {
  int screenW = Game::kScreenWidth;
  int screenH = Game::kScreenHeigth;
  GetScreenState(&screenW, &screenH, NULL);

  // HPバーのY座標を計算
  const int barY = screenH - kHpBarHeight - kHpBarMargin;

  // タックルUIのY座標をHPバーに合わせる
  const int tackleUIY = barY;

  // 銃UI画像の描画
  int gunHandle = -1;
  int gunImageWidth = 0;
  int gunImageHeight = 0;
  int gunImageMarginX = 0;
  int gunImageMarginY = 0;

  WeaponType currentWeaponType = weaponManager.GetCurrentWeaponType();
  bool isLowAmmo = weaponManager.IsLowAmmo();
  bool isInfiniteAmmo = weaponManager.IsInfiniteAmmo();
  int currentAmmo = weaponManager.GetCurrentAmmo();

  switch (currentWeaponType) {
  case WeaponType::AssaultRifle:
    gunImageWidth = kARImageWidth;
    gunImageHeight = kARImageHeight;
    gunImageMarginX = kARImageMarginX;
    gunImageMarginY = kARImageMarginY;
    if (currentAmmo == 0 && !isInfiniteAmmo) {
      // 弾切れ時は点滅させずそのまま
      gunHandle = m_noAmmoARImageHandle;
    } else if (isLowAmmo) {
      // 低弾薬時は点滅させる
      float blinkAlpha = (sinf(weaponManager.GetLowAmmoBlinkTimer() * 2.0f *
                               DX_PI_F / kWarningBlinkSpeed) +
                          1.0f) *
                         0.5f;
      int alphaInt = static_cast<int>(blinkAlpha * 255);
      SetDrawBlendMode(DX_BLENDMODE_ALPHA, alphaInt);
      gunHandle = m_noAmmoARImageHandle;
    } else {
      gunHandle = m_arImageHandle;
    }
    break;
  case WeaponType::Shotgun:
    gunImageWidth = kSGImageWidth;
    gunImageHeight = kSGImageHeight;
    gunImageMarginX = kSGImageMarginX;
    gunImageMarginY = kSGImageMarginY;
    if (currentAmmo == 0 && !isInfiniteAmmo) {
      // 弾切れ時は点滅させずそのまま
      gunHandle = m_noAmmoSGImageHandle;
    } else if (isLowAmmo) {
      // 低弾薬時は点滅させる
      float blinkAlpha = (sinf(weaponManager.GetLowAmmoBlinkTimer() * 2.0f *
                               DX_PI_F / kWarningBlinkSpeed) +
                          1.0f) *
                         0.5f;
      int alphaInt = static_cast<int>(blinkAlpha * 255);
      SetDrawBlendMode(DX_BLENDMODE_ALPHA, alphaInt);
      gunHandle = m_noAmmoSGImageHandle;
    } else {
      gunHandle = m_sgImageHandle;
    }
    break;
  default:
    break;
  }

  int gunImageY = tackleUIY - gunImageHeight - gunImageMarginY;
  int gunImageX = screenW - gunImageWidth - gunImageMarginX;

  DrawExtendGraph(gunImageX, gunImageY, gunImageX + gunImageWidth,
                  gunImageY + gunImageHeight, gunHandle, true);

  // ブレンドモードをリセット
  if (isLowAmmo && currentAmmo > 0) {
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
  }

  // 残弾数の表示
  int ammoTextWidth = GetDrawStringWidthToHandle(
      kAmmoTextMaxWidthStr, strlen(kAmmoTextMaxWidthStr), m_fontHandle);

  // 弾薬数UIの位置をAR基準で固定計算
  int arGunImageX = screenW - kARImageWidth - kARImageMarginX;
  int arGunImageY = tackleUIY - kARImageHeight - kARImageMarginY;
  int ammoTextX = arGunImageX - kAmmoTextGunOffsetX - ammoTextWidth;
  int ammoTextY = arGunImageY + (kARImageHeight - kAmmoTextHeight) * 0.5f +
                  kAmmoTextGunOffsetY;

  // 弾薬無限モードの場合は「∞」を表示
  if (isInfiniteAmmo) {
    DrawFormatStringToHandle(ammoTextX, ammoTextY, kColorWhite, m_fontHandle,
                             "∞");
  } else {
    // デフォルトの色を決定
    int textColor = isLowAmmo ? kColorLowAmmo : kColorWhite;

    // フラッシュタイマーが作動中なら色を補間
    if (ammoTextFlashTimer > 0.0f) {
      float flashProgress = ammoTextFlashTimer / 60.0f;

      // ターゲットの色（デフォルト色）のRGB成分
      int targetR = (textColor >> 16) & 0xFF;
      int targetG = (textColor >> 8) & 0xFF;
      int targetB = textColor & 0xFF;

      // フラッシュの色（黄色）のRGB成分
      int flashR = 255;
      int flashG = 255;
      int flashB = 0;

      // 線形補間
      int currentR = static_cast<int>(flashR * flashProgress +
                                      targetR * (1.0f - flashProgress));
      int currentG = static_cast<int>(flashG * flashProgress +
                                      targetG * (1.0f - flashProgress));
      int currentB = static_cast<int>(flashB * flashProgress +
                                      targetB * (1.0f - flashProgress));

      textColor = GetColor(currentR, currentG, currentB);
    }

    DrawFormatStringToHandle(ammoTextX, ammoTextY, textColor, m_fontHandle,
                             "%d", currentAmmo);
  }
}

void PlayerUI::DrawShieldUI(const PlayerShieldSystem &shieldSystem) {
  int screenW, screenH;
  GetScreenState(&screenW, &screenH, NULL);

  // 盾耐久値の描画
  float shieldBarAnim = shieldSystem.GetBarAnim();
  float maxShieldDurability = shieldSystem.GetMaxDurability();
  float shieldDurabilityRate = shieldBarAnim / maxShieldDurability;
  if (shieldDurabilityRate < 0.0f)
    shieldDurabilityRate = 0.0f;
  if (shieldDurabilityRate > 1.0f)
    shieldDurabilityRate = 1.0f;

  // 盾のテクスチャサイズを取得
  int shieldTexW, shieldTexH;
  GetGraphSize(m_shieldImageHandle, &shieldTexW, &shieldTexH);

  // 盾ゲージのサイズと位置
  const int shieldGaugeHeight = 225; // 縦向きのゲージの高さ
  const int shieldGaugeWidth = (int)((float)shieldGaugeHeight * shieldTexW /
                                     shieldTexH); // 縦向きのゲージの幅
  float scale = (float)shieldGaugeHeight / shieldTexH;

  int shieldGaugeX = screenW - shieldGaugeWidth - kHpBarMargin;
  int shieldGaugeY = kShieldUIYPosition + kShieldUIYOffset;

  // ゲージの背景（半透明の盾）
  SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
  DrawRotaGraph3F(shieldGaugeX + shieldGaugeWidth * 0.5f,
                  shieldGaugeY + shieldGaugeHeight * 0.5f, shieldTexW * 0.5f,
                  shieldTexH * 0.5f, scale, scale, 0.0f, m_shieldImageHandle,
                  true);
  SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

  // ゲージ本体
  if (shieldDurabilityRate > 0.0f) {
    int filledWidth = (int)(shieldGaugeWidth * shieldDurabilityRate);
    // 描画範囲を設定してクリッピング
    SetDrawArea(shieldGaugeX, shieldGaugeY, shieldGaugeX + filledWidth,
                shieldGaugeY + shieldGaugeHeight);

    // 盾を満タン状態で描画
    DrawRotaGraph3F(shieldGaugeX + shieldGaugeWidth * 0.5f,
                    shieldGaugeY + shieldGaugeHeight * 0.5f, shieldTexW * 0.5f,
                    shieldTexH * 0.5f, scale, scale, 0.0f, m_shieldImageHandle,
                    true);

    // 描画範囲をリセット
    SetDrawArea(0, 0, screenW, screenH);
  }
}

void PlayerUI::DrawWarningUI(bool isLowHealth, float lowHealthBlinkTimer,
                             const PlayerWeaponManager &weaponManager) {
  int screenW, screenH;
  GetScreenState(&screenW, &screenH, NULL);

  // 警告表示ロジック
  // 体力低下と弾薬低下の警告を分離して処理
  // 体力低下の警告
  if (isLowHealth) {
    float alpha =
        (sinf(lowHealthBlinkTimer * 2.0f * DX_PI_F / kWarningBlinkSpeed) +
         1.0f) *
        0.5f;
    int alphaInt = static_cast<int>(alpha * 255);
    int drawX = (screenW - kWarningImageSize) * 0.5f;
    int drawY = (screenH - kWarningImageSize) * 0.5f + kWarningImageYOffset;

    // 弾薬警告も表示する必要がある場合は、体力警告を左にずらす
    bool isLowAmmoForHealth = weaponManager.IsLowAmmo();
    bool isNoAmmoWarningForHealth = weaponManager.IsNoAmmoWarning();
    bool isSwitchingWeaponForHealth = weaponManager.IsSwitchingWeapon();
    bool prevWeaponHadLowAmmoForHealth =
        weaponManager.GetPrevWeaponHadLowAmmo();
    bool prevWeaponHadNoAmmoForHealth = weaponManager.GetPrevWeaponHadNoAmmo();
    if (isLowAmmoForHealth || isNoAmmoWarningForHealth ||
        (isSwitchingWeaponForHealth &&
         (prevWeaponHadLowAmmoForHealth || prevWeaponHadNoAmmoForHealth))) {
      drawX =
          (screenW * 0.5f) - kWarningImageSize - (kWarningImageSpacing * 0.5f);
    }

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, alphaInt);
    DrawExtendGraph(drawX, drawY, drawX + kWarningImageSize,
                    drawY + kWarningImageSize, m_noHealthImageHandle, true);

    const char *text = "体力低下";
    int textWidth =
        GetDrawStringWidthToHandle(text, strlen(text), m_warningFontHandle);
    int textX = drawX + (kWarningImageSize - textWidth) / 2;
    int textY = drawY + kWarningImageSize + kWarningTextYOffset;
    unsigned int textColor = (alphaInt << 24) | kColorWhite;
    DrawStringToHandle(textX, textY, text, textColor, m_warningFontHandle);

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
  }

  // 弾薬低下の警告
  bool isLowAmmo = weaponManager.IsLowAmmo();
  bool isSwitchingWeapon = weaponManager.IsSwitchingWeapon();
  bool prevWeaponHadLowAmmo = weaponManager.GetPrevWeaponHadLowAmmo();
  bool prevWeaponHadNoAmmo = weaponManager.GetPrevWeaponHadNoAmmo();
  float weaponSwitchTimer = weaponManager.GetWeaponSwitchTimer();
  float weaponSwitchDuration = weaponManager.GetWeaponSwitchDuration();

  bool isNoAmmoWarning = weaponManager.IsNoAmmoWarning();
  bool currentNeedsWarning = isLowAmmo || isNoAmmoWarning;
  bool prevNeedsWarning = prevWeaponHadLowAmmo || prevWeaponHadNoAmmo;

  bool shouldDraw = false;
  float fadeAlpha = 1.0f;

  if (isSwitchingWeapon) {
    float halfDuration = weaponSwitchDuration / 2.0f;
    if (weaponSwitchTimer < halfDuration) {
      // フェードアウト
      if (prevNeedsWarning) {
        shouldDraw = true;
        fadeAlpha = 1.0f - (weaponSwitchTimer / halfDuration);
      }
    } else {
      // フェードイン
      if (currentNeedsWarning) {
        shouldDraw = true;
        fadeAlpha = (weaponSwitchTimer - halfDuration) / halfDuration;
      }
    }
  } else if (currentNeedsWarning) {
    shouldDraw = true;
  }

  if (shouldDraw) {
    bool isFadingOut =
        isSwitchingWeapon && (weaponSwitchTimer < weaponSwitchDuration / 2.0f);
    bool isNoAmmo = isFadingOut ? prevWeaponHadNoAmmo : isNoAmmoWarning;
    const char *text = isNoAmmo ? "残弾なし" : "残弾僅か";

    float lowAmmoBlinkTimer = weaponManager.GetLowAmmoBlinkTimer();
    float blinkAlpha =
        (sinf(lowAmmoBlinkTimer * 2.0f * DX_PI_F / kWarningBlinkSpeed) + 1.0f) *
        0.5f;
    int alphaInt = static_cast<int>(blinkAlpha * fadeAlpha * 255);

    int drawX = (screenW - kWarningImageSize) * 0.5f;
    int drawY = (screenH - kWarningImageSize) * 0.5f + kWarningImageYOffset;

    // 体力警告も表示する必要がある場合は、弾薬警告を右にずらす
    if (isLowHealth) {
      drawX = (screenW * 0.5f) + (kWarningImageSpacing * 0.5f);
    }

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, alphaInt);
    DrawExtendGraph(drawX, drawY, drawX + kWarningImageSize,
                    drawY + kWarningImageSize, m_noAmmoImageHandle, true);

    int textWidth =
        GetDrawStringWidthToHandle(text, strlen(text), m_warningFontHandle);
    int textX = drawX + (kWarningImageSize - textWidth) / 2;
    int textY = drawY + kWarningImageSize + kWarningTextYOffset;
    unsigned int textColor = (alphaInt << 24) | kColorWhite;
    DrawStringToHandle(textX, textY, text, textColor, m_warningFontHandle);

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
  }
}

void PlayerUI::DrawLockOnUI(EnemyBase *lockedOnEnemy) {
  if (lockedOnEnemy) {
    constexpr float kLockOnUISize = 64.0f;
    constexpr float kLockOnUIYOffset =
        90.0f; // UIを足元から上に移動させるためのオフセット

    VECTOR enemyPos = lockedOnEnemy->GetPos();
    enemyPos.y += kLockOnUIYOffset; // Y座標を調整して体の中心に近づける
    VECTOR screenPos = ConvWorldPosToScreenPos(enemyPos);

    if (screenPos.z > 0) // 画面内にあるか
    {
      float halfSize = kLockOnUISize / 2.0f;
      DrawExtendGraph(screenPos.x - halfSize, screenPos.y - halfSize,
                      screenPos.x + halfSize, screenPos.y + halfSize,
                      m_lockOnUIHandle, true);
    }
  }
}

void PlayerUI::DrawGuardText(bool isGuarding, EnemyBase *lockedOnEnemy,
                             bool isTargetAvailable) {
  // ガード中にターゲットがいない場合にテキストを表示
  if (isGuarding && !lockedOnEnemy && !isTargetAvailable) {
    const char *text = "ターゲットなし";
    int screenW, screenH;
    GetScreenState(&screenW, &screenH, NULL);
    int textWidth =
        GetDrawStringWidthToHandle(text, strlen(text), m_warningFontHandle);
    int textX = (screenW - textWidth) / 2;
    int textY = screenH / 2 + 30; // レティクルの少し下に表示
    DrawStringToHandle(textX, textY, text, kColorWhite, m_warningFontHandle);
  }
}
