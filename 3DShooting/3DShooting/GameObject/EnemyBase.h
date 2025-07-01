#pragma once
#include "DxLib.h"
#include <memory>
#include <vector>
#include "Player.h"

class Bullet;
//class Player;
class Collider;
class SphereCollider;
class CapsuleCollider;


/// <summary>
/// 敵の基底クラス
/// </summary>
class EnemyBase abstract
{
public:
	EnemyBase();
	virtual ~EnemyBase() = default;

	virtual void Init() abstract;
	virtual void Update(std::vector<Bullet>& bullets, const Player::TackleInfo& tackleInfo, const Player& player) abstract;
	virtual void Draw() abstract;


	/// <summary>
	/// 弾や攻撃を受ける処理(基底で共通処理) 
	/// </summary>
	/// <param name="bullets">弾のリスト</param>
	virtual void CheckHitAndDamage(std::vector<Bullet>& bullets);

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
	/// <param name="rayStart">弾のRayの始点</param>
	/// <param name="rayEnd">弾のRayの終点</param>
	/// <returns>当たった部位</returns>
	virtual HitPart CheckHitPart(const VECTOR& rayStart, const VECTOR& rayEnd, VECTOR& outHtPos, float& outHtDistSq) const { return HitPart::None; }

	/// <summary>
	/// タックルでダメージを受けたかどうかのフラグを取得
	/// </summary>
	virtual void ResetTackleHitFlag() abstract;

protected:
	// ダメージ計算用
	virtual float CalcDamage(float bulletDamage, HitPart part) const abstract;


protected:
	VECTOR m_pos; // 位置

	std::shared_ptr<Player> m_pTargetPlayer;  // ターゲットプレイヤー

	HitPart m_lastHitPart; // 最後に当たった部位

	int   m_modelHandle;       // モデルハンドル
	int   m_hitDisplayTimer;   // ヒット表示タイマー
	int   m_attackCooldown;    // 攻撃クールダウンタイマー
	int   m_attackCooldownMax; // 攻撃クールダウンの最大値
	int   m_attackHitFrame;    // 攻撃ヒットフレーム

	float m_hp;              // 体力
	float m_attackPower;     // 攻撃力

	bool  m_isAlive;         // 生存状態フラグ
	bool  m_isTackleHit;     // タックルで既にダメージを受けたか
	bool  m_isAttacking;     // 攻撃中かどうか
};