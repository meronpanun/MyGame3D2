#include "DxLib.h"
#include "FirstAidKitItem.h"
#include "Player.h"
#include "DebugUtil.h"
#include <cassert>
#include <algorithm>
#include <cmath>

namespace
{
	// 初期半径
	constexpr float kInitialRadius = 50.0f; 

	// 回復量
	constexpr float kHealAmount = 30.0f; // 回復アイテムが回復する量
}

// カプセルと球の当たり判定
bool CapsuleSphereHit(const VECTOR& capA, const VECTOR& capB, float capRadius, const VECTOR& sphereCenter, float sphereRadius)
{
	// カプセルの線分上の最近接点を求める
	VECTOR ab = VSub(capB, capA);
	VECTOR ac = VSub(sphereCenter, capA);
	float abLenSq = ab.x * ab.x + ab.y * ab.y + ab.z * ab.z;
	float t = 0.0f;
	if (abLenSq > 0.0f)
	{
		t = (ac.x * ab.x + ac.y * ab.y + ac.z * ab.z) / abLenSq;
		t = std::clamp(t, 0.0f, 1.0f);
	}
	VECTOR closest = VAdd(capA, VScale(ab, t));
	// 最近接点と球中心の距離
	float distSq = (closest.x - sphereCenter.x) * (closest.x - sphereCenter.x)
		+ (closest.y - sphereCenter.y) * (closest.y - sphereCenter.y)
		+ (closest.z - sphereCenter.z) * (closest.z - sphereCenter.z);
	float hitDist = capRadius + sphereRadius;
	return distSq <= hitDist * hitDist;
}

FirstAidKitItem::FirstAidKitItem():
	m_radius(kInitialRadius),
	m_pos(VGet(0.0f, 0.0f, 0.0f)),
	m_isHit(false),
	m_isUsed(false) 
{
}

FirstAidKitItem::~FirstAidKitItem()
{
}

void FirstAidKitItem::Init()
{
}

void FirstAidKitItem::Update(Player* player)
{
	if (IsUsed()) return; // 既に使用されていたら描画しない

	// プレイヤーのカプセル情報を取得
	VECTOR capA, capB;
	float capRadius;
	player->GetCapsuleInfo(capA, capB, capRadius);

	// カプセル-球判定
	m_isHit = CapsuleSphereHit(capA, capB, capRadius, m_pos, m_radius);

	// プレイヤーの体力が満タンでなく、当たっていたら回復
	if (m_isHit && player->GetHealth() < player->GetMaxHealth())
	{
		player->AddHp(kHealAmount);
		m_isUsed = true; // 1回だけ使えるように
	}
}

void FirstAidKitItem::Draw()
{
	if (IsUsed()) return; // 既に使用されていたら描画しない

    // 球の描画
    DrawSphere3D(m_pos, m_radius, 16, 0xff0000, 0xff8080, true);
}