#pragma once
#include "EffekseerForDXLib.h"
#include "AttackType.h"
#include "Camera.h"
#include <vector>

// 武器の種類
enum class WeaponType
{
	AssaultRifle, // アサルトライフル
	Shotgun       // ショットガン
};

class Camera;
class Effect;
class Bullet;
class EnemyBase;
class EnemyNormal;
class CapsuleCollider;
class DirectionIndicator;
class ShellCasing;
class AnimationManager;

/// <summary>
/// プレイヤークラス
/// </summary>
class Player
{
public:
	Player();
	virtual ~Player();

	void Init();
	void Update(const std::vector<EnemyBase*>& enemyList);
	void Draw3D();
	void DrawShield();
	void DrawUI();

	/// <summary>
	/// エフェクトフィードバック構造体
	/// </summary>
	struct EffectFeedback
	{
		int colorR = 255;
		int colorG = 0;
		int colorB = 0;
		float timer = 0.0f;
		float alpha = 0.0f;
		float duration = 45.0f;
		void Trigger(float d, int r, int g, int b)
		{
			timer	 = d;
			alpha    = 1.0f;
			duration = d;
			colorR   = r; 
			colorG   = g; 
			colorB   = b;
		}
	};

	/// <summary>
	/// タックル情報構造体
	/// </summary>
	struct TackleInfo
	{
		VECTOR capA = { 0,0,0 }; // タックル判定カプセルのA点
		VECTOR capB = { 0,0,0 }; // タックル判定カプセルのB点
		float  radius = 0.0f;      // タックル判定カプセルの半径
		float  damage = 0.0f;	   // タックルのダメージ量
		bool   isTackling = false; // タックル中かどうか
		int    tackleId = 0;       // タックルID
	};

	// カメラを取得する
	const std::shared_ptr<Camera>& GetCamera() const { return m_pCamera; }

    // 弾を撃つ
    void Shoot(std::vector<Bullet>& bullets);

	/// <summary>
	/// プレイヤーがダメージを受ける
	/// </summary>
	/// <param name="damage">受けるダメージ量</param>
	/// <param name="attackerPos">攻撃者の位置（オプション）</param>
	void TakeDamage(float damage, const VECTOR& attackerPos = VGet(0, 0, 0));

	/// <summary>
	/// プレイヤーの位置を取得する
	/// </summary>
	/// <returns>プレイヤーの位置</returns>
	VECTOR GetPos() const { return m_modelPos; }

	/// <summary>
	/// プレイヤーの位置を設定する
	/// </summary>
	/// <param name="pos">設定する位置</param>
	void SetPos(const VECTOR& pos) { m_pos = pos; }

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
	/// タックル情報を取得する
	/// </summary>
	/// <returns>タックル情報</returns>
	TackleInfo GetTackleInfo() const;

	// カプセル情報を取得
	void GetCapsuleInfo(VECTOR& capA, VECTOR& capB, float& radius) const;

	// パリィが成功したかどうか
	bool IsJustGuarded() const;

    // 体力を加算する
    void AddHp(float value);

    // プレイヤーがガード中かどうか
    bool IsGuarding() const { return m_isGuarding; }

	// 体力を取得する
	float GetHealth() const { return m_health; }

	// 最大体力を取得する
	float GetMaxHealth() const { return m_maxHealth; }

	/// <summary>
	/// 弾薬回復用関数（上限なし）
	/// </summary>
	/// <param name="value">弾薬数</param>
	void AddAmmo(int value);

	/// <summary>
	/// 弾薬無限モードを設定する
	/// </summary>
	/// <param name="isInfinite">無限にするかどうか</param>
	void SetInfiniteAmmo(bool isInfinite) { m_isInfiniteAmmo = isInfinite; }

	/// <summary>
	/// 弾薬無限モードかどうかを取得する
	/// </summary>
	/// <returns>弾薬無限モードならtrue</returns>
	bool IsInfiniteAmmo() const { return m_isInfiniteAmmo; }

	// プレイヤーのカプセルコライダー取得
	std::shared_ptr<CapsuleCollider> GetBodyCollider() const;

	/// <summary>
	/// 回復SEハンドルを取得する
	/// </summary>
	int GetRecoverySEHandle() const { return m_recoverySEHandle; }

