#pragma once
#include "DxLib.h"
#include <memory>

class Player;

// 敵の状態を表す列挙型
enum class EnemyState 
{
	IDLE,
	CHASE,
	ATTACK,
	DEAD
};

class EnemyBase abstract
{
public:
	EnemyBase() {};
	virtual ~EnemyBase() = default;

	virtual void Init()    abstract;
	virtual void Update()  abstract;
	virtual void Draw()    abstract;

	// 共通の属性
	struct Stats 
	{
		float maxHealth;    // 最大体力
		float health;       // 現在の体力
		float moveSpeed;    // 移動速度
		float attackPower;  // 攻撃力
		float attackRange;  // 攻撃範囲
		float attackCooldown; // 攻撃クールダウン
	};

	// ゲッター
	const Stats& GetStats() const { return m_stats; }
	const VECTOR& GetPos() const { return m_pos; }
	bool IsDead() const { return m_stats.health <= 0; }

protected:
	Stats m_stats;
	VECTOR m_pos;
	EnemyState m_currentState;
	std::shared_ptr<Player> m_targetPlayer;

	int m_modelHandle;
	float m_attackTimer;

	// 共通のメソッド
	virtual void UpdateState() = 0;
	virtual void UpdateMovement() = 0;
	virtual void UpdateAttack() = 0;
//	virtual void TakeDamage(float damage);
};

