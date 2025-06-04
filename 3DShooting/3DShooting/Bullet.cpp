#include "Bullet.h"
#include "DxLib.h"

namespace
{
	// �e�̔��a
	constexpr float kBulletRadius = 2.0f;
	// �e�̑��x
	constexpr float kBulletSpeed = 25.0f;

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

	// Ray�̎n�_�ƏI�_
	VECTOR rayStart = m_pos;
	VECTOR rayDir = VNorm(m_dir);
	float rayLength = m_speed;
	VECTOR rayEnd = VAdd(rayStart, VScale(rayDir, rayLength));

	// ������Ray���菈�����s���i��F�G��ǂƂ̓����蔻��j
	// ��: if (CheckHit(rayStart, rayEnd)) { m_isActive = false; }

	// Ray�̏I�_�܂Ői�߂�
	m_pos = rayEnd;

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
#ifdef _DEBUG
	if (!m_isActive) return;

	// Ray�̎n�_�ƏI�_
	VECTOR rayStart = m_pos;
	VECTOR rayDir = VNorm(m_dir);
	float rayLength = m_speed;
	VECTOR rayEnd = VAdd(rayStart, VScale(rayDir, rayLength));

	// �f�o�b�O�p��Ray��`��
	DrawLine3D(rayStart, rayEnd, 0xff0000);
#endif
}

// �e�̍X�V
void Bullet::UpdateBullets(std::vector<Bullet>& bullets)
{
	for (auto& bullet : bullets)
	{
		bullet.Update();
	}
	// �e�̍폜
	bullets.erase(std::remove_if(bullets.begin(), bullets.end(), [](const Bullet& bullet){
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

