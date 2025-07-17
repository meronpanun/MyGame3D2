#include "DxLib.h"
#include "FirstAidKitItem.h"
#include "CapsuleCollider.h"
#include "Player.h"
#include "DebugUtil.h"
#include <cassert>
#include <algorithm>
#include <cmath>

namespace
{
	// 初期半径
	constexpr float kInitialRadius = 20.0f; 

	constexpr float kHealAmount = 30.0f; // 回復量
	constexpr float kDropGravity = 0.5f; // 落下重力加速度
	constexpr float kGroundY = 0.0f;     // 地面の高さ
}

FirstAidKitItem::FirstAidKitItem():
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
	m_modelHandle = MV1LoadModel("data/model/FirstAidKit.mv1");
	assert(m_modelHandle != -1);
}

FirstAidKitItem::~FirstAidKitItem()
{
	// モデルの解放
	MV1DeleteModel(m_modelHandle);
}

void FirstAidKitItem::Init()
{
	m_isDropping = true;
	m_velocityY = 0.0f;
	m_collider.SetCenter(m_pos);
	m_collider.SetRadius(m_radius);

	// モデルのスケール調整
	MV1SetScale(m_modelHandle, VGet(0.5f, 0.5f, 0.5f));
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
	else
	{
		// 落下しきった後は回転
		const float kRotateSpeed = 0.05f; // 回転速度
		m_rotY += kRotateSpeed;
		if (m_rotY > DX_TWO_PI) m_rotY -= DX_TWO_PI;
	}

	// コライダーの位置を更新
	m_collider.SetCenter(m_pos);
	m_collider.SetRadius(m_radius);

	// プレイヤーのカプセルコライダーを取得
	const Collider* playerCollider = dynamic_cast<const Collider*>(player->GetBodyCollider().get());

	// オブジェクト指向的な当たり判定
	m_isHit = m_collider.Intersects(playerCollider);

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

	// モデルを描画
	MV1SetPosition(m_modelHandle, m_pos);
	MV1SetRotationXYZ(m_modelHandle, VGet(0.0f, m_rotY, 0.0f));
	MV1DrawModel(m_modelHandle);
}