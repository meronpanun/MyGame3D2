#include "Bullet.h"
#include "DxLib.h"
#include <algorithm>

namespace
{
	// 弾の半径
	constexpr float kBulletRadius = 2.0f;
	// 弾の速度
	constexpr float kBulletSpeed = 25.0f;

	// 画面外に出たら非アクティブにする範囲
	constexpr int kScreenBoundary = 1000;
}

Bullet::Bullet(VECTOR position, VECTOR direction, float damage) :
	m_pos(position),
	m_dir(direction),
	m_speed(kBulletSpeed),
	m_isActive(true),
	m_radius(kBulletRadius),
	m_damage(damage)
{
}

Bullet::~Bullet()
{
}

void Bullet::Init()
{
}

void Bullet::Update()
{
	if (!m_isActive) return;

	// Rayの始点と終点
	VECTOR rayStart = m_pos;	    // 弾の位置
	VECTOR rayDir   = VNorm(m_dir); // 弾の方向を正規化
	float rayLength = m_speed;	    // 弾の速度をRayの長さとする
	VECTOR rayEnd   = VAdd(rayStart, VScale(rayDir, rayLength)); // Rayの終点

// ここでRayとの衝突判定を行う(例：地形との当たり判定)
	// 注: if (CheckHit(rayStart, rayEnd)) { m_isActive = false; }

	// Rayの終点まで進む
	m_pos = rayEnd;

	// 画面外に出たら非アクティブにする
	if (m_pos.x < -kScreenBoundary || m_pos.x > kScreenBoundary ||
		m_pos.y < -kScreenBoundary || m_pos.y > kScreenBoundary ||
		m_pos.z < -kScreenBoundary || m_pos.z > kScreenBoundary)
	{
		m_isActive = false;
	}
}

void Bullet::Draw() const
{
#ifdef _DEBUG
	if (!m_isActive) return;

	// Rayの始点と終点
	VECTOR rayStart = m_pos;        // 弾の位置
	VECTOR rayDir   = VNorm(m_dir); // 弾の方向を正規化
	float rayLength = m_speed;      // 弾の速度をRayの長さとする
	VECTOR rayEnd   = VAdd(rayStart, VScale(rayDir, rayLength)); // Rayの終点

	// デバッグ用にRayを描画
	DrawLine3D(rayStart, rayEnd, 0xff0000);
#endif
}

// 弾の削除
void Bullet::UpdateBullets(std::vector<Bullet>& bullets)
{
	for (auto& bullet : bullets) 
	{
		bullet.Update();
	}

	// 弾が非アクティブな場合は削除
	bullets.erase(std::remove_if(bullets.begin(), bullets.end(), [](const Bullet& bullet) {
		return !bullet.IsActive(); 
	}), bullets.end());
}

// 弾の描画
void Bullet::DrawBullets(const std::vector<Bullet>& bullets)
{
	for (const auto& bullet : bullets)
	{
		bullet.Draw();
	}
}

// 弾の非アクティブ化
void Bullet::Deactivate()
{
	m_isActive = false;
}

