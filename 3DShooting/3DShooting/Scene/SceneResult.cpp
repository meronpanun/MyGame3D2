#include "SceneResult.h"
#include "ScoreManager.h"
#include "DxLib.h"
#include "SceneMain.h"

namespace
{

}

SceneResult::SceneResult()
{
}

SceneResult::~SceneResult()
{
}

void SceneResult::Init()
{
}

SceneBase* SceneResult::Update()
{
	return nullptr;
}

void SceneResult::Draw()
{
    int screenW, screenH;
    GetScreenState(&screenW, &screenH, nullptr);
    SetFontSize(48);
    DrawString(screenW / 2 - 200, screenH / 2 - 50, "ゲームクリア！", 0x00ff00);
    SetFontSize(36);
    char scoreStr[64];
    sprintf_s(scoreStr, sizeof(scoreStr), "合計スコア: %d", ScoreManager::Instance().GetTotalScore());
    DrawString(screenW / 2 - 150, screenH / 2 + 20, scoreStr, 0xffffff);

    // 敵撃破数
    int killCount = ScoreManager::Instance().GetBodyKillCount() + ScoreManager::Instance().GetHeadKillCount();
    char killStr[64];
    sprintf_s(killStr, sizeof(killStr), "倒した敵の数: %d", killCount);
    DrawString(screenW / 2 - 150, screenH / 2 + 60, killStr, 0xffffff);

    // クリアタイム
    char timeStr[64];
    sprintf_s(timeStr, sizeof(timeStr), "クリアタイム: %.1f秒", SceneMain::GetElapsedTime());
    DrawString(screenW / 2 - 150, screenH / 2 + 100, timeStr, 0xffffff);
    SetFontSize(16);
}
