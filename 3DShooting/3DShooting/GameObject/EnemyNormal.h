#pragma once
#include "EnemyBase.h"
#include <vector>
#include <string>

class Bullet;
class Player;

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
    /// 当たり判定を行う
    /// </summary>
    /// <param name="bullet">弾の情報</param>
    /// <returns>当たったかどうか</returns>
    virtual bool IsHit(const Bullet& bullet) const override;

    /// <summary>
    /// デバッグ用の当たり判定を描画する
    /// </summary>
    virtual void DrawCollisionDebug() const override;

    /// <summary>
    /// どこに当たったかを判定する
    /// </summary>
    /// <param name="bullet">弾の情報</param>
    /// <returns>当たった部位</returns>
    HitPart CheckHitPart(const Bullet& bullet) const override;

	/// <summary>
	/// ダメージを計算する
	/// </summary>
	/// <param name="bullet">弾の情報</param>
	/// <param name="part">当たった部位</param>
	/// <returns>計算されたダメージ</returns>
	float CalcDamage(const Bullet& bullet, HitPart part) const override;

	// 敵がタックルダメージを受ける処理
    VECTOR GetAABBMinWorld() const override { return VAdd(m_pos, m_aabbMin); }
    VECTOR GetAABBMaxWorld() const override { return VAdd(m_pos, m_aabbMax); }

    /// <summary>
	/// タックルダメージを受ける処理
    /// </summary>
    void ResetTackleHitFlag() { m_isTackleHit = false; }

private:
    VECTOR m_aabbMin; // AABB最小座標
    VECTOR m_aabbMax; // AABB最大座標
    VECTOR m_headPos; // ヘッドショット判定用中心座標

	int m_lastTackleId; // 最後にタックルを受けたID
    int m_currentAnimIndex; // 現在再生中のアニメーションインデックス

    float m_animTime; // アニメーションの経過時間
    float m_headRadius; // ヘッドショット判定用半径  

    bool m_isTackleHit; // 1フレームで複数回ダメージを受けないためのフラグ
    bool m_currentAnimLoop;
    bool m_hasAttackHit;
};

