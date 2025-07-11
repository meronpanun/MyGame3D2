#include "DxLib.h"
#include "game.h"
#include "SceneManager.h"
#include "SceneTitle.h"
//#include "EffekseerForDXLib.h"

// プログラムは WinMain から始まります
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	/*コンソールDebug用*/
#ifdef _DEBUG
	AllocConsole();                                        
	FILE* out = 0; freopen_s(&out, "CON", "w", stdout); // stdout
	FILE* in  = 0; freopen_s(&in, "CON", "r", stdin);   // stdin
#endif

	// フルスクリーンではなく、ウインドウモードで開くようにする
	ChangeWindowMode(Game::kDefaultWindowMode);
	// ウインドウのタイトルを設定する
	SetMainWindowText(Game::kWindowTitle);
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

	//int shadowH = MakeShadowMap(1024, 1024); // シャドウマップのグラフを作成
	//auto lightDir = VNorm(VGet(1.0f, -1.0f, 1.0f)); // ライトの方向を設定
	//SetLightDirection(lightDir); // ライトの方向を設定
	//SetShadowMapDrawArea(shadowH, VGet(-100.0f, -100.0f, -100.0f), VGet(100.0f, 100.0f, 100.0f)); // シャドウマップの描画範囲を設定
	//SetShadowMapLightDirection(shadowH, lightDir); // シャドウマップのライト方向を設定

	//Effekseer関係初期化
	SetUseDirect3DVersion(DX_DIRECT3D_11);
	// 引数には画面に表示する最大パーティクル数を設定する。
	/*if (Effekseer_Init(8000) == -1)
	{
		DxLib_End();
		return -1;
	}*/
	// フルスクリーンウインドウの切り替えでリソースが消えるのを防ぐ
	SetChangeScreenModeGraphicsSystemResetFlag(FALSE);

	// DXライブラリのデバイスロストした時のコールバックを設定する
	//Effekseer_SetGraphicsDeviceLostCallbackFunctions();

	SceneManager* pScene = new SceneManager();
	pScene->Init();

	// ゲームループ
	while (ProcessMessage() == 0)	// Windowsが行う処理を待つ必要がある
	{
		// DXライブラリのカメラとEffekseerのカメラを同期する。
		//Effekseer_Sync3DSetting();

		// エスケープキーが押されたらループを抜ける
		if (CheckHitKey(KEY_INPUT_RETURN))
		{
			break;
		}

		// 今回のループが始まった時間を覚えておく
		LONGLONG time = GetNowHiPerformanceCount();

		// 画面全体をクリアする
		ClearDrawScreen();

		//ShadowMap_DrawSetup(shadowH); // シャドウマップの描画設定

		// ゲームの処理
		pScene->Update();
		pScene->Draw();
		//ShadowMap_DrawEnd(); // シャドウマップの描画終了

		// 画面の切り替わりを待つ必要がある
		ScreenFlip();	// 1/60秒経過するまで待つ
		
		//SetUseShadowMap(0, shadowH); // シャドウマップの使用を無効にする
		//pScene->Draw();
		//SetUseShadowMap(0, -1); // シャドウマップの使用を無効にする

		// FPS(Frame Per Second)60に固定
		while (GetNowHiPerformanceCount() - time < 16667)
		{
		}
	}

	// Effekseerを終了する。
	//Effkseer_End();

	DxLib_End();				// ＤＸライブラリ使用の終了処理

#ifdef _DEBUG//コンソールDebug用
	fclose(out); fclose(in); FreeConsole();//コンソール解放
#endif

	return 0;				// ソフトの終了 
}