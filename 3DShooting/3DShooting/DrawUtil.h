#pragma once

/// <summary>
/// DxLib の低レベル描画 API を使った汎用描画ユーティリティ関数群。
/// </summary>
namespace DrawUtil
{
    /// <summary>
    /// 上から下へ線形グラデーションした矩形をポリゴン描画する。
    /// </summary>
    /// <param name="x1">左端 X 座標</param>
    /// <param name="y1">上端 Y 座標</param>
    /// <param name="x2">右端 X 座標</param>
    /// <param name="y2">下端 Y 座標</param>
    /// <param name="topColor">上辺のカラー（0xRRGGBB 形式）</param>
    /// <param name="bottomColor">下辺のカラー（0xRRGGBB 形式）</param>
    void DrawGradientBox(int x1, int y1, int x2, int y2,
                         unsigned int topColor, unsigned int bottomColor);
}
