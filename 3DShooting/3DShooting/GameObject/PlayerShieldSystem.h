#pragma once
#include "EffekseerWarningSuppress.h"
#include "Stage.h"
#include <vector>

class Camera;
class Effect;
class EnemyBase;

/// <summary>
/// プレイヤーのガード・盾管理クラス
/// </summary>
class PlayerShieldSystem
{
public:
    PlayerShieldSystem();
    ~PlayerShieldSystem();

    /// <summary>
    /// 初期化処理
    /// </summary>
    /// <param name="maxDurability">最大耐久値</param>
    /// <param name="regenRate">耐久値の回復速度</param>
    void Init(float maxDurability, float regenRate);

    /// <summary>
    /// 毎フレームの更新処理
    /// </summary>
    void Update(float deltaTime, Camera* pCamera, const VECTOR& playerPos,
        bool isGuarding, bool isTackling, bool isSwitchingWeapon,
        float yawDelta, bool isMoving);

    /// <summary>
    /// 盾の描画処理
    /// </summary>
    void Draw(Camera* pCamera, const VECTOR& playerPos, bool isTackling,
        bool isSwitchingWeapon, float weaponSwitchTimer, float weaponSwitchDuration,
        float startAnimOffsetY = 0.0f);

    /// <summary>
    /// ガード中かどうかを返す
    /// </summary>
    /// <returns>ガード中ならtrue</returns>
    bool IsGuarding() const { return m_isGuarding; }

    /// <summary>
    /// ガード状態を設定する
    /// </summary>
    /// <param name="guarding">ガード状態にするかどうか</param>
    void SetGuarding(bool guarding) { m_isGuarding = guarding; }

    /// <summary>
    /// 前フレームでガードしていたかどうかを返す
    /// </summary>
    /// <returns>前フレームでガードしていたならtrue</returns>
    bool WasGuarding() const { return m_wasGuarding; }

    /// <summary>
    /// 前フレームのガード状態を設定する
    /// </summary>
    void SetWasGuarding(bool was) { m_wasGuarding = was; }

    /// <summary>
    /// ガードアニメーションのタイマーを取得する
    /// </summary>
    float GetGuardAnimTimer() const { return m_guardAnimTimer; }

    /// <summary>
    /// ガードアニメーションの総時間を取得する
    /// </summary>
    float GetGuardAnimDuration() const { return m_guardAnimDuration; }

    /// <summary>
    /// 盾の現在耐久値を取得する
    /// </summary>
    float GetDurability() const { return m_shieldDurability; }

    /// <summary>
    /// 盾の最大耐久値を取得する
    /// </summary>
    float GetMaxDurability() const { return m_maxShieldDurability; }

    /// <summary>
    /// HPバーアニメーション用の耐久値を取得する
    /// </summary>
    float GetBarAnim() const { return m_shieldBarAnim; }

    /// <summary>
    /// 盾が壊れているかどうかを返す
    /// </summary>
    /// <returns>壊れているならtrue</returns>
    bool IsShieldBroken() const { return m_isShieldBroken; }

    /// <summary>
    /// 盾でダメージを受ける
    /// </summary>
    /// <param name="damage">受けるダメージ量</param>
    /// <param name="pEffect">エフェクトポインタ</param>
    /// <param name="pCamera">カメラポインタ</param>
    /// <param name="playerPos">プレイヤー位置</param>
    /// <returns>盾を貫通した残りダメージ量</returns>
    float TakeDamage(float damage, Effect* pEffect, Camera* pCamera, const VECTOR& playerPos);

    /// <summary>
    /// ジャストガード（パリィ）に成功しているかどうかを返す
    /// </summary>
    /// <returns>ジャストガード成功ならtrue</returns>
    bool IsJustGuarded() const;

    /// <summary>
    /// ガードタイマー（ガード開始からのフレーム数）を取得する
    /// </summary>
    int GetGuardTimer() const { return m_guardTimer; }

    /// <summary>
    /// ガードエフェクトの更新処理
    /// </summary>
    void UpdateGuardEffect(Effect* pEffect, Camera* pCamera, const VECTOR& playerPos,
        bool isSwitchingWeapon);

    /// <summary>
    /// スパークエフェクトの更新処理
    /// </summary>
    void UpdateSparkEffect(Effect* pEffect, const VECTOR& playerPos, Camera* pCamera);

    /// <summary>
    /// 盾の揺れオフセットを取得する
    /// </summary>
    VECTOR GetShieldSwayOffset() const { return m_shieldSwayOffset; }

    /// <summary>
    /// 盾の回転揺れオフセットを取得する
    /// </summary>
    VECTOR GetShieldSwayRotOffset() const { return m_shieldSwayRotOffset; }

    /// <summary>
    /// 盾 UI 画像ハンドルを取得する
    /// </summary>
    int GetShieldImageHandle() const;

    /// <summary>
    /// シールドソーを投げる
    /// </summary>
    /// <param name="pCamera">カメラポインタ</param>
    /// <param name="playerPos">プレイヤー位置</param>
    /// <returns>投げられた場合はtrue</returns>
    bool ThrowShield(Camera* pCamera, const VECTOR& playerPos);

    /// <summary>
    /// シールドソーの更新処理
    /// </summary>
    void UpdateShieldThrow(float deltaTime, Camera* pCamera, const VECTOR& playerPos,
        const std::vector<class EnemyBase*>& enemyList,
        const std::vector<Stage::StageCollisionData>& collisionData,
        Effect* pEffect, bool isGuarding, bool wasGuarding);

    /// <summary>
    /// シールドソーの 3D 描画
    /// </summary>
    void DrawShieldThrow(Camera* pCamera, const VECTOR& playerPos) const;

    /// <summary>
    /// 盾をプレイヤーの位置へ即座に帰還させる
    /// </summary>
    void ImmediateReturnShield(const VECTOR& playerPos);

    /// <summary>
    /// シールドソーが投擲中かどうかを返す
    /// </summary>
    /// <returns>投擲中ならtrue</returns>
    bool IsShieldThrown() const { return m_isShieldThrown; }

private:
    // ハンドル類
    int m_shieldModelHandle;
    int m_shieldImageHandle;
    int m_guardEffectHandle;
    int m_sparkEffectHandle;
    int m_sparkEffectTimer;       // スパークエフェクトの残りフレーム数

    // ガード状態
    int   m_guardTimer;           // ガード開始からのフレーム数（パリィ判定用）
    float m_shieldDurability;
    float m_shieldBarAnim;        // HPバーアニメーション用の耐久値
    float m_maxShieldDurability;
    float m_shieldRegenRate;
    float m_guardAnimTimer;
    float m_guardAnimDuration;
    float m_shieldAnimTimer;
    float m_shieldAnimDuration;
    float m_idleSwayTimer;
    bool  m_isShieldBroken;
    bool  m_isGuarding;
    bool  m_wasGuarding;
    bool  m_isShieldAnimating;
    bool  m_isShieldRecovering;
    bool  m_isTutorial;
    bool  m_wasSwitchingWeapon;   // 前フレームの武器切り替え状態（エフェクト再生用）

    // 盾の揺れ
    VECTOR m_shieldSwayOffset;
    VECTOR m_shieldSwayRotOffset;

    // シールドソー
    enum class ShieldThrowState
    {
        Idle,      // 待機中
        Throwing,  // 投擲中（前方移動）
        Returning  // 帰還中
    };
    ShieldThrowState m_shieldThrowState;
    bool     m_isShieldThrown;
    VECTOR   m_shieldThrowPos;              // シールドの現在位置
    VECTOR   m_shieldThrowDir;              // シールドの移動方向
    VECTOR   m_shieldThrowStartPos;         // 投擲開始位置
    float    m_shieldThrowDistance;         // 投擲済み距離
    float    m_shieldThrowSpeed;            // シールドの移動速度
    float    m_shieldThrowMaxRange;         // 最大投擲距離
    float    m_shieldThrowDamage;           // シールドソーのダメージ量
    float    m_shieldThrowRotationTimer;    // 回転タイマー
    float    m_shieldThrowCooldownTimer;    // クールダウンタイマー
    VECTOR   m_shieldCurrentForward;        // Slerp 補間中の盾の前進方向（描画に使用）
    float    m_shieldThrowFailedAnimTimer;  // 投擲失敗アニメーションタイマー
    bool     m_isShieldThrowFailedAnimating; // 投擲失敗アニメーション中か
    intptr_t m_shieldThrowHitEnemyId;       // 同フレームでヒット済みの敵 ID（重複ヒット防止）
    int      m_shieldReflectCount;          // 反射回数

    // ブーメラン SE
    int   m_boomerangTotalTime;    // ブーメラン SE の総再生時間（ms）
    bool  m_isBoomerangFading;     // フェードアウト中か
    float m_boomerangFadeVolume;   // フェードアウト用音量比率
    float m_shieldHitSECooldown;   // ヒット SE 再生クールダウン
};
