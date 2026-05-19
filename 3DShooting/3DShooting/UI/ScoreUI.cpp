#include "ScoreUI.h"
#include "Game.h"
#include "ScoreManager.h"
#include <cstdarg>

namespace
{
    // スコアポップアップ関連
    constexpr float kScorePopupDuration  = 120.0f; // ポップアップの表示フレーム数
    constexpr int   kScorePopupX         =  100;   // ポップアップ表示X座標オフセット（画面中央から）
    constexpr int   kScorePopupY         =  -50;   // ポップアップ表示Y座標オフセット（画面中央から）
    constexpr float kFadeOutDuration     =  20.0f; // フェードアウト開始タイマー閾値（フレーム）
    constexpr float kTimerDecrement      =   1.0f; // 1フレームあたりのタイマー減算量
    constexpr int   kPopupLineHeight     =  32;    // ポップアップ行間の基準高さ
    constexpr float kPopupLineSpacing    =   1.2f; // 行間スペーシング倍率

    // コンボアニメーション関連
    constexpr float kComboPulseInitial      =  1.3f;  // コンボ増加時の初期パルススケール
    constexpr float kComboAnimDuration      = 20.0f;  // コンボ増加演出の持続フレーム数
    constexpr float kComboBaseScalePerCount =  0.02f; // コンボ数1当たりのベーススケール増加量
    constexpr float kComboBaseScaleMax      =  1.8f;  // コンボベーススケールの上限
    constexpr float kComboPulseLerpSpeed    =  0.2f;  // パルスがベーススケールへ戻る速度

    // コンボゲージ・表示関連
    constexpr int   kComboMinDisplay    =   2;   // コンボUIを表示する最小コンボ数
    constexpr int   kComboYAbovePopup   =  80;   // コンボ表示がポップアップ基点より上にある量
    constexpr int   kComboBarWidth      = 150;   // コンボゲージの幅
    constexpr int   kComboBarHeight     =   6;   // コンボゲージの高さ
    constexpr int   kComboBarYOffset    =  45;   // コンボゲージのテキストからのYオフセット

    // コンボ色変化の閾値
    constexpr int kComboColorOrangeThreshold =  8; // オレンジ色になるコンボ数
    constexpr int kComboColorYellowThreshold =  3; // 黄色になるコンボ数
    constexpr int kComboColorCyanThreshold   =  1; // 水色になるコンボ数

    // コンボ警告アニメーション関連
    constexpr float kComboWarnThreshold   = 0.5f;  // 警告アニメーション開始のタイマー割合
    constexpr float kComboBarWarnThreshold= 0.3f;  // ゲージ色を赤に変えるタイマー割合
    constexpr float kComboWarnPulseMax    = 0.2f;  // 警告時パルスの最大値
    constexpr int   kComboWarnBlinkPeriod = 400;   // 警告点滅の周期（ミリ秒）
    constexpr float kComboWarnBlendBase   = 0.3f;  // 警告カラーブレンドのベース値
    constexpr float kComboWarnBlendPulse  = 0.7f;  // 警告カラーブレンドのパルス係数

    // コンボテキスト描画関連
    constexpr int kShadowOffset = 2; // 影のオフセット（固定・スケール前）

    // 色関連
    constexpr unsigned int kColorTotalScore  = 0x00ffcc; // 合計スコアポップアップ色（シアン）
    constexpr unsigned int kColorHeadShot    = 0xffd700; // ヘッドショットポップアップ色（金色）
    constexpr unsigned int kColorBodyShot    = 0xeeeeee; // ボディショットポップアップ色（薄灰白色）
    constexpr unsigned int kColorBarBg       = 0x333333; // コンボゲージ背景色
    constexpr unsigned int kColorComboNormal = 0xffffff; // コンボ通常色（白）
    constexpr unsigned int kColorComboCyan   = 0x00ffff; // コンボ1以上の色（水色）
    constexpr unsigned int kColorComboYellow = 0xffff00; // コンボ3以上の色（黄色）
    constexpr unsigned int kColorComboOrange = 0xffaa00; // コンボ8以上の色（オレンジ）
    constexpr unsigned int kColorComboBarWarn= 0xff3333; // ゲージ警告色（赤）
    constexpr unsigned int kColorComboWarn   = 0xff1e1e; // テキスト警告目標色（R=255, G=B=30）
    constexpr unsigned int kColorShadow      = 0x000000; // 影の色
    constexpr int          kAlphaMax         =      255; // アルファ最大値

    // フォント関連
    constexpr char kFontName[]  = "Arial Black";
    constexpr int  kScoreFontSize = 24;
    constexpr int  kFontThickness =  4;
    constexpr int  kFontType      = DX_FONTTYPE_ANTIALIASING_EDGE_8X8;
}

