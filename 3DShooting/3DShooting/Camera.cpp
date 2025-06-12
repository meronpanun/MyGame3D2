#include "Camera.h"
#include "Mouse.h"

namespace
{
    constexpr float kPitchLimit = DX_PI_F / 4.0f; // カメラの角度を45度に制限
    constexpr float kCameraXPos = 8.0f;           // カメラのX軸
    constexpr float kCameraYPos = 90.0f;          // カメラのY軸
    constexpr float kCameraZPos = 25.0f;          // カメラのZ軸
    constexpr float kCameraNear = 10.0f;          // カメラの近くの距離
    constexpr float kCameraFar  = 1800.0f;        // カメラの遠くの距離
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
    m_fovLerpSpeed(0.15f) 
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

    // カメラの位置を更新
    m_pos    = VAdd(m_playerPos, rotatedOffset);
    m_target = VAdd(m_pos, forward);

    // FOVを滑らかに補間
    m_fov += (m_targetFov - m_fov) * m_fovLerpSpeed;

    // カメラの設定を更新
    SetCameraPositionAndTarget_UpVecY(m_pos, m_target);
    SetupCamera_Perspective(m_fov); // FOVを毎フレーム反映
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
