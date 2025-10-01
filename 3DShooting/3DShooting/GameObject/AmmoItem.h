#pragma once
#include "EffekseerForDXLib.h"
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
	void Update(Player* player) override;
	void Draw() override;

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

private:
	VECTOR m_pos;
	SphereCollider m_collider;

	int m_modelHandle;

	float m_radius;
	float m_velocityY;
	float m_rotY;

	bool m_isHit;
	bool m_isUsed;
	bool m_isDropping;
};

