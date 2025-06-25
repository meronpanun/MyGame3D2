#include "DxLib.h"
#include "CapsuleCollider.h"
#include "SphereCollider.h"
#include <algorithm>
#include <cmath>

CapsuleCollider::CapsuleCollider(const VECTOR& segmentA, const VECTOR& segmentB, float radius)
    : m_segmentA(segmentA), m_segmentB(segmentB), m_radius(radius)
{
}

CapsuleCollider::~CapsuleCollider()
{
}

void CapsuleCollider::Init()
{
}

void CapsuleCollider::Update()
{
}

bool CapsuleCollider::Intersects(const Collider* other) const
{
    if (!other) return false;

    // CapsuleCollider同士の判定
    const CapsuleCollider* capsule = dynamic_cast<const CapsuleCollider*>(other);
    if (capsule)
    {
        VECTOR d1 = VSub(m_segmentB, m_segmentA);
        VECTOR d2 = VSub(capsule->m_segmentB, capsule->m_segmentA);
        VECTOR r = VSub(m_segmentA, capsule->m_segmentA);

        float a = VDot(d1, d1);
        float e = VDot(d2, d2);
        float f = VDot(d2, r);
        float s = 0.0f, t = 0.0f;
        float c = VDot(d1, r);
        float b = VDot(d1, d2);
        float denom = a * e - b * b;

        if (denom != 0.0f)
        {
            s = std::clamp((b * f - c * e) / denom, 0.0f, 1.0f);
        }
        else
        {
            s = 0.0f;
        }

        t = (b * s + f) / e;
        t = std::clamp(t, 0.0f, 1.0f);

        s = (b * t - c) / a;
        s = std::clamp(s, 0.0f, 1.0f);

        VECTOR p1 = VAdd(m_segmentA, VScale(d1, s));
        VECTOR p2 = VAdd(capsule->m_segmentA, VScale(d2, t));
        float distSq = VDot(VSub(p1, p2), VSub(p1, p2));
        float radiusSum = m_radius + capsule->m_radius;
        return distSq <= radiusSum * radiusSum;
    }

    // SphereColliderとの判定（CapsuleCollider側からSphereColliderを判定する）
    const SphereCollider* sphere = dynamic_cast<const SphereCollider*>(other);
    if (sphere)
    {
        // SphereCollider側で実装しているので、そちらを呼び出す
        return sphere->Intersects(this);
    }

    return false; // 未知のコライダータイプ
}