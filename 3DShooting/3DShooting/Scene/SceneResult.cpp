#include "SceneResult.h"
#include "ScoreManager.h"
#include "DxLib.h"
#include "SceneMain.h"
#include "SceneTitle.h"
#include "Mouse.h"

namespace
{
    constexpr int kButtonWidth = 220;
    constexpr int kButtonHeight = 60;
    constexpr int kButtonSpacing = 40;
}

SceneResult::SceneResult()
{
}

SceneResult::~SceneResult()
{
}

void SceneResult::Init()
{
    // マウスカーソルの表示/非表示を設定
    SetMouseDispFlag(true);
    // スコア保存
    ScoreManager::Instance().SaveScore(ScoreManager::Instance().GetTotalScore());
}

SceneBase* SceneResult::Update()
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

void SceneResult::Draw()
{
    int screenW, screenH;
    GetScreenState(&screenW, &screenH, nullptr);
    // タイトル
    SetFontSize(32);
    DrawString(screenW / 2 - 100, 30, "ゲームクリア！", 0x00ff00);
    SetFontSize(20);
    int y = 80;
    char scoreStr[64];
    sprintf_s(scoreStr, sizeof(scoreStr), "合計スコア: %d", ScoreManager::Instance().GetTotalScore());
    DrawString(screenW / 2 - 100, y, scoreStr, 0xffffff);
    y += 28;
    int killCount = ScoreManager::Instance().GetBodyKillCount() + ScoreManager::Instance().GetHeadKillCount();
    char killStr[64];
    sprintf_s(killStr, sizeof(killStr), "倒した敵の数: %d", killCount);
    DrawString(screenW / 2 - 100, y, killStr, 0xffffff);
    y += 28;
    char timeStr[64];
    sprintf_s(timeStr, sizeof(timeStr), "クリアタイム: %.1f秒", SceneMain::GetElapsedTime());
    DrawString(screenW / 2 - 100, y, timeStr, 0xffffff);
    y += 36;
    // ハイスコア表示
    SetFontSize(18);
    DrawString(screenW / 2 - 100, y, "--- ハイスコア ---", 0xffff00);
    y += 24;
    const auto& scores = ScoreManager::Instance().GetHighScores();
    for (int i = 0; i < 3 && i < (int)scores.size(); ++i) {
        char highStr[64];
        sprintf_s(highStr, sizeof(highStr), "%d位: %d", i+1, scores[i]);
        DrawString(screenW / 2 - 100, y, highStr, 0xffffff);
        y += 22;
    }
    SetFontSize(16);
    // ボタン描画
    int btnY = screenH - 80;
    int btnW = 120;
    int btnH = 36;
    int btnSpacing = 24;
    int centerX = screenW / 2;
    // タイトルボタン
    int titleBtnX1 = centerX - btnW - btnSpacing/2;
    int titleBtnY1 = btnY;
    int titleBtnX2 = centerX - btnSpacing/2;
    int titleBtnY2 = btnY + btnH;
    DrawBox(titleBtnX1, titleBtnY1, titleBtnX2, titleBtnY2, 0x888888, true);
    DrawString(titleBtnX1 + 10, titleBtnY1 + 8, "タイトルに戻る", 0xffffff);
    // リトライボタン
    int retryBtnX1 = centerX + btnSpacing/2;
    int retryBtnY1 = btnY;
    int retryBtnX2 = centerX + btnW + btnSpacing/2;
    int retryBtnY2 = btnY + btnH;
    DrawBox(retryBtnX1, retryBtnY1, retryBtnX2, retryBtnY2, 0x888888, true);
    DrawString(retryBtnX1 + 25, retryBtnY1 + 8, "リトライ", 0xffffff);
}
