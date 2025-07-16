﻿#pragma once
#include "DxLib.h"
#include <vector>
#include <memory>

class Camera;
class Effect;
class Bullet;
class EnemyBase;
class EnemyNormal;
class CapsuleCollider;

class Player
{
public:
	Player();
	virtual ~Player();

	void Init();
	void Update(const std::vector<EnemyBase*>& enemyList);
	void Draw();

	/// <summary>
	/// カメラを取得する
	/// </summary>
	/// <returns>カメラのポインタ</returns>
	std::shared_ptr<Camera> GetCamera() const { return m_pCamera; }

	/// <summary>
	/// プレイヤーがダメージを受ける
	/// </summary>
	/// <param name="damage">受けるダメージ量</param>
	void TakeDamage(float damage);

	/// <summary>
	/// プレイヤーの位置を取得する
	/// </summary>
	/// <returns>プレイヤーの位置</returns>
	VECTOR GetPos() const { return m_modelPos; }

	void SetPos(const VECTOR& pos) { m_pos = pos; } // プレイヤーの位置を設定する

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

	/// <summary>
	/// タックル情報構造体
	/// </summary>
	struct TackleInfo 
	{
		VECTOR capA		  = { 0,0,0 }; // タックル判定カプセルのA点
		VECTOR capB		  = { 0,0,0 }; // タックル判定カプセルのB点
		float  radius	  = 0.0f;      // タックル判定カプセルの半径
		float  damage	  = 0.0f;	   // タックルのダメージ量
		bool   isTackling = false;	   // タックル中かどうか
		int    tackleId   = 0;         // タックルID
	};

	/// <summary>
	/// タックル情報を取得する
	/// </summary>
	/// <returns>タックル情報</returns>
	TackleInfo GetTackleInfo() const;

	/// <summary>
	/// プレイヤーのカプセル当たり判定情報を取得
	/// </summary>
	/// <param name="capA">カプセルのA端点</param>
	/// <param name="capB">カプセルのB端点</param>
	/// <param name="radius">カプセルの半径</param>
	void GetCapsuleInfo(VECTOR& capA, VECTOR& capB, float& radius) const;

	/// <summary>
	/// プレイヤーの体力を取得する
	/// </summary>
	/// <returns>プレイヤーの体力</returns>
	float GetHealth() const { return m_health; }

	/// <summary>
	/// 最大体力を取得する
	/// </summary>
	/// <returns>最大体力</returns>
	float GetMaxHealth() const { return m_maxHealth; }

	void AddHp(float value);
	void AddAmmo(int value); // 弾薬回復用関数（上限なし）

	// TODO:アニメーション処理は敵も行うので後でクラス化

	/// <summary>
	/// アニメーションデータ構造体
	/// </summary>
	struct AnimData
	{
		int   attachNo = -1;    // アタッチされているアニメーションの番号
		float count = 0.0f;  // アニメーションのカウント
		bool  isLoop = false; // ループアニメーションかどうか
		bool  isEnd = false; // アニメーションが終了したかどうか
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

	// プレイヤーのカプセルコライダー取得
	std::shared_ptr<CapsuleCollider> GetBodyCollider() const;

	// 回復エフェクトを発動する関数
	void TriggerHealEffect();

	// 弾薬エフェクトを発動する関数
	void TriggerAmmoEffect();

private:
	/// <summary>
	/// 弾を発射する
	/// </summary>
	void Shoot(std::vector<Bullet>& bullets);

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
	std::shared_ptr<Camera>		 m_pCamera;		 // カメラのポインタ
	std::shared_ptr<Camera>		 m_pDebugCamera; // デバッグ用カメラのポインタ
	//std::shared_ptr<Effect>	     m_pEffect;		 // エフェクトのポインタ
	std::vector<Bullet>			 m_bullets;      // 弾の管理
	std::shared_ptr<EnemyNormal> m_pEnemy;       // 敵のポインタ
	std::shared_ptr<CapsuleCollider> m_pBodyCollider; // プレイヤーのカプセルコライダー

	// プレイヤーの位置を保持するメンバー変数
	VECTOR m_pos;
	VECTOR m_modelPos;
	VECTOR m_tackleDir; // タックルの方向

	// アニメーションデータ
	AnimData m_nextAnimData;
	AnimData m_prevAnimData;

	unsigned char m_prevKeyState[256]{}; // 前回のキー入力状態
	
	float m_animBlendRate; // アニメーションのブレンド率
	float m_health;        // 現在の体力
	float m_jumpVelocity;  // ジャンプの速度
	float m_maxHealth;     // 最大体力
	float m_moveSpeed;    // 移動速度
	float m_runSpeed;      // 走る速度
	float m_bulletPower; // 弾の威力
	VECTOR m_scale; // スケール
	float m_tackleCooldownMax; // タックルクールタイム
	float m_tackleSpeed;        // タックル時の速度
	float m_tackleDamage;       // タックルダメージ
	int m_initialAmmo;     // 初期弾薬数

	int   m_modelHandle;    // プレイヤーモデルのハンドル
	int   m_swordHandle;    // 剣のハンドル
	int   m_shootSEHandle;  // シュートのSEハンドル
	int   m_ammo;		    // プレイヤーの弾薬数	
	int   m_tackleFrame;    // タックルのフレーム数
	int   m_tackleCooldown; // タックルのクールダウンタイマー
	int   m_tackleId;       // タックルID

	bool  m_isMoving;	   // プレイヤーが移動中かどうか
	bool  m_isJumping;	   // プレイヤーがジャンプ中かどうか
	bool  m_isWasRunning;  // 前回の移動状態が走っていたかどうか
	bool  m_hasShot;       // プレイヤーがショット可能かどうか
	bool  m_isTackling;    // タックル中かどうか
	float m_damageEffectTimer; // ダメージエフェクト用タイマー
	float m_damageEffectAlpha; // ダメージエフェクト用アルファ値
	float m_healEffectTimer;
	float m_healEffectAlpha; // ヒールエフェクト用アルファ値
	float m_ammoEffectTimer = 0.0f; // 弾薬エフェクト用タイマー
	float m_ammoEffectAlpha = 0.0f; // 弾薬エフェクト用アルファ値

    struct EffectFeedback {
        float timer = 0.0f;
        float alpha = 0.0f;
        int colorR = 255;
        int colorG = 0;
        int colorB = 0;
        float duration = 45.0f;
        void Trigger(float d, int r, int g, int b) {
            timer = d;
            alpha = 1.0f;
            duration = d;
            colorR = r; colorG = g; colorB = b;
        }
    };
    EffectFeedback m_damageEffect;
    EffectFeedback m_healEffect;
    EffectFeedback m_ammoEffect;
    void DrawEffectFeedback(EffectFeedback& effect);
};

