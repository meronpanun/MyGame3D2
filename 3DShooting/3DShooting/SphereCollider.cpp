#include "EffekseerWarningSuppress.h"
#include "SphereCollider.h"
#include "CapsuleCollider.h"
#include <algorithm>
#include <cmath>

namespace
{
    constexpr float kRayLengthSqEpsilon = 0.0001f; // レイ長さの二乗がこれ未満なら退化レイと見なす
    constexpr float kSegmentLenSqEpsilon = 0.0f;   // 線分長さの二乗がこれより大きい場合に射影を計算する
}

SphereCollider::SphereCollider(const VECTOR& center, float radius)
    : m_center(center)
    , m_radius(radius)
{
}

bool SphereCollider::IsIntersects(const Collider* other) const
{
    if (!other) return false;

    // 球同士の判定
    const SphereCollider* sphere = dynamic_cast<const SphereCollider*>(other);
    if (sphere)
    {
        float dx     = m_center.x - sphere->m_center.x;
        float dy     = m_center.y - sphere->m_center.y;
        float dz     = m_center.z - sphere->m_center.z;
        float distSq = dx * dx + dy * dy + dz * dz;
        float radiusSum = m_radius + sphere->m_radius;
        return distSq <= radiusSum * radiusSum;
    }

    // 球とカプセルの判定
    const CapsuleCollider* capsule = dynamic_cast<const CapsuleCollider*>(other);
    if (capsule)
    {
        VECTOR capA     = capsule->GetSegmentA();
        VECTOR capB     = capsule->GetSegmentB();
        float capRadius = capsule->GetRadius();

        // カプセル軸線分 AB 上で球の中心に最も近い点を求める
        VECTOR ab     = VSub(capB, capA);
        VECTOR ac     = VSub(m_center, capA);
        float abLenSq = VDot(ab, ab);
        float t       = 0.0f;

        if (abLenSq > kSegmentLenSqEpsilon)
        {
            t = VDot(ac, ab) / abLenSq;
            t = (std::max)(0.0f, (std::min)(1.0f, t)); // [0, 1] にクランプ
        }

        VECTOR closest  = VAdd(capA, VScale(ab, t));
        float distSq    = VDot(VSub(m_center, closest), VSub(m_center, closest));
        float radiusSum = m_radius + capRadius;
        return distSq <= radiusSum * radiusSum;
    }

    return false; // 未対応のコライダー型
}

bool SphereCollider::IsIsIntersectsRay(const VECTOR& rayStart, const VECTOR& rayEnd, VECTOR& outHitPos, float& outHitDistSq) const
{
    VECTOR rayDir     = VSub(rayEnd, rayStart);
    float rayLengthSq = VDot(rayDir, rayDir);

    // 退化レイ（長さがほぼゼロ）の場合：始点が球内にあればヒット
    if (rayLengthSq < kRayLengthSqEpsilon)
    {
        float distSq = VDot(VSub(m_center, rayStart), VSub(m_center, rayStart));
        if (distSq <= m_radius * m_radius)
        {
            outHitPos    = rayStart;
            outHitDistSq = 0.0f;
            return true;
        }
        return false;
    }

    // 二次方程式 at² + bt + c = 0 を解く（レイと球の交差判定）
    VECTOR oc        = VSub(rayStart, m_center);
    float a          = rayLengthSq;
    float b          = 2.0f * VDot(oc, rayDir);
    float c          = VDot(oc, oc) - m_radius * m_radius;
    float discriminant = b * b - 4.0f * a * c;

    if (discriminant < 0)
    {
        return false; // 交差なし
    }

    float t  = (-b - sqrtf(discriminant)) / (2.0f * a); // 手前の交点パラメータ
    float t1 = (-b + sqrtf(discriminant)) / (2.0f * a); // 奥の交点パラメータ

    // レイ範囲 [0, 1] 内で最も手前の交点を採用する
    if (t < 0.0f || t > 1.0f)
    {
        t = t1;
        if (t < 0.0f || t > 1.0f)
        {
            return false; // どちらの交点もレイ範囲外
        }
    }

    outHitPos    = VAdd(rayStart, VScale(rayDir, t));
    outHitDistSq = VDot(VSub(outHitPos, rayStart), VSub(outHitPos, rayStart));
    return true;
}
