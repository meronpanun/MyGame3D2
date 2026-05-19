#pragma once
#include "EffekseerWarningSuppress.h"

/// <summary>
/// コライダーの基底クラス。
/// カプセル・球など各形状のコライダーはこのクラスを継承し、
/// 当たり判定メソッドをオーバーライドして実装する。
/// </summary>
class Collider
{
public:
    Collider() = default;
    virtual ~Collider() = default;

    /// <summary>
    /// 他のコライダーと交差しているかどうかを判定する
    /// </summary>
    /// <param name="other">判定対象のコライダー</param>
    /// <returns>交差していれば true</returns>
    virtual bool IsIntersects(const Collider* other) const = 0;

    /// <summary>
    /// Ray との交差判定を行う
    /// </summary>
    /// <param name="rayStart">Ray の始点</param>
    /// <param name="rayEnd">Ray の終点</param>
    /// <param name="outHtPos">交差位置（出力）</param>
    /// <param name="outHtDistSq">交差位置までの距離の二乗（出力）</param>
    /// <returns>交差していれば true</returns>
    virtual bool IsIsIntersectsRay(const VECTOR& rayStart, const VECTOR& rayEnd, VECTOR& outHtPos, float& outHtDistSq) const = 0;
};
