#include "Camera.h"
#include "Mouse.h"

namespace
{
    constexpr float kPitchLimit = DX_PI_F / 4.0f; // �J�����̊p�x��45�x�ɐ���
    constexpr float kCameraXPos = 8.0f;           // �J������X��
    constexpr float kCameraYPos = 90.0f;          // �J������Y��
    constexpr float kCameraZPos = 25.0f;          // �J������Z��
    constexpr float kCameraNear = 10.0f;          // �J�����̋߂��̋���
    constexpr float kCameraFar  = 1800.0f;        // �J�����̉����̋���
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
    // �J�����̐ݒ�
    SetCameraPositionAndTarget_UpVecY(m_pos, m_target);
    SetupCamera_Perspective(m_fov);
    SetCameraNearFar(kCameraNear, kCameraFar);
}

void Camera::Update()
{
    // �}�E�X�̈ړ��ʂɊ�Â��ăJ�����̉�]�p�x���X�V
    Mouse::UpdateCameraRotation(m_yaw, m_pitch, m_sensitivity);

    // �s�b�`�p�x�ɐ�����݂���
    if (m_pitch > kPitchLimit)
    {
        m_pitch = kPitchLimit;
    }
    else if (m_pitch < -kPitchLimit)
    {
        m_pitch = -kPitchLimit;
    }

    // �J�����̉�]�s����쐬
    MATRIX rotYaw    = MGetRotY(m_yaw);
    MATRIX rotPitch  = MGetRotX(-m_pitch);
    MATRIX cameraRot = MMult(rotPitch, rotYaw);

    // �J�����̌������v�Z
    VECTOR forward = VTransform(VGet(0.0f, 0.0f, 1.0f), cameraRot);

    // �J�����̃I�t�Z�b�g����]������
    VECTOR rotatedOffset = VTransform(m_offset, cameraRot);

    // �J�����̈ʒu���X�V
    m_pos    = VAdd(m_playerPos, rotatedOffset);
    m_target = VAdd(m_pos, forward);

    // FOV�����炩�ɕ��
    m_fov += (m_targetFov - m_fov) * m_fovLerpSpeed;

    // �J�����̐ݒ���X�V
    SetCameraPositionAndTarget_UpVecY(m_pos, m_target);
    SetupCamera_Perspective(m_fov); // FOV�𖈃t���[�����f
}

// �J�����̊��x��ݒ�
void Camera::SetSensitivity(float sensitivity)
{
    m_sensitivity = sensitivity;
}

// �J�����̈ʒu��ݒ�
void Camera::SetCameraToDxLib()
{
    // �J�����̈ʒu�ƒ����_��ݒ�
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
