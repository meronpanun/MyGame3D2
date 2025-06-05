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
	virtual void Update(const std::vector<Bullet>& bullets) abstract; // 弾リストを受け取る
	virtual void Draw()    abstract;

	// 弾や攻撃を受ける処理(基底で共通処理)
	virtual void CheckHitAndDamage(const std::vector<Bullet>& bullets) abstract;

	// 当たり判定
	virtual bool IsHit(const Bullet& bullet) const abstract;

	// ダメージ処理
	virtual void TakeDamage(float damage) abstract;

	// デバッグ用当たり判定描画
	virtual void DrawCollisionDebug() const {}

	// どこに当たったか
	enum class HitPart {
		None,
		Body,
		Head
	};

	// 派生クラスでどこに当たったか判定する仮想関数
	virtual HitPart CheckHitPart(const Bullet& bullet) const { return HitPart::None; }


protected:
	VECTOR m_pos;
	std::shared_ptr<Player> m_targetPlayer;

	int   m_modelHandle;
	float m_colRadius; // 当たり判定用半径
};

