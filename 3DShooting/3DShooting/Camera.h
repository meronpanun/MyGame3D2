#pragma once
#include "EffekseerWarningSuppress.h"

/// <summary>
/// DxLib の MV1 モデル用カメラクラス。
/// 位置・注視点・FOV のほか、Head Bobbing / Sway / Shake / 着地揺れ / 死亡アニメーションを管理する。
/// </summary>
class Camera
{
public:
    /// <summary>
    /// コンストラクタ（各パラメータをデフォルト値で初期化する）
    /// </summary>
    Camera();

    virtual ~Camera() = default;

    /// <summary>
    /// 初期化処理（カメラ設定を DxLib に適用する）
    /// </summary>
    void Init();

    /// <summary>
    /// 毎フレームの更新処理（回転・シェイク・Head Bobbing・FOV 補間など）
    /// </summary>
    /// <param name="isInputDisabled">true の場合マウス入力を無視し、マウスを中央に固定する</param>
    void Update(bool isInputDisabled = false);

    /// <summary>
    /// カメラの感度を設定する
    /// </summary>
    /// <param name="sensitivity">感度値</param>
    void SetSensitivity(float sensitivity);

    /// <summary>
    /// カメラの現在位置を取得する
    /// </summary>
    /// <returns>カメラの位置</returns>
    VECTOR GetPos() const { return m_pos; }

    /// <summary>
    /// カメラの現在注視点を取得する
    /// </summary>
    /// <returns>カメラの注視点</returns>
    VECTOR GetTarget() const { return m_target; }

    /// <summary>
    /// カメラのオフセットを取得する
    /// </summary>
    /// <returns>カメラのオフセット</returns>
    VECTOR GetOffset() const { return m_offset; }

    /// <summary>
    /// カメラのオフセットを設定する
    /// </summary>
    /// <param name="offset">設定するオフセット値</param>
    void SetOffset(const VECTOR& offset) { m_offset = offset; }

    /// <summary>
    /// カメラの位置を直接設定する
    /// </summary>
    /// <param name="pos">設定する位置</param>
    void SetPos(const VECTOR& pos) { m_pos = pos; }

    /// <summary>
    /// カメラの注視点を直接設定する
    /// </summary>
    /// <param name="target">設定する注視点</param>
    void SetTarget(const VECTOR& target) { m_target = target; }

    /// <summary>
    /// プレイヤーの位置を設定する（カメラ位置の基点として使用）
    /// </summary>
    /// <param name="playerPos">プレイヤーの位置</param>
    void SetPlayerPos(const VECTOR& playerPos) { m_playerPos = playerPos; }

    /// <summary>
    /// カメラの Yaw 角度を取得する
    /// </summary>
    /// <returns>Yaw 角度（ラジアン）</returns>
    float GetYaw()   const { return m_yaw; }

    /// <summary>
    /// カメラのピッチ角度を取得する
    /// </summary>
    /// <returns>ピッチ角度（ラジアン）</returns>
    float GetPitch() const { return m_pitch; }

    /// <summary>
    /// カメラの位置と注視点を DxLib に設定する
    /// </summary>
    void SetCameraToDxLib();

    /// <summary>
    /// カメラの視野角（FOV）の目標値を設定する（滑らかに補間される）
    /// </summary>
    /// <param name="fov">目標視野角（ラジアン）</param>
    void SetFOV(float fov);

    /// <summary>
    /// 現在の視野角（FOV）を取得する
    /// </summary>
    /// <returns>現在の視野角（ラジアン）</returns>
    float GetFOV() const;

    /// <summary>
    /// 視野角（FOV）をデフォルト値に戻す
    /// </summary>
    void ResetFOV();

    /// <summary>
    /// カメラのオフセットをデフォルト値に戻す
    /// </summary>
    void ResetOffset();

    /// <summary>
    /// 目標 FOV を直接設定する
    /// </summary>
    /// <param name="fov">目標 FOV（ラジアン）</param>
    void SetTargetFOV(float fov);

    /// <summary>
    /// カメラシェイクを開始する
    /// </summary>
    /// <param name="intensity">シェイクの強度</param>
    /// <param name="duration">シェイクの持続フレーム数</param>
    void Shake(float intensity, float duration);

    /// <summary>
    /// 現在のシェイクオフセットを取得する
    /// </summary>
    /// <returns>シェイクオフセット</returns>
    VECTOR GetShakeOffset() const { return m_shakeOffset; }

    /// <summary>
    /// Head Bobbing オフセットを取得する
    /// </summary>
    /// <returns>Head Bobbing によるオフセット</returns>
    VECTOR GetHeadBobOffset() const { return m_headBobOffset; }

    /// <summary>
    /// Head Bobbing（歩行時の画面揺れ）の状態を設定する
    /// </summary>
    /// <param name="isMoving">移動中かどうか</param>
    /// <param name="isRunning">走行中かどうか</param>
    void SetHeadBobbingState(bool isMoving, bool isRunning);

    /// <summary>
    /// 着地時の揺れを適用する
    /// </summary>
    /// <param name="intensity">揺れの強度</param>
    void ApplyLandingSway(float intensity);

    /// <summary>
    /// 着地時の揺れオフセットを取得する
    /// </summary>
    /// <returns>着地時の揺れオフセット</returns>
    VECTOR GetLandingSwayOffset() const;

    /// <summary>
    /// ジャンプ時の揺れを適用する
    /// </summary>
    /// <param name="intensity">揺れの強度</param>
    void ApplyJumpSway(float intensity);

    /// <summary>
    /// ジャンプ時の揺れオフセットを取得する
    /// </summary>
    /// <returns>ジャンプ時の揺れオフセット</returns>
    VECTOR GetJumpSwayOffset() const;

    /// <summary>
    /// 前フレームからの Yaw 差分を取得する
    /// </summary>
    /// <returns>Yaw の差分（ラジアン）</returns>
    float GetYawDelta() const { return m_yawDelta; }

    /// <summary>
    /// 死亡アニメーションを更新する（毎フレーム呼び出す）
    /// </summary>
    /// <param name="timer">アニメーション経過時間（秒）</param>
    void PlayDeathAnimation(float timer);

private:
    /// <summary>
    /// Head Bobbing の揺れ量を毎フレーム更新する
    /// </summary>
    void UpdateHeadBobbing();

    /// <summary>
    /// カメラ横移動時の Sway 揺れ量を毎フレーム更新する
    /// </summary>
    void UpdateSway();

    /// <summary>
    /// 線形補間（Lerp）を行う
    /// </summary>
    /// <param name="current">現在の値</param>
    /// <param name="target">目標値</param>
    /// <param name="speed">補間速度（0〜1）</param>
    /// <returns>補間された値</returns>
    float Lerp(float current, float target, float speed);

private:
    // 位置・注視点管理
    VECTOR m_pos;           // カメラの位置
    VECTOR m_target;        // カメラの注視点
    VECTOR m_offset;        // カメラのオフセット（プレイヤー位置からの相対位置）
    VECTOR m_defaultOffset; // デフォルトのオフセット
    VECTOR m_playerPos;     // プレイヤーの位置（カメラ基点）

    // 回転管理
    float m_yaw;         // ヨー角度（ラジアン）
    float m_pitch;       // ピッチ角度（ラジアン）
    float m_roll;        // ロール角度（ラジアン）
    float m_sensitivity; // カメラの感度
    float m_prevYaw;     // 前フレームのヨー角度
    float m_yawDelta;    // ヨー角度の差分

    // 視野角（FOV）管理
    float m_fov;          // カメラの現在視野角（ラジアン）
    float m_defaultFov;   // デフォルトの FOV
    float m_targetFov;    // 目標 FOV（補間先）
    float m_fovLerpSpeed; // FOV 補間速度

    // シェイク演出管理
    VECTOR m_shakeOffset;    // シェイクオフセット
    float  m_shakeIntensity; // シェイクの強度
    int    m_shakeDuration;  // シェイクの残り持続フレーム数

    // Head Bobbing 管理
    VECTOR m_headBobOffset;      // Head Bobbing によるオフセット
    float  m_headBobTimer;       // Head Bobbing のタイマー
    float  m_headBobIntensity;   // 現在の Head Bobbing 強度
    float  m_targetBobIntensity; // 目標の Head Bobbing 強度
    float  m_headBobSpeed;       // 現在の Head Bobbing 速度
    float  m_targetBobSpeed;     // 目標の Head Bobbing 速度
    bool   m_isMoving;           // 移動中かどうか
    bool   m_isRunning;          // 走行中かどうか

    // 着地時の揺れ管理
    VECTOR m_landingSwayOffset;    // 着地時の揺れオフセット
    float  m_landingSwayTimer;     // 着地揺れタイマー
    float  m_landingSwayIntensity; // 着地揺れの強度

    // ジャンプ時の揺れ管理
    VECTOR m_jumpSwayOffset;    // ジャンプ時の揺れオフセット
    float  m_jumpSwayTimer;     // ジャンプ揺れタイマー
    float  m_jumpSwayIntensity; // ジャンプ揺れの強度

    // Sway 管理
    VECTOR m_swayOffset;    // Sway によるカメラ位置オフセット
    VECTOR m_swayRotOffset; // Sway による回転オフセット

    // 死亡アニメーション管理
    bool  m_isDeathAnimationPlaying; // 死亡アニメーション再生中かどうか
    float m_deathAnimationTimer;     // 死亡アニメーションの経過時間（秒）
    bool  m_hasBounced;              // バウンド演出が完了したかどうか
};
