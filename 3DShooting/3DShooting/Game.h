#pragma once
#include <vector>

class Player;
class SceneManager;

/// <summary>
/// ゲームの基本情報を定義するクラス
/// </summary>
class Game
{
public:
    // 画面情報を定数定義
    //static constexpr int kScreenWidth  = 1920;
    //static constexpr int kScreenHeigth = 1080;
    static constexpr int kScreenWidth  = 1280;
    static constexpr int kScreenHeigth = 720;
    //static constexpr int kScreenWidth  = 640;
    //static constexpr int kScreenHeigth = 480;
    static constexpr int kColorBitNum  = 32;

    static constexpr bool kDefaultWindowMode = true;

	// ウインドウのタイトル
	static constexpr const char* kWindowTitle = "WAVEBREAKER";

    // グローバルなカメラ感度
    static float g_cameraSensitivity;

    // プレイヤーへのポインタ
    static Player* m_pPlayer;

    static SceneManager* m_pSceneManager; 

    // タイムスケール関連
    static float g_timeScale;
    static float g_targetTimeScale;
    static float g_timeScaleDuration;
    static float g_timeScaleTimer;
    static float g_initialTimeScale;  // 追加：補間開始時のスケール

    static void SetTimeScale(float scale, float duration);
    static void UpdateTimeScale();
    static float GetTimeScale();
};
