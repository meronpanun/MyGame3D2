#pragma once
#include "SceneBase.h"
#include <memory>
#include <chrono>
#include <vector>

class Player;
class Camera;

/// <summary>
/// �Q�[���V�[���N���X
/// </summary>
class SceneMain : public SceneBase
{
public:
	SceneMain();
	virtual~SceneMain();

	void Init() override;
	SceneBase* Update() override;
	void Draw() override;

	/// <summary>
	/// �|�[�Y��Ԃ�ݒ肷��
	/// </summary>
	/// <param name="paused">�|�[�Y���</param>
	void SetPaused(bool paused);

	// �J�������擾���郁�\�b�h��ǉ�
	Camera* GetCamera() const { return m_pCamera.get(); }
	
	/// <summary>
	/// �J�����̊��x��ݒ肷��
	/// </summary>
	/// <param name="sensitivity">���x</param>
	void SetCameraSensitivity(float sensitivity);

private:
	/// <summary>
	/// �|�[�Y���j���[��`��
	/// </summary>
	void DrawPauseMenu();

private:
	std::unique_ptr<Player> m_pPlayer;
	std::shared_ptr<Camera> m_pCamera;

	// �|�[�Y�ɓ������^�C�~���O�̎��Ԃ��L�^����
	std::chrono::steady_clock::time_point m_pauseStartTime;

	bool  m_isPaused;               // �|�[�Y��Ԃ��Ǘ�����t���O
	bool  m_isReturningFromOption;  // �I�v�V�����V�[������߂������ǂ����𔻒肷��t���O
	bool  m_isEscapePressed;        // �G�X�P�[�v�L�[�̉�����Ԃ��Ǘ�����t���O
	int   m_skyDomeHandle;          // �X�J�C�h�[���̃n���h��
	int   m_skyDomeTextureHandle;   // �X�J�C�h�[���̃e�N�X�`���n���h��
	float m_cameraSensitivity;      // �J�����̊��x
};

