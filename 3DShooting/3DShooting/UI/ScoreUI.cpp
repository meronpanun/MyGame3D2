#include "ScoreUI.h"
#include "Game.h"
#include "ScoreManager.h"
#include <cstdarg>

ScoreUI::ScoreUI()
    : m_totalScorePopupValue(0)
    , m_totalScorePopupTimer(0.0f)
    , m_scoreFont(-1)
{
}

ScoreUI::~ScoreUI()
{
    if (m_scoreFont != -1)
    {
        DeleteFontToHandle(m_scoreFont);
    }
}

void ScoreUI::Init()
{
    m_scoreFont = CreateFontToHandle("Arial Black", 24, 4, DX_FONTTYPE_ANTIALIASING_EDGE_8X8);
}

void ScoreUI::Update(float deltaTime)
{
    float timeScale = Game::GetTimeScale();
    
    // ポップアップタイマー更新
    for (auto it = m_popups.begin(); it != m_popups.end(); )
    {
        it->timer -= 1.0f * timeScale;
        if (it->timer <= 0)
        {
            it = m_popups.erase(it);
        }
        else
        {
            ++it;
        }
    }

    if (m_totalScorePopupTimer > 0)
    {
        m_totalScorePopupTimer -= 1.0f * timeScale;
        if (m_totalScorePopupTimer <= 0)
        {
            m_totalScorePopupValue = 0;
        }
    }
}

void ScoreUI::AddScorePopup(int value, bool isHeadShot)
{
    m_popups.push_back({ value, isHeadShot, kScorePopupDuration, kScorePopupDuration });
    m_totalScorePopupValue += value;
    m_totalScorePopupTimer = kScorePopupDuration;
}

void ScoreUI::Draw()
{
    if (m_popups.empty() && m_totalScorePopupTimer <= 0) return;

    float scale = Game::GetUIScale();
    int fontSize = 32;
    int scaledPopupOffsetY = static_cast<int>(fontSize * 1.2f * scale);
    
    int popupBaseX = Game::GetScreenWidth() * 0.5f + static_cast<int>(kScorePopupX * scale);
    int popupBaseY = Game::GetScreenHeight() * 0.5f + static_cast<int>(kScorePopupY * scale);
    
    int idx = 0;
    float lastComboRate = ScoreManager::Instance().GetLastComboRate();
    int displayCombo = (ScoreManager::Instance().GetCombo() > 1) ? ScoreManager::Instance().GetCombo() : 1;

    int alpha = 255;
    if (m_totalScorePopupTimer < 20)
    {
        alpha = static_cast<int>(255 * m_totalScorePopupTimer / 20.0f);
    }
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

    auto DrawShadowText = [&](int x, int y, unsigned int color, const char* format, ...) {
        va_list args;
        va_start(args, format);
        char buf[256];
        vsprintf_s(buf, format, args);
        va_end(args);
        DrawStringToHandle(x + 2, y + 2, buf, 0x000000, m_scoreFont);
        DrawStringToHandle(x, y, buf, color, m_scoreFont);
    };

    // 合計スコア
    if (lastComboRate > 1.0f)
    {
        DrawShadowText(popupBaseX, popupBaseY + idx * scaledPopupOffsetY, 0x00ffcc, "+%d (×%.2f)", m_totalScorePopupValue, lastComboRate);
    }
    else
    {
        DrawShadowText(popupBaseX, popupBaseY + idx * scaledPopupOffsetY, 0x00ffcc, "+%d", m_totalScorePopupValue);
    }
    idx++;

    // 内訳
    int lastIsHeadShot = -1;
    for (const auto& popup : m_popups)
    {
        if (lastIsHeadShot == -1 || lastIsHeadShot != static_cast<int>(popup.isHeadShot))
        {
            if (popup.timer < 20) {
                int detailAlpha = static_cast<int>(alpha * popup.timer / 20.0f);
                SetDrawBlendMode(DX_BLENDMODE_ALPHA, detailAlpha);
            }

            if (popup.isHeadShot)
            {
                DrawShadowText(popupBaseX, popupBaseY + idx * scaledPopupOffsetY, 0xffd700, "200pt HEADSHOT ×%d", displayCombo);
            }
            else
            {
                DrawShadowText(popupBaseX, popupBaseY + idx * scaledPopupOffsetY, 0xeeeeee, "100pt ZOMBIE KILL ×%d", displayCombo);
            }
            idx++;
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
        }
        lastIsHeadShot = static_cast<int>(popup.isHeadShot);
    }
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}
