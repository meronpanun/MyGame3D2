#pragma once
#include "EffekseerWarningSuppress.h"
#include "SphereCollider.h"
#include "ItemBase.h"

class Player;

/// <summary>
/// 弾薬アイテムクラス
/// </summary>
class AmmoItem : public ItemBase
{
public:
	AmmoItem();
	virtual ~AmmoItem();

	void Init() override;
	void Update(Player* player, const std::vector<Stage::StageCollisionData>& collisionData, const CollisionGrid* pGrid) override;
	void Draw() override;
	void DrawCollisionDebug() override;

	/// <summary>
	/// 位置設定
	/// </summary>
	/// <param name="pos">位置</param>
	void SetPos(const VECTOR& pos) override { m_pos = pos; }

	/// <summary>
	/// 使用済みかどうか
	/// </summary>
	/// <returns>true: 使用済み, false: 未使用</returns>
	bool IsUsed() const override { return m_isUsed; }

	/// <summary>
	/// 寿命が切れているかどうか
	/// </summary>
	/// <returns>true: 寿命切れ, false: まだ有効</returns>
	bool IsExpired() const override { return m_lifeTimer <= 0; }

	static void LoadModel();
	static void DeleteModel();

private:
	VECTOR m_pos;
	SphereCollider m_collider;

	int m_modelHandle;   // モデルハンドル

	float m_radius;      // 半径
	float m_velocityY;   // 落下速度
	float m_rotY;        // Y軸回転角度

	bool m_isHit;        // プレイヤーと接触しているかどうか
	bool m_isUsed;       // アイテムが使用されたかどうか
	bool m_isDropping;   // 落下中かどうか
	int m_lifeTimer;     // 残り寿命（フレーム数）

	static int s_modelHandle;
};
