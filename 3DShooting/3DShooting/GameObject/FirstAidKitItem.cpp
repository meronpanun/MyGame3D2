#include "DxLib.h"
#include "FirstAidKitItem.h"
#include "Player.h"
#include "DebugUtil.h"
#include <cassert>
#include <algorithm>
#include <cmath>

namespace
{
	
	constexpr float kInitialRadius = 20.0f; 

	
	constexpr float kHealAmount = 30.0f; 
	constexpr float kDropGravity = 0.5f; // 落下重力加速度
	constexpr float kGroundY = 0.0f; // 地面の高さ
}

bool CapsuleSphereHit(const VECTOR& capA, const VECTOR& capB, float capRadius, const VECTOR& sphereCenter, float sphereRadius)
{
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
	m_isUsed(false),
	m_isDropping(true),
	m_velocityY(0.0f) 
{
}

FirstAidKitItem::~FirstAidKitItem()
{
}

void FirstAidKitItem::Init()
{
	m_isDropping = true;
	m_velocityY = 0.0f;
}

void FirstAidKitItem::Update(Player* player)
{
	if (IsUsed()) return;

	// ドロップ演出（落下処理）
	if (m_isDropping) 
	{
		m_velocityY -= kDropGravity;
		m_pos.y += m_velocityY;
		if (m_pos.y <= kGroundY) 
		{
			m_pos.y = kGroundY;
			m_velocityY = 0.0f;
			m_isDropping = false;
		}
	}

	// プレイヤーのカプセル情報を取得
	VECTOR capA, capB;
	float capRadius;
	player->GetCapsuleInfo(capA, capB, capRadius);

	// カプセル-球判定
	m_isHit = CapsuleSphereHit(capA, capB, capRadius, m_pos, m_radius);

	// プレイヤーの体力が満タンでなく、かつ当たっていれば回復
	if (m_isHit && player->GetHealth() < player->GetMaxHealth())
	{
		player->AddHp(kHealAmount);
		m_isUsed = true;
	}
}

void FirstAidKitItem::Draw()
{
	if (IsUsed()) return; 

    DrawSphere3D(m_pos, m_radius, 16, 0xff0000, 0xff8080, true);
}