#pragma once
#include "ItemBase.h"
#include "DxLib.h"
#include "SphereCollider.h"

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
	void Update(Player* player) override;
	void Draw() override;

	void SetPos(const VECTOR& pos) { m_pos = pos; }
	bool IsUsed() const override { return m_isUsed; }

private:
	VECTOR m_pos;

	SphereCollider m_collider;

	float m_radius;
	float m_velocityY; // 落下速度

	bool m_isHit;
	bool m_isUsed; // アイテムが使用されたかどうか
	bool m_isDropping; // 落下中かどうか
};

