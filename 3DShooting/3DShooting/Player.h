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
	/// フィールドを描画する
	/// </summary>
	void DrawField();

	void TakeDamage(float damage);

	/// <summary>
	/// プレイヤーの位置を取得する
	/// </summary>
	/// <returns>プレイヤーの位置</returns>
	VECTOR GetPos() const { return m_pos; }


private:

	/// <summary>
	/// 弾を発射する
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
	/// 銃の位置を取得する
	/// </summary>
	/// <returns>銃の位置</returns>
	VECTOR GetGunPos() const;
	VECTOR GetGunRot() const;

	//	void Reload();

private:

	std::shared_ptr<Camera> m_pCamera;

	std::shared_ptr<Effect> m_pEffect;

	// プレイヤーの位置を保持するメンバー変数
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

	float m_health; // プレイヤーの体力
};

