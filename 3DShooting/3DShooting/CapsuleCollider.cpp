#include "EffekseerWarningSuppress.h"
#include "CapsuleCollider.h"
#include "SphereCollider.h"
#include <algorithm>
#include <cmath>
#include <float.h>

namespace
{
    constexpr float kLengthEpsilon = 0.0001f; // 長さがゼロに近いとみなす閾値
}

/// <summary>
/// 線分 pq と点 c の間の最近接点を線分 pq 上で求めて返す
/// </summary>
static VECTOR ClosestPtPointSegment(const VECTOR& c, const VECTOR& p, const VECTOR& q)
{
    VECTOR ab = VSub(q, p);                            // 線分 pq のベクトル
    float  t  = VDot(VSub(c, p), ab) / VDot(ab, ab); // 線分 pq 上への投影パラメータ

    return VAdd(p, VScale(ab, std::clamp(t, 0.0f, 1.0f)));
}

/// <summary>
/// Ray と球の交差判定を行う
/// </summary>
static bool IsIsIntersectsRaySphere(
    const VECTOR& rayStart, const VECTOR& rayEnd,
    const VECTOR& sphereCenter, float sphereRadius,
    VECTOR& outHtPos, float& outHtDistSq)
{
    VECTOR rayDir      = VSub(rayEnd, rayStart); // Ray の方向ベクトル
    float  rayLengthSq = VDot(rayDir, rayDir);   // Ray の長さの二乗

    // Ray の長さがゼロに近い場合は点と球の交差判定を行う
    if (rayLengthSq < kLengthEpsilon)
    {
        float distSq = VDot(VSub(sphereCenter, rayStart), VSub(sphereCenter, rayStart));

        if (distSq <= sphereRadius * sphereRadius)
        {
            outHtPos    = rayStart;
            outHtDistSq = 0.0f;
            return true;
        }
        return false;
    }

    VECTOR oc = VSub(rayStart, sphereCenter); // Ray 始点から球中心へのベクトル

    float a            = rayLengthSq;
    float b            = 2.0f * VDot(oc, rayDir);
    float c            = VDot(oc, oc) - sphereRadius * sphereRadius;
    float discriminant = b * b - 4.0f * a * c; // 判別式

    if (discriminant < 0)
    {
        return false;
    }

    float sqrtD = std::sqrt(discriminant);
    float t0    = (-b - sqrtD) / (2.0f * a);
    float t1    = (-b + sqrtD) / (2.0f * a);

    // Ray 範囲（0〜1）内で最も近い交点を採用する
    float t = t0;
    if (t < 0.0f || t > 1.0f)
    {
        t = t1;
        if (t < 0.0f || t > 1.0f)
        {
            return false;
        }
    }

    outHtPos    = VAdd(rayStart, VScale(rayDir, t));
    outHtDistSq = VDot(VSub(outHtPos, rayStart), VSub(outHtPos, rayStart));
    return true;
}


CapsuleCollider::CapsuleCollider(const VECTOR& segmentA, const VECTOR& segmentB, float radius)
    : m_segmentA(segmentA)
    , m_segmentB(segmentB)
    , m_radius(radius)
{
}

bool CapsuleCollider::IsIntersects(const Collider* other) const
{
    if (!other) return false;

    // カプセル同士の判定
    const CapsuleCollider* capsule = dynamic_cast<const CapsuleCollider*>(other);
    if (capsule)
    {
        // 2 つの線分間の最短距離を求めるロジック
        VECTOR d1    = VSub(m_segmentB, m_segmentA);
        VECTOR d2    = VSub(capsule->m_segmentB, capsule->m_segmentA);
        VECTOR r     = VSub(m_segmentA, capsule->m_segmentA);

        float a     = VDot(d1, d1);
        float e     = VDot(d2, d2);
        float f     = VDot(d2, r);
        float c     = VDot(d1, r);
        float b     = VDot(d1, d2);
        float denom = a * e - b * b;

        float s = (denom != 0.0f) ? std::clamp((b * f - c * e) / denom, 0.0f, 1.0f) : 0.0f;
        float t = (b * s + f) / e;

        // t が範囲外の場合、s を再計算
        if (t < 0.0f)
        {
            t = 0.0f;
            s = std::clamp(c / a, 0.0f, 1.0f);
        }
        else if (t > 1.0f)
        {
            t = 1.0f;
            s = std::clamp((c + b) / a, 0.0f, 1.0f);
        }

        // 最近接点間の距離を計算し、半径の和と比較する
        VECTOR p1        = VAdd(m_segmentA, VScale(d1, s));
        VECTOR p2        = VAdd(capsule->m_segmentA, VScale(d2, t));
        float  distSq    = VDot(VSub(p1, p2), VSub(p1, p2));
        float  radiusSum = m_radius + capsule->m_radius;

        return distSq <= radiusSum * radiusSum;
    }

    // SphereCollider との判定（SphereCollider 側で実装済みのため委譲）
    const SphereCollider* sphere = dynamic_cast<const SphereCollider*>(other);
    if (sphere)
    {
        return sphere->IsIntersects(this);
    }

    return false;
}

bool CapsuleCollider::IsIsIntersectsRay(const VECTOR& rayStart, const VECTOR& rayEnd, VECTOR& outHitPos, float& outHitDistSq) const
{
    VECTOR rayDir      = VSub(rayEnd, rayStart);
    float  rayLengthSq = VDot(rayDir, rayDir);

    // Ray の長さがゼロに近い場合は点とカプセルの交差判定を行う
    if (rayLengthSq < kLengthEpsilon)
    {
        VECTOR closest = ClosestPtPointSegment(rayStart, m_segmentA, m_segmentB);
        float  distSq  = VDot(VSub(rayStart, closest), VSub(rayStart, closest));

        if (distSq <= m_radius * m_radius)
        {
            outHitPos    = rayStart;
            outHitDistSq = 0.0f;
            return true;
        }
        return false;
    }

    float  minT = FLT_MAX;
    bool   hit  = false;
    VECTOR currentHitPos;
    float  currentHitDistSq = 0.0f;

    VECTOR OA      = VSub(rayStart, m_segmentA);
    VECTOR AB      = VSub(m_segmentB, m_segmentA);
    float  abLenSq = VDot(AB, AB);

    // カプセルが点になっている場合は球として判定
    if (abLenSq < kLengthEpsilon)
    {
        return IsIsIntersectsRaySphere(rayStart, rayEnd, m_segmentA, m_radius, outHitPos, outHitDistSq);
    }

    VECTOR u = VNorm(AB); // カプセル軸方向の単位ベクトル

    // Ray とカプセル軸の平行成分を除いた垂直成分で円筒部との交差判定
    VECTOR v = VSub(rayDir, VScale(u, VDot(rayDir, u)));
    VECTOR w = VSub(OA,     VScale(u, VDot(OA,     u)));

    float a            = VDot(v, v);
    float b            = 2.0f * VDot(v, w);
    float c            = VDot(w, w) - m_radius * m_radius;
    float discriminant = b * b - 4.0f * a * c;

    if (discriminant >= 0)
    {
        float sqrtDiscr = sqrtf(discriminant);
        float tCyl0     = (-b - sqrtDiscr) / (2.0f * a);
        float tCyl1     = (-b + sqrtDiscr) / (2.0f * a);

        for (float tCyl : {tCyl0, tCyl1})
        {
            if (tCyl < 0.0f || tCyl > 1.0f) continue;

            VECTOR pIntersect = VAdd(rayStart, VScale(rayDir, tCyl));
            VECTOR projOnAxis = VAdd(m_segmentA, VScale(u, VDot(VSub(pIntersect, m_segmentA), u)));
            float  tAxis      = VDot(VSub(projOnAxis, m_segmentA), AB) / abLenSq;

            // 交点が円筒部（セグメント AB）の範囲内にあるか確認
            if (tAxis >= 0.0f && tAxis <= 1.0f)
            {
                currentHitPos    = pIntersect;
                currentHitDistSq = VDot(VSub(currentHitPos, rayStart), VSub(currentHitPos, rayStart));

                if (currentHitDistSq < minT)
                {
                    minT         = currentHitDistSq;
                    outHitPos    = currentHitPos;
                    outHitDistSq = currentHitDistSq;
                    hit          = true;
                }
            }
        }
    }

    // 端球 A との判定
    if (IsIsIntersectsRaySphere(rayStart, rayEnd, m_segmentA, m_radius, currentHitPos, currentHitDistSq))
    {
        if (currentHitDistSq < minT)
        {
            minT         = currentHitDistSq;
            outHitPos    = currentHitPos;
            outHitDistSq = currentHitDistSq;
            hit          = true;
        }
    }

    // 端球 B との判定
    if (IsIsIntersectsRaySphere(rayStart, rayEnd, m_segmentB, m_radius, currentHitPos, currentHitDistSq))
    {
        if (currentHitDistSq < minT)
        {
            minT         = currentHitDistSq;
            outHitPos    = currentHitPos;
            outHitDistSq = currentHitDistSq;
            hit          = true;
        }
    }

    return hit;
}
