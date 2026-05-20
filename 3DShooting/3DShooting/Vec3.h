#pragma once
#include "DxLib.h"
#include <cmath>

/// <summary>
/// 3 次元浮動小数点ベクトル。
/// 全メソッドをインラインで提供するヘッダーオンリークラス。
/// DxLib の VECTOR 型との相互変換もサポートする。
/// </summary>
class Vec3
{
public:
    float x = 0.0f; // X 成分
    float y = 0.0f; // Y 成分
    float z = 0.0f; // Z 成分

    /// <summary>ゼロベクトルで初期化するデフォルトコンストラクタ</summary>
    Vec3() = default;

    /// <summary>各成分を指定するコンストラクタ</summary>
    /// <param name="posX">X 成分</param>
    /// <param name="posY">Y 成分</param>
    /// <param name="posZ">Z 成分</param>
    Vec3(float posX, float posY, float posZ) : x(posX), y(posY), z(posZ) {}

    /// <summary>DxLib の VECTOR から変換するコンストラクタ</summary>
    /// <param name="v">変換元の VECTOR</param>
    Vec3(const VECTOR& v) : x(v.x), y(v.y), z(v.z) {}

    /// <summary>各成分をまとめて設定する</summary>
    /// <param name="a">新しい X 成分</param>
    /// <param name="b">新しい Y 成分</param>
    /// <param name="c">新しい Z 成分</param>
    void SetPos(float a, float b, float c) { x = a; y = b; z = c; }

    // --- 算術複合代入演算子 ---

    Vec3& operator+=(const Vec3& v) { x += v.x; y += v.y; z += v.z; return *this; }
    Vec3& operator-=(const Vec3& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
    Vec3& operator*=(float s)       { x *= s;   y *= s;   z *= s;   return *this; }
    Vec3& operator/=(float s)       { x /= s;   y /= s;   z /= s;   return *this; }

    // --- 算術二項演算子 ---

    Vec3 operator+(const Vec3& v) const { return Vec3{ x + v.x, y + v.y, z + v.z }; }
    Vec3 operator-(const Vec3& v) const { return Vec3{ x - v.x, y - v.y, z - v.z }; }
    Vec3 operator*(float s)       const { return Vec3{ x * s,   y * s,   z * s   }; }
    Vec3 operator/(float s)       const { return Vec3{ x / s,   y / s,   z / s   }; }

    // --- 比較演算子 ---

    bool operator==(const Vec3& v) const { return x == v.x && y == v.y && z == v.z; }
    bool operator!=(const Vec3& v) const { return !(*this == v); }

    /// <summary>ベクトルの長さ（ノルム）を返す</summary>
    /// <returns>長さ</returns>
    float Length() const { return sqrtf(x * x + y * y + z * z); }

    /// <summary>
    /// 正規化したベクトルを返す。
    /// 長さがゼロのときは自身をそのまま返す。
    /// </summary>
    /// <returns>単位ベクトル（長さがゼロなら自身）</returns>
    Vec3 Normalize() const
    {
        float len = Length();
        if (len > 0.0f) return (*this) / len;
        return *this;
    }

    /// <summary>DxLib の VECTOR 型に変換して返す</summary>
    /// <returns>同じ成分を持つ VECTOR</returns>
    VECTOR ToDxVECTOR() const { return VGet(x, y, z); }
};
