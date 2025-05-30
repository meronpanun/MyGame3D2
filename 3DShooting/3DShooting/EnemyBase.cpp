#include "EnemyBase.h"

EnemyBase::EnemyBase()
	: m_stats{ 100.0f, 100.0f, 1.0f, 10.0f, 5.0f, 1.0f },
	m_pos{ 0, 0, 0 },
	m_currentState(EnemyState::IDLE),
	m_modelHandle(-1),
	m_attackTimer(0.0f)
{
}
