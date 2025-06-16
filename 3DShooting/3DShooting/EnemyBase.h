#pragma once
#include "DxLib.h"
#include <memory>
#include <vector>

class Bullet;
class Player;

/// <summary>
/// 敵の基底クラス
/// </summary>
class EnemyBase abstract
{
public:
	EnemyBase();
	virtual ~EnemyBase() = default;

	virtual void Init() abstract;
	virtual void Update(std::vector<Bullet>& bullets, const Player::TackleInfo& tackleInfo) abstract;
	virtual void Draw() abstract;
	

	/// <summary>
	/// 弾や攻撃を受ける処理(基底で共通処理) 
	/// </summary>
	/// <param name="bullets">弾のリスト</param>
	virtual void CheckHitAndDamage(std::vector<Bullet>& bullets); 

	/// <summary>
	/// 敵が弾に当たったかどうかをチェックする
	/// </summary>
	/// <param name="bullet">弾の情報</param>
	/// <returns>当たったかどうか</returns>
	virtual bool IsHit(const Bullet& bullet) const abstract;

	/// <summary>
	/// 敵がダメージを受ける処理
	/// </summary>
	/// <param name="damage">受けるダメージ量</param>
	virtual void TakeDamage(float damage);

	/// <summary>
	/// タックルダメージを受ける処理
	/// </summary>
	/// <param name="damage">受けるダメージ量</param>
	virtual void TakeTackleDamage(float damage);

	/// <summary>
	/// デバッグ用の当たり判定を描画する
	/// </summary>
	virtual void DrawCollisionDebug() const {}

	// 生存判定
	virtual bool IsAlive() const { return m_isAlive; }

	// 位置取得
	virtual VECTOR GetPos() const { return m_pos; }

	// 当たり判定の部位
	enum class HitPart {
		None,
		Body,
		Head
	};

	// 敵の状態管理
	enum class EnemyState {
		Idle,       // 待機状態
		Moving,     // 移動状態
		Attacking,  // 攻撃状態
		Damaged,    // ダメージを受けた状態
		Dead        // 死亡状態
	};

	/// <summary>
	/// 派生クラスでどこに当たったか判定する仮想関数
	/// </summary>
	/// <param name="bullet">弾の情報</param>
	/// <returns>当たった部位</returns>
	virtual HitPart CheckHitPart(const Bullet& bullet) const { return HitPart::None; }

	// AABBの最小座標と最大座標を取得する
	virtual VECTOR GetAABBMinWorld() const abstract;
	virtual VECTOR GetAABBMaxWorld() const abstract;

	/// <summary>
	/// タックルでダメージを受けたかどうかのフラグを取得
	/// </summary>
	virtual void ResetTackleHitFlag() abstract;

	/// <summary>
	/// 敵の攻撃を更新する
	/// </summary>
	virtual void UpdateAttack();

	/// <summary>
	/// プレイヤー攻撃処理
	/// </summary>
	/// <param name="player">攻撃対象のプレイヤー</param>
	virtual void AttackPlayer(Player* player);

protected:
	// ダメージ計算用
    virtual float CalcDamage(const Bullet& bullet, HitPart part) const abstract;
    

protected:
	VECTOR m_pos; // 位置

	std::shared_ptr<Player> m_targetPlayer;  // ターゲットプレイヤー

	HitPart m_lastHitPart; // 最後に当たった部位

	int   m_modelHandle;       // モデルハンドル
	int   m_hitDisplayTimer;   // ヒット表示タイマー
	int   m_attackCooldown;    // 攻撃クールダウンタイマー
	int   m_attackCooldownMax; // 攻撃クールダウンの最大値

	float m_colRadius;       // 当たり判定用半径
	float m_hp;              // 体力
	float m_attackPower;     // 攻撃力
	float m_attackRange;     // 攻撃範囲

	bool  m_isAlive;         // 生存状態フラグ
	bool  m_isTackleHit;     // タックルで既にダメージを受けたか
};

