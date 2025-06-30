#pragma once
#include "Collider.h"

/// <summary>
/// 球体コライダークラス
/// </summary>
class SphereCollider : public Collider
{
public:
    SphereCollider(const VECTOR& center = VGet(0, 0, 0), float radius = 1.0f);
    virtual ~SphereCollider() = default;

    /// <summary>
	/// 当たり判定を行う
    /// </summary>
	/// <param name="other">他のコライダー</param>
	/// <returns>true: 当たっている, false: 当たっていない</returns>
    bool Intersects(const Collider* other) const override;

    /// <summary>
	/// Rayとの当たり判定を行う
    /// </summary>
	/// <param name="rayStart">Rayの始点</param>
    /// <param name="rayEnd"></param>
    /// <param name="outHtPos"></param>
    /// <param name="outHtDistSq"></param>
    /// <returns></returns>
    bool IntersectsRay(const VECTOR& rayStart, const VECTOR& rayEnd, VECTOR& outHitPos, float& outHitDistSq) const override;

    VECTOR GetCenter() const { return m_center; }
    float GetRadius() const { return m_radius; }

    void SetCenter(const VECTOR& center) { m_center = center; }
    void SetRadius(float radius) { m_radius = radius; }

private:
    VECTOR m_center;
    float  m_radius;
};