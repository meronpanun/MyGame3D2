#include "EffekseerForDXLib.h"
#include "AmmoItem.h"
#include "CapsuleCollider.h"
#include "Player.h"
#include "DebugUtil.h"
#include <cassert>
#include <algorithm>
#include <cmath>

int AmmoItem::s_modelHandle = -1;

namespace
{
	constexpr int   kAmmoAmount    = 30;    // 回復する弾薬数
	constexpr float kInitialRadius = 20.0f; // 初期の半径
	constexpr float kDropGravity   = 0.5f;  // ドロップ時の重力加速度
	constexpr float kGroundY       = 0.0f;  // 地面のY座標
	constexpr float kRotateSpeed   = 0.05f; // 回転速度
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
	// モデルの複製
	m_modelHandle = MV1DuplicateModel(s_modelHandle);
	assert(m_modelHandle != -1);
}

AmmoItem::~AmmoItem()
{
	// モデルの解放
	MV1DeleteModel(m_modelHandle);
}

void AmmoItem::LoadModel()
{
	s_modelHandle = MV1LoadModel("data/model/AmmoBox.mv1");
	assert(s_modelHandle != -1);
}

void AmmoItem::DeleteModel()
{
	MV1DeleteModel(s_modelHandle);
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
	else
	{
		m_rotY += kRotateSpeed;
		if (m_rotY > DX_TWO_PI) m_rotY -= DX_TWO_PI;
	}

	m_collider.SetCenter(m_pos);
	m_collider.SetRadius(m_radius);

	const Collider* playerCollider = dynamic_cast<const Collider*>(player->GetBodyCollider().get());
	m_isHit = m_collider.IsIntersects(playerCollider);

	// 当たれば弾薬加算
	if (m_isHit)
	{
		PlaySoundMem(player->GetAmmoItemSEHandle(), DX_PLAYTYPE_BACK); // 弾薬アイテムSE再生
		// 現在の武器に応じて弾薬を加算
		switch (player->GetCurrentWeaponType())
		{
		case WeaponType::AssaultRifle:
			player->AddARAmmo(kAmmoAmount);
			break;
		case WeaponType::Shotgun:
			player->AddSGAmmo(kAmmoAmount);
			break;
		default:
			// どちらでもない場合は何もしない、またはデフォルトの処理
			break;
		}
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
