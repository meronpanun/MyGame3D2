#include "EnemyBase.h"
#include "Bullet.h"

EnemyBase::EnemyBase(): 
	m_pos{ 0, 0, 0 },
	m_modelHandle(-1),
	m_colRadius(1.0f),
	m_targetPlayer(nullptr)
{
}