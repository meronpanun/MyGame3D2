#include "Camera.h"
#include "Mouse.h"
#include <cmath>

namespace
{
	// カメラ関連の定数
    constexpr float kPitchLimit = DX_PI_F / 4.0f;   // カメラの角度を45度に制限
    constexpr float kCameraXPos = 8.0f;             // カメラのX軸
    constexpr float kCameraYPos = 90.0f;            // カメラのY軸
    constexpr float kCameraZPos = 20.0f;            // カメラのZ軸
    constexpr float kCameraNear = 1.0f;             // カメラの近くの距離
    constexpr float kCameraFar = 15000.0f;          // カメラの遠くの距離

    // Head Bobbing関連の定数
    constexpr float kWalkBobIntensity      = 2.0f;  // 歩行時の揺れ強度
    constexpr float kRunBobIntensity       = 3.5f;  // 走行時の揺れ強度
    constexpr float kWalkBobSpeed          = 6.0f;  // 歩行時の揺れ速度
    constexpr float kRunBobSpeed           = 10.0f; // 走行時の揺れ速度
    constexpr float kBobIntensityLerpSpeed = 0.12f; // 強度の補間速度
    constexpr float kBobSpeedLerpSpeed     = 0.10f; // 速度の補間速度
    constexpr float kBobVerticalStrength   = 1.0f;  // 縦方向の揺れ強度
    constexpr float kBobHorizontalStrength = 0.4f;  // 横方向の揺れ強度
    constexpr float kBobRollStrength       = 0.15f; // Z軸回転の強度

    // 走行時の追加効果の強度
    constexpr float kRunVerticalNoiseStrength   = 0.1f;  // 走行時の縦揺れノイズ強度
    constexpr float kRunHorizontalNoiseStrength = 0.08f; // 走行時の横揺れノイズ強度

    // 着地時の揺れ関連の定数
    constexpr float kLandingSwaySpeed   = 12.0f; // 揺れの速さ
    constexpr float kLandingSwayDamping = 0.9f;  // 揺れの減衰率

    // ジャンプ時の揺れ関連の定数
    constexpr float kJumpSwaySpeed   = 20.0f; // 揺れの速さ
    constexpr float kJumpSwayDamping = 0.85f;  // 揺れの減衰率
}

Camera::Camera() :
    m_pos(VGet(0, 0, 0)),
    m_target(VGet(0, 0, 0)),
    m_offset(VGet(kCameraXPos, kCameraYPos, kCameraZPos)),
    m_defaultOffset(VGet(kCameraXPos, kCameraYPos, kCameraZPos)),
    m_playerPos(VGet(0, 0, 0)),
    m_yaw(DX_PI_F),
    m_pitch(0.0f),
    m_sensitivity(0.1f),
    m_fov(DX_PI_F * 0.5f),
    m_defaultFov(DX_PI_F * 0.5f), 
    m_targetFov(DX_PI_F * 0.5f),
    m_fovLerpSpeed(0.15f),
    m_headBobOffset(VGet(0, 0, 0)),
    m_headBobTimer(0.0f),
    m_headBobIntensity(0.0f),
    m_targetBobIntensity(0.0f),
    m_headBobSpeed(0.0f),
    m_targetBobSpeed(0.0f),
    m_isMoving(false),
    m_isRunning(false),
    m_landingSwayOffset(VGet(0, 0, 0)),
    m_landingSwayTimer(0.0f),
    m_landingSwayIntensity(0.0f),
    m_jumpSwayOffset(VGet(0, 0, 0)),
    m_jumpSwayTimer(0.0f),
    m_jumpSwayIntensity(0.0f)
{
}

Camera::~Camera()
{
}

void Camera::Init()
{
    // カメラの設定
    SetCameraPositionAndTarget_UpVecY(m_pos, m_target);
    SetupCamera_Perspective(m_fov);
    SetCameraNearFar(kCameraNear, kCameraFar);
}

void Camera::Update()
{
    // マウスの移動量に基づいてカメラの回転角度を更新
    Mouse::UpdateCameraRotation(m_yaw, m_pitch, m_sensitivity);

    // シェイク処理
    if (m_shakeDuration > 0)
    {
        m_shakeOffset.y = (rand() / (float)RAND_MAX - 0.5f) * 2.0f * m_shakeIntensity;
        m_shakeOffset.z = (rand() / (float)RAND_MAX - 0.5f) * 2.0f * m_shakeIntensity;
        m_shakeDuration--;
    }
    else
    {
        m_shakeOffset = VGet(0, 0, 0);
    }

    // Head Bobbing更新
    UpdateHeadBobbing();

    // 着地時の揺れを更新
    m_landingSwayOffset = VGet(0, 0, 0);
    if (m_landingSwayIntensity > 0.01f)
    {
        m_landingSwayTimer += 1.0f / 60.0f;
        float sway = sinf(m_landingSwayTimer * kLandingSwaySpeed) * m_landingSwayIntensity;
        m_landingSwayOffset.y = sway;

        m_landingSwayIntensity *= kLandingSwayDamping;
        if (m_landingSwayIntensity < 0.01f)
        {
            m_landingSwayIntensity = 0.0f;
            m_landingSwayTimer = 0.0f;
        }
    }

    // ジャンプ時の揺れを更新
    m_jumpSwayOffset = VGet(0, 0, 0);
    if (m_jumpSwayIntensity > 0.01f)
    {
        m_jumpSwayTimer += 1.0f / 60.0f;
        float sway = sinf(m_jumpSwayTimer * kJumpSwaySpeed) * m_jumpSwayIntensity;
        m_jumpSwayOffset.y = sway;

        m_jumpSwayIntensity *= kJumpSwayDamping;
        if (m_jumpSwayIntensity < 0.01f)
        {
            m_jumpSwayIntensity = 0.0f;
            m_jumpSwayTimer = 0.0f;
        }
    }

    // ピッチ角度に制限を設ける
    if (m_pitch > kPitchLimit)
    {
        m_pitch = kPitchLimit;
    }
    else if (m_pitch < -kPitchLimit)
    {
        m_pitch = -kPitchLimit;
    }

    // カメラの回転行列を作成
    MATRIX rotYaw    = MGetRotY(m_yaw);
    MATRIX rotPitch  = MGetRotX(-m_pitch);
    MATRIX cameraRot = MMult(rotPitch, rotYaw);

    // カメラの向きを計算
    VECTOR forward = VTransform(VGet(0.0f, 0.0f, 1.0f), cameraRot);

    // カメラのオフセットを回転させる
    VECTOR rotatedOffset = VTransform(m_offset, cameraRot);

    	// カメラの位置を更新（シェイクとHead Bobbingオフセットを適用）
        m_pos = VAdd(m_playerPos, rotatedOffset);
        m_pos = VAdd(m_pos, m_shakeOffset);
        m_pos = VAdd(m_pos, m_headBobOffset);
        m_pos = VAdd(m_pos, m_landingSwayOffset);
    	m_pos = VAdd(m_pos, m_jumpSwayOffset);
    
        m_target = VAdd(m_pos, forward);
    
        // FOVを滑らかに補間
        m_fov += (m_targetFov - m_fov) * m_fovLerpSpeed;
    // カメラの設定を更新
    SetCameraPositionAndTarget_UpVecY(m_pos, m_target);
    SetupCamera_Perspective(m_fov);
    SetCameraNearFar(kCameraNear, kCameraFar);
}

void Camera::UpdateHeadBobbing()
{
    // 目標値を設定
    if (m_isMoving)
    {
        if (m_isRunning)
        {
            m_targetBobIntensity = kRunBobIntensity;
            m_targetBobSpeed = kRunBobSpeed;
        }
        else
        {
            m_targetBobIntensity = kWalkBobIntensity;
            m_targetBobSpeed = kWalkBobSpeed;
        }
    }
    else
    {
        m_targetBobIntensity = 0.0f;
        m_targetBobSpeed = 0.0f;
    }

    // 現在の値を目標値に向けて滑らかに補間
    m_headBobIntensity = Lerp(m_headBobIntensity, m_targetBobIntensity, kBobIntensityLerpSpeed);
    m_headBobSpeed = Lerp(m_headBobSpeed, m_targetBobSpeed, kBobSpeedLerpSpeed);

    // タイマーを更新（移動中のみ）
    if (m_isMoving && m_headBobIntensity > 0.1f)
    {
        m_headBobTimer += m_headBobSpeed * (1.0f / 60.0f); // 60FPS基準
    }

    // Head Bobbingオフセットを計算
    if (m_headBobIntensity > 0.01f)
    {
        // 基本的な正弦波による上下の動き（走行・歩行共通）
        float verticalBob = sinf(m_headBobTimer) * m_headBobIntensity * kBobVerticalStrength;

        // 横方向の動き（歩行時の左右の重心移動を表現）
        // 歩行時は2倍周期（左右の足の動きを表現）
        float horizontalBob = sinf(m_headBobTimer * 0.5f) * m_headBobIntensity * kBobHorizontalStrength;

        // 走行時の追加要素
        if (m_isRunning)
        {
            // より細かい振動を追加（呼吸や歩幅の不規則性を表現）
            float verticalNoise = sinf(m_headBobTimer * 2.5f) * m_headBobIntensity * kRunVerticalNoiseStrength;
            float horizontalNoise = cosf(m_headBobTimer * 1.8f) * m_headBobIntensity * kRunHorizontalNoiseStrength;

            verticalBob += verticalNoise;
            horizontalBob += horizontalNoise;

            // より滑らかなロール要素を追加
            float rollBob = sinf(m_headBobTimer * 0.6f) * m_headBobIntensity * kBobRollStrength;
            horizontalBob += rollBob;

            // ランニング時の息づかいを表現する微細な縦揺れ
            float breathingBob = sinf(m_headBobTimer * 3.2f) * m_headBobIntensity * 0.05f;
            verticalBob += breathingBob;
        }

        // オフセットを設定
        m_headBobOffset = VGet(horizontalBob, verticalBob, 0.0f);
    }
    else
    {
        m_headBobOffset = VGet(0, 0, 0);
    }
}

float Camera::Lerp(float current, float target, float speed)
{
    return current + (target - current) * speed;
}

void Camera::SetHeadBobbingState(bool isMoving, bool isRunning)
{
    m_isMoving = isMoving;
    m_isRunning = isRunning;
}

void Camera::ApplyLandingSway(float intensity)
{
    m_landingSwayIntensity = intensity;
    m_landingSwayTimer = 0.0f;
}

VECTOR Camera::GetLandingSwayOffset() const
{
    return m_landingSwayOffset;
}

void Camera::ApplyJumpSway(float intensity)
{
	m_jumpSwayIntensity = intensity;
	m_jumpSwayTimer = 0.0f;
}

VECTOR Camera::GetJumpSwayOffset() const
{
	return m_jumpSwayOffset;
}

// カメラの感度を設定
void Camera::SetSensitivity(float sensitivity)
{
    m_sensitivity = sensitivity;
}

// カメラの位置を設定
void Camera::SetCameraToDxLib()
{
    // カメラの位置と注視点を設定
    SetCameraPositionAndTarget_UpVecY(m_pos, m_target);
}

void Camera::SetFOV(float fov)
{
    m_targetFov = fov;
}

float Camera::GetFOV() const
{
    return m_fov;
}

void Camera::ResetFOV()
{
    m_targetFov = m_defaultFov;
}

void Camera::ResetOffset()
{
    m_offset = m_defaultOffset;
}

void Camera::SetTargetFOV(float fov)
{
    m_targetFov = fov;
}

void Camera::Shake(float intensity, float duration)
{
    m_shakeIntensity = intensity;
    m_shakeDuration = duration;
}