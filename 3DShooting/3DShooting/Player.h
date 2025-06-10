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
	/// カメラを取得する
	/// </summary>
	/// <returns>カメラのポインタ</returns>
	std::shared_ptr<Camera> GetCamera() const { return m_pCamera; }

	/// <summary>
	/// フィールドを描画する
	/// </summary>
	void DrawField();

	/// <summary>
	/// プレイヤーがダメージを受ける
	/// </summary>
	/// <param name="damage">受けるダメージ量</param>
	void TakeDamage(float damage);  

	/// <summary>
	/// プレイヤーの位置を取得する
	/// </summary>
	/// <returns>プレイヤーの位置</returns>
	VECTOR GetPos() const { return m_pos; }

	/// <summary>
	/// 弾の取得
	/// </summary>
	/// <returns>弾のベクター</returns>
	std::vector<Bullet>& GetBullets();

	/// <summary>
	///  プレイヤーがショット可能かどうか
	/// </summary>
	/// <returns>ショット可能ならtrue</returns>
	bool HasShot(); 

private:

	/// <summary>
	/// 弾を発射する
	/// </summary>
	void Shoot();

	/// <summary>
	/// アニメーションデータ構造体
	/// </summary>
	struct AnimData
	{
		int   attachNo = -1;
		float count = 0.0f;
		bool  isLoop = false;
		bool  isEnd = false;
	};

	/// <summary>
	/// アニメーションをアタッチする
	/// </summary>
	/// <param name="data">アニメーションデータ</param>
	/// <param name="animName">アニメーション名</param>
	/// <param name="isLoop">ループするかどうか</param>
	void AttachAnime(AnimData& data, const char* animName, bool isLoop); 

	/// <summary>
	///  アニメーションの更新
	/// </summary>
	/// <param name="data">アニメーションデータ</param>
	void UpdateAnime(AnimData& data); 

	/// <summary>
	///  アニメーションのブレンドを更新
	/// </summary>
	void UpdateAnimeBlend(); 

	/// <summary>
	/// アニメーションを変更する
	/// </summary>
	/// <param name="animName">アニメーション名</param>
	/// <param name="isLoop">ループするかどうか</param>
	void ChangeAnime(const char* animName, bool isLoop); 

	/// <summary>
	/// 銃の位置を取得する
	/// </summary>
	/// <returns>銃の位置</returns>
	VECTOR GetGunPos() const;
	VECTOR GetGunRot() const;

private:
	std::shared_ptr<Camera> m_pCamera; // カメラのポインタ
	std::shared_ptr<Effect> m_pEffect; // エフェクトのポインタ
	std::vector<Bullet> m_bullets; // 弾の管理

	// プレイヤーの位置を保持するメンバー変数
	VECTOR m_pos;
	VECTOR m_modelPos;

	// アニメーションデータ
	AnimData m_nextAnimData;
	AnimData m_prevAnimData;

	
	float m_animBlendRate; // アニメーションのブレンド率
	float m_stamina;	   // プレイヤーのスタミナ
	float m_health;        // プレイヤーの体力
	float m_jumpVelocity;  // ジャンプの速度

	int   m_modelHandle;    // プレイヤーモデルのハンドル
	int   m_swordHandle;    // 剣のハンドル
	int   m_shootSEHandle;  // シュートのSEハンドル
	int   m_shotCooldown;   // ショットのクールダウンタイマー
	int   m_ammo;		    // プレイヤーの弾薬数	
	int   m_lockOnTargetId; // ロックオンターゲットのID

	bool  m_isMoving;	   // プレイヤーが移動中かどうか
	bool  m_isJumping;	   // プレイヤーがジャンプ中かどうか
	bool  m_isWasRunning;  // 前回の移動状態が走っていたかどうか
	bool  m_isCanRun;	   // プレイヤーが走れるかどうか
	bool  m_hasShot;       // プレイヤーがショット可能かどうか
	bool  m_isLockOn;      // ロックオン状態かどうか
};

