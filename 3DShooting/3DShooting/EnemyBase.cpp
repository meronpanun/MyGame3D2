#include "Player.h"
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
	m_hitDisplayTimer(0),
	m_isAlive(true),
	m_isTackleHit(false)
{
}

void EnemyBase::CheckHitAndDamage(std::vector<Bullet>& bullets)
{
	for (auto& bullet : bullets)
	{
		// 弾が非アクティブならスキップ
		if (!bullet.IsActive()) continue;

		// どこに当たったのかをチェック
		HitPart part = CheckHitPart(bullet);

		if (part == HitPart::Head || part == HitPart::Body)
		{
			float damage = CalcDamage(bullet, part);
			TakeDamage(damage);

			m_lastHitPart = part;
			m_hitDisplayTimer = kDefaultHitDisplayDuration;

			bullet.Deactivate();
			break;
		}
	}
}

// 敵がダメージを受ける処理
void EnemyBase::TakeDamage(float damage)
{
	m_hp -= damage;
	if (m_hp <= 0.0f)
	{
		m_hp = 0.0f;
		m_isAlive = false;
	}
}

// 敵がタックルダメージを受ける処理
void EnemyBase::TakeTackleDamage(float damage)
{
	TakeDamage(damage); // デフォルトは通常ダメージと同じ

	// ヒット表示を体ヒットとして更新
	m_lastHitPart = HitPart::Body;
	m_hitDisplayTimer = kDefaultHitDisplayDuration; // 1秒間表示
}
