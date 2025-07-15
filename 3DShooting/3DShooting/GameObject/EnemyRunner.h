#pragma once
#include "EnemyBase.h"
#include "AnimationManager.h" 
#include <vector>
#include <memory>
#include <functional>

class Bullet;
class Player;
class Collider;
class CapsuleCollider;
class SphereCollider;

/// <summary>
/// 走る敵クラス
/// </summary>
class EnemyRunner : public EnemyBase
{
public:
	EnemyRunner();
	virtual ~EnemyRunner();

	void Init() override;
	void Update(std::vector<Bullet>& bullets, const Player::TackleInfo& tackleInfo, const Player& player, const std::vector<EnemyBase*>& enemyList) override;
	void Draw() override;

	/// <summary>
	/// デバッグ用の当たり判定を描画する
	/// </summary>
	virtual void DrawCollisionDebug() const override;

	/// <summary>
	/// どこに当たったかを判定する
	/// </summary>
	/// <param name="rayStart">弾のRayの始点</param>
	/// <param name="rayEnd">弾のRayの終点</param>
	/// <returns>当たった部位</returns>
	HitPart CheckHitPart(const VECTOR& rayStart, const VECTOR& rayEnd, VECTOR& outHtPos, float& outHtDistSq) const override;

	/// <summary>
	/// ダメージを計算する
	/// </summary>
	/// <param name="bulletDamage">弾のダメージ量</param>
	/// <param name="part">当たった部位</param>
	/// <returns>計算されたダメージ</returns>
	float CalcDamage(float bulletDamage, HitPart part) const override;

	/// <summary>
	/// タックルダメージを受ける処理
	/// </summary>
	void ResetTackleHitFlag() { m_isTackleHit = false; }

	/// <summary>
	/// アイテムドロップ時のコールバック関数を設定する
	/// </summary>
	/// <param name="cb">コールバック関数</param>
	void SetOnDropItemCallback(std::function<void(const VECTOR&)> cb);

	void SetModelHandle(int handle);
	int GetModelHandle() const { return m_modelHandle; }

private:
	void ChangeAnimation(AnimState newAnimState, bool loop); // アニメーション切り替え関数

	bool CanAttackPlayer(const Player& player);

private:
	VECTOR m_headPosOffset; // ヘッドショット判定用座標

	std::shared_ptr<CapsuleCollider> m_pBodyCollider; // 体のコライダー
	std::shared_ptr<SphereCollider>  m_pHeadCollider;  // 頭のコライダー
	std::shared_ptr<SphereCollider>  m_pAttackRangeCollider; // 攻撃範囲のコライダー
	std::shared_ptr<CapsuleCollider> m_pAttackHitCollider;   // 攻撃ヒット判定用のコライダー

	// アイテムドロップ時のコールバック関数
	std::function<void(const VECTOR&)> m_onDropItem;

	AnimState m_currentAnimState;	     // 現在のアニメーション状態
	AnimationManager m_animationManager; // EnemyRunnerがアニメーションマネージャーを所有

	int m_lastTackleId;        // 最後にタックルを受けたID
	int m_attackEndDelayTimer; // 攻撃終了までの遅延タイマー

	float m_animTime;   // アニメーションの経過時間
	float m_chaseSpeed; // 追跡速度

	bool m_isTackleHit;		  // 1フレームで複数回ダメージを受けないためのフラグ
	bool m_hasAttackHit;	  // 攻撃がヒットしたかどうか
	bool m_isDeadAnimPlaying; // 死亡アニメーション再生中フラグ
	bool m_isItemDropped = false; // アイテムドロップ済みフラグ
};