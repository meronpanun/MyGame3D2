#pragma once
#include "DxLib.h"  
#include <vector>
#include <string>

/// <summary>
/// デバッグユーティリティクラス
/// </summary>
class DebugUtil
{
public:
    /// <summary>
    /// 3Dカプセルのデバッグ描画 
    /// </summary>
	/// <param name="a">カプセルの始点</param>
	/// <param name="b">カプセルの終点</param>
	/// <param name="radius">カプセルの半径</param>
	/// <param name="div">分割数</param>
	/// <param name="color">カプセルの色</param>
	/// <param name="fill">trueならカプセルを塗りつぶす</param>
    static void DrawCapsule(const VECTOR& a, const VECTOR& b, float radius, int div, int color, bool fill = false);

    /// <summary>
	/// 3D球のデバッグ描画  
    /// </summary>
	/// <param name="center">球の中心座標</param>
	/// <param name="radius">球の半径</param>
	/// <param name="div">分割数</param>
	/// <param name="color">球の色</param>
	/// <param name="fill">trueなら球を塗りつぶす</param>
    static void DrawSphere(const VECTOR& center, float radius, int div, int color, bool fill = false);

    /// <summary>
	/// 2Dデバックメッセージを描画する
    /// </summary>
	/// <param name="x">X座標</param>
	/// <param name="y">Y座標</param>
	/// <param name="color">メッセージの色</param>
	/// <param name="msg">メッセージ内容</param>
    static void DrawMessage(int x, int y, unsigned int color, const std::string& msg);

    /// <summary>
	/// 2Dデバックフォーマット文字列を描画する
    /// </summary>
	/// <param name="x">X座標</param>
	/// <param name="y">Y座標</param>
	/// <param name="color">メッセージの色</param>
	/// <param name="format">フォーマット文字列</param>
	/// <param name="">可変引数</param>
    static void DrawFormat(int x, int y, unsigned int color, const char* format, ...);

    /// <summary>
	/// ロゴスキップキーが押されたかどうかをチェックする
    /// </summary>
	/// <returns>trueならスキップキーが押された</returns>
    static bool IsSkipLogoKeyPressed();

    /// <summary>
	/// デバッグウィンドウを表示する
    /// </summary>
    static void ShowDebugWindow();

	static bool IsDebugWindowVisible();

private:
	static bool s_isVisible;
};

