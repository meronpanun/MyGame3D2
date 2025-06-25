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

    if (rayLengthSq < 0.0001f) // レイの長さが0に近い場合
    {
        // 点と球の判定にフォールバック
        float distSq = VDot(VSub(m_center, rayStart), VSub(m_center, rayStart));
        if (distSq <= m_radius * m_radius)
        {
            out_hitPos = rayStart; // 始点自体が球内部なら始点をヒット位置とする
            out_hitDistSq = 0.0f;
            return true;
        }
        return false;
    }

    VECTOR m = VSub(rayStart, m_center);
    float b = VDot(m, rayDir);
    float c = VDot(m, m) - m_radius * m_radius;

    // レイの始点が球の外側かつレイが球から離れていく場合
    if (c > 0.0f && b > 0.0f)
    {
        return false;
    }

    float discr = b * b - c;

    // 判別式が負の場合、交差しない
    if (discr < 0.0f)
    {
        return false;
    }

    // 交点までの距離 (始点から)
    float t = -b - std::sqrt(discr);

    // 交点がレイの範囲外の場合 (0から1の範囲)
    if (t < 0.0f)
    {
        t = 0.0f; // 始点が球内にある場合、tは負になりうる
    }

    // レイの長さに正規化
    t /= std::sqrt(rayLengthSq);

    // レイの終点を超えている場合
    if (t > 1.0f)
    {
        return false;
    }

    out_hitPos = VAdd(rayStart, VScale(rayDir, t));
    out_hitDistSq = VDot(VSub(out_hitPos, rayStart), VSub(out_hitPos, rayStart));
    return true;
}