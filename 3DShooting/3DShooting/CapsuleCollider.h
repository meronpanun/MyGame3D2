#pragma once
#include "Collider.h"

/// <summary>
/// カプセル型のコライダー
/// </summary>
class CapsuleCollider : public Collider
{
public:
    CapsuleCollider(const VECTOR& segmentA = VGet(0, 0, 0), const VECTOR& segmentB = VGet(0, 1, 0), float radius = 1.0f);
    virtual ~CapsuleCollider();

    /// <summary>
	/// 他のコライダーと当たっているかどうかを判定する
    /// </summary>
	/// <param name="other">他のコライダー</param>s
	/// <returns>trueなら当たっている</returns>
    bool Intersects(const Collider* other) const override; 

    /// <summary>
	/// Rayとの当たり判定を行う
    /// </summary>
	/// <param name="rayStart">Rayの始点</param>
	/// <param name="rayEnd">Rayの終点</param>
    /// <param name="out_hitPos"></param>
    /// <param name="out_hitDistSq"></param>
    /// <returns></returns>
    bool IntersectsRay(const VECTOR& rayStart, const VECTOR& rayEnd, VECTOR& out_hitPos, float& out_hitDistSq) const override;

    VECTOR GetSegmentA() const { return m_segmentA; }
    VECTOR GetSegmentB() const { return m_segmentB; }
    float GetRadius() const { return m_radius; }

    void SetSegment(const VECTOR& a, const VECTOR& b) { m_segmentA = a; m_segmentB = b; }
    void SetRadius(float radius) { m_radius = radius; }

private:
    VECTOR m_segmentA;
    VECTOR m_segmentB;
    float  m_radius;
};