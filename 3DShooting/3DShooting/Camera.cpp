#include "Camera.h"
#include "InputManager.h"
#include "Game.h"
#include <cmath>

namespace
{
    // カメラ基本設定
    constexpr float kPitchLimit   = 89.0f * (DX_PI_F / 180.0f); // カメラの上下最大角度（ラジアン）
    constexpr float kCameraXPos   =  8.0f;  // カメラのデフォルトX軸オフセット
    constexpr float kCameraYPos   = 90.0f;  // カメラのデフォルトY軸オフセット
    constexpr float kCameraZPos   = 20.0f;  // カメラのデフォルトZ軸オフセット
    constexpr float kCameraNear   = 10.0f;  // カメラのニアクリップ距離
    constexpr float kCameraFar    = 25000.0f; // カメラのファークリップ距離

    // コンストラクタ初期値
    constexpr float kInitialYaw        = DX_PI_F;        // カメラの初期ヨー角（プレイヤー背後方向）
    constexpr float kDefaultSensitivity= 0.1f;           // カメラ感度のデフォルト値
    constexpr float kDefaultFov        = DX_PI_F * 0.5f; // 視野角のデフォルト値（90度）
    constexpr float kFovLerpSpeed      = 0.15f;          // FOV 補間速度

    // Head Bobbing 関連
    constexpr float kWalkBobIntensity       = 2.0f;  // 歩行時の揺れ強度
    constexpr float kRunBobIntensity        = 3.5f;  // 走行時の揺れ強度
    constexpr float kWalkBobSpeed           = 6.0f;  // 歩行時の揺れ速度
    constexpr float kRunBobSpeed            = 10.0f; // 走行時の揺れ速度
    constexpr float kBobIntensityLerpSpeed  = 0.12f; // 強度の補間速度
    constexpr float kBobSpeedLerpSpeed      = 0.10f; // 速度の補間速度
    constexpr float kBobVerticalStrength    = 1.0f;  // 縦方向の揺れ強度
    constexpr float kBobHorizontalStrength  = 0.4f;  // 横方向の揺れ強度
    constexpr float kBobRollStrength        = 0.15f; // Z軸回転の強度
    constexpr float kBobMinSpeedThreshold   = 0.1f;  // Bobbing が動くための最小速度閾値
    constexpr float kBobMinIntensityThreshold= 0.01f;// Bobbing の最小強度閾値
    constexpr float kBobHorizontalPeriodFactor = 0.5f; // 横揺れの周波数係数（左右の足踏みを表現）

    // 走行時の追加効果
    constexpr float kRunVerticalNoiseStrength   = 0.1f;  // 走行時の縦揺れノイズ強度
    constexpr float kRunHorizontalNoiseStrength = 0.08f; // 走行時の横揺れノイズ強度
    constexpr float kRunVerticalNoiseFreq       = 2.5f;  // 縦揺れノイズの周波数係数
    constexpr float kRunHorizontalNoiseFreq     = 1.8f;  // 横揺れノイズの周波数係数
    constexpr float kRunRollFreq                = 0.6f;  // ロール揺れの周波数係数
    constexpr float kRunBreathingFreq           = 3.2f;  // 息遣い揺れの周波数係数
    constexpr float kRunBreathingBobStrength    = 0.05f; // 息遣い揺れの強度

    // 着地時の揺れ
    constexpr float kLandingSwaySpeed   = 12.0f; // 揺れの速さ
    constexpr float kLandingSwayDamping =  0.9f; // 揺れの減衰率

    // ジャンプ時の揺れ
    constexpr float kJumpSwaySpeed   = 20.0f; // 揺れの速さ
    constexpr float kJumpSwayDamping =  0.85f;// 揺れの減衰率

    // 揺れの最小強度閾値（着地・ジャンプ・Head Bobbing 共通）
    constexpr float kMinEffectIntensity = 0.01f;

    // Sway（カメラ左右移動時の揺れ）
    constexpr float kSwayAmount    = 0.15f;  // Sway の強さ
    constexpr float kSwayRotAmount = 0.005f; // Sway の回転強さ
    constexpr float kSwayDamping   = 0.9f;   // Sway の減衰率

    // 死亡アニメーション
    constexpr float kDeathFallDuration    = 1.5f;            // 倒れる演出の時間（秒）
    constexpr float kDeathBounceDuration  = 0.5f;            // バウンド演出の時間（秒）
    constexpr float kDeathTotalDuration   = kDeathFallDuration + kDeathBounceDuration;
    constexpr float kDeathXFallOffset     = -20.0f;          // 死亡時の最終X軸オフセット
    constexpr float kDeathFinalPitch      = DX_PI_F * 0.5f;  // 死亡時の最終ピッチ角（真下向き = 90度）
    constexpr float kDeathRollAngle       = DX_PI_F * 0.25f; // 死亡時の最終ロール角（45度）
    constexpr float kDeathShakeIntensity  = 10.0f;           // 着地時のシェイク強度
    constexpr float kDeathShakeDuration   = 30.0f;           // 着地時のシェイク持続フレーム数
    constexpr float kDeathBounceAmplY     =  8.0f;           // バウンド時の Y 振幅
    constexpr float kDeathBounceAmplPitch =  0.1f;           // バウンド時のピッチ振幅
    constexpr float kDeathBounceDecay     =  5.0f;           // バウンドの減衰率
    constexpr float kDeathBounceFrequency =  3.0f;           // バウンドの周波数

    // 固定デルタタイム（1/60 秒）
    constexpr float kFixedDeltaTime = 1.0f / 60.0f;
}

Camera::Camera()
    : m_pos(VGet(0, 0, 0))
    , m_target(VGet(0, 0, 0))
    , m_offset(VGet(kCameraXPos, kCameraYPos, kCameraZPos))
    , m_defaultOffset(VGet(kCameraXPos, kCameraYPos, kCameraZPos))
    , m_playerPos(VGet(0, 0, 0))
    , m_yaw(kInitialYaw)
    , m_pitch(0.0f)
    , m_roll(0.0f)
    , m_sensitivity(kDefaultSensitivity)
    , m_fov(kDefaultFov)
    , m_defaultFov(kDefaultFov)
    , m_targetFov(kDefaultFov)
    , m_fovLerpSpeed(kFovLerpSpeed)
    , m_shakeOffset(VGet(0, 0, 0))
    , m_shakeIntensity(0.0f)
    , m_shakeDuration(0)
    , m_headBobOffset(VGet(0, 0, 0))
    , m_headBobTimer(0.0f)
    , m_headBobIntensity(0.0f)
    , m_targetBobIntensity(0.0f)
    , m_headBobSpeed(0.0f)
    , m_targetBobSpeed(0.0f)
    , m_isMoving(false)
    , m_isRunning(false)
    , m_landingSwayOffset(VGet(0, 0, 0))
    , m_landingSwayTimer(0.0f)
    , m_landingSwayIntensity(0.0f)
    , m_jumpSwayOffset(VGet(0, 0, 0))
    , m_jumpSwayTimer(0.0f)
    , m_jumpSwayIntensity(0.0f)
    , m_prevYaw(0.0f)
    , m_yawDelta(0.0f)
    , m_swayOffset(VGet(0, 0, 0))
    , m_swayRotOffset(VGet(0, 0, 0))
    , m_isDeathAnimationPlaying(false)
    , m_deathAnimationTimer(0.0f)
    , m_hasBounced(false)
{
}

void Camera::Init()
{
    SetCameraPositionAndTarget_UpVecY(m_pos, m_target);
    SetupCamera_Perspective(m_fov);
    SetCameraNearFar(kCameraNear, kCameraFar);
}

void Camera::Update(bool isInputDisabled)
{
    if (m_isDeathAnimationPlaying)
    {
        float progress     = m_deathAnimationTimer / kDeathTotalDuration;
        if (progress > 1.0f) progress = 1.0f;

        float fallProgress = m_deathAnimationTimer / kDeathFallDuration;
        if (fallProgress > 1.0f) fallProgress = 1.0f;

        // イージングを使って自然な落下を表現
        float easeInFallProgress = fallProgress * fallProgress * fallProgress;

        m_pitch    = -kPitchLimit * (1.0f - easeInFallProgress) - kDeathFinalPitch * easeInFallProgress;
        m_offset.y =  kCameraYPos * (1.0f - easeInFallProgress);
        m_offset.x =  kCameraXPos * (1.0f - easeInFallProgress) + kDeathXFallOffset * easeInFallProgress;
        m_roll     =  kDeathRollAngle * easeInFallProgress; // 45度傾ける

        if (m_deathAnimationTimer > kDeathFallDuration)
        {
            if (!m_hasBounced)
            {
                Shake(kDeathShakeIntensity, kDeathShakeDuration);
                m_hasBounced = true;
            }

            float bounceTimer    = m_deathAnimationTimer - kDeathFallDuration;
            float bounceProgress = bounceTimer / kDeathBounceDuration;

            float bounceY = kDeathBounceAmplY
                * expf(-kDeathBounceDecay * bounceProgress)
                * cosf(kDeathBounceFrequency * 2.0f * DX_PI_F * bounceProgress);
            m_offset.y += bounceY;

            float bouncePitch = kDeathBounceAmplPitch
                * expf(-kDeathBounceDecay * bounceProgress)
                * cosf(kDeathBounceFrequency * 2.0f * DX_PI_F * bounceProgress);
            m_pitch += bouncePitch;
        }
    }
    else
    {
        if (!isInputDisabled)
        {
            // マウスの移動量に基づいてカメラの回転角度を更新
            InputManager::GetInstance()->UpdateCameraRotation(m_yaw, m_pitch, m_sensitivity);
        }
        else
        {
            // 演出中もマウスを中央に固定し、操作復帰時のかくつきを防ぐ
            SetMousePoint(static_cast<int>(Game::GetScreenWidth() * 0.5f),
                          static_cast<int>(Game::GetScreenHeight() * 0.5f));
        }
        m_roll       = 0.0f;  // 通常時はロールなし
        m_hasBounced = false; // リセット
    }

    // Yaw の差分を計算
    m_yawDelta = m_yaw - m_prevYaw;
    m_prevYaw  = m_yaw;

    // シェイク処理
    if (m_shakeDuration > 0)
    {
        // ランダムなオフセットを生成
        m_shakeOffset.x = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX) - 0.5f) * 2.0f * m_shakeIntensity;
        m_shakeOffset.y = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX) - 0.5f) * 2.0f * m_shakeIntensity;
        m_shakeOffset.z = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX) - 0.5f) * 2.0f * m_shakeIntensity;
        m_shakeDuration--;
    }
    else
    {
        m_shakeOffset = VGet(0, 0, 0);
    }

    // Head Bobbing 更新
    UpdateHeadBobbing();

    // Sway 更新
    UpdateSway();

    // 着地時の揺れを更新
    m_landingSwayOffset = VGet(0, 0, 0);
    if (m_landingSwayIntensity > kMinEffectIntensity)
    {
        m_landingSwayTimer += kFixedDeltaTime;
        float sway = sinf(m_landingSwayTimer * kLandingSwaySpeed) * m_landingSwayIntensity;
        m_landingSwayOffset.y = sway;

        m_landingSwayIntensity *= kLandingSwayDamping;
        if (m_landingSwayIntensity < kMinEffectIntensity)
        {
            m_landingSwayIntensity = 0.0f;
            m_landingSwayTimer     = 0.0f;
        }
    }

    // ジャンプ時の揺れを更新
    m_jumpSwayOffset = VGet(0, 0, 0);
    if (m_jumpSwayIntensity > kMinEffectIntensity)
    {
        m_jumpSwayTimer += kFixedDeltaTime;
        float sway = sinf(m_jumpSwayTimer * kJumpSwaySpeed) * m_jumpSwayIntensity;
        m_jumpSwayOffset.y = sway;

        m_jumpSwayIntensity *= kJumpSwayDamping;
        if (m_jumpSwayIntensity < kMinEffectIntensity)
        {
            m_jumpSwayIntensity = 0.0f;
            m_jumpSwayTimer     = 0.0f;
        }
    }

    // ピッチ角度を制限
    if      (m_pitch >  kPitchLimit) m_pitch =  kPitchLimit;
    else if (m_pitch < -kPitchLimit) m_pitch = -kPitchLimit;

    // カメラの回転行列を作成
    MATRIX rotYaw    = MGetRotY(m_yaw);
    MATRIX rotPitch  = MGetRotX(-m_pitch);
    MATRIX rotRoll   = MGetRotZ(m_roll);
    MATRIX cameraRot = MMult(MMult(rotRoll, rotPitch), rotYaw);

    // カメラの向きを計算
    VECTOR forward = VTransform(VGet(0.0f, 0.0f, 1.0f), cameraRot);

    // オフセットには Yaw 回転のみ適用（ピッチ適用すると下向き時にカメラが地面に潜る）
    VECTOR rotatedOffset = VTransform(m_offset, rotYaw);

    // カメラの位置を更新（各エフェクトオフセットを加算）
    m_pos = VAdd(m_playerPos,       rotatedOffset);
    m_pos = VAdd(m_pos,             m_shakeOffset);
    m_pos = VAdd(m_pos,             m_headBobOffset);
    m_pos = VAdd(m_pos,             m_landingSwayOffset);
    m_pos = VAdd(m_pos,             m_jumpSwayOffset);
    m_pos = VAdd(m_pos,             m_swayOffset);

    m_target = VAdd(m_pos, forward);

    // FOV を滑らかに補間
    m_fov += (m_targetFov - m_fov) * m_fovLerpSpeed;

    SetCameraPositionAndTarget_UpVecY(m_pos, m_target);
    SetupCamera_Perspective(m_fov);
    SetCameraNearFar(kCameraNear, kCameraFar);
}

void Camera::UpdateSway()
{
    // Yaw の差分に基づいて Sway を計算
    m_swayOffset.x    -= m_yawDelta * kSwayAmount;
    m_swayRotOffset.z  = m_yawDelta * kSwayRotAmount;

    // Sway を減衰させる
    m_swayOffset.x    *= kSwayDamping;
    m_swayRotOffset.z *= kSwayDamping;
}

void Camera::UpdateHeadBobbing()
{
    // 目標値を設定
    if (m_isMoving)
    {
        m_targetBobIntensity = m_isRunning ? kRunBobIntensity  : kWalkBobIntensity;
        m_targetBobSpeed     = m_isRunning ? kRunBobSpeed      : kWalkBobSpeed;
    }
    else
    {
        m_targetBobIntensity = 0.0f;
        m_targetBobSpeed     = 0.0f;
    }

    // 現在の値を目標値に向けて滑らかに補間
    m_headBobIntensity = Lerp(m_headBobIntensity, m_targetBobIntensity, kBobIntensityLerpSpeed);
    m_headBobSpeed     = Lerp(m_headBobSpeed,     m_targetBobSpeed,     kBobSpeedLerpSpeed);

    // タイマーを更新（移動中のみ）
    if (m_isMoving && m_headBobIntensity > kBobMinSpeedThreshold)
    {
        m_headBobTimer += m_headBobSpeed * kFixedDeltaTime;
    }

    // Head Bobbing オフセットを計算
    if (m_headBobIntensity > kBobMinIntensityThreshold)
    {
        // 基本の正弦波による上下の動き（走行・歩行共通）
        float verticalBob   = sinf(m_headBobTimer) * m_headBobIntensity * kBobVerticalStrength;

        // 横方向の動き（歩行時の左右の重心移動を表現、半周期で左右の足を再現）
        float horizontalBob = sinf(m_headBobTimer * kBobHorizontalPeriodFactor)
                              * m_headBobIntensity * kBobHorizontalStrength;

        // 走行時の追加要素
        if (m_isRunning)
        {
            // 縦・横に異なる周波数のノイズを加えてより激しい動きを表現
            float verticalNoise   = sinf(m_headBobTimer * kRunVerticalNoiseFreq)
                                    * m_headBobIntensity * kRunVerticalNoiseStrength;
            float horizontalNoise = cosf(m_headBobTimer * kRunHorizontalNoiseFreq)
                                    * m_headBobIntensity * kRunHorizontalNoiseStrength;

            verticalBob   += verticalNoise;
            horizontalBob += horizontalNoise;

            // 滑らかなロール要素を追加
            float rollBob = sinf(m_headBobTimer * kRunRollFreq) * m_headBobIntensity * kBobRollStrength;
            horizontalBob += rollBob;

            // 微細な息遣い揺れ
            float breathingBob = sinf(m_headBobTimer * kRunBreathingFreq)
                                 * m_headBobIntensity * kRunBreathingBobStrength;
            verticalBob += breathingBob;
        }

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
    m_isMoving  = isMoving;
    m_isRunning = isRunning;
}

void Camera::ApplyLandingSway(float intensity)
{
    m_landingSwayIntensity = intensity;
    m_landingSwayTimer     = 0.0f;
}

VECTOR Camera::GetLandingSwayOffset() const
{
    return m_landingSwayOffset;
}

void Camera::ApplyJumpSway(float intensity)
{
    m_jumpSwayIntensity = intensity;
    m_jumpSwayTimer     = 0.0f;
}

VECTOR Camera::GetJumpSwayOffset() const
{
    return m_jumpSwayOffset;
}

void Camera::SetSensitivity(float sensitivity)
{
    m_sensitivity = sensitivity;
}

void Camera::SetCameraToDxLib()
{
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
    m_shakeDuration  = static_cast<int>(duration);
}

void Camera::PlayDeathAnimation(float timer)
{
    m_isDeathAnimationPlaying = true;
    m_deathAnimationTimer     = timer;
}
