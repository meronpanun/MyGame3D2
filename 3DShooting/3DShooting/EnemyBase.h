#pragma once
#include "DxLib.h"
#include <memory>
#include <vector>

class Player;
class Bullet;

/// <summary>
/// 敵の基底クラス
/// </summary>
class EnemyBase abstract
{
public:
	EnemyBase();
	virtual ~EnemyBase() = default;

	virtual void Init()    abstract;
	virtual void Update()  abstract;
	virtual void Draw()    abstract;

	// 弾や攻撃を受ける処理(基底で共通処理)
	void CheckHitAndDamage(const std::vector<Bullet>& bullets);

	// 当たり判定(派生クラスで実装)
	virtual bool IsHit(const Bullet& bullet) const abstract;

	// ダメージ処理
	virtual void TakeDamage(float damage);

	// デバッグ用当たり判定描画
	virtual void DrawCollisionDebug() const {}

protected:
	VECTOR m_pos;
	std::shared_ptr<Player> m_targetPlayer;

	int   m_modelHandle;
	float m_colRadius; // 当たり判定用半径
};

