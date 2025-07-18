#pragma once
#include <vector>

/// <summary>
/// ゲームの基本情報を定義するクラス
/// </summary>
class Game
{
public:
    // 画面情報を定数定義
    //static constexpr int kScreenWidth  = 1920;
    //static constexpr int kScreenHeigth = 1080;
    //static constexpr int kScreenWidth  = 1280;
    //static constexpr int kScreenHeigth = 720;
    static constexpr int kScreenWidth  = 640;
    static constexpr int kScreenHeigth = 480;
    static constexpr int kColorBitNum  = 32;

    static constexpr bool kDefaultWindowMode = true;

	// ウインドウのタイトル
	static constexpr const char* kWindowTitle = "3DShootingGame";

    // グローバルなカメラ感度
    static float g_cameraSensitivity;
};
