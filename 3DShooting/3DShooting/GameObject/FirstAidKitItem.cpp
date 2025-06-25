#include "DxLib.h"
#include "FirstAidKitItem.h"
#include "Player.h"
#include "DebugUtil.h"
#include <cassert>
#include <algorithm>
#include <cmath>

namespace
{
	// �������a
	constexpr float kInitialRadius = 50.0f; 

	// �񕜗�
	constexpr float kHealAmount = 30.0f; // �񕜃A�C�e�����񕜂����
}

// �J�v�Z���Ƌ��̓����蔻��
bool CapsuleSphereHit(const VECTOR& capA, const VECTOR& capB, float capRadius, const VECTOR& sphereCenter, float sphereRadius)
{
	// �J�v�Z���̐�����̍ŋߐړ_�����߂�
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
	// �ŋߐړ_�Ƌ����S�̋���
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
	if (IsUsed()) return; // ���Ɏg�p����Ă�����`�悵�Ȃ�

	// �v���C���[�̃J�v�Z�������擾
	VECTOR capA, capB;
	float capRadius;
	player->GetCapsuleInfo(capA, capB, capRadius);

	// �J�v�Z��-������
	m_isHit = CapsuleSphereHit(capA, capB, capRadius, m_pos, m_radius);

	// �v���C���[�̗̑͂����^���łȂ��A�������Ă������
	if (m_isHit && player->GetHealth() < player->GetMaxHealth())
	{
		player->AddHp(kHealAmount);
		m_isUsed = true; // 1�񂾂��g����悤��
	}
}

void FirstAidKitItem::Draw()
{
	if (IsUsed()) return; // ���Ɏg�p����Ă�����`�悵�Ȃ�

    // ���̕`��
    DrawSphere3D(m_pos, m_radius, 16, 0xff0000, 0xff8080, true);
}