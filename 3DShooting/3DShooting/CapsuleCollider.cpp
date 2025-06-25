#include "DxLib.h"
#include "CapsuleCollider.h"
#include "SphereCollider.h"
#include <algorithm>
#include <cmath>
#include <float.h>

// ヘルパー関数: 最も近い点（線分-点）
// 線分pqと点cの間の最短距離を計算し、線分pq上の最短点も返す
VECTOR ClosestPtPointSegment(const VECTOR& c, const VECTOR& p, const VECTOR& q)
{
    VECTOR ab = VSub(q, p);
    float t = VDot(VSub(c, p), ab) / VDot(ab, ab);
    return VAdd(p, VScale(ab, std::clamp(t, 0.0f, 1.0f)));
}

// Rayと球体の交差判定のヘルパー関数 (SphereCollider::IntersectsRayから流用・調整)
// SphereColliderのRay判定が正しく機能している前提
bool IntersectsRaySphere(const VECTOR& rayStart, const VECTOR& rayEnd, const VECTOR& sphereCenter, float sphereRadius, VECTOR& out_hitPos, float& out_hitDistSq)
{
    VECTOR rayDir = VSub(rayEnd, rayStart);
    float rayLengthSq = VDot(rayDir, rayDir);

    if (rayLengthSq < 0.0001f) // レイの長さが0に近い場合
    {
        float distSq = VDot(VSub(sphereCenter, rayStart), VSub(sphereCenter, rayStart));
        if (distSq <= sphereRadius * sphereRadius)
        {
            out_hitPos = rayStart;
            out_hitDistSq = 0.0f;
            return true;
        }
        return false;
    }

    VECTOR m = VSub(rayStart, sphereCenter);
    float b = VDot(m, rayDir);
    float c = VDot(m, m) - sphereRadius * sphereRadius;

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

    if (t < 0.0f) // 始点が球内にある場合
    {
        t = 0.0f;
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
    // ... (SphereColliderとの判定ロジックは変更なし) ...
    if (!other) return false;

    // CapsuleCollider同士の判定
    const CapsuleCollider* capsule = dynamic_cast<const CapsuleCollider*>(other);
    if (capsule)
    {
        // 2つの線分間の最短距離を求めるロジック
        // ref: Real-Time Collision Detection by Christer Ericson, Chapter 5.1.8 Closest Point on Line Segments
        VECTOR d1 = VSub(m_segmentB, m_segmentA);
        VECTOR d2 = VSub(capsule->m_segmentB, capsule->m_segmentA);
        VECTOR r = VSub(m_segmentA, capsule->m_segmentA);

        float a = VDot(d1, d1); // squared length of segment 1
        float e = VDot(d2, d2); // squared length of segment 2
        float f = VDot(d2, r);  // dot product of segment 2 with vector from segment 2 start to segment 1 start

        float s = 0.0f, t = 0.0f;
        float c = VDot(d1, r);
        float b = VDot(d1, d2);
        float denom = a * e - b * b; // denominator of the equations

        if (denom != 0.0f)
        {
            s = std::clamp((b * f - c * e) / denom, 0.0f, 1.0f);
        }
        else // line segments are parallel
        {
            // 分母が0の場合（レイとカプセルの軸が平行）、最短距離を正しく計算するために特別な処理が必要
            // ここでは簡易的に、片方の線分の端点からもう一方の線分への最短点を求める
            s = 0.0f;
        }

        t = (b * s + f) / e;

        if (t < 0.0f)
        {
            t = 0.0f;
            s = std::clamp((c) / a, 0.0f, 1.0f);
        }
        else if (t > 1.0f)
        {
            t = 1.0f;
            s = std::clamp((c + b) / a, 0.0f, 1.0f);
        }

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

bool CapsuleCollider::IntersectsRay(const VECTOR& rayStart, const VECTOR& rayEnd, VECTOR& out_hitPos, float& out_hitDistSq) const
{
    VECTOR rayDir = VSub(rayEnd, rayStart);
    float rayLengthSq = VDot(rayDir, rayDir);

    if (rayLengthSq < 0.0001f) // レイの長さが0に近い場合、点とカプセルの判定にフォールバック
    {
        VECTOR closestPointOnCapsuleSegment = ClosestPtPointSegment(rayStart, m_segmentA, m_segmentB);
        float distSq = VDot(VSub(rayStart, closestPointOnCapsuleSegment), VSub(rayStart, closestPointOnCapsuleSegment));
        if (distSq <= m_radius * m_radius)
        {
            out_hitPos = rayStart;
            out_hitDistSq = 0.0f;
            return true;
        }
        return false;
    }

    float minT = FLT_MAX; // Rayのパラメータtの最小値
    bool hit = false;
    VECTOR currentHitPos;
    float currentHitDistSq;

    // --- 1. カプセルの円柱部分とRayの交差判定 ---
    // カプセルの軸ABとRayの方向ベクトルDで、Rayをカプセルの中心線に対して垂直な平面に投影し、円と線の交差判定に帰着させる
    // または、以下のようなアルゴリズムを用いる:
    // ref: Real-Time Collision Detection by Christer Ericson, Chapter 5.3.3 Ray vs. Capsule
    VECTOR A = m_segmentA;
    VECTOR B = m_segmentB;
    float r = m_radius;

    VECTOR OA = VSub(rayStart, A); // Ray始点からカプセル軸Aへのベクトル
    VECTOR AB = VSub(B, A);        // カプセル軸のベクトル

    float dot_dir_AB = VDot(rayDir, AB);
    float dot_OA_AB = VDot(OA, AB);
    float dot_AB_AB = VDot(AB, AB); // カプセル軸の長さの2乗

    VECTOR Q = VSub(VSub(VScale(rayDir, dot_AB_AB), VScale(AB, dot_dir_AB)), VSub(VScale(OA, dot_AB_AB), VScale(AB, dot_OA_AB)));
    float a = VDot(Q, Q);
    float b = 2.0f * VDot(Q, VSub(VScale(OA, dot_AB_AB), VScale(AB, dot_OA_AB)));
    float c = VDot(VSub(VScale(OA, dot_AB_AB), VScale(AB, dot_OA_AB)), VSub(VScale(OA, dot_AB_AB), VScale(AB, dot_OA_AB))) - r * r * dot_AB_AB * dot_AB_AB;

    float discr = b * b - 4.0f * a * c;

    if (discr >= 0.0f)
    {
        float sqrt_discr = std::sqrt(discr);
        float t0 = (-b - sqrt_discr) / (2.0f * a);
        float t1 = (-b + sqrt_discr) / (2.0f * a);

        if (t0 <= 1.0f && t1 >= 0.0f) // Rayがカプセルと交差する範囲にある
        {
            float t_intersect =(std::max)(0.0f, t0); // レイの始点より手前の交差は無視

            if (t_intersect <= 1.0f)
            {
                // 交差したレイ上の点P
                VECTOR P = VAdd(rayStart, VScale(rayDir, t_intersect));

                // Pがカプセル軸ABの間に位置するかチェック
                // Pから軸ABへの最短点Sを求める
                VECTOR S = ClosestPtPointSegment(P, A, B);

                // PとSの距離が半径r以下であれば有効な衝突
                if (VDot(VSub(P, S), VSub(P, S)) <= r * r + 0.0001f) // 誤差許容
                {
                    currentHitPos = P;
                    currentHitDistSq = VDot(VSub(P, rayStart), VSub(P, rayStart));
                    if (currentHitDistSq < minT) // より近いヒットを優先
                    {
                        minT = currentHitDistSq;
                        out_hitPos = currentHitPos;
                        out_hitDistSq = currentHitDistSq;
                        hit = true;
                    }
                }
            }
        }
    }


    // --- 2. カプセルの両端の半球とRayの交差判定 ---
    // RayとA端の球の交差
    if (IntersectsRaySphere(rayStart, rayEnd, A, r, currentHitPos, currentHitDistSq))
    {
        if (currentHitDistSq < minT)
        {
            minT = currentHitDistSq;
            out_hitPos = currentHitPos;
            out_hitDistSq = currentHitDistSq;
            hit = true;
        }
    }

    // RayとB端の球の交差
    if (IntersectsRaySphere(rayStart, rayEnd, B, r, currentHitPos, currentHitDistSq))
    {
        if (currentHitDistSq < minT)
        {
            minT = currentHitDistSq;
            out_hitPos = currentHitPos;
            out_hitDistSq = currentHitDistSq;
            hit = true;
        }
    }

    return hit;
}