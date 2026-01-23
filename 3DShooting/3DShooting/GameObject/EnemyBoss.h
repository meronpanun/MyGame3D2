#pragma once
#include "EnemyBase.h"
#include "AnimationManager.h"
#include <vector>
#include <memory>

class Bullet;
class Player;
class SphereCollider;
class CapsuleCollider;

/// <summary>
/// ボスクラス
/// </summary>
class EnemyBoss : public EnemyBase
{
public:
	EnemyBoss();
	virtual ~EnemyBoss();

	// モデルロード・アンロード
	static void LoadModel();
	static void DeleteModel();

	void Init() override;
	void Update(std::vector<Bullet>& bullets, const Player::TackleInfo& tackleInfo, const Player& player, const std::vector<EnemyBase*>& enemyList, const std::vector<Stage::StageCollisionData>& collisionData, Effect* pEffect = nullptr) override;
	void Draw() override;
	bool IsBoss() const override { return true; }

	// ダメージ処理
	void TakeDamage(float damage, AttackType type) override;
	void TakeTackleDamage(float damage) override;

	// コライダー取得
	std::shared_ptr<CapsuleCollider> GetBodyCollider() const override;

	// タックルヒットフラグのリセット
	void ResetTackleHitFlag() override { m_isTackleHit = false; }

	// 衝突判定
	HitPart CheckHitPart(const VECTOR& rayStart, const VECTOR& rayEnd, VECTOR& outHtPos, float& outHtDistSq) const override;

protected:
	// ダメージ計算
	float CalcDamage(float bulletDamage, HitPart part) const override;

	// デバッグ描画
	void DrawCollisionDebug() const override;

private:
	/// <summary>
	/// アニメーションを変更する
	/// </summary>
	/// <param name="newAnimState">新しいアニメーション状態</param>
	/// <param name="loop">ループ再生するかどうか</param>
	void ChangeAnimation(AnimState newAnimState, bool loop);

	/// <summary>
	/// 攻撃可能か判定
	/// </summary>
	bool CanAttackPlayer(const Player& player);

private:
	static int s_modelHandle;

	AnimationManager m_animationManager; // アニメーション管理
	AnimState m_currentAnimState;        // 現在のアニメーション状態
	bool m_isDeadAnimPlaying;            // 死亡アニメーション再生中フラグ
	float m_animTime;                    // アニメーションの経過時間

	// フレームインデックスキャッシュ
	int m_headNodeIndex;
	int m_headTopEndNodeIndex;
	int m_handRNodeIndex;
	int m_handLNodeIndex;

	std::shared_ptr<CapsuleCollider> m_pBodyCollider;        // 体の当たり判定
	std::shared_ptr<SphereCollider>  m_pHeadCollider;        // 頭の当たり判定
	std::shared_ptr<SphereCollider>  m_pAttackRangeCollider; // 攻撃範囲
	std::shared_ptr<CapsuleCollider> m_pAttackHitCollider;   // 攻撃判定(腕など)
	std::shared_ptr<SphereCollider>  m_pWeakCollider;		 // 弱点

	float m_chaseSpeed; // 追跡速度
	int m_attackEndDelayTimer; // 攻撃後の硬直タイマー
	bool m_isAttackHit; // 攻撃がヒットしたか

	// 遠距離攻撃用
	// 遠距離攻撃用
	struct HomingBullet
	{
		VECTOR pos;
		VECTOR dir;
		float speed;
		bool active;
		float damage;
		int effectHandle;
		float distTraveled; // 移動距離

		// パラメータ
		float turnRate; // 旋回性能

		HomingBullet() :
			pos(VGet(0, 0, 0)), dir(VGet(0, 0, 0)), speed(0), active(false), damage(0), effectHandle(-1), distTraveled(0), turnRate(0.05f) {
		}
	};

	std::vector<HomingBullet> m_homingBullets;
	int m_longRangeAttackCooldown;
	bool m_hasShotLongRange;
};

