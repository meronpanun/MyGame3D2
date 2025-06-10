#pragma once
#include "EnemyBase.h"

class Bullet;

/// <summary>
/// 通常の敵クラス
/// </summary>
class EnemyNormal : public EnemyBase
{
public:
    EnemyNormal(); 
    virtual ~EnemyNormal();

    void Init() override;
	void Update(const std::vector<Bullet>& bullets) override; // 弾のリストを受け取る
    void Draw() override;

    /// <summary>
    /// 当たり判定を行う関数
    /// </summary>
    /// <param name="bullet">弾の情報</param>
    /// /// <returns>当たったかどうか</returns>
    virtual bool IsHit(const Bullet& bullet) const override;

    /// <summary>
    /// デバッグ用の当たり判定を描画する関数
    /// </summary>
    virtual void DrawCollisionDebug() const override;

    /// <summary>
    /// どこに当たったかを判定する関数
    /// </summary>
    /// <param name="bullet">弾の情報</param>
    /// <returns>当たった部位</returns>
    HitPart CheckHitPart(const Bullet& bullet) const override;

    /// <summary>
    /// 敵が弾に当たったかどうかをチェックし、ダメージを受ける処理
    /// </summary>
    /// /// <param name="bullets">弾のリスト</param>
    virtual void CheckHitAndDamage(std::vector<Bullet>& bullets) override; 

    /// <summary>
    /// 敵がダメージを受ける処理
    /// </summary>
    /// <param name="damage">受けるダメージ量</param>
    void TakeDamage(float damage) override;

private:
    VECTOR m_aabbMin; // AABB最小座標
    VECTOR m_aabbMax; // AABB最大座標
    VECTOR m_headPos; // ヘッドショット判定用中心座標

    float m_headRadius; // ヘッドショット判定用半径
};

