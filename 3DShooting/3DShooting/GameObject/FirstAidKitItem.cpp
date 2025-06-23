#include "DxLib.h"
#include "FirstAidKitItem.h"
#include "Player.h"
#include "DebugUtil.h"
#include <cassert>
#include <algorithm>

namespace
{
	// 初期半径
	constexpr float kInitialRadius = 50.0f; 

	// 回復量
	constexpr float kHealAmount = 30.0f; // 回復アイテムが回復する量
}

FirstAidKitItem::FirstAidKitItem():
	m_radius(kInitialRadius),
	m_isActive(true),
	m_pos(VGet(0.0f, 0.0f, 0.0f))
{
}

FirstAidKitItem::~FirstAidKitItem()
{
}

void FirstAidKitItem::Init()
{
}

void FirstAidKitItem::Update()
{
    if (!m_isActive) return;

    OutputDebugStringA("Update: isActive\n");

    Player* player = Player::GetInstance();
    if (!player) {
        OutputDebugStringA("Update: player is null\n");
        return;
    }
    OutputDebugStringA("Update: got player\n");

    VECTOR capA, capB;
    float capsuleRadius;
    player->GetCapsuleInfo(capA, capB, capsuleRadius);
    OutputDebugStringA("Update: got capsule info\n");

    float dist = GetCapsuleSphereDistance(capA, capB, capsuleRadius, m_pos, m_radius);
    OutputDebugStringA("Update: got distance\n");

    if (dist <= 0.0f)
    {
        OutputDebugStringA("Update: hit!\n");
        if (player->GetHealth() < player->GetMaxHealth())
        {
            player->AddHp(kHealAmount);
            m_isActive = false;
            OutputDebugStringA("Update: healed\n");
        }
    }
}


void FirstAidKitItem::Draw()
{
    if (!m_isActive) return;
    // 球の描画
    DrawSphere3D(m_pos, m_radius, 16, 0xff0000, 0xff8080, true);
}

float FirstAidKitItem::GetCapsuleSphereDistance(const VECTOR& capA, const VECTOR& capB, float capRadius, const VECTOR& spherePos, float sphereRadius)
{
    // 線分capA-capBと点spherePosの最近接点を求める
    VECTOR ab = VSub(capB, capA);
    VECTOR ap = VSub(spherePos, capA);
    float t = (ab.x * ap.x + ab.y * ap.y + ab.z * ap.z) / (ab.x * ab.x + ab.y * ab.y + ab.z * ab.z);
    t = std::clamp(t, 0.0f, 1.0f);
    VECTOR closest = VAdd(capA, VScale(ab, t));
    float dist = VSize(VSub(closest, spherePos));
    return dist - capRadius - sphereRadius;
}