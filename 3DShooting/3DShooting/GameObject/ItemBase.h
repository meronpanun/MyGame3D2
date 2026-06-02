#pragma once
#include "EffekseerWarningSuppress.h"
#include <vector>
#include "Stage.h"

class Player;
class CollisionGrid;

/// <summary>
/// アイテム基底クラス
/// </summary>
class ItemBase
{
public:
    ItemBase() = default;
    virtual ~ItemBase() = default;

    /// <summary>
    /// 初期化処理
    /// </summary>
    virtual void Init() = 0;

    /// <summary>
    /// 更新処理
    /// </summary>
    virtual void Update(Player* player, const std::vector<Stage::StageCollisionData>& collisionData, const CollisionGrid* pGrid) = 0;

    /// <summary>
    /// 描画処理
    /// </summary>
    virtual void Draw() = 0;

    /// <summary>
    /// アイテムが使用済みかどうかを返す
    /// </summary>
    virtual bool IsUsed() const = 0;

    /// <summary>
    /// アイテムの寿命が切れたかどうかを返す
    /// </summary>
    virtual bool IsExpired() const { return false; }

    /// <summary>
    /// 位置を設定する
    /// </summary>
    virtual void SetPos(const VECTOR& pos) = 0;

    /// <summary>
    /// コリジョンのデバッグ描画フラグを設定する
    /// </summary>
    static void SetDrawCollision(bool isDraw) { s_shouldDrawCollision = isDraw; }

    /// <summary>
    /// コリジョンのデバッグ描画フラグを取得する
    /// </summary>
    static bool ShouldDrawCollision() { return s_shouldDrawCollision; }

protected:
    /// <summary>
    /// デバッグ用の当たり判定を描画する
    /// </summary>
    virtual void DrawCollisionDebug() {}

    inline static bool s_shouldDrawCollision = false; // デバッグ用コリジョン描画フラグ
};
