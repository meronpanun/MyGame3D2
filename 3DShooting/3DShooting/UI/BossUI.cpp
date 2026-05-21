#include "BossUI.h"
#include "DrawUtil.h"
#include "EnemyBase.h"
#include "WaveManager.h"
#include "Game.h"
#include <algorithm>

namespace 
{
    // ボスHPバー関連
    constexpr int kBossHpBarWidth  = 900;
    constexpr int kBossHpBarHeight = 24;
    constexpr int kBossHpBarY      = 225;
    constexpr int kBossHpTextY     = 170;

    // 色関連
    constexpr unsigned int kColorWhite       = 0xffffff;
    constexpr unsigned int kColorHpBarBg     = 0x303030;
    constexpr unsigned int kColorHpBarTop    = 0xff4040; // ボスHPバー上部（明るい赤）
    constexpr unsigned int kColorHpBarBottom = 0x800000; // ボスHPバー下部（暗い赤）
    constexpr unsigned int kColorHpBarFill   = 0xcc0000; // ボスは真紅（バックアップ用）
    constexpr unsigned int kColorHpBarDamage = 0xaaaaaa; // 被ダメ時はグレー
    constexpr unsigned int kColorHpBarBorder = 0xffffff;
    constexpr float kAnimSpeed = 0.05f; // HPバーの追従速度

    constexpr int   kShadowOffset        =   2;    // 文字の影オフセット（ピクセル）
    constexpr int   kGlossAlpha          = 100;    // 光沢ラインのアルファ値
    constexpr float kGlossHeightRatio    =   0.2f; // 光沢ラインの高さ割合（バー高さに対する比率）
    constexpr float kScaleChangeTolerance =  0.001f; // スケール変更検知のしきい値
}

BossUI::BossUI(WaveManager* waveManager)
    : m_pWaveManager(waveManager)
    , m_healthBarAnim(0.0f)
    , m_font("ＭＳ ゴシック", 36, 3, DX_FONTTYPE_ANTIALIASING_EDGE_8X8)
    , m_prevScale(1.0f)
{
}

void BossUI::Init()
{
    ReloadFonts(1.0f);
}

void BossUI::Update(float deltaTime)
{
    // スケール変更検知
    float currentScale = Game::GetUIScale();
    if (fabsf(currentScale - m_prevScale) > kScaleChangeTolerance)
    {
        ReloadFonts(currentScale);
        m_prevScale = currentScale;
    }
}

void BossUI::Draw() 
{
    if (!m_pWaveManager) return;

    // 生存しているボスを探す
    const auto& enemyList = m_pWaveManager->GetEnemyList();
    EnemyBase *pBoss = nullptr;
    for (auto &enemy : enemyList) 
    {
        if (enemy->IsBoss() && enemy->IsAlive()) 
        {
            pBoss = enemy.get();
            break;
        }
    }

    if (!pBoss) 
    {
        m_healthBarAnim = 0.0f;
        return;
    }

    float hp = pBoss->GetHp();
    float maxHp = pBoss->GetMaxHp();

    // 初回時やリセット用
    if (m_healthBarAnim <= 0.0f) 
    {
        m_healthBarAnim = hp;
    }

    // アニメーション更新（タイムスケールを考慮）
    float speed = kAnimSpeed * Game::GetTimeScale();
    if (m_healthBarAnim > hp) 
    {
        m_healthBarAnim -= (m_healthBarAnim - hp) * speed;
        if (m_healthBarAnim < hp) m_healthBarAnim = hp;
    } 
    else if (m_healthBarAnim < hp) 
    {
        m_healthBarAnim = hp;
    }

    DrawBossHPBar(hp, maxHp);
}


void BossUI::DrawBossHPBar(float hp, float maxHp)
{
    int screenW = Game::GetScreenWidth();
    float scale = Game::GetUIScale();

    int barW = static_cast<int>(kBossHpBarWidth * scale);
    int barH = static_cast<int>(kBossHpBarHeight * scale);
    int barY = static_cast<int>(kBossHpBarY * scale);

    int barX = static_cast<int>((screenW - barW) * 0.5f);

    // HP割合
    float hpRate = hp / maxHp;
    float animRate = m_healthBarAnim / maxHp;

    // 背景
    DrawBox(barX, barY, barX + barW, barY + barH, kColorHpBarBg, true);

    // アニメーションバー（ダメージ演出用）
    if (m_healthBarAnim > hp) 
    {
        int animW = static_cast<int>(barW * animRate);
        DrawBox(barX, barY, barX + animW, barY + barH, kColorHpBarDamage, true);
    }

    // HPバー本体（グラデーション）
    if (hpRate > 0.0f)
    {
        int hpW = static_cast<int>(barW * hpRate);
        DrawUtil::DrawGradientBox(barX, barY, barX + hpW, barY + barH, kColorHpBarTop, kColorHpBarBottom);
        
        // 光沢ライン（上部）
        int glossH = static_cast<int>(barH * kGlossHeightRatio);
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, kGlossAlpha);
        DrawBox(barX, barY, barX + hpW, barY + glossH, 0xffffff, true);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }

    // 枠
    DrawBox(barX, barY, barX + barW, barY + barH, kColorHpBarBorder, false);

    // ボス名テキスト（影付き）
    const char *bossName = "BOSS";
    int textW = GetDrawStringWidthToHandle(bossName, static_cast<int>(strlen(bossName)), m_font);
    int textX = static_cast<int>((screenW - textW) * 0.5f);

    int textY = static_cast<int>(kBossHpTextY * scale);
    
    DrawStringToHandle(textX + kShadowOffset, textY + kShadowOffset, bossName, 0x000000, m_font);
    DrawStringToHandle(textX, textY, bossName, kColorWhite, m_font);
}

void BossUI::ReloadFonts(float scale)
{
    m_font.Reload(scale);
}
