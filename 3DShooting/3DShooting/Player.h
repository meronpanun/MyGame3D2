#pragma once
#include "DxLib.h"
#include <vector>
#include <memory>

class Camera;
class Effect;

/// <summary>
/// �v���C���[�N���X
/// </summary>
class Player
{
public:
	Player();
	virtual ~Player();

	void Init();
	void Update();
	void Draw();

	/// <summary>
	/// �J�����ւ̃A�N�Z�X���
	/// </summary>
	/// <returns>�J�����̋��L�|�C���^</returns>
	std::shared_ptr<Camera> GetCamera() const { return m_pCamera; }

	/// <summary>
	/// �t�B�[���h�̕`��
	/// </summary>
	void DrawField();

private:
	/// <summary>
	/// �e�𔭎�
	/// </summary>
	void Shoot();

	/// <summary>
	/// �A�j���[�V������ԊǗ��\����
	/// </summary>
	struct AnimData
	{
		int   attachNo = -1;    // �A�^�b�`�ԍ�
		float count    = 0.0f;  // �A�j���J�E���g
		bool  isLoop   = false; // �A�j���[�V�������I����������[�v���邩�Atrue:���[�v���� false:�Ō�̃t���[���Œ�~
		bool  isEnd    = false; // �A�j���[�V�������I��������
	};

	/// <summary>
	/// �A�j���[�V�������A�^�b�`
	/// </summary>
	/// <param name="data">�A�j���[�V�����f�[�^</param>
	/// <param name="animName">�A�j���[�V������</param>
	/// <param name="isLoop">���[�v���邩</param>
	void AttachAnime(AnimData& data, const char* animName, bool isLoop);

	/// <summary>
	/// �A�j���[�V�������X�V
	/// </summary>
	/// <param name="data">�A�j���[�V�����f�[�^</param>
	void UpdateAnime(AnimData& data);

	/// <summary>
	/// �A�j���[�V�����u�����h���̍X�V
	/// </summary>
	void UpdateAnimeBlend();

	/// <summary>
	/// �A�j���[�V������ύX
	/// </summary>
	/// <param name="animName">�A�j���[�V������</param>
	/// <param name="isLoop">���[�v���邩</param>
	void ChangeAnime(const char* animName, bool isLoop);

	/// <summary>
	/// �e�̈ʒu�Ɖ�]���擾
	/// </summary>
	VECTOR GetGunPos() const;
	VECTOR GetGunRot() const;

//	void Reload(); // �����[�h�Ǘ�

private:
	// �J�����̊Ǘ�
	std::shared_ptr<Camera> m_pCamera;
	// �G�t�F�N�g�̊Ǘ�
	std::shared_ptr<Effect> m_pEffect; 

	// ���f���̈ʒu
	VECTOR m_modelPos;

	/* �A�j���[�V�����Đ��ɕK�v�ȃf�[�^���܂Ƃ߂����� */
	AnimData m_nextAnimData; // �Ō�ɐݒ肵���A�j�����
	AnimData m_prevAnimData; // ��O�ɐݒ肵���A�j�����

	// �A�j���[�V�����u�����h��
	float m_animBlendRate;  // 0.0f:�O�̃A�j���[�V���� 1.0f:���̃A�j���[�V����
	int   m_modelHandle;    // ���f���̃n���h��
	int   m_shieldHandle;   // ���̃��f���n���h��
	int   m_shootSEHandle;  // �e������SE�̃n���h��
	bool  m_isMoving;       // �ړ������ǂ���
	bool  m_isWasRunning;   // �����Ă������ǂ���

	float m_stamina;  // �X�^�~�i
	bool  m_isCanRun; // ����邩�ǂ���

	int  m_ammo;        // ���݂̒e��
//	int  m_maxAmmo;     // �ő�e��
//	int  m_reloadTimer; // �����[�h����
//	bool m_isReloading; // �����[�h�����ǂ���

	int m_shotCooldown; // ���˃N�[���^�C��
};

