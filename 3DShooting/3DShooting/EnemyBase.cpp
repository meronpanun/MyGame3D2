#include "EnemyBase.h"
#include "Bullet.h"

// デフォルトのヒット表示持続時間
namespace
{
	constexpr int   kDefaultHitDisplayDuration = 60;     // 1秒間表示
	constexpr float kDefaultInitialHP          = 100.0f; // デフォルトの初期体力
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