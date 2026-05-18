#pragma once
#include "DxLib.h"

/// <summary>
/// プレイヤーのステータス（体力・死亡状態など）を管理するクラス
/// </summary>
class PlayerStatus
{
public:
    PlayerStatus();
    ~PlayerStatus() = default;

    /// <summary>
    /// 初期化処理
    /// </summary>
    /// <param name="maxHp">最大体力</param>
    void Init(float maxHp);

    /// <summary>
    /// 毎フレームの更新処理
    /// </summary>
    /// <param name="deltaTime">経過時間</param>
    void Update(float deltaTime);

    /// <summary>
    /// 現在の体力を取得する
    /// </summary>
    float GetHealth() const { return m_health; }

    /// <summary>
    /// 最大体力を取得する
    /// </summary>
    float GetMaxHealth() const { return m_maxHealth; }

    /// <summary>
    /// HPバーアニメーション用の体力値を取得する
    /// </summary>
    float GetHealthBarAnim() const { return m_healthBarAnim; }

    /// <summary>
    /// 死亡しているかどうかを返す
    /// </summary>
    /// <returns>死亡しているならtrue</returns>
    bool IsDead() const { return m_isDead; }

    /// <summary>
    /// 無敵状態かどうかを返す
    /// </summary>
    /// <returns>無敵ならtrue</returns>
    bool IsInvincible() const { return m_isInvincible; }

    /// <summary>
    /// 体力が低下しているかどうかを返す
    /// </summary>
    /// <returns>低体力ならtrue</returns>
    bool IsLowHealth() const { return m_isLowHealth; }

    /// <summary>
    /// 低体力警告の点滅タイマーを取得する
    /// </summary>
    float GetLowHealthBlinkTimer() const { return m_lowHealthBlinkTimer; }

    /// <summary>
    /// 死亡タイマーを取得する
    /// </summary>
    float GetDeathTimer() const { return m_deathTimer; }

    /// <summary>
    /// 体力を直接設定する
    /// </summary>
    void SetHealth(float health);

    /// <summary>
    /// 無敵状態を設定する
    /// </summary>
    void SetInvincible(bool invincible) { m_isInvincible = invincible; }

    /// <summary>
    /// 死亡状態を設定する
    /// </summary>
    void SetDead(bool dead) { m_isDead = dead; }

    /// <summary>
    /// 体力を加算する（最大値でクランプ）
    /// </summary>
    /// <param name="value">加算する体力値</param>
    void AddHp(float value);

    /// <summary>
    /// ダメージを受ける
    /// </summary>
    /// <param name="damage">ダメージ量</param>
    void TakeDamage(float damage);

private:
    float m_health;
    float m_maxHealth;
    float m_healthBarAnim;        // HPバーアニメーション用の体力値
    float m_lowHealthBlinkTimer;  // 低体力警告の点滅タイマー
    float m_deathTimer;           // 死亡後のタイマー

    bool m_isDead;
    bool m_isInvincible;
    bool m_isLowHealth;

    static constexpr float kHpBarAnimSpeed    = 1.5f;  // HPバーのアニメーション速度
    static constexpr float kLowHealthThreshold = 30.0f; // 低体力と判定する体力値の閾値
};
