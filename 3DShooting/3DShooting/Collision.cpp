#include "Collision.h"
#include "CollisionGrid.h"
#include <algorithm>
#include <cmath>

namespace
{
    constexpr int   kCollisionIterations  = 4;       // 衝突解決の反復回数
    constexpr float kGroundTolerance      = 0.5f;    // 接地判定に用いる半径外側マージン
    constexpr float kNormalEpsilon        = 0.0001f; // 法線計算でゼロとみなす閾値
    constexpr float kWallSlopeThreshold   = 0.5f;    // 壁面とみなす法線 Y 成分の閾値
    constexpr float kGroundSlopeThreshold = 0.6f;    // 接地とみなす法線 Y 成分の閾値
    constexpr float kPenetrationEpsilon   = 0.001f;  // 押し出し処理を行う最小めり込み量
    constexpr float kRayEpsilon           = 1e-6f;   // Ray-三角形交差判定の平行判定閾値

    /// <summary>
    /// 三角形 abc 上で点 p に最も近い点を求めて返す
    /// ボロノイ領域分割アルゴリズム（"Real-Time Collision Detection" Christer Ericson 5.1.5節 参照）を使用。
    /// 三角形を「頂点・辺・内部」の 7 つのボロノイ領域に分割し、
    /// 点 p がどの領域に属するかを内積で判定して最近接点を返す。
    /// </summary>
    VECTOR GetClosestPointOnTriangle(const VECTOR& p, const VECTOR& a, const VECTOR& b, const VECTOR& c)
    {
        VECTOR ab = VSub(b, a);
        VECTOR ac = VSub(c, a);
        VECTOR ap = VSub(p, a);

        // p が頂点 a のボロノイ領域内にあるか判定
        float d1 = VDot(ab, ap);
        float d2 = VDot(ac, ap);

        if (d1 <= 0.0f && d2 <= 0.0f) return a; // 最近接点は頂点 a

        VECTOR bp = VSub(p, b);
        float d3 = VDot(ab, bp);
        float d4 = VDot(ac, bp);

        if (d3 >= 0.0f && d4 <= d3) return b; // 最近接点は頂点 b

        // p が辺 ab のボロノイ領域内にあるか判定
        float vc = d1 * d4 - d3 * d2;
        if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f)
        {
            float v = d1 / (d1 - d3); // 辺 ab 上のパラメータ
            return VAdd(a, VScale(ab, v));
        }

        VECTOR cp = VSub(p, c);
        float d5 = VDot(ab, cp);
        float d6 = VDot(ac, cp);

        if (d6 >= 0.0f && d5 <= d6) return c; // 最近接点は頂点 c

        // p が辺 ac のボロノイ領域内にあるか判定
        float vb = d5 * d2 - d1 * d6;
        if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f)
        {
            float w = d2 / (d2 - d6); // 辺 ac 上のパラメータ
            return VAdd(a, VScale(ac, w));
        }

        // p が辺 bc のボロノイ領域内にあるか判定
        float va    = d3 * d6 - d5 * d4;
        if (va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f)
        {
            float w = (d4 - d3) / ((d4 - d3) + (d5 - d6)); // 辺 bc 上のパラメータ
            return VAdd(b, VScale(VSub(c, b), w));
        }

        // p は三角形内部にある → 重心座標（va, vb, vc）で内部点を算出
        float denom = 1.0f / (va + vb + vc);
        float v     = vb * denom;
        float w     = vc * denom;
        return VAdd(a, VAdd(VScale(ab, v), VScale(ac, w)));
    }
}

CollisionResult Collision::CheckStageCollision(VECTOR& position, float capsuleHeight, float capsuleRadius, float colliderYOffset, const std::vector<Stage::StageCollisionData>& collisionData, const CollisionGrid* pGrid)
{
    CollisionResult result;
    result.isGrounded = false;

    const float kGroundToleranceSq = (capsuleRadius + kGroundTolerance) * (capsuleRadius + kGroundTolerance);

    // 空間分割グリッドを利用して判定対象ポリゴンを絞り込む。
    // 地形判定は毎フレーム多数の敵から呼ばれるため、バッファを関数内 static で
    // 再利用してヒープ確保を避ける（更新はメインスレッドで逐次実行される前提）。
    static std::vector<const Stage::StageCollisionData*> nearbyTriangles;
    nearbyTriangles.clear();
    bool useGrid = (pGrid != nullptr);
    if (useGrid)
    {
        pGrid->GetNearbyTriangles(position, nearbyTriangles);
        // 周囲にポリゴンがない場合は安全のため全件判定に切り替える
        if (nearbyTriangles.empty()) useGrid = false;
    }

    // 1フレームで複数ポリゴンにめり込む可能性があるので、
    // 押し出しを何度か繰り返して安定した衝突解決をする
    for (int i = 0; i < kCollisionIterations; ++i)
    {
        VECTOR checkPos = VAdd(position, VGet(0.0f, colliderYOffset, 0.0f));

        // カプセルの下端・中央・上端を代表点として衝突判定に使用する
        VECTOR capA = VAdd(checkPos, VGet(0.0f, -capsuleHeight * 0.5f, 0.0f));
        VECTOR capB = VAdd(checkPos, VGet(0.0f,  capsuleHeight * 0.5f, 0.0f));

        auto processTriangle = [&](const Stage::StageCollisionData& data)
        {
            // カプセルの下端・中央・上端の3点それぞれについてポリゴンとの最近接点を求める
            const VECTOR points[3] = { capA, checkPos, capB };

            for (const auto& p : points)
            {
                VECTOR closest = GetClosestPointOnTriangle(p, data.v1, data.v2, data.v3);
                VECTOR diff    = VSub(p, closest);
                float  distSq  = VDot(diff, diff);

                if (distSq >= kGroundToleranceSq) continue;

                float dist = sqrtf(distSq);

                VECTOR v12       = VSub(data.v2, data.v1);
                VECTOR v13       = VSub(data.v3, data.v1);
                VECTOR triNormal = VNorm(VCross(v12, v13));
                VECTOR normal    = triNormal;

                // 壁面（Y 成分が閾値以下）かつ法線が上向きの場合は水平成分のみで押し出す
                if (triNormal.y <= kWallSlopeThreshold && normal.y > 0.0f)
                {
                    normal.y    = 0.0f;
                    float sq    = normal.x * normal.x + normal.z * normal.z;
                    if (sq > kNormalEpsilon)
                    {
                        float len = sqrtf(sq);
                        normal.x /= len;
                        normal.z /= len;
                    }
                    else
                    {
                        normal = VGet(0.0f, 0.0f, 0.0f);
                    }
                }

                if (normal.y > kGroundSlopeThreshold)
                {
                    result.isGrounded         = true;
                    result.groundNormal       = normal;
                    result.groundedObjectName = data.name;
                }

                if (distSq < capsuleRadius * capsuleRadius)
                {
                    float penetration = capsuleRadius - dist;
                    if (penetration > kPenetrationEpsilon)
                    {
                        position = VAdd(position, VScale(normal, penetration));
                        checkPos = VAdd(position, VGet(0.0f, colliderYOffset, 0.0f));
                        capA     = VAdd(checkPos, VGet(0.0f, -capsuleHeight * 0.5f, 0.0f));
                        capB     = VAdd(checkPos, VGet(0.0f,  capsuleHeight * 0.5f, 0.0f));
                    }
                }
            }
        };

        if (useGrid)
        {
            for (const auto* pData : nearbyTriangles)
            {
                processTriangle(*pData);
            }
        }
        else
        {
            for (const auto& data : collisionData)
            {
                processTriangle(data);
            }
        }
    }
    return result;
}

bool Collision::IntersectRayTriangle(const VECTOR& rayOrig, const VECTOR& rayDir, const VECTOR& v0, const VECTOR& v1, const VECTOR& v2, float& outT)
{
    // Möller–Trumbore アルゴリズムによるレイ–三角形交差判定
    // 参考: "Fast, Minimum Storage Ray/Triangle Intersection", Möller & Trumbore (1997)
    // 重心座標 (u, v, 1-u-v) を直接求め、outT にレイの始点からの距離を格納する

    VECTOR edge1 = VSub(v1, v0);
    VECTOR edge2 = VSub(v2, v0);
    VECTOR h     = VCross(rayDir, edge2); // レイ方向と edge2 の外積（スケール計算用）
    float  a     = VDot(edge1, h);        // 行列式: 0 に近ければレイと三角形が平行

    if (a > -kRayEpsilon && a < kRayEpsilon) return false; // 平行判定（交差なし）

    float  f = 1.0f / a;                  // 行列式の逆数（除算を1回にまとめる）
    VECTOR s = VSub(rayOrig, v0);         // レイ始点から頂点 v0 へのベクトル
    float  u = f * VDot(s, h);           // 重心座標 u（0〜1 の範囲外なら三角形外）

    if (u < 0.0f || u > 1.0f) return false;

    VECTOR q = VCross(s, edge1);
    float  v = f * VDot(rayDir, q);      // 重心座標 v（u + v > 1 なら三角形外）

    if (v < 0.0f || u + v > 1.0f) return false;

    float t = f * VDot(edge2, q);        // レイ始点からの交差距離

    if (t > kRayEpsilon) // t > 0 ならレイの正方向に交差
    {
        outT = t;
        return true;
    }
    return false;
}
