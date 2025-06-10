#pragma once
#include "DxLib.h"
#include <vector>
#include <memory>

class Camera;
class Effect;
class Bullet;

class Player
{
public:
	Player();
	virtual ~Player();

	void Init();
	void Update();
	void Draw();

	/// <summary>
	/// �J�������擾����
	/// </summary>
	/// <returns>�J�����̃|�C���^</returns>
	std::shared_ptr<Camera> GetCamera() const { return m_pCamera; }

	/// <summary>
	/// �t�B�[���h��`�悷��
	/// </summary>
	void DrawField();

	/// <summary>
	/// �v���C���[���_���[�W���󂯂�
	/// </summary>
	/// <param name="damage">�󂯂�_���[�W��</param>
	void TakeDamage(float damage);  

	/// <summary>
	/// �v���C���[�̈ʒu���擾����
	/// </summary>
	/// <returns>�v���C���[�̈ʒu</returns>
	VECTOR GetPos() const { return m_pos; }

	/// <summary>
	/// �e�̎擾
	/// </summary>
	/// <returns>�e�̃x�N�^�[</returns>
	std::vector<Bullet>& GetBullets();

	/// <summary>
	///  �v���C���[���V���b�g�\���ǂ���
	/// </summary>
	/// <returns>�V���b�g�\�Ȃ�true</returns>
	bool HasShot(); 

private:

	/// <summary>
	/// �e�𔭎˂���
	/// </summary>
	void Shoot();

	/// <summary>
	/// �A�j���[�V�����f�[�^�\����
	/// </summary>
	struct AnimData
	{
		int   attachNo = -1;
		float count = 0.0f;
		bool  isLoop = false;
		bool  isEnd = false;
	};

	/// <summary>
	/// �A�j���[�V�������A�^�b�`����
	/// </summary>
	/// <param name="data">�A�j���[�V�����f�[�^</param>
	/// <param name="animName">�A�j���[�V������</param>
	/// <param name="isLoop">���[�v���邩�ǂ���</param>
	void AttachAnime(AnimData& data, const char* animName, bool isLoop); 

	/// <summary>
	///  �A�j���[�V�����̍X�V
	/// </summary>
	/// <param name="data">�A�j���[�V�����f�[�^</param>
	void UpdateAnime(AnimData& data); 

	/// <summary>
	///  �A�j���[�V�����̃u�����h���X�V
	/// </summary>
	void UpdateAnimeBlend(); 

	/// <summary>
	/// �A�j���[�V������ύX����
	/// </summary>
	/// <param name="animName">�A�j���[�V������</param>
	/// <param name="isLoop">���[�v���邩�ǂ���</param>
	void ChangeAnime(const char* animName, bool isLoop); 

	/// <summary>
	/// �e�̈ʒu���擾����
	/// </summary>
	/// <returns>�e�̈ʒu</returns>
	VECTOR GetGunPos() const;
	VECTOR GetGunRot() const;

private:
	std::shared_ptr<Camera> m_pCamera; // �J�����̃|�C���^
	std::shared_ptr<Effect> m_pEffect; // �G�t�F�N�g�̃|�C���^
	std::vector<Bullet> m_bullets; // �e�̊Ǘ�

	// �v���C���[�̈ʒu��ێ����郁���o�[�ϐ�
	VECTOR m_pos;
	VECTOR m_modelPos;

	// �A�j���[�V�����f�[�^
	AnimData m_nextAnimData;
	AnimData m_prevAnimData;

	
	float m_animBlendRate; // �A�j���[�V�����̃u�����h��
	float m_stamina;	   // �v���C���[�̃X�^�~�i
	float m_health;        // �v���C���[�̗̑�
	float m_jumpVelocity;  // �W�����v�̑��x

	int   m_modelHandle;    // �v���C���[���f���̃n���h��
	int   m_swordHandle;    // ���̃n���h��
	int   m_shootSEHandle;  // �V���[�g��SE�n���h��
	int   m_shotCooldown;   // �V���b�g�̃N�[���_�E���^�C�}�[
	int   m_ammo;		    // �v���C���[�̒e��	
	int   m_lockOnTargetId; // ���b�N�I���^�[�Q�b�g��ID

	bool  m_isMoving;	   // �v���C���[���ړ������ǂ���
	bool  m_isJumping;	   // �v���C���[���W�����v�����ǂ���
	bool  m_isWasRunning;  // �O��̈ړ���Ԃ������Ă������ǂ���
	bool  m_isCanRun;	   // �v���C���[������邩�ǂ���
	bool  m_hasShot;       // �v���C���[���V���b�g�\���ǂ���
	bool  m_isLockOn;      // ���b�N�I����Ԃ��ǂ���
};

