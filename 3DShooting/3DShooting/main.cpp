#include "EffekseerWarningSuppress.h"
#include "Game.h"
#include "SceneManager.h"
#include "SceneTitle.h"

namespace
{
    constexpr int       kMaxParticles       = 8000;  // Effekseer の最大パーティクル数
    constexpr long long kFrameIntervalMicros = 16667; // 60FPS 固定の目標フレーム時間（マイクロ秒）
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
#ifdef _DEBUG
    // デバッグコンソールを確保してログ出力を有効化
    AllocConsole();
    FILE* out = nullptr;
    freopen_s(&out, "CON", "w", stdout);
    FILE* in = nullptr;
    freopen_s(&in, "CON", "r", stdin);
#endif

    ChangeWindowMode(Game::IsWindowMode());
    SetMainWindowText(Game::kWindowTitle);
    SetGraphMode(Game::GetScreenWidth(), Game::GetScreenHeight(), Game::GetColorBitNum());
    SetAlwaysRunFlag(true); // 非アクティブ時も処理を継続する

    if (DxLib_Init() == -1)
    {
        return -1;
    }
    SetDrawScreen(DX_SCREEN_BACK);

    // 3D 描画設定
    SetUseZBuffer3D(true);
    SetWriteZBuffer3D(true);
    SetUseBackCulling(true);

    // Effekseer 初期化
    SetUseDirect3DVersion(DX_DIRECT3D_11);
    if (Effekseer_Init(kMaxParticles) == -1)
    {
        DxLib_End();
        return -1;
    }

    // フルスクリーン切り替え時にリソースが消えないようにする
    SetChangeScreenModeGraphicsSystemResetFlag(false);
    Effekseer_SetGraphicsDeviceLostCallbackFunctions();

    SceneManager* pScene = new SceneManager();
    pScene->Init();
    Game::SetSceneManager(pScene);

    // ゲームループ
    while (ProcessMessage() == 0)
    {
        Effekseer_Sync3DSetting(); // DxLib カメラと Effekseer カメラを同期

        if (CheckHitKey(KEY_INPUT_RETURN)) break;

        LONGLONG time = GetNowHiPerformanceCount();

        ClearDrawScreen();

        pScene->Update();
        pScene->Draw();

        ScreenFlip();

        // 60FPS に固定
        while (GetNowHiPerformanceCount() - time < kFrameIntervalMicros)
        {
        }
    }

    delete pScene;

    Effkseer_End();
    DxLib_End();

#ifdef _DEBUG
    fclose(out);
    fclose(in);
    FreeConsole();
#endif

    return 0;
}
