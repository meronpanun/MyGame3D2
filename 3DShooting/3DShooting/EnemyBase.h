#pragma once
#include "DxLib.h"
#include <memory>

// 前方宣言
class Player;

// 敵の状態を表す列挙型
enum class EnemyState {
	IDLE,
	CHASE,
	ATTACK,
	DEAD
};

class EnemyBase
{
public:
	EnemyBase();
	virtual ~EnemyBase() = default;

	virtual void Init() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;

	// 共通の属性
	struct Stats {
		float maxHealth;    // 最大体力
		float health;       // 現在の体力
		float moveSpeed;    // 移動速度
		float attackPower;  // 攻撃力
		float attackRange;  // 攻撃範囲
		float attackCooldown; // 攻撃クールダウン
	};

	// ゲッター
	const Stats& GetStats() const { return m_stats; }
	const VECTOR& GetPosition() const { return m_position; }
	bool IsDead() const { return m_stats.health <= 0; }

protected:
	Stats m_stats;
	VECTOR m_position;
	EnemyState m_currentState;
	std::shared_ptr<Player> m_targetPlayer;

	int m_modelHandle;
	float m_attackTimer;

	// 共通のメソッド
	virtual void UpdateState() = 0;
	virtual void UpdateMovement() = 0;
	virtual void UpdateAttack() = 0;
	virtual void TakeDamage(float damage);
};

