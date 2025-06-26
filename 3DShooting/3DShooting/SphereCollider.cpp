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

bool SphereCollider::IntersectsRay(const VECTOR& rayStart, const VECTOR& rayEnd, VECTOR& out_hitPos, float& out_hitDistSq) const
{
    VECTOR rayDir = VSub(rayEnd, rayStart);
    float rayLengthSq = VDot(rayDir, rayDir);

    // レイの長さが非常に短い、またはゼロの場合
    if (rayLengthSq < 0.0001f)
    {
        // レイの始点と球の中心の距離を計算
        float distSq = VDot(VSub(m_center, rayStart), VSub(m_center, rayStart));
        // 始点が球内部にあればヒット
        if (distSq <= m_radius * m_radius)
        {
            out_hitPos = rayStart;
            out_hitDistSq = 0.0f;
            return true;
        }
        return false;
    }

    VECTOR oc = VSub(rayStart, m_center);
    float a = rayLengthSq; // VDot(rayDir, rayDir)
    float b = 2.0f * VDot(oc, rayDir);
    float c = VDot(oc, oc) - m_radius * m_radius;
    float discriminant = b * b - 4 * a * c;

    if (discriminant < 0)
    {
        return false; // 交差しない
    }
    else
    {
        float t = (-b - std::sqrt(discriminant)) / (2.0f * a);
        float t1 = (-b + std::sqrt(discriminant)) / (2.0f * a); // 二つ目の交点

        // Rayの範囲 (0から1) 内で、最も近い交点を探す
        if (t < 0.0f || t > 1.0f) // 最初の交点が範囲外
        {
            t = t1; // 二つ目の交点を試す
            if (t < 0.0f || t > 1.0f) // 二つ目の交点も範囲外
            {
                return false; // どちらの交点もRayの範囲外
            }
        }

        // レイが球の内部から始まり、外側へ向かう場合も考慮
        // tが負であっても、始点が球の内部にあり、t*t*rayLengthSqが半径の二乗より小さければ、
        // そのまま計算を進めても問題ない。
        // ただし、Rayの線分としての衝突を正確に扱うにはtを[0,1]にクランプする必要がある。
        // ここでは、tを[0,1]の範囲に限定し、その範囲内の最初のヒットを見つける

        out_hitPos = VAdd(rayStart, VScale(rayDir, t));
        out_hitDistSq = VDot(VSub(out_hitPos, rayStart), VSub(out_hitPos, rayStart));
        return true;
    }
}