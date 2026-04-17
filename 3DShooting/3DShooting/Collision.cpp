#include "Collision.h"
#include <algorithm>
#include <cmath>

namespace
{
    // トライアングル上の最近接点を求める関数
    VECTOR GetClosestPointOnTriangle(const VECTOR& p, const VECTOR& a,const VECTOR& b, const VECTOR& c)
    {
        VECTOR ab = VSub(b, a);
        VECTOR ac = VSub(c, a);
        VECTOR ap = VSub(p, a);

        float d1 = VDot(ab, ap);
        float d2 = VDot(ac, ap);

        if (d1 <= 0.0f && d2 <= 0.0f) return a;

        VECTOR bp = VSub(p, b);
        float d3 = VDot(ab, bp);
        float d4 = VDot(ac, bp);

        if (d3 >= 0.0f && d4 <= d3) return b;

        float vc = d1 * d4 - d3 * d2;
        if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f)
        {
            float v = d1 / (d1 - d3);
            return VAdd(a, VScale(ab, v));
        }

        VECTOR cp = VSub(p, c);
        float d5 = VDot(ab, cp);
        float d6 = VDot(ac, cp);

        if (d6 >= 0.0f && d5 <= d6) return c;

        float vb = d5 * d2 - d1 * d6;
        if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f)
        {
            float w = d2 / (d2 - d6);
            return VAdd(a, VScale(ac, w));
        }

        float va = d3 * d6 - d5 * d4;
        if (va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f)
        {
            float w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
            return VAdd(b, VScale(VSub(c, b), w));
        }

        float denom = 1.0f / (va + vb + vc);
        float v = vb * denom;
        float w = vc * denom;
        return VAdd(a, VAdd(VScale(ab, v), VScale(ac, w)));
    }
}

CollisionResult Collision::CheckStageCollision(VECTOR& position, float capsuleHeight, float capsuleRadius, float colliderYOffset, const std::vector<Stage::StageCollisionData>& collisionData)
{
    CollisionResult result;
    result.isGrounded = false;

    const int kIterations = 4;
    const float kGroundTolerance = 0.5f;
    const float kGroundToleranceSq = (capsuleRadius + kGroundTolerance) * (capsuleRadius + kGroundTolerance);

    for (int i = 0; i < kIterations; ++i)
    {
        VECTOR checkPos = VAdd(position, VGet(0.0f, colliderYOffset, 0.0f));
        float radius = capsuleRadius;

        VECTOR capA = VAdd(checkPos, VGet(0, -capsuleHeight * 0.5f, 0));
        VECTOR capB = VAdd(checkPos, VGet(0, capsuleHeight * 0.5f, 0));

        for (const auto& data : collisionData)
        {
            VECTOR points[] = { capA, checkPos, capB };

            for (const auto& p : points)
            {
                VECTOR closest = GetClosestPointOnTriangle(p, data.v1, data.v2, data.v3);
                VECTOR diff = VSub(p, closest);
                float distSq = VDot(diff, diff);

                if (distSq < kGroundToleranceSq)
                {
                    float dist = sqrtf(distSq);
                    VECTOR v12 = VSub(data.v2, data.v1);
                    VECTOR v13 = VSub(data.v3, data.v1);
                    VECTOR triNormal = VNorm(VCross(v12, v13));

                    VECTOR normal;

                    if (dist > 0.0001f)
                    {
                        normal = VScale(diff, 1.0f / dist);
                        // 進行可能なスロープ(y > 0.5)の場合は、
                        // ポリゴンの継ぎ目で引っかからないように面の法線を使用する
                        if (triNormal.y > 0.5f)
                        {
                            normal = triNormal;
                        }
                        // 進行不可能な壁や急斜面の場合
                        else if (triNormal.y <= 0.5f)
                        {
                            normal = triNormal;
                        }
                    }
                    else
                    {
                        normal = triNormal;
                    }

                    // 進行不可能な壁や急斜面にこすりつけた際、
                    // わずかに上方向に押し出されて徐々に空中に浮き上がり、
                    // 再び着地してまたSEが鳴るのを完全に防ぐため、
                    // 上方向への押し出し（反発力）をゼロにして水平に滑らせる
                    if (triNormal.y <= 0.5f && normal.y > 0.0f)
                    {
                        normal.y = 0.0f;
                        float sq = normal.x * normal.x + normal.z * normal.z;
                        if (sq > 0.0001f)
                        {
                            float len = std::sqrt(sq);
                            normal.x /= len;
                            normal.z /= len;
                        }
                        else
                        {
                            normal = VGet(0.0f, 0.0f, 0.0f);
                        }
                    }

                    if (normal.y > 0.6f)
                    {
                        result.isGrounded = true;
                        result.groundNormal = normal;
                        result.groundedObjectName = data.name;
                    }

                    if (distSq < radius * radius)
                    {
                        float penetration = radius - dist;
                        if (penetration > 0.001f)
                        {
                            position = VAdd(position, VScale(normal, penetration));
                            checkPos = VAdd(position, VGet(0.0f, colliderYOffset, 0.0f));
                            capA = VAdd(checkPos, VGet(0, -capsuleHeight * 0.5f, 0));
                            capB = VAdd(checkPos, VGet(0, capsuleHeight * 0.5f, 0));
                        }
                    }
                }
            }
        }
    }
    return result;
}

bool Collision::IntersectRayTriangle(const VECTOR& rayOrig, const VECTOR& rayDir, const VECTOR& v0, const VECTOR& v1, const VECTOR& v2, float& outT)
{
    constexpr float kEpsilon = 1e-6f;

    VECTOR edge1 = VSub(v1, v0);
    VECTOR edge2 = VSub(v2, v0);
    VECTOR h = VCross(rayDir, edge2);
    float a = VDot(edge1, h);

    if (a > -kEpsilon && a < kEpsilon) return false; 

    float f = 1.0f / a;
    VECTOR s = VSub(rayOrig, v0);
    float u = f * VDot(s, h);

    if (u < 0.0f || u > 1.0f) return false;

    VECTOR q = VCross(s, edge1);
    float v = f * VDot(rayDir, q);

    if (v < 0.0f || u + v > 1.0f) return false;

    float t = f * VDot(edge2, q);

    if (t > kEpsilon) 
    {
        outT = t;
        return true;
    }
    else return false;
}
