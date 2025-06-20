#include "Player.h"
#include "EnemyBase.h"
#include "Bullet.h"

namespace
{
	constexpr int   kDefaultHitDisplayDuration = 60;     // 1秒間表示
	constexpr float kDefaultInitialHP          = 100.0f; // デフォルトの初期体力 

	constexpr float kDefaultCooldownMax = 60;     // 攻撃クールダウンの最大値
	constexpr float kDefaultAttackPower = 10.0f;  // 攻撃力
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
	m_isTackleHit(false),
	m_attackCooldown(0),
	m_attackCooldownMax(kDefaultCooldownMax),
	m_attackPower(kDefaultAttackPower),
	m_attackHitFrame(0),
	m_isAttacking(false)
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

// 敵の攻撃を更新する処理
void EnemyBase::UpdateAttack()
{
}

// プレイヤーに攻撃を行う処理
void EnemyBase::AttackPlayer(Player* player)
{
	if (!player) return;
	player->TakeDamage(m_attackPower);
}

