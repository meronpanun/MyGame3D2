#pragma once
#include "Player.h"
#include "EffekseerForDXLib.h"
#include "AttackType.h"
#include <memory>
#include <vector>
#include <functional>

class Bullet;
class Collider;
class SphereCollider;
class CapsuleCollider;
class Effect;

/// <summary>
/// 敵の基底クラス
/// </summary>
class EnemyBase abstract
{
public:
	EnemyBase();
	virtual ~EnemyBase() = default;

	virtual void Init() abstract;
	virtual void Update(std::vector<Bullet>& bullets, const Player::TackleInfo& tackleInfo, const Player& player, const std::vector<EnemyBase*>& enemyList, const std::vector<Stage::StageCollisionData>& collisionData, Effect* pEffect = nullptr) abstract;
	virtual void Draw() abstract;

	/// <summary>
	/// 当たり判定の部位
	/// </summary>
	enum class HitPart
	{
		None,
		Body,
		Head
	};

	/// <summary>
	/// 敵の状態管理 
	/// </summary>
	enum class EnemyState
	{
		Idle,       // 待機状態
		Moving,     // 移動状態
		Attacking,  // 攻撃状態
		Damaged,    // ダメージを受けた状態
		Dead        // 死亡状態
	};

	/// <summary>
	/// アニメーション状態
	/// </summary>
	enum class AnimState
	{
		Idle,    // 待機
		Walk,    // 歩行
		Back,    // 後退
		Run,     // 走行
		Attack,  // 攻撃
		Dead     // 死亡
	};

	virtual int GetModelHandle() const { return -1; }

	/// <summary>
	/// 弾や攻撃を受ける処理(基底で共通処理) 
	/// </summary>
	/// <param name="bullets">弾のリスト</param>
	/// <param name="pEffect">エフェクトクラスのポインタ</param>
	virtual void CheckHitAndDamage(std::vector<Bullet>& bullets, Effect* pEffect = nullptr);

	virtual void TakeDamage(float damage, AttackType type);
	
	/// <summary>
	/// タックルダメージを受ける処理
	/// </summary>
	/// <param name="damage">受けるダメージ量</param>
	virtual void TakeTackleDamage(float damage);
	
	/// <summary>
	/// デバッグ用の当たり判定を描画する
	/// </summary>
	virtual void DrawCollisionDebug() const {}
	
	/// <summary>
	/// 生存判定
	/// </summary>
	/// <returns>生存しているならtrue</returns>
	virtual bool IsAlive() const { return m_isAlive; }
	
	/// <summary>
	/// 位置取得
	/// </summary>
	/// <returns>敵の位置</returns>
	virtual VECTOR GetPos() const { return m_pos; }
	
	/// <summary>
	/// 位置設定
	/// </summary>
	/// <param name="pos">敵の位置</param>
	virtual void SetPos(const VECTOR& pos) { m_pos = pos; }
	
	/// <summary>
	/// 体力設定
	/// </summary>
	/// <param name="hp">体力</param>
	virtual void SetHp(float hp) { m_hp = hp; }
	
	/// <summary>
	/// 死亡コールバックを設定する
	/// </summary>
	/// <param name="callback">死亡時に呼ばれるコールバック関数</param>
	virtual void SetOnDeathCallback(std::function<void(const VECTOR&)> callback) { m_onDeathCallback = callback; }	

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
	
	/// <summary>
	/// アイテムドロップ時のコールバックを設定する
	/// </summary>
	/// <param name="cb">アイテムドロップ時に呼ばれるコールバック関数</param>
	virtual void SetOnDropItemCallback(std::function<void(const VECTOR&)> cb) {}
	
	/// <summary>
	/// 敵の状態を設定する
	/// </summary>
	/// <param name="active">アクティブ状態かどうか</param>
	void SetActive(bool active) { m_isActive = active; }
	
	/// <summary>
	/// 敵がアクティブかどうかを取得する
	/// </summary>
	/// <returns>アクティブならtrue</returns>
	bool IsActive() const { return m_isActive; }
	
	/// <summary>
	/// ヒット時のコールバックを設定する
	/// </summary>
	/// <param name="cb">ヒット時に呼ばれるコールバック関数</param>
	void SetOnHitCallback(std::function<void(HitPart)> cb) { m_onHitCallback = cb; }
	
	/// <summary>
	/// 最後に当たった攻撃の種類を取得する
	/// </summary>
	/// <returns>最後に当たった攻撃の種類</returns>
	AttackType GetLastAttackType() const { return m_lastAttackType; }
	
	/// <summary>
	/// 最後に当たった攻撃の種類を設定する
	/// </summary>
	/// <param name="type">最後に当たった攻撃の種類</param>
	void SetLastAttackType(AttackType type) { m_lastAttackType = type; }
	
	/// <summary>
	/// 最後に当たった部位を取得する
	/// </summary>
	/// <param name="cb">最後に当たった部位</param>
	virtual void SetOnDeathWithTypeCallback(std::function<void(const VECTOR&, AttackType)> cb) { m_onDeathWithTypeCallback = cb; }

	/// <summary>
	/// 敵が死亡した際に呼ばれる処理
	/// </summary>
	virtual void OnDeath() {}

	/// <summary>
	/// ボディコライダーを取得する
	/// </summary>
	/// <returns>ボディコライダー</returns>
	virtual std::shared_ptr<CapsuleCollider> GetBodyCollider() const = 0;
	
protected:
    /// <summary>
    /// 弾のダメージを計算する
    /// </summary>
    /// <param name="bulletDamage">弾のダメージ量</param>
    /// <param name="part">当たった部位</param>
    /// <returns>計算されたダメージ量</returns>
    virtual float CalcDamage(float bulletDamage, HitPart part) const abstract;

	/// <summary>
	/// ステージとの当たり判定を更新する
	/// </summary>
	/// <param name="collisionData">ステージの当たり判定データ</param>
	void UpdateStageCollision(const std::vector<Stage::StageCollisionData>& collisionData);
	
protected:
    VECTOR m_pos; // 位置

    std::shared_ptr<Player> m_pTargetPlayer;  // ターゲットプレイヤー
    std::function<void(const VECTOR&)> m_onDeathCallback; // 死亡コールバック
    std::function<void(HitPart)> m_onHitCallback; // 部位情報付き
    std::function<void(const VECTOR&, AttackType)> m_onDeathWithTypeCallback;
	
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
    bool  m_isActive;		 // デフォルトはアクティブ

    AttackType m_lastAttackType = AttackType::None;
};