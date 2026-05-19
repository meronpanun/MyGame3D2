#pragma once
#include "DebugMenu.h"
#include "EffekseerWarningSuppress.h"
#include <vector>
#include <string>

/// <summary>
/// デバッグ描画・入力チェック・デバッグウィンドウ管理を提供するユーティリティクラス。
/// インスタンス化せずに static メソッドとして使用する。
/// </summary>
class DebugUtil
{
public:
    /// <summary>
    /// 3D カプセルを描画する
    /// </summary>
    /// <param name="a">頂点 A</param>
    /// <param name="b">頂点 B</param>
    /// <param name="radius">半径</param>
    /// <param name="div">分割数</param>
    /// <param name="color">色</param>
    /// <param name="fill">塗り潰しフラグ</param>
    static void DrawCapsule(const VECTOR& a, const VECTOR& b, float radius, int div, int color, bool fill = false);

    /// <summary>
    /// 3D 球を描画する
    /// </summary>
    /// <param name="center">中心座標</param>
    /// <param name="radius">半径</param>
    /// <param name="div">分割数</param>
    /// <param name="color">色</param>
    /// <param name="fill">塗り潰しフラグ</param>
    static void DrawSphere(const VECTOR& center, float radius, int div, int color, bool fill = false);

    /// <summary>
    /// 2D メッセージを描画する
    /// </summary>
    /// <param name="x">X 座標</param>
    /// <param name="y">Y 座標</param>
    /// <param name="color">色</param>
    /// <param name="msg">メッセージ文字列</param>
    static void DrawMessage(int x, int y, unsigned int color, const std::string& msg);

    /// <summary>
    /// 2D フォーマット文字列を描画する
    /// </summary>
    /// <param name="x">X 座標</param>
    /// <param name="y">Y 座標</param>
    /// <param name="color">色</param>
    /// <param name="format">フォーマット文字列</param>
    /// <param name="...">可変引数</param>
    static void DrawFormat(int x, int y, unsigned int color, const char* format, ...);

    /// <summary>
    /// ロゴスキップキーが押されたかどうかをチェックする
    /// </summary>
    /// <returns>押されたら true</returns>
    static bool IsSkipLogoKeyPressed();

    /// <summary>
    /// デバッグウィンドウの表示/非表示を切り替える（F1 キーで切り替え）
    /// </summary>
    static void ShowDebugWindow();

    /// <summary>
    /// デバッグウィンドウが現在表示されているかどうかを返す
    /// </summary>
    /// <returns>表示中なら true</returns>
    static bool IsDebugWindowVisible();

private:
    static bool      s_isVisible;  // デバッグウィンドウの表示状態
    static DebugMenu s_debugMenu;  // デバッグメニューインスタンス
};