	/// <summary>
	/// 弾薬アイテムSEハンドルを取得する
	/// </summary>
	int GetAmmoItemSEHandle() const { return m_ammoItemSEHandle; }

	/// <summary>
	/// 無敵モードを設定する
	/// </summary>
	/// <param name="isInvincible">無敵にするかどうか</param>
	void SetInvincible(bool isInvincible) { m_isInvincible = isInvincible; }

	/// <summary>
	/// 無敵モードかどうかを取得する
	/// </summary>
	/// <returns>無敵モードならtrue</returns>
	bool IsInvincible() const { return m_isInvincible; }

	/// <summary>
	/// ロックオン可能な敵がいるかどうかを取得する
	/// </summary>
	/// <returns>ロックオン可能な敵がいるならtrue</returns>
	bool IsTargetAvailable() const { return m_isTargetAvailable; }

	/// <summary>
	/// 敵に照準が合っているかどうかを取得する
	/// </summary>
	/// <returns>照準が合っているならtrue</returns>
	bool IsAimingAtEnemy() const;
	
	/// <summary>
	/// 体力が低下しているかどうかを取得する
	/// </summary>
	/// <returns>体力が低下しているならtrue</returns>
	bool IsLowHealth() const { return m_isLowHealth; }
		
		/// <summary>	/// 攻撃制限を設定する
	/// </summary>
	/// <param name="allowedAttack">許可する攻撃タイプ</param>
	void SetAttackRestrictions(AttackType allowedAttack);
	  
	/// <summary>
	/// プレイヤーが死亡しているかどうか
	/// </summary>
	/// <returns>死亡しているならtrue</returns>
	bool IsDead() const { return m_isDead; }

	/// <summary>
	/// 方向インジケーターを設定する
	/// </summary>
	/// <param name="directionIndicator">方向インジケーターのポインタ</param>
	void SetDirectionIndicator(DirectionIndicator* directionIndicator) { m_pDirectionIndicator = directionIndicator; }

	/// <summary>
	/// 銃を揺らす
	/// </summary>
	/// <param name="power">揺れの強さ</param>
	/// <param name="duration">揺れの持続時間</param>
	void ShakeGun(float power, float duration);

	/// <summary>
	/// パリィエフェクトを再生する
	/// </summary>
	void PlayParryEffect(const VECTOR& pos);

	/// <summary>
	/// エフェクトを設定する
	/// </summary>
	/// <param name="pEffect">エフェクトのポインタ</param>
	void SetEffect(Effect* pEffect) { m_pEffect = pEffect; }

	/// <summary>
	/// アニメーションマネージャーを設定する
	/// </summary>
	/// <param name="pAnimManager">アニメーションマネージャーのポインタ</param>
	void SetAnimationManager(AnimationManager* pAnimManager) { m_pAnimManager = pAnimManager; }
	    
private:
   	AttackType m_allowedAttackType = AttackType::None;
   	/// <summary>
   	/// 死亡時の更新処理
   	/// </summary>
   	void DeathUpdate();

	/// <summary>
	/// 銃の位置を取得する
	/// </summary>
	/// <returns>銃の位置</returns>
	VECTOR GetGunPos() const;
	VECTOR GetGunRot() const;

	/// <summary>
	/// 薬莢排出口の位置を取得する
	/// </summary>
	/// <returns>薬莢排出口の位置</returns>
	VECTOR GetEjectionPortPos() const;

	/// <summary>
	/// エフェクトフィードバックを描画する
	/// </summary>
	/// <param name="effect">エフェクトフィードバック構造体</param>
	void DrawEffectFeedback(EffectFeedback& effect);

private:
	std::vector<ShellCasing>     m_shellCasings;
	std::shared_ptr<Camera>		 m_pCamera;		 // カメラのポインタ
	std::shared_ptr<Camera>		 m_pDebugCamera; // デバッグ用カメラのポインタ
	std::vector<Bullet>			 m_bullets;      // 弾の管理
	std::shared_ptr<CapsuleCollider> m_pBodyCollider; // プレイヤーのカプセルコライダー
	Effect* m_pEffect; // エフェクトのポインタ
	AnimationManager* m_pAnimManager; // アニメーションマネージャーのポインタ
	bool m_isSGAnimPlaying;
	float m_sGAnimTime;

