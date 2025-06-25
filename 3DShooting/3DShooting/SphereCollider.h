#pragma once
#include "Collider.h"

class SphereCollider : public Collider
{
public:
    SphereCollider(const VECTOR& center = VGet(0, 0, 0), float radius = 1.0f);
    virtual ~SphereCollider() = default;

    void Init() override;
    void Update() override;

    bool Intersects(const Collider* other) const override;
    bool IntersectsRay(const VECTOR& rayStart, const VECTOR& rayEnd, VECTOR& out_hitPos, float& out_hitDistSq) const override;

    VECTOR GetCenter() const { return m_center; }
    float GetRadius() const { return m_radius; }

    void SetCenter(const VECTOR& center) { m_center = center; }
    void SetRadius(float radius) { m_radius = radius; }

private:
    VECTOR m_center;
    float  m_radius;
};