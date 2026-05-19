#pragma once
#include "EffekseerWarningSuppress.h"
#include "Stage.h"
#include <vector>

/// <summary>
/// ステージとの衝突判定結果を格納する構造体
/// </summary>
struct CollisionResult
{
    bool        isGrounded         = false;         // 接地しているかどうか
    VECTOR      groundNormal       = VGet(0, 1, 0); // 接地面の法線（初期値は真上）
    std::string groundedObjectName;                  // 接地しているオブジェクト名
};

/// <summary>
/// ステージ・Ray との衝突判定を提供するユーティリティクラス。
/// インスタンス化せずに static メソッドとして使用する。
/// </summary>
class Collision
{
public:
    /// <summary>
    /// カプセルコライダーとステージポリゴンの衝突判定を行い、位置を補正する
    /// </summary>
    /// <param name="position">キャラクターの位置（衝突時に押し出し補正される）</param>
    /// <param name="capsuleHeight">カプセルの高さ</param>
    /// <param name="capsuleRadius">カプセルの半径</param>
    /// <param name="colliderYOffset">カプセル中心の Y オフセット</param>
    /// <param name="collisionData">判定対象のステージポリゴンデータ一覧</param>
    /// <param name="pGrid">空間分割グリッド（nullptr の場合は全件判定）</param>
    /// <returns>衝突結果（接地状態・法線・オブジェクト名）</returns>
    static CollisionResult CheckStageCollision(
        VECTOR& position,
        float capsuleHeight,
        float capsuleRadius,
        float colliderYOffset,
        const std::vector<Stage::StageCollisionData>& collisionData,
        const class CollisionGrid* pGrid = nullptr);

    /// <summary>
    /// Möller–Trumbore 法による Ray と三角形の交差判定を行う
    /// </summary>
    /// <param name="rayOrig">Ray の始点</param>
    /// <param name="rayDir">Ray の方向ベクトル（正規化不要）</param>
    /// <param name="v0">三角形の頂点 0</param>
    /// <param name="v1">三角形の頂点 1</param>
    /// <param name="v2">三角形の頂点 2</param>
    /// <param name="outT">交差パラメータ t（出力）。交点は rayOrig + t * rayDir</param>
    /// <returns>交差していれば true</returns>
    static bool IntersectRayTriangle(
        const VECTOR& rayOrig,
        const VECTOR& rayDir,
        const VECTOR& v0,
        const VECTOR& v1,
        const VECTOR& v2,
        float& outT);
};
