#include "EnemyBase.h"
#include "Bullet.h"

EnemyBase::EnemyBase(): 
	m_pos{ 0, 0, 0 },
	m_modelHandle(-1)
{
}

void EnemyBase::CheckHitAndDamage(const std::vector<Bullet>& bullets)
{
	// 派生クラスでIsHitを実装
	for (const auto& bullet : bullets) 
	{
		if (IsHit(bullet)) 
		{
			TakeDamage(bullet.GetRadius()); // 仮: 弾の半径をダメージに
			// bullet.Deactivate(); // 必要に応じて
		}
	}
}

void EnemyBase::TakeDamage(float damage)
{
}