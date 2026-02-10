#include "BossUI.h"
#include "EnemyBase.h"
#include "Game.h"
#include <algorithm>

namespace 
{
    // ボスHPバー関連
    constexpr int kBossHpBarWidth = 900;
    constexpr int kBossHpBarHeight = 24;
    constexpr int kBossHpBarY = 225;
    constexpr int kBossHpTextY = 170;

    // 色関連
    constexpr unsigned int kColorWhite = 0xffffff;
    constexpr unsigned int kColorHpBarBg = 0x303030;
    constexpr unsigned int kColorHpBarFill = 0xcc0000;   // ボスは真紅
    constexpr unsigned int kColorHpBarDamage = 0xaaaaaa; // 被ダメ時はグレー
    constexpr unsigned int kColorHpBarBorder = 0xffffff;

    constexpr float kAnimSpeed = 0.05f; // HPバーの追従速度
} 

BossUI::BossUI()
    : m_healthBarAnim(0.0f)
    , m_fontHandle(-1)
    , m_prevScale(1.0f)
{
    ReloadFonts(1.0f);
}

BossUI::~BossUI() 
{
    DeleteFontToHandle(m_fontHandle);
}

void BossUI::Draw(const std::vector<std::shared_ptr<EnemyBase>> &enemyList) 
{
    // 生存しているボスを探す
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

    // アニメーション更新
    if (m_healthBarAnim > hp) 
    {
        m_healthBarAnim -= (m_healthBarAnim - hp) * kAnimSpeed;
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
    // スケール変更検知
    float currentScale = Game::GetUIScale();
    if (fabsf(currentScale - m_prevScale) > 0.001f) 
    {
        ReloadFonts(currentScale);
        m_prevScale = currentScale;
    }

    int screenW = Game::GetScreenWidth();
     float scale = Game::GetUIScale();

    int barW = static_cast<int>(kBossHpBarWidth * scale);
    int barH = static_cast<int>(kBossHpBarHeight * scale);
    int barY = static_cast<int>(kBossHpBarY * scale);

    int barX = (screenW - barW) / 2;

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

    // HPバー本体
    int hpW = static_cast<int>(barW * hpRate);
    DrawBox(barX, barY, barX + hpW, barY + barH, kColorHpBarFill, true);

    // 枠
    DrawBox(barX, barY, barX + barW, barY + barH, kColorHpBarBorder, false);

    // ボス名テキスト
    const char *bossName = "BOSS";
    int textW = GetDrawStringWidthToHandle(bossName, static_cast<int>(strlen(bossName)), m_fontHandle);
    DrawStringToHandle((screenW - textW) / 2, static_cast<int>(kBossHpTextY * scale), bossName, kColorWhite, m_fontHandle);
}

void BossUI::ReloadFonts(float scale) 
{
    if (m_fontHandle != -1) 
    {
        DeleteFontToHandle(m_fontHandle);
    }
    m_fontHandle = CreateFontToHandle("ＭＳ ゴシック", static_cast<int>(36 * scale), 3, DX_FONTTYPE_ANTIALIASING_EDGE_8X8);
}
