#include "DxLib.h"
#include "AmmoItem.h"
#include "CapsuleCollider.h"
#include "Player.h"
#include "DebugUtil.h"
#include <cassert>
#include <algorithm>
#include <cmath>

namespace
{
	constexpr float kInitialRadius = 20.0f;
	constexpr int kAmmoAmount = 30; // 回復する弾薬数
	constexpr float kDropGravity = 0.5f;
	constexpr float kGroundY = 0.0f;
}

AmmoItem::AmmoItem() :
	m_modelHandle(-1),
	m_radius(kInitialRadius),
	m_pos(VGet(0.0f, 0.0f, 0.0f)),
	m_collider(m_pos, m_radius),
	m_isHit(false),
	m_isUsed(false),
	m_isDropping(true),
	m_velocityY(0.0f),
	m_rotY(0.0f)
{
	// モデルの読み込み
	m_modelHandle = MV1LoadModel("data/model/AmmoBox.mv1");
	assert(m_modelHandle != -1);
}

AmmoItem::~AmmoItem()
{
	MV1DeleteModel(m_modelHandle);
}

void AmmoItem::Init()
{
	m_isDropping = true;
	m_velocityY = 0.0f;
	m_collider.SetCenter(m_pos);
	m_collider.SetRadius(m_radius);
	MV1SetScale(m_modelHandle, VGet(3.0f, 3.0f, 3.0f));
}

void AmmoItem::Update(Player* player)
{
	if (IsUsed()) return;

	// ドロップ演出(落下処理)
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
	else
	{
		const float kRotateSpeed = 0.05f;
		m_rotY += kRotateSpeed;
		if (m_rotY > DX_TWO_PI) m_rotY -= DX_TWO_PI;
	}

	m_collider.SetCenter(m_pos);
	m_collider.SetRadius(m_radius);

	const Collider* playerCollider = dynamic_cast<const Collider*>(player->GetBodyCollider().get());
	m_isHit = m_collider.Intersects(playerCollider);

	// 弾薬が満タンかどうかは問わず、当たれば回復
	if (m_isHit)
	{
		player->AddAmmo(kAmmoAmount); // プレイヤーに弾薬を加算
		m_isUsed = true;
	}
}

void AmmoItem::Draw()
{
	if (IsUsed()) return;
	MV1SetPosition(m_modelHandle, m_pos);
	MV1SetRotationXYZ(m_modelHandle, VGet(0.0f, m_rotY, 0.0f));
	MV1DrawModel(m_modelHandle);
}
