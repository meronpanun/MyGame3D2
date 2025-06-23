#pragma once
#include "ItemBase.h"

class Player;

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

private:
	VECTOR m_pos;

	float m_radius; 

	bool m_isHit;
	bool m_isUsed; // アイテムが使用されたかどうか
};

