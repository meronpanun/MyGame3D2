#include "EnemyBase.h"
#include "Bullet.h"

EnemyBase::EnemyBase(): 
	m_pos{ 0, 0, 0 },
	m_modelHandle(-1)
{
}

void EnemyBase::CheckHitAndDamage(const std::vector<Bullet>& bullets)
{
	// �h���N���X��IsHit������
	for (const auto& bullet : bullets) 
	{
		if (IsHit(bullet)) 
		{
			TakeDamage(bullet.GetRadius()); // ��: �e�̔��a���_���[�W��
			// bullet.Deactivate(); // �K�v�ɉ�����
		}
	}
}

void EnemyBase::TakeDamage(float damage)
{
}