#include "DxLib.h"
#include "SphereCollider.h"
#include "CapsuleCollider.h" 
#include <cmath> 
#include <algorithm>


SphereCollider::SphereCollider(const VECTOR& center, float radius)
    : m_center(center), m_radius(radius)
{
}

void SphereCollider::Init()
{
}

void SphereCollider::Update()
{
}

bool SphereCollider::Intersects(const Collider* other) const
{
    if (!other) return false;

    // SphereCollider同士の判定
    const SphereCollider* sphere = dynamic_cast<const SphereCollider*>(other);
    if (sphere)
    {
        float dx = m_center.x - sphere->m_center.x;
        float dy = m_center.y - sphere->m_center.y;
        float dz = m_center.z - sphere->m_center.z;
        float distSq = dx * dx + dy * dy + dz * dz;
        float radiusSum = m_radius + sphere->m_radius;
        return distSq <= radiusSum * radiusSum;
    }

    // CapsuleColliderとの判定
    const CapsuleCollider* capsule = dynamic_cast<const CapsuleCollider*>(other);
    if (capsule)
    {
        // 球とカプセルの当たり判定ロジック
        VECTOR capA = capsule->GetSegmentA();
        VECTOR capB = capsule->GetSegmentB();
        float capRadius = capsule->GetRadius();

        VECTOR ab = VSub(capB, capA);
        VECTOR ac = VSub(m_center, capA);

        float abLenSq = VDot(ab, ab);
        float t = 0.0f;

        if (abLenSq > 0.0f)
        {
            t = VDot(ac, ab) / abLenSq;
            t = (std::max)(0.0f, (std::min)(1.0f, t));
        }

        VECTOR closest = VAdd(capA, VScale(ab, t));

        float distSq = VDot(VSub(m_center, closest), VSub(m_center, closest));
        float radiusSum = m_radius + capRadius;

        return distSq <= radiusSum * radiusSum;
    }

    return false; // 未知のコライダータイプ
}