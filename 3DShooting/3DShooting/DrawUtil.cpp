#include "DrawUtil.h"
#include "DxLib.h"

void DrawUtil::DrawGradientBox(int x1, int y1, int x2, int y2,
                                unsigned int topColor, unsigned int bottomColor)
{
    VERTEX2D Vertex[6];
    float fx1 = static_cast<float>(x1);
    float fy1 = static_cast<float>(y1);
    float fx2 = static_cast<float>(x2);
    float fy2 = static_cast<float>(y2);

    // 0xRRGGBB 形式から RGB を抽出
    unsigned char topR = (topColor >> 16) & 0xFF;
    unsigned char topG = (topColor >>  8) & 0xFF;
    unsigned char topB =  topColor        & 0xFF;

    unsigned char btmR = (bottomColor >> 16) & 0xFF;
    unsigned char btmG = (bottomColor >>  8) & 0xFF;
    unsigned char btmB =  bottomColor        & 0xFF;

    // 左上
    Vertex[0].pos = VGet(fx1, fy1, 0.0f);
    Vertex[0].rhw = 1.0f;
    Vertex[0].u   = 0.0f;
    Vertex[0].v   = 0.0f;
    Vertex[0].dif = GetColorU8(topR, topG, topB, 255);
    // 右上
    Vertex[1].pos = VGet(fx2, fy1, 0.0f);
    Vertex[1].rhw = 1.0f;
    Vertex[1].u   = 0.0f;
    Vertex[1].v   = 0.0f;
    Vertex[1].dif = GetColorU8(topR, topG, topB, 255);
    // 左下
    Vertex[2].pos = VGet(fx1, fy2, 0.0f);
    Vertex[2].rhw = 1.0f;
    Vertex[2].u   = 0.0f;
    Vertex[2].v   = 0.0f;
    Vertex[2].dif = GetColorU8(btmR, btmG, btmB, 255);

    // 左下（再利用）
    Vertex[3] = Vertex[2];
    // 右上（再利用）
    Vertex[4] = Vertex[1];
    // 右下
    Vertex[5].pos = VGet(fx2, fy2, 0.0f);
    Vertex[5].rhw = 1.0f;
    Vertex[5].u   = 0.0f;
    Vertex[5].v   = 0.0f;
    Vertex[5].dif = GetColorU8(btmR, btmG, btmB, 255);

    DrawPolygon2D(Vertex, 2, DX_NONE_GRAPH, true);
}
