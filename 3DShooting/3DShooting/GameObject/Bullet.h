#pragma once
#include "AttackType.h"
#include "EffekseerWarningSuppress.h"
#include "Stage.h"
#include <vector>

/// <summary>
/// 弾クラス
/// </summary>
class Bullet
{
public:
    Bullet(VECTOR position, VECTOR direction, AttackType attackType, float damage = 10.0f, float attenuationStartDist = 0.0f, float attenuationEndDist = 0.0f, float minDamageRatio = 1.0f);
    virtual ~Bullet();

    void Init();
    void Update(const VECTOR &playerPos, const std::vector<Stage::StageCollisionData> &collisionData);
    void Draw() const;

    /// <summary>
    /// 弾の現在の位置を取得
    /// </summary>
    /// <returns>現在位置</returns>
    VECTOR GetPos() const { return m_pos; }

    /// <summary>
    /// 弾の前フレームの位置を取得（Rayの始点として使用）
    /// </summary>
    /// <returns>前フレームの位置</returns>
    VECTOR GetPrevPos() const { return m_prevPos; }

    /// <summary>
    /// 弾が有効かどうか
    /// </summary>
    /// <returns>有効ならtrue</returns>
    bool IsActive() const { return m_isActive; }

    /// <summary>
    /// 弾の更新
    /// </summary>
    /// <param name="bullets">弾の配列</param>
    /// <param name="playerPos">プレイヤーの位置</param>
    /// <param name="collisionData">ステージの当たり判定データ</param>
    static void UpdateBullets(std::vector<Bullet> &bullets, const VECTOR &playerPos, const std::vector<Stage::StageCollisionData> &collisionData);

    /// <summary>
    /// 弾の描画
    /// </summary>
    /// <param name="bullets">弾の配列</param>
    static void DrawBullets(const std::vector<Bullet> &bullets);

    /// <summary>
    /// 弾を非アクティブにする
    /// </summary>
    void Deactivate();

    /// <summary>
    /// 弾のダメージを取得
    /// </summary>
    /// <returns>ダメージ値</returns>
    float GetDamage() const;

    /// <summary>
    /// 攻撃の種類を取得
    /// </summary>
    /// <returns>攻撃の種別</returns>
    AttackType GetAttackType() const { return m_attackType; }

private:
    VECTOR m_pos;      // 現在の位置
    VECTOR m_prevPos;  // 前フレームの位置（Rayの始点）
    VECTOR m_spawnPos; // 発射位置
    VECTOR m_dir;      // 進行方向

    float m_speed;  // 速度
    float m_damage; // ダメージ量

    // 減衰用パラメータ
    float m_attenuationStartDist; // 減衰開始距離
    float m_attenuationEndDist;   // 減衰終了距離
    float m_minDamageRatio;       // 最低ダメージ倍率

    bool m_isActive;         // アクティブ状態かどうか
    AttackType m_attackType; // 攻撃の種別
};