ScoreUI::ScoreUI()
    : m_totalScorePopupValue(0)
    , m_totalScorePopupTimer(0.0f)
    , m_scoreFont(-1)
    , m_prevCombo(0)
    , m_comboPulseScale(1.0f)
    , m_comboAnimTimer(0.0f)
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
    m_scoreFont = CreateFontToHandle(kFontName, kScoreFontSize, kFontThickness, kFontType);

    // シーン遷移時に前回の状態が残らないようリセット
    ScoreManager::Instance().ResetAll();
    m_prevCombo          = 0;
    m_comboPulseScale    = 1.0f;
    m_comboAnimTimer     = 0.0f;
    m_popups.clear();
    m_totalScorePopupValue = 0;
    m_totalScorePopupTimer = 0.0f;
}

void ScoreUI::Update(float deltaTime)
{
    float timeScale = Game::GetTimeScale();

    // ポップアップタイマー更新
    for (auto it = m_popups.begin(); it != m_popups.end(); )
    {
        it->timer -= kTimerDecrement * timeScale;
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
        m_totalScorePopupTimer -= kTimerDecrement * timeScale;
        if (m_totalScorePopupTimer <= 0)
        {
            m_totalScorePopupValue = 0;
        }
    }

    // コンボアニメーション更新
    int currentCombo = ScoreManager::Instance().GetCombo();
    if (currentCombo > m_prevCombo && currentCombo > 0)
    {
        // コンボが増えた瞬間の演出
        m_comboPulseScale = kComboPulseInitial;
        m_comboAnimTimer  = kComboAnimDuration;
    }
    m_prevCombo = currentCombo;

    if (currentCombo > 0)
    {
        // ベーススケール（コンボ数に応じてじわじわ大きくする）
        float baseScale = 1.0f + (currentCombo * kComboBaseScalePerCount);
        if (baseScale > kComboBaseScaleMax) baseScale = kComboBaseScaleMax;

        // パルス（増えた瞬間の膨らみ）をベーススケールへ戻していく
        m_comboPulseScale += (baseScale - m_comboPulseScale) * kComboPulseLerpSpeed * timeScale;

        if (m_comboAnimTimer > 0) m_comboAnimTimer -= kTimerDecrement * timeScale;
    }
    else
    {
        m_comboPulseScale = 1.0f;
        m_comboAnimTimer  = 0.0f;
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
    if (m_popups.empty() && m_totalScorePopupTimer <= 0 && ScoreManager::Instance().GetCombo() < kComboMinDisplay) return;

    float scale = Game::GetUIScale();
    int scaledPopupOffsetY = static_cast<int>(kPopupLineHeight * kPopupLineSpacing * scale);

    int popupBaseX = static_cast<int>(Game::GetScreenWidth()  * 0.5f) + static_cast<int>(kScorePopupX * scale);
    int popupBaseY = static_cast<int>(Game::GetScreenHeight() * 0.5f) + static_cast<int>(kScorePopupY * scale);

    int idx = 0;
    float lastComboRate  = ScoreManager::Instance().GetLastComboRate();
    int   displayCombo   = (ScoreManager::Instance().GetCombo() > 1) ? ScoreManager::Instance().GetCombo() : 1;

    // 合計スコアポップアップのフェードアウト
    int alpha = kAlphaMax;
    if (m_totalScorePopupTimer < kFadeOutDuration)
    {
        alpha = static_cast<int>(kAlphaMax * m_totalScorePopupTimer / kFadeOutDuration);
    }
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

    auto DrawShadowText = [&](int x, int y, unsigned int color, const char* format, ...) {
        va_list args;
        va_start(args, format);
        char buf[256];
        vsprintf_s(buf, format, args);
        va_end(args);
        DrawStringToHandle(x + kShadowOffset, y + kShadowOffset, buf, kColorShadow, m_scoreFont);
        DrawStringToHandle(x, y, buf, color, m_scoreFont);
    };

    // 合計スコア
    if (m_totalScorePopupTimer > 0)
    {
        if (lastComboRate > 1.0f)
        {
            DrawShadowText(popupBaseX, popupBaseY + idx * scaledPopupOffsetY, kColorTotalScore, "+%d (x%.2f)", m_totalScorePopupValue, lastComboRate);
        }
        else
        {
            DrawShadowText(popupBaseX, popupBaseY + idx * scaledPopupOffsetY, kColorTotalScore, "+%d", m_totalScorePopupValue);
        }
        idx++;
    }

    // 内訳（ヘッドショット/ボディショット別）
    if (!m_popups.empty())
    {
        int lastIsHeadShot = -1;
        for (const auto& popup : m_popups)
        {
            if (lastIsHeadShot == -1 || lastIsHeadShot != static_cast<int>(popup.isHeadShot))
            {
                if (popup.timer < kFadeOutDuration)
                {
                    int detailAlpha = static_cast<int>(alpha * popup.timer / kFadeOutDuration);
                    SetDrawBlendMode(DX_BLENDMODE_ALPHA, detailAlpha);
                }

                if (popup.isHeadShot)
                {
                    DrawShadowText(popupBaseX, popupBaseY + idx * scaledPopupOffsetY, kColorHeadShot, "200pt HEADSHOT x%d", displayCombo);
                }
                else
                {
                    DrawShadowText(popupBaseX, popupBaseY + idx * scaledPopupOffsetY, kColorBodyShot, "100pt ZOMBIE KILL x%d", displayCombo);
                }
                idx++;
                SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
            }
            lastIsHeadShot = static_cast<int>(popup.isHeadShot);
        }
    }
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    // --- コンボ情報の描画（2コンボ以上で表示） ---
    int comboCount = ScoreManager::Instance().GetCombo();
    if (comboCount >= kComboMinDisplay)
    {
        int comboX = static_cast<int>(Game::GetScreenWidth()  * 0.5f) + static_cast<int>(kScorePopupX * scale);
        int comboY = static_cast<int>(Game::GetScreenHeight() * 0.5f) + static_cast<int>((kScorePopupY - kComboYAbovePopup) * scale);

        // 1. タイマー比率とアニメーションの計算
        int   timer      = ScoreManager::Instance().GetComboTimer();
        float timerRatio = static_cast<float>(timer) / ScoreManager::kComboGraceFrame;

        // コンボ数に応じた基本色
        unsigned int comboColor = kColorComboNormal;
        if      (comboCount >= kComboColorOrangeThreshold) comboColor = kColorComboOrange;
        else if (comboCount >= kComboColorYellowThreshold) comboColor = kColorComboYellow;
        else if (comboCount >= kComboColorCyanThreshold)   comboColor = kColorComboCyan;

        unsigned int barColor = (timerRatio > kComboBarWarnThreshold) ? kColorComboCyan : kColorComboBarWarn;

        float warningPulse = 0.0f;
        if (timerRatio < kComboWarnThreshold)
        {
            float intensity  = (kComboWarnThreshold - timerRatio) / kComboWarnThreshold;
            float blinkPhase = static_cast<float>(GetNowCount() % kComboWarnBlinkPeriod) / kComboWarnBlinkPeriod;
            float t          = (sinf(blinkPhase * DX_PI_F * 2.0f) + 1.0f) * 0.5f;

            warningPulse = t * kComboWarnPulseMax * intensity;

            // テキスト色を警告色（赤）へ近づける
            int baseR   = (comboColor      >> 16) & 0xFF;
            int baseG   = (comboColor      >> 8)  & 0xFF;
            int baseB   =  comboColor              & 0xFF;
            int targetR = (kColorComboWarn >> 16) & 0xFF;
            int targetG = (kColorComboWarn >> 8)  & 0xFF;
            int targetB =  kColorComboWarn         & 0xFF;

            float blend = (t * kComboWarnBlendPulse + kComboWarnBlendBase) * intensity;
            if (blend > 1.0f) blend = 1.0f;

            int r = static_cast<int>(baseR + (targetR - baseR) * blend);
            int g = static_cast<int>(baseG + (targetG - baseG) * blend);
            int b = static_cast<int>(baseB + (targetB - baseB) * blend);
            comboColor = GetColor(r, g, b);

            // ゲージ色もインテンシティに応じて赤みを増す
            barColor = GetColor(kAlphaMax,
                static_cast<int>(kAlphaMax * (1.0f - intensity)),
                static_cast<int>(kAlphaMax * (1.0f - intensity)));
        }

        // 2. コンボゲージ描画
        int barW = static_cast<int>(kComboBarWidth  * scale);
        int barH = static_cast<int>(kComboBarHeight * scale);
        int barX = comboX;
        int barY = comboY + static_cast<int>(kComboBarYOffset * scale);

        DrawBox(barX, barY, barX + barW, barY + barH, kColorBarBg, true);
        DrawBox(barX, barY, barX + static_cast<int>(barW * timerRatio), barY + barH, barColor, true);

        // 3. コンボ数テキスト描画（影付き・スケール対応）
        char comboBuf[64];
        sprintf_s(comboBuf, "%d COMBO", comboCount);
        float currentScale = m_comboPulseScale + warningPulse;
        int   textWidth    = GetDrawStringWidthToHandle(comboBuf, static_cast<int>(strlen(comboBuf)), m_scoreFont);
        int   drawX        = comboX - static_cast<int>(textWidth    * (currentScale - 1.0f) * 0.5f);
        int   drawY        = comboY - static_cast<int>(kScoreFontSize * (currentScale - 1.0f) * 0.5f);

        SetDrawMode(DX_DRAWMODE_BILINEAR);
        int shadowOffset = static_cast<int>(kShadowOffset * currentScale);
        DrawRotaStringToHandle(drawX + shadowOffset, drawY + shadowOffset,
            currentScale, currentScale, 0.0, 0.0, 0.0, kColorShadow, m_scoreFont, 0, false, comboBuf);
        DrawRotaStringToHandle(drawX, drawY,
            currentScale, currentScale, 0.0, 0.0, 0.0, comboColor, m_scoreFont, 0, false, comboBuf);
        SetDrawMode(DX_DRAWMODE_NEAREST);
    }
}
