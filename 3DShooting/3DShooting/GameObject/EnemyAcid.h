#pragma once
#include "EnemyBase.h"
#include <vector>
#include <string>
#include <functional>
#include <memory>
#include "AnimationManager.h" 
#include "SphereCollider.h"   
#include "CapsuleCollider.h"  


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
    /// タックルダメージを受けたかどうかのフラグをリセットする
    /// </summary>
    void ResetTackleHitFlag() override { m_isTackleHit = false; }

    /// <summary>
    /// アイテムドロップ時のコールバック関数を設定する
    /// </summary>
    /// <param name="cb">コールバック関数</param>
    void SetOnDropItemCallback(std::function<void(const VECTOR&)> cb);

    void SetModelHandle(int handle);
    int GetModelHandle() const { return m_modelHandle; }

    // ダメージ処理
    void TakeDamage(float damage) override;

private:
	struct AcidBall
	{
		VECTOR pos; // 弾の位置
		VECTOR dir; // 弾の進行方向

        bool active = false;
        float radius = 12.0f;
        float damage = 30.0f;

        void Update() 
        {
            if (!active) return;
            dir.y -= 0.4f; // 重力
            pos = VAdd(pos, dir);
            if (pos.y < 0.0f) active = false; // 地面で消滅
        }
	};

    // アニメーション切り替え関数
    void ChangeAnimation(AnimState newAnimState, bool loop);

    // プレイヤーに攻撃可能かどうかを判定する
    bool CanAttackPlayer(const Player& player);

    // 酸を吐く攻撃処理
    void ShootAcidBullet(std::vector<Bullet>& bullets, const Player& player);

private:
    VECTOR m_headPosOffset; // ヘッドショット判定用オフセット座標
    AnimationManager m_animationManager; // アニメーション管理
    std::shared_ptr<CapsuleCollider> m_pBodyCollider;      // 体のコライダー
    std::shared_ptr<SphereCollider>  m_pHeadCollider;      // 頭のコライダー
    std::shared_ptr<SphereCollider>  m_pAttackRangeCollider; // 攻撃範囲のコライダー
    std::vector<AcidBall> m_acidBalls;
    std::function<void(const VECTOR&)> m_onDropItem; // アイテムドロップコールバック
    // 酸を吐く場所のオフセット
    VECTOR m_acidBulletSpawnOffset;
    AnimState m_currentAnimState; // 現在のアニメーション状態

    int m_attackEndDelayTimer; // 攻撃後の硬直時間
    int m_backAnimCount;
    int m_lastTackleId; // 最後にタックルを受けたID

    float m_animTime;   // 現在のアニメーション再生時間
    float m_chaseSpeed; // 追跡速度

    bool m_hasAttacked;           // 攻撃アニメーション中に一度だけ攻撃ヒット判定を行うためのフラグ
    bool m_isDeadAnimPlaying;     // 死亡アニメーション再生中フラグ
    bool m_isItemDropped; // アイテムドロップ済みフラグ
};