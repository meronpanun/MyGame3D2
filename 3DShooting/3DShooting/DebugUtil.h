#pragma once
#include "DebugMenu.h"
#include "EffekseerForDXLib.h"  
#include <vector>
#include <string>

/// <summary>
/// デバッグクラス
/// </summary>
class DebugUtil
{
public:
    static void DrawCapsule(const VECTOR& a, const VECTOR& b, float radius, int div, int color, bool fill = false);
    static void DrawSphere(const VECTOR& center, float radius, int div, int color, bool fill = false);
    static void DrawMessage(int x, int y, unsigned int color, const std::string& msg);
    static void DrawFormat(int x, int y, unsigned int color, const char* format, ...);
    static bool IsSkipLogoKeyPressed();
    static void ShowDebugWindow();
    static bool IsDebugWindowVisible();

private:
    static bool s_isVisible; // デバッグウィンドウの表示状態
    static DebugMenu s_debugMenu;
};