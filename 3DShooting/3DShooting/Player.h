#pragma once
#include "DxLib.h"
#include <vector>
#include <memory>

class Camera;
class Effect;

class Player
{
public:
	Player();
	virtual ~Player();

	void Init();
	void Update();
	void Draw();


	std::shared_ptr<Camera> GetCamera() const { return m_pCamera; }


	/// <summary>
	/// �t�B�[���h��`�悷��
	/// </summary>
	void DrawField();

	void TakeDamage(float damage);

	/// <summary>
	/// �v���C���[�̈ʒu���擾����
	/// </summary>
	/// <returns>�v���C���[�̈ʒu</returns>
	VECTOR GetPos() const { return m_pos; }


private:

	/// <summary>
	/// �e�𔭎˂���
	/// </summary>
	void Shoot();

	struct AnimData
	{
		int   attachNo = -1;
		float count = 0.0f;
		bool  isLoop = false;
		bool  isEnd = false;
	};


	void AttachAnime(AnimData& data, const char* animName, bool isLoop);


	void UpdateAnime(AnimData& data);


	void UpdateAnimeBlend();

	void ChangeAnime(const char* animName, bool isLoop);

	/// <summary>
	/// �e�̈ʒu���擾����
	/// </summary>
	/// <returns>�e�̈ʒu</returns>
	VECTOR GetGunPos() const;
	VECTOR GetGunRot() const;

	//	void Reload();

private:

	std::shared_ptr<Camera> m_pCamera;

	std::shared_ptr<Effect> m_pEffect;

	// �v���C���[�̈ʒu��ێ����郁���o�[�ϐ�
	VECTOR m_pos;

	VECTOR m_modelPos;


	AnimData m_nextAnimData;
	AnimData m_prevAnimData;


	float m_animBlendRate;
	int   m_modelHandle;
	int   m_shieldHandle;
	int   m_shootSEHandle;
	bool  m_isMoving;
	bool  m_isWasRunning;

	float m_stamina;
	bool  m_isCanRun;

	int  m_ammo;
	//	int  m_maxAmmo;   
	//	int  m_reloadTimer;
	//	bool m_isReloading;

	int m_shotCooldown;

	float m_health; // �v���C���[�̗̑�
};