	// プレイヤーの位置を保持するメンバー変数
	VECTOR m_pos;
	VECTOR m_modelPos;
	VECTOR m_tackleDir; // タックルの方向
	VECTOR m_scale; // スケール

	EffectFeedback m_damageEffect;
	EffectFeedback m_healEffect;
	EffectFeedback m_ammoEffect;

	unsigned char m_prevKeyState[256]{}; // 前回のキー入力状態

	int   m_fontHandle;					   // フォントハンドル
	int   m_hpFontHandle;			       // HPフォントハンドル
	int   m_aRInitAmmo;		               // 初期弾薬数
	int   m_aRHandle;					   // アサルトライフルモデルのハンドル
	int   m_sGHandle;					   // ショットガンモデルのハンドル
	int   m_shieldModelHandle;			   // 盾モデルのハンドル
	int   m_shieldImageHandle;			   // 盾のUI画像のハンドル
	int   m_ammoImageHandle;			   // 弾のハンドル
	int   m_shotSEHandle;			       // ショットのSEハンドル
	int   m_sGShotSEHandle;				   // ショットガンのショットSEハンドル
	int   m_playerHitSEHandle;		       // 被弾SEのハンドル
	int   m_tackleSEHandle;				   // タックルSEのハンドル
	int   m_recoverySEHandle;			   // 回復アイテムSEのハンドル
	int   m_ammo;						   // プレイヤーの弾薬数	
	int   m_tackleFrame;				   // タックルのフレーム数
	int   m_tackleCooldown;				   // タックルのクールダウンタイマー
	int   m_tackleId;					   // タックルID
	int   m_concentrationLineEffectHandle; // 集中線エフェクトハンドル
	int   m_ammoItemSEHandle;			   // 弾薬アイテムSEのハンドル
	int   m_landingSEHandle;			   // 着地SEのハンドル
	int   m_ejectionPortFrame;			   // 薬莢排出口フレーム
	int   m_noAmmoImageHandle;			   // 弾薬切れUI画像のハンドル
	int   m_aRImageHandle;	               // アサルトライフルUI画像のハンドル
	int   m_lowAmmoARImageHandle;          // 弾が少ない時のアサルトライフルUI画像のハンドル
	int   m_noAmmoARImageHandle;           // 弾が0の時のアサルトライフルUI画像のハンドル
	int   m_sGImageHandle;                 // ショットガンUI画像のハンドル
	int   m_lowAmmoSGImageHandle;          // 弾が少ない時のショットガンUI画像のハンドル
	int   m_noAmmoSGImageHandle;           // 弾が0の時のショットガンUI画像のハンドル
	int   m_healthUiImageHandle;		   // HPUI画像のハンドル

	bool  m_isLowAmmo;					   // 弾薬が少ないかどうかのフラグ
	float m_lowAmmoBlinkTimer;			   // 弾薬切れUIの点滅タイマー
	bool  m_showLowAmmoWarning;			   // 弾薬切れUIの表示フラグ

	int   m_noHealthImageHandle;           // 体力低下UI画像のハンドル
	bool  m_isLowHealth;                   // 体力が少ないかどうかのフラグ
	float m_lowHealthBlinkTimer;           // 体力低下UIの点滅タイマー

	float m_health;				// 現在の体力
	float m_healthBarAnim;		// HPバーのアニメーション用体力値
	float m_healthBarAnimTimer; // HPバーアニメーション用タイマー
	float m_jumpVelocity;		// ジャンプの速度
	float m_maxHealth;		    // 最大体力
	float m_moveSpeed;			// 移動速度
	float m_runSpeed;			// 走る速度
	float m_bulletPower;		// 弾の威力
	float m_tackleCooldownMax;  // タックルクールタイム
	float m_tackleSpeed;        // タックル時の速度
	float m_tackleDamage;       // タックルダメージ
	float m_damageEffectTimer;  // ダメージエフェクト用タイマー
	float m_damageEffectAlpha;  // ダメージエフェクト用アルファ値
	float m_healEffectTimer;    // ヒールエフェクト用タイマー	
	float m_healEffectAlpha;    // ヒールエフェクト用アルファ値
	float m_ammoEffectTimer;    // 弾薬エフェクト用タイマー
	float m_ammoEffectAlpha;    // 弾薬エフェクト用アルファ値
	float m_shootCooldown;      // 発射クールタイム
	float m_shootCooldownTimer; // クールタイムタイマー
	float m_aRShootRate;        // アサルトライフルの1秒あたりの発射回数

