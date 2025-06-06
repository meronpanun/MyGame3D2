#include "EnemyBase.h"
#include "Bullet.h"

// �f�t�H���g�̃q�b�g�\����������
namespace
{
	constexpr int   kDefaultHitDisplayDuration = 60;     // 1�b�ԕ\��
	constexpr float kDefaultInitialHP          = 100.0f; // �f�t�H���g�̏����̗�
}

EnemyBase::EnemyBase() :
	m_pos{ 0, 0, 0 },
	m_modelHandle(-1),
	m_colRadius(1.0f), 
	m_targetPlayer(nullptr),
	m_hp(kDefaultInitialHP),
	m_lastHitPart(HitPart::None),
	m_hitDisplayTimer(0)         
{
}