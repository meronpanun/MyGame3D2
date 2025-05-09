#pragma once
#include "DxLib.h"

/// <summary>
/// �J�����N���X
/// </summary>
class Camera
{
public:
	Camera();
	virtual ~Camera();

	void Init();
	void Update();

	/// <summary>
	/// �J�����̊��x��ݒ� 
	/// </summary>
	/// <param name="sensitivity">���x</param> 
	void SetSensitivity(float sensitivity);

	/// <summary>
	/// �J�����̈ʒu���擾
	/// </summary>
	/// <returns>�J�����̈ʒu</returns>
	VECTOR GetPosition() const { return m_pos; }
	/// <summary>
	/// �J�����̒����_���擾
	/// </summary>
	/// <returns>�J�����̒����_</returns>
	VECTOR GetTarget() const { return m_target; }
	/// <summary>
	/// �J�����̃I�t�Z�b�g���擾
	/// </summary>
	/// <returns>�J�����̃I�t�Z�b�g</returns>
	VECTOR GetOffset() const { return m_offset; }

	/// <summary>
	/// �J�����̈ʒu��ݒ�
	/// </summary>
	/// <param name="pos">�J�����̈ʒu</param>
	void SetPosition(const VECTOR& pos) { m_pos = pos; }
	/// <summary>
	/// �J�����̒����_��ݒ�
	/// </summary>
	/// <param name="target">�J�����̒����_</param>
	void SetTarget(const VECTOR& target) { m_target = target; }

	/// <summary>
	/// �v���C���[�̈ʒu��ݒ�
	/// </summary>
	/// <param name="playerPos">�v���C���[�̈ʒu</param>
	void SetPlayerPos(const VECTOR& playerPos) { m_playerPos = playerPos; }

	/// <summary>
	/// �J�����̉�]�p�x���擾
	/// </summary>
	/// <returns>�J�����̉�]�p�x</returns>
	float GetYaw()   const { return m_yaw; }
	/// <summary>
	/// �J�����̃s�b�`�p�x���擾
	/// </summary>
	/// <returns>�J�����̃s�b�`�p�x</returns>
	float GetPitch() const { return m_pitch; }

private:
	VECTOR m_pos;	    // �J�����̈ʒu
	VECTOR m_target;    // �J�����̒����_
	VECTOR m_offset;    // �J�����̃I�t�Z�b�g
	VECTOR m_playerPos; // �v���C���[�̈ʒu

	// �J�����̉�]�p�x
	float m_yaw;
	float m_pitch;
	float m_sensitivity;
};

