#pragma once
#include "Collider.h"

/// <summary>
/// カプセル型コライダークラス。
/// 2 点のセグメント（A・B）と半径で定義されるカプセル形状を用いて、
/// カプセル同士・球・Ray との当たり判定を提供する。
/// </summary>
class CapsuleCollider : public Collider
{
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    /// <param name="segmentA">カプセルのセグメント始点（デフォルト: 原点）</param>
    /// <param name="segmentB">カプセルのセグメント終点（デフォルト: (0,1,0)）</param>
    /// <param name="radius">カプセルの半径（デフォルト: 1.0）</param>
    CapsuleCollider(const VECTOR& segmentA = VGet(0, 0, 0), const VECTOR& segmentB = VGet(0, 1, 0), float radius = 1.0f);
    virtual ~CapsuleCollider() = default;

    /// <summary>
    /// 他のコライダーと交差しているかどうかを判定する
    /// </summary>
    /// <param name="other">判定対象のコライダー</param>
    /// <returns>交差していれば true</returns>
    bool IsIntersects(const Collider* other) const override;

    /// <summary>
    /// Ray との交差判定を行う
    /// </summary>
    /// <param name="rayStart">Ray の始点</param>
    /// <param name="rayEnd">Ray の終点</param>
    /// <param name="outHitPos">交差位置（出力）</param>
    /// <param name="outHitDistSq">交差位置までの距離の二乗（出力）</param>
    /// <returns>交差していれば true</returns>
    bool IsIsIntersectsRay(const VECTOR& rayStart, const VECTOR& rayEnd, VECTOR& outHitPos, float& outHitDistSq) const override;

    /// <summary>
    /// カプセルのセグメント始点を取得する
    /// </summary>
    /// <returns>セグメント始点 A</returns>
    VECTOR GetSegmentA() const { return m_segmentA; }

    /// <summary>
    /// カプセルのセグメント終点を取得する
    /// </summary>
    /// <returns>セグメント終点 B</returns>
    VECTOR GetSegmentB() const { return m_segmentB; }

    /// <summary>
    /// カプセルの半径を取得する
    /// </summary>
    /// <returns>カプセルの半径</returns>
    float GetRadius() const { return m_radius; }

    /// <summary>
    /// カプセルのセグメント A・B を設定する
    /// </summary>
    /// <param name="a">セグメント始点 A</param>
    /// <param name="b">セグメント終点 B</param>
    void SetSegment(const VECTOR& a, const VECTOR& b) { m_segmentA = a; m_segmentB = b; }

    /// <summary>
    /// カプセルの半径を設定する
    /// </summary>
    /// <param name="radius">設定する半径</param>
    void SetRadius(float radius) { m_radius = radius; }

private:
    VECTOR m_segmentA; // カプセルのセグメント始点 A
    VECTOR m_segmentB; // カプセルのセグメント終点 B
    float  m_radius;   // カプセルの半径
};
