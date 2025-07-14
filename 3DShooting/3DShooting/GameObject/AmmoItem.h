#pragma once
#include "DxLib.h"
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

	void SetPos(const VECTOR& pos) override { m_pos = pos; }
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

