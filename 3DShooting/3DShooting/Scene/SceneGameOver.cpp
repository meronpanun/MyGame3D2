#include "SceneGameOver.h"
#include "DxLib.h"
#include "SceneTitle.h"
#include "SceneMain.h"
#include "Mouse.h"

namespace
{
    constexpr int kButtonWidth = 220;
    constexpr int kButtonHeight = 60;
    constexpr int kButtonSpacing = 40;
}

SceneGameOver::SceneGameOver(int wave, int killCount, int score)
	: m_wave(wave), m_killCount(killCount), m_score(score)
{
}

SceneGameOver::~SceneGameOver()
{
}

void SceneGameOver::Init()
{
    // マウスカーソルの表示/非表示を設定
    SetMouseDispFlag(true);
}

SceneBase* SceneGameOver::Update()
{
    if (Mouse::IsTriggerLeft())
    {
        int screenW, screenH;
        GetScreenState(&screenW, &screenH, nullptr);
        int centerX = screenW / 2;
        int baseY = screenH / 2 + 180;
        // タイトルに戻るボタン
        int titleBtnX1 = centerX - kButtonWidth - kButtonSpacing/2;
        int titleBtnY1 = baseY;
        int titleBtnX2 = centerX - kButtonSpacing/2;
        int titleBtnY2 = baseY + kButtonHeight;
        // リトライボタン
        int retryBtnX1 = centerX + kButtonSpacing/2;
        int retryBtnY1 = baseY;
        int retryBtnX2 = centerX + kButtonWidth + kButtonSpacing/2;
        int retryBtnY2 = baseY + kButtonHeight;
        Vec2 mousePos = Mouse::GetPos();
        if (mousePos.x >= titleBtnX1 && mousePos.x <= titleBtnX2 && mousePos.y >= titleBtnY1 && mousePos.y <= titleBtnY2)
        {
            return new SceneTitle(true);
        }
        if (mousePos.x >= retryBtnX1 && mousePos.x <= retryBtnX2 && mousePos.y >= retryBtnY1 && mousePos.y <= retryBtnY2)
        {
            return new SceneMain();
        }
    }
    return nullptr;
}

void SceneGameOver::Draw()
{
    int screenW, screenH;
    GetScreenState(&screenW, &screenH, nullptr);
    SetFontSize(48);
    DrawString(screenW / 2 - 200, screenH / 2 - 100, "Game Over", 0xff0000);
    SetFontSize(36);
    char waveStr[64];
    sprintf_s(waveStr, sizeof(waveStr), "到達ウェーブ: %d", m_wave);
    DrawString(screenW / 2 - 150, screenH / 2 - 20, waveStr, 0xffffff);
    char killStr[64];
    sprintf_s(killStr, sizeof(killStr), "倒した敵の数: %d", m_killCount);
    DrawString(screenW / 2 - 150, screenH / 2 + 20, killStr, 0xffffff);
    char scoreStr[64];
    sprintf_s(scoreStr, sizeof(scoreStr), "スコア: %d", m_score);
    DrawString(screenW / 2 - 150, screenH / 2 + 60, scoreStr, 0xffffff);
    SetFontSize(16);
    // ボタン描画
    int centerX = screenW / 2;
    int baseY = screenH / 2 + 180;
    int titleBtnX1 = centerX - kButtonWidth - kButtonSpacing/2;
    int titleBtnY1 = baseY;
    int titleBtnX2 = centerX - kButtonSpacing/2;
    int titleBtnY2 = baseY + kButtonHeight;
    int retryBtnX1 = centerX + kButtonSpacing/2;
    int retryBtnY1 = baseY;
    int retryBtnX2 = centerX + kButtonWidth + kButtonSpacing/2;
    int retryBtnY2 = baseY + kButtonHeight;
    // タイトルボタン
    DrawBox(titleBtnX1, titleBtnY1, titleBtnX2, titleBtnY2, 0x888888, true);
    DrawString(titleBtnX1 + 30, titleBtnY1 + 18, "タイトルに戻る", 0xffffff);
    // リトライボタン
    DrawBox(retryBtnX1, retryBtnY1, retryBtnX2, retryBtnY2, 0x888888, true);
    DrawString(retryBtnX1 + 50, retryBtnY1 + 18, "リトライ", 0xffffff);
}
