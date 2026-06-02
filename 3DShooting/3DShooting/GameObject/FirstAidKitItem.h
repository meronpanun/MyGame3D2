#pragma once
#include "ItemBase.h"
#include "EffekseerWarningSuppress.h"
#include "SphereCollider.h"
#include "Stage.h"

class Player;
class SphereCollider;

/// <summary>
/// 回復アイテムクラス
/// </summary>
class FirstAidKitItem : public ItemBase
{
public:
    FirstAidKitItem();
    virtual ~FirstAidKitItem();

    void Init() override;
    void Update(Player* player, const std::vector<Stage::StageCollisionData>& collisionData, const CollisionGrid* pGrid) override;
    void Draw() override;
    void DrawCollisionDebug() override;

    void SetPos(const VECTOR& pos) override { m_pos = pos; }
    bool IsUsed() const override { return m_isUsed; }
    bool IsExpired() const override { return m_lifeTimer <= 0; }

    /// <summary>
    /// モデルの読み込み（共有）
    /// </summary>
    static void LoadModel();

    /// <summary>
    /// モデルの解放（共有）
    /// </summary>
    static void DeleteModel();

private:
    VECTOR m_pos;

    SphereCollider m_collider;

    int m_modelHandle; // モデルハンドル

    float m_radius;    // 当たり判定半径
    float m_velocityY; // Y方向の落下速度
    float m_rotY;      // Y軸の回転角度

    bool m_isHit;      // プレイヤーと接触したかどうか
    bool m_isUsed;     // アイテムが使用されたかどうか
    bool m_isDropping; // ドロップ落下中かどうか
    int  m_lifeTimer;  // 残り寿命（フレーム数）

    static int s_modelHandle; // 共有モデルハンドル
};
