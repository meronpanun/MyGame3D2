#include "DebugUtil.h"
#include "EffekseerWarningSuppress.h"
#include "DebugMenu.h"
#include "Game.h"
#include "SceneManager.h"
#include "SceneMain.h"
#include <cstdarg>

namespace
{
    constexpr int          kFormatBufSize        = 256;      // DrawFormat のバッファサイズ
    constexpr int          kDebugWindowAlpha     = 128;      // デバッグウィンドウ背景の不透明度
    constexpr int          kDebugMenuX           = 40;       // デバッグメニュー描画 X 座標
    constexpr int          kDebugMenuY           = 40;       // デバッグメニュー描画 Y 座標
    constexpr unsigned int kColorDebugBackground = 0x000000; // デバッグウィンドウ背景色
}

bool      DebugUtil::s_isVisible  = false;
DebugMenu DebugUtil::s_debugMenu;

void DebugUtil::DrawCapsule(const VECTOR& a, const VECTOR& b, float radius, int div, int color, bool fill)
{
    DrawCapsule3D(a, b, radius, div, color, color, fill);
}

void DebugUtil::DrawSphere(const VECTOR& center, float radius, int div, int color, bool fill)
{
    DrawSphere3D(center, radius, div, color, color, fill);
}

void DebugUtil::DrawMessage(int x, int y, unsigned int color, const std::string& msg)
{
    DrawString(x, y, msg.c_str(), color);
}

void DebugUtil::DrawFormat(int x, int y, unsigned int color, const char* format, ...)
{
    char    buf[kFormatBufSize];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);
    DrawString(x, y, buf, color);
}

bool DebugUtil::IsSkipLogoKeyPressed()
{
    return CheckHitKey(KEY_INPUT_S) != 0;
}

void DebugUtil::ShowDebugWindow()
{
    // F1 キーが押された瞬間に表示/非表示を切り替える
    static int prevF1 = 0;
    int        nowF1  = CheckHitKey(KEY_INPUT_F1);
    if (nowF1 && !prevF1)
    {
        s_isVisible = !s_isVisible;

        if (s_isVisible)
        {
            // デバッグウィンドウが開く時は必ずマウスを表示
            SetMouseDispFlag(true);
        }
        else
        {
            // デバッグウィンドウを閉じる時は現在のシーンに応じてマウスの表示を決定
            if (Game::GetSceneManager())
            {
                SceneBase* currentScene = Game::GetSceneManager()->GetCurrentScene();
                if (dynamic_cast<SceneMain*>(currentScene))
                {
                    SetMouseDispFlag(false); // ゲーム本体では非表示
                }
                else
                {
                    SetMouseDispFlag(true); // それ以外のシーンでは表示
                }
            }
        }
    }
    prevF1 = nowF1;

    if (!s_isVisible) return;

    // デバッグウィンドウの背景を半透明で描画
    int screenW, screenH;
    GetScreenState(&screenW, &screenH, NULL);

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, kDebugWindowAlpha);
    DrawBox(0, 0, screenW, screenH, kColorDebugBackground, true);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    s_debugMenu.Update();
    s_debugMenu.Draw(kDebugMenuX, kDebugMenuY);
}

bool DebugUtil::IsDebugWindowVisible()
{
    return s_isVisible;
}
