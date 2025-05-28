#pragma once
#include "SceneBase.h"

/// <summary>
/// �^�C�g���V�[���N���X
/// </summary>
class SceneTitle : public SceneBase
{
public:
	SceneTitle(bool skipLogo = false);
	virtual ~SceneTitle();

	void Init() override;
	SceneBase* Update() override;
	void Draw() override;

	void SkipLogo();

private:
	int m_logoHandle;	   // �^�C�g�����S�̃n���h��
	int m_fadeAlpha;	   // �t�F�[�h�̃A���t�@�l
	int m_fadeFrame;	   // �t�F�[�h�̃t���[���J�E���g
	int m_sceneFadeAlpha;  // �V�[���t�F�[�h�̃A���t�@�l
	int m_waitFrame;	   // �ҋ@�t���[��
	bool m_isFadeComplete; // �t�F�[�h�����t���O
	bool m_isFadeOut;	   // �t�F�[�h�A�E�g�t���O
	bool m_skipLogo;       // ���S�X�L�b�v�p�̃t���O
	bool m_isSceneFadeIn;  // �V�[���t�F�[�h�C���t���O
};

