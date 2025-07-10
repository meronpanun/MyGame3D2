#include "Bullet.h"
#include "DxLib.h"
#include <algorithm>

namespace
{
	// 弾の速度
	constexpr float kBulletSpeed = 50.0f;

	// プレイヤーからの画面外判定距離
	constexpr float kPlayerBoundaryDistance = 1000.0f;
}

Bullet::Bullet(VECTOR position, VECTOR direction, float damage) :
	m_pos(position),
	m_prevPos(position), 
	m_dir(direction),
	m_speed(kBulletSpeed),
	m_isActive(true),
	m_damage(damage)
{
}

Bullet::~Bullet()
{
}

void Bullet::Init()
{
}

void Bullet::Update(const VECTOR& playerPos)
{
	if (!m_isActive) return;

	m_prevPos = m_pos; // 現在の位置を前フレームの位置として保存
	m_pos = VAdd(m_pos, VScale(VNorm(m_dir), m_speed)); // 新しい位置を計算

	// プレイヤーからの距離を計算
	VECTOR toPlayer = VSub(m_pos, playerPos);
	float distanceToPlayer = sqrtf(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y + toPlayer.z * toPlayer.z);

	// プレイヤーから一定距離以上離れたら非アクティブにする
	if (distanceToPlayer > kPlayerBoundaryDistance)
	{
		m_isActive = false;
	}
}

void Bullet::Draw() const
{
#ifdef _DEBUG
	if (!m_isActive) return;

	// Rayのデバッグ描画
	DrawLine3D(m_prevPos, m_pos, 0xffff00); // 黄色の線
	DrawSphere3D(m_pos, 2.0f, 16, 0xffff00, 0xffff00, false); // 仮の弾の描画
#endif
}

void Bullet::UpdateBullets(std::vector<Bullet>& bullets, const VECTOR& playerPos)
{
	for (auto& bullet : bullets)
	{
		bullet.Update(playerPos);
	}

	// 非アクティブな弾を削除
	bullets.erase(std::remove_if(bullets.begin(), bullets.end(), [](const Bullet& b) { return !b.IsActive(); }), bullets.end());
}

void Bullet::DrawBullets(const std::vector<Bullet>& bullets)
{
	for (const auto& bullet : bullets)
	{
		bullet.Draw();
	}
}

void Bullet::Deactivate()
{
	m_isActive = false;
}