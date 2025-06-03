#include "Bullet.h"
#include "DxLib.h"

namespace
{
	// 弾の半径
	constexpr float kBulletRadius = 5.0f;
	// 弾の速度
	constexpr float kBulletSpeed = 20.0f;

	// 画面外に出たら非アクティブにする範囲
	constexpr int kScreenBoundary = 1000;
}

Bullet::Bullet(VECTOR position, VECTOR direction) :
	m_pos(position),
	m_dir(direction),
	m_speed(kBulletSpeed),
	m_isActive(true),
	m_radius(kBulletRadius)
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

	// 弾の移動
	m_pos = VAdd(m_pos, VScale(m_dir, m_speed));

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
	if (!m_isActive) return;

	// 弾の描画
	DrawSphere3D(m_pos, m_radius, 32, 0xffff00, 0xffff00, true);
}

// 弾の更新
void Bullet::UpdateBullets(std::vector<Bullet>& bullets)
{
	for (auto& bullet : bullets)
	{
		bullet.Update();
	}
	// 弾の削除
	bullets.erase(std::remove_if(bullets.begin(), bullets.end(), [](const Bullet& bullet)
		{
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

// 弾を非アクティブ化
void Bullet::Deactivate()
{
	m_isActive = false;
}

