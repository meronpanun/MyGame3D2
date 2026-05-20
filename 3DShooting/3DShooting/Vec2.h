#pragma once
#include <cmath>

/// <summary>
/// 2 次元浮動小数点ベクトル。
/// 全メソッドをインラインで提供するヘッダーオンリークラス。
/// </summary>
class Vec2
{
public:
    float x = 0.0f; // X 成分
    float y = 0.0f; // Y 成分

    /// <summary>ゼロベクトルで初期化するデフォルトコンストラクタ</summary>
    Vec2() = default;

    /// <summary>各成分を指定するコンストラクタ</summary>
    /// <param name="posX">X 成分</param>
    /// <param name="posY">Y 成分</param>
    Vec2(float posX, float posY) : x(posX), y(posY) {}

    /// <summary>各成分をまとめて設定する</summary>
    /// <param name="a">新しい X 成分</param>
    /// <param name="b">新しい Y 成分</param>
    void SetPos(float a, float b) { x = a; y = b; }

    // --- 算術複合代入演算子 ---

    Vec2& operator+=(const Vec2& v) { x += v.x; y += v.y; return *this; }
    Vec2& operator-=(const Vec2& v) { x -= v.x; y -= v.y; return *this; }
    Vec2& operator*=(float s)       { x *= s;   y *= s;   return *this; }
    Vec2& operator/=(float s)       { x /= s;   y /= s;   return *this; }

    // --- 算術二項演算子 ---

    Vec2 operator+(const Vec2& v) const { return Vec2{ x + v.x, y + v.y }; }
    Vec2 operator-(const Vec2& v) const { return Vec2{ x - v.x, y - v.y }; }
    Vec2 operator*(float s)       const { return Vec2{ x * s,   y * s   }; }
    Vec2 operator/(float s)       const { return Vec2{ x / s,   y / s   }; }

    // --- 比較演算子 ---

    bool operator==(const Vec2& v) const { return x == v.x && y == v.y; }
    bool operator!=(const Vec2& v) const { return !(*this == v); }

    /// <summary>ベクトルの長さ（ノルム）を返す</summary>
    /// <returns>長さ</returns>
    float Length() const { return sqrtf(x * x + y * y); }

    /// <summary>
    /// 正規化したベクトルを返す。
    /// 長さがゼロのときは自身をそのまま返す。
    /// </summary>
    /// <returns>単位ベクトル（長さがゼロなら自身）</returns>
    Vec2 Normalize() const
    {
        float len = Length();
        if (len > 0.0f) return (*this) / len;
        return *this;
    }
};
