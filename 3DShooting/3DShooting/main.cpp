#include "DxLib.h"
#include "game.h"
#include "SceneManager.h"
#include "EffekseerForDXLib.h"

// プログラムは WinMain から始まります
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// フルスクリーンではなく、ウインドウモードで開くようにする
	ChangeWindowMode(Game::kDefaultWindowMode);
	// 画面のサイズを変更する
	SetGraphMode(Game::kScreenWidth, Game::kScreenHeigth, Game:: kColorBitNum);

	if (DxLib_Init() == -1)		// ＤＸライブラリ初期化処理
	{
		return -1;			// エラーが起きたら直ちに終了
	}
	// 描画先を裏画面にする
	SetDrawScreen(DX_SCREEN_BACK);

	// 3D関連の設定
	SetUseZBuffer3D(true);	 // 3D描画でZBufferを使用する
	SetWriteZBuffer3D(true); // 3D描画でZBufferに書き込む
	SetUseBackCulling(true); // 裏面カリングを有効にする

	//Effekseer関係初期化
	SetUseDirect3DVersion(DX_DIRECT3D_11);
	// 引数には画面に表示する最大パーティクル数を設定する。
	if (Effekseer_Init(8000) == -1)
	{
		DxLib_End();
		return -1;
	}
	// フルスクリーンウインドウの切り替えでリソースが消えるのを防ぐ
	SetChangeScreenModeGraphicsSystemResetFlag(FALSE);

	// DXライブラリのデバイスロストした時のコールバックを設定する
	Effekseer_SetGraphicsDeviceLostCallbackFunctions();

	SceneManager* pScene = new SceneManager();
	pScene->Init();

	// ゲームループ
	while (ProcessMessage() == 0)	// Windowsが行う処理を待つ必要がある
	{
		// DXライブラリのカメラとEffekseerのカメラを同期する。
		Effekseer_Sync3DSetting();

		// エスケープキーが押されたらループを抜ける
		if (CheckHitKey(KEY_INPUT_RETURN))
		{
			break;
		}

		// 今回のループが始まった時間を覚えておく
		LONGLONG time = GetNowHiPerformanceCount();

		// 画面全体をクリアする
		ClearDrawScreen();


		// ゲームの処理
		pScene->Update();
		pScene->Draw();

		// 画面の切り替わりを待つ必要がある
		ScreenFlip();	// 1/60秒経過するまで待つ

		// FPS(Frame Per Second)60に固定
		while (GetNowHiPerformanceCount() - time < 16667)
		{
		}
	}

	// Effekseerを終了する。
	Effkseer_End();

	DxLib_End();				// ＤＸライブラリ使用の終了処理

	return 0;				// ソフトの終了 
}