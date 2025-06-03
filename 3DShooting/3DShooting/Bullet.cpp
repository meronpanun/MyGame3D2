#include "Bullet.h"
#include "DxLib.h"

namespace
{
	// �e�̔��a
	constexpr float kBulletRadius = 5.0f;
	// �e�̑��x
	constexpr float kBulletSpeed = 20.0f;

	// ��ʊO�ɏo�����A�N�e�B�u�ɂ���͈�
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

	// �e�̈ړ�
	m_pos = VAdd(m_pos, VScale(m_dir, m_speed));

	// ��ʊO�ɏo�����A�N�e�B�u�ɂ���
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

	// �e�̕`��
	DrawSphere3D(m_pos, m_radius, 32, 0xffff00, 0xffff00, true);
}

// �e�̍X�V
void Bullet::UpdateBullets(std::vector<Bullet>& bullets)
{
	for (auto& bullet : bullets)
	{
		bullet.Update();
	}
	// �e�̍폜
	bullets.erase(std::remove_if(bullets.begin(), bullets.end(), [](const Bullet& bullet)
		{
			return !bullet.IsActive();
		}), bullets.end());
}

// �e�̕`��
void Bullet::DrawBullets(const std::vector<Bullet>& bullets)
{
	for (const auto& bullet : bullets)
	{
		bullet.Draw();
	}
}

// �e���A�N�e�B�u��
void Bullet::Deactivate()
{
	m_isActive = false;
}