	bool  m_isMoving;	    // プレイヤーが移動中かどうか
	bool  m_isJumping;	    // プレイヤーがジャンプ中かどうか
	bool  m_wasJumping;	    // 前のフレームでジャンプしていたかどうか
	bool  m_isWasRunning;   // 前回の移動状態が走っていたかどうか
	bool  m_hasShot;        // プレイヤーがショット可能かどうか
	bool  m_isTackling;     // タックル中かどうか
	bool  m_isInvincible;   // 無敵モードかどうか
	bool  m_isInfiniteAmmo; // 弾薬無限モードかどうか

	// 盾のアニメーション関連
	bool  m_isShieldAnimating;  // 盾がアニメーション中か
	bool  m_isShieldRecovering; // 盾が回復中か
	float m_shieldAnimTimer;    // 盾のアニメーションタイマー
	float m_shieldAnimDuration; // 盾のアニメーション時間

	float m_shieldDurability;   // 盾の耐久値
	float m_shieldBarAnim;      // 盾のUIアニメーション用の耐久値
	float m_maxShieldDurability; // 盾の最大耐久値
	float m_shieldRegenRate;     // 盾の回復速度
	bool m_isShieldBroken; // 盾が壊れているか
	int m_guardTimer;      // ガードしてからのフレーム数

private:
	int   m_warningFontHandle;  // 警告用フォントハンドル
	bool  m_isNoAmmoWarning;    // 弾薬切れ警告表示フラグ
	float m_ammoTextFlashTimer; // 弾薬テキストのフラッシュタイマー
	
	// Sway管理
	float  m_idleSwayTimer;      // 待機時の揺れタイマー
	VECTOR m_gunSwayOffset;
	VECTOR m_gunSwayRotOffset;
	VECTOR m_shieldSwayOffset;
	VECTOR m_shieldSwayRotOffset;

	// 銃のシェイク関連
	VECTOR m_gunShakeOffset;      // 銃のシェイクオフセット
	float  m_gunShakeTimer;       // 銃のシェイクタイマー
	float  m_gunShakePower;       // 銃のシェイクの強さ

	// 死亡関連
	bool m_isDead;      // 死亡フラグ
	float m_deathTimer; // 死亡アニメーションタイマー

	// 方向インジケーター
	DirectionIndicator* m_pDirectionIndicator;

	// ロックオン関連
	bool m_isLockingOn;         // ロックオン中か
	EnemyBase* m_lockedOnEnemy; // ロックオンした敵
	int m_lockOnUIHandle;       // ロックオンUIのハンドル
	bool m_isTargetAvailable;   // ロックオン可能な敵がいるか
	bool m_isAimingAtEnemy;     // 敵に照準が合っているか

	// ガード関連
	bool  m_ignoreGuardInput;   // ガード入力を無視するか
	bool  m_isGuarding;         // ガード中か
	bool  m_wasGuarding;        // 前フレームでガードしていたか
	float m_guardAnimTimer;     // ガードアニメーションのタイマー
	float m_guardAnimDuration;  // ガードアニメーションの時間
	int m_guardEffectHandle;    // ガードエフェクトのハンドル
	int m_sparkEffectHandle;    // スパークエフェクトのハンドル
	int m_sparkEffectTimer;     // スパークエフェクトのタイマー
	float m_guardEffectScale;   // ガードエフェクトのスケール
	int m_parryEffectHandle;    // パリィエフェクトのハンドル

	WeaponType m_currentWeaponType = WeaponType::AssaultRifle; // 現在装備している武器の種類

	/// <summary>
	/// 武器を切り替える
	/// </summary>
	/// <param name="weaponType">切り替える武器の種類</param>
	void SwitchWeapon(WeaponType weaponType);
};