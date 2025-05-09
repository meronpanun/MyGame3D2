#pragma once
#include "Vec2.h"

/* �}�E�X�̓��͏����擾���� */
namespace Mouse
{
	// �}�E�X�̓��͏�Ԏ擾
	void Update();

	/// <summary>
	/// �}�E�X�̈ʒu���擾
	/// </summary>
	/// <returns>�}�E�X�̈ʒu</returns>
	Vec2 GetPos();

	/// <summary>
	/// �}�E�X�̍��{�^����������Ă��邩�̔���
	/// </summary>
	/// <returns>������Ă���Ȃ�true</returns>
	bool IsPressLeft();
	/// <summary>
	/// �}�E�X�̍��{�^�����g���K�[���ꂽ�u�Ԃ̔���
	/// </summary>
	/// <returns>�g���K�[���ꂽ�u�ԂȂ�true</returns>
	bool IsTriggerLeft();
	/// <summary>
	/// �}�E�X�̍��{�^���������ꂽ�u�Ԃ̔���
	/// </summary>
	/// <returns>�����ꂽ�u�ԂȂ�true</returns>
	bool IsReleaseLeft();

	/// <summary>
	/// �J�����̉�]�p�x���X�V
	/// </summary>
	/// <param name="cameraYaw">�J�����̃��[�p�x</param>
	/// <param name="cameraPitch">�J�����̃s�b�`�p�x</param>
	void UpdateCameraRotation(float& cameraYaw, float& cameraPitch, float sensitivity);
}

