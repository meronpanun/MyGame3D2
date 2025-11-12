#pragma once
#include "EnemyBase.h"
#include "AnimationManager.h" 
#include "SphereCollider.h"   
#include "CapsuleCollider.h"  
#include <string>
#include <vector>
#include <functional>
#include <memory>

class Player;
class Collider;

/// <summary>
/// 遠距離型ゾンビクラス
/// </summary>
class EnemyAcid : public EnemyBase
{
public:
    EnemyAcid();
    virtual ~EnemyAcid();

    void Init() override;
    void Update(std::vector<Bullet>& bullets, const Player::TackleInfo& tackleInfo, const Player& player, const std::vector<EnemyBase*>& enemyList, Effect* pEffect = nullptr) override;
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
    /// タックルダメージを受けたかどうかのフラグをリセットする
    /// </summary>
    void ResetTackleHitFlag() override { m_isTackleHit = false; }

    /// <summary>
    /// アイテムドロップ時のコールバック関数を設定する
    /// </summary>
    /// <param name="cb">コールバック関数</param>
    void SetOnDropItemCallback(std::function<void(const VECTOR&)> cb);

    /// <summary>
	/// モデルハンドルを取得する
    /// </summary>
	/// <returns>モデルハンドル</returns>
    int GetModelHandle() const { return m_modelHandle; }

    /// <summary>
	/// ダメージを受ける処理
    /// </summary>
	/// <param name="damage">受けるダメージ量</param>
	/// <param name="type">攻撃の種類</param>
    void TakeDamage(float damage, AttackType type) override;

    /// <summary>
	/// タックルダメージを受ける処理 
    /// </summary>
	/// <param name="damage">受けるダメージ量</param>
    void TakeTackleDamage(float damage) override;

    /// <summary>
	/// モデルの読み込み(共有)
    /// </summary>
    static void LoadModel();

    /// <summary>
	/// モデルの解放(共有)
	/// </summary>
	static void DeleteModel();

	/// <summary>
	/// ボディコライダーを取得する
	/// </summary>
	/// <returns>ボディコライダー</returns>
	std::shared_ptr<CapsuleCollider> GetBodyCollider() const override;
    
    /// <summary>
    /// パリィされた時に呼び出される
    /// </summary>
    void OnParried();

private:

	/// <summary>
	/// 酸の弾構造体
	/// </summary>
	struct AcidBall
	{
		VECTOR pos; // 弾の位置
		VECTOR dir; // 弾の進行方向

		bool active = false;
		float radius = 12.0f;
		float damage = 0.0f;
		float speed = 5.0f; // 弾の速度
		int effectHandle = -1;
		bool isReflected = false; // パリィで反射されたか
		EnemyBase* owner = nullptr; // この弾の所有者

		void Update()
		{
			if (!active) return;

			// 反射された弾はオーナーを追尾する
			if (isReflected && owner)
			{
				VECTOR targetPos = owner->GetPos();
				// 敵の中心あたりを狙うオフセット
				targetPos.y += 50.0f;
				VECTOR newDir = VNorm(VSub(targetPos, pos));
				dir = newDir;
			}

			pos = VAdd(pos, VScale(dir, speed));
			if (pos.y < 0.0f) active = false; // 地面で消滅
		}
	};

    /// <summary>
	/// アニメーションを変更する
    /// </summary>
	/// <param name="newAnimState">新しいアニメーション状態</param>
	/// <param name="loop">ループ再生するかどうか</param>
    void ChangeAnimation(AnimState newAnimState, bool loop);

    /// <summary>
	/// プレイヤーに攻撃可能かどうかを判定する
    /// </summary>
	/// <param name="player">プレイヤーオブジェクト</param>
	/// <returns>攻撃可能ならtrue</returns>
    bool CanAttackPlayer(const Player& player);

    /// <summary>
	/// 酸を吐く攻撃を行う
    /// </summary>
	/// <param name="bullets">弾のリスト</param>
	/// <param name="player">プレイヤーオブジェクト</param>
	/// <param name="pEffect">エフェクトオブジェクト</param>
    void ShootAcidBullet(std::vector<Bullet>& bullets, const Player& player, Effect* pEffect);

private:
    VECTOR m_headPosOffset;              // ヘッドショット判定用オフセット座標
    VECTOR m_acidBulletSpawnOffset;      // 酸を吐く場所のオフセット
    AnimationManager m_animationManager; // アニメーション管理
    AnimState m_currentAnimState;        // 現在のアニメーション状態
    std::shared_ptr<CapsuleCollider> m_pBodyCollider;        // 体のコライダー
    std::shared_ptr<SphereCollider>  m_pHeadCollider;        // 頭のコライダー
    std::shared_ptr<SphereCollider>  m_pAttackRangeCollider; // 攻撃範囲のコライダー
    std::vector<AcidBall> m_acidBalls;
    std::function<void(const VECTOR&)> m_onDropItem; // アイテムドロップコールバック

    int m_attackEndDelayTimer; // 攻撃後の硬直時間
	int m_backAnimCount;       // 後退アニメーションのカウント
    int m_lastTackleId;        // 最後にタックルを受けたID

    float m_animTime;   // 現在のアニメーション再生時間
    float m_chaseSpeed; // 追跡速度

    bool m_hasAttacked;       // 攻撃アニメーション中に一度だけ攻撃ヒット判定を行うためのフラグ
    bool m_isDeadAnimPlaying; // 死亡アニメーション再生中フラグ
    bool m_isItemDropped;     // アイテムドロップ済みフラグ
    bool m_isStunned;         // 怯み状態か
    int m_stunTimer;          // 怯みタイマー

    static int s_modelHandle; // 共有モデルハンドル

#ifdef _DEBUG
    bool m_shouldDrawParryCollider; // パリィコライダーを描画するか
    VECTOR m_debugParryCapA;        // デバッグ用パリィカプセルのA点
    VECTOR m_debugParryCapB;        // デバッグ用パリィカプセルのB点
    float m_debugParryRadius;       // デバッグ用パリィカプセルの半径
#endif
};