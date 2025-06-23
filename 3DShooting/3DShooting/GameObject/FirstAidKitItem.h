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
	void Update() override;
	void Draw() override;

	// 位置と半径を追加
	void SetPosition(const VECTOR& pos) { m_pos = pos; }

	const VECTOR& GetPosition() const { return m_pos; }

	float GetRadius() const { return m_radius; }

	// カプセルと球の最短距離計算関数の宣言
	float GetCapsuleSphereDistance(const VECTOR& capA, const VECTOR& capB, float capRadius, const VECTOR& spherePos, float sphereRadius);


private:
	VECTOR m_pos;

	float m_radius; 
	
	bool m_isActive;
};

