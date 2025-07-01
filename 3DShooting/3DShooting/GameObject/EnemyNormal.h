#pragma once
#include "EnemyBase.h"
#include <vector>
#include <string>
#include <functional>
#include <memory>

class Bullet;
class Player;
class Collider;
class SphereCollider;
class CapsuleCollider;

/// <summary>
/// 通常の敵クラス
/// </summary>
class EnemyNormal : public EnemyBase
{
public:
    EnemyNormal();
    virtual ~EnemyNormal();

    void Init() override;
    void Update(std::vector<Bullet>& bullets, const Player::TackleInfo& tackleInfo, const Player& player) override;
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

private:
    VECTOR m_headPosOffset; // ヘッドショット判定用オフセット座標

    std::shared_ptr<CapsuleCollider> m_pBodyCollider; // 体のコライダー
    std::shared_ptr<SphereCollider> m_pHeadCollider;  // 頭のコライダー

    // 攻撃範囲のコライダー
    std::shared_ptr<SphereCollider> m_pAttackRangeCollider;

    // 攻撃ヒット判定用のコライダー
    std::shared_ptr<CapsuleCollider> m_pAttackHitCollider;

    // アイテムドロップ時のコールバック関数
    std::function<void(const VECTOR&)> m_onDropItem;

    int m_lastTackleId;     // 最後にタックルを受けたID
    int m_currentAnimIndex; // 現在再生中のアニメーションインデックス

    float m_animTime;   // アニメーションの経過時間

    bool m_isTackleHit;     // 1フレームで複数回ダメージを受けないためのフラグ
    bool m_currentAnimLoop; // 現在のアニメーションがループするかどうか 
    bool m_hasAttackHit;    // 攻撃がヒットしたかどうか
};