#pragma once
#include "EffekseerForDXLib.h"

class Camera;
class Effect;

/// <summary>
/// プレイヤーのガード・盾管理クラス
/// </summary>
class PlayerShieldSystem
{
public:
	PlayerShieldSystem();
	~PlayerShieldSystem();

	void Init(float maxDurability, float regenRate);
	void Update(float deltaTime, Camera* pCamera, const VECTOR& playerPos, bool isGuarding, bool isTackling, bool isSwitchingWeapon, float weaponSwitchTimer, float weaponSwitchDuration, float yawDelta);
	void Draw(Camera* pCamera, const VECTOR& playerPos, bool isTackling, bool isSwitchingWeapon, float weaponSwitchTimer, float weaponSwitchDuration);

	/// <summary>
	/// ガード状態かどうか
	/// </summary>
	/// <returns>true: ガード状態, false: ガード状態でない</returns>
	bool IsGuarding() const { return m_isGuarding; }

	/// <summary>
	/// ガード状態設定
	/// </summary>
	/// <param name="guarding">ガード状態</param>
	void SetGuarding(bool guarding) { m_isGuarding = guarding; }

	/// <summary>
	/// 前フレームでガードしていたかどうか
	/// </summary>
	/// <returns>true: 前フレームでガードしていた, false: 前フレームでガードしていなかった</returns>
	bool WasGuarding() const { return m_wasGuarding; }

	/// <summary>
	/// 前フレームでガードしていたかどうか設定
	/// </summary>
	/// <param name="was">前フレームでガードしていたかどうか</param>
	void SetWasGuarding(bool was) { m_wasGuarding = was; }

	/// <summary>
	/// ガードアニメーションタイマー取得
	/// </summary>
	/// <returns>ガードアニメーションタイマー値</returns>
	float GetGuardAnimTimer() const { return m_guardAnimTimer; }

	/// <summary>
	/// ガードアニメーション時間取得
	/// </summary>
	/// <returns>ガードアニメーション時間</returns>
	float GetGuardAnimDuration() const { return m_guardAnimDuration; }

	/// <summary>
	/// 盾の耐久値取得
	/// </summary>
	/// <returns>盾の耐久値</returns>
	float GetDurability() const { return m_shieldDurability; }

	/// <summary>
	/// 盾の最大耐久値取得
	/// </summary>
	/// <returns>盾の最大耐久値</returns>
	float GetMaxDurability() const { return m_maxShieldDurability; }

	/// <summary>
	/// 盾バーアニメーション値取得
	/// </summary>
	/// <returns>盾バーアニメーション値</returns>
	float GetBarAnim() const { return m_shieldBarAnim; }

	/// <summary>
	/// 盾が壊れているかどうか
	/// </summary>
	/// <returns>true: 壊れている, false: 壊れていない</returns>
	bool IsShieldBroken() const { return m_isShieldBroken; }

	/// <summary>
	/// 盾でダメージを受ける
	/// </summary>
	/// <param name="damage">ダメージ量</param>
	/// <param name="pEffect">エフェクトポインタ</param>
	/// <param name="pCamera">カメラポインタ</param>
	/// <param name="playerPos">プレイヤー位置ベクトル</param>
	/// <returns>残りのダメージ量</returns>
	float TakeDamage(float damage, Effect* pEffect, Camera* pCamera, const VECTOR& playerPos);

	/// <summary>
	/// ジャストガード判定
	/// </summary>
	/// <returns>true: ジャストガード成功, false: できなかった</returns>
	bool IsJustGuarded() const;

	/// <summary>
	/// ガードタイマー取得
	/// </summary>
	/// <returns>ガードタイマー値</returns>
	int GetGuardTimer() const { return m_guardTimer; }

	/// <summary>
	/// ガードエフェクト更新
	/// </summary>
	/// <param name="pEffect">エフェクトポインタ</param>
	/// <param name="pCamera">カメラポインタ</param>
	/// <param name="playerPos">プレイヤー位置ベクトル</param>
	/// <param name="isSwitchingWeapon">武器切り替え中かどうか</param>
	void UpdateGuardEffect(Effect* pEffect, Camera* pCamera, const VECTOR& playerPos, bool isSwitchingWeapon);

	/// <summary>
	/// スパークエフェクト更新
	/// </summary>
	/// <param name="pEffect">エフェクトポインタ</param>
	/// <param name="playerPos">プレイヤー位置ベクトル</param>
	/// <param name="pCamera">カメラポインタ</param>
	void UpdateSparkEffect(Effect* pEffect, const VECTOR& playerPos, Camera* pCamera);

	/// <summary>
	/// 盾のSwayオフセット取得
	/// </summary>
	/// <returns>Swayオフセットベクトル</returns>
	VECTOR GetShieldSwayOffset() const { return m_shieldSwayOffset; }

	/// <summary>
	/// 盾のSway回転オフセット取得
	/// </summary>
	/// <returns>Sway回転オフセットベクトル</returns>
	VECTOR GetShieldSwayRotOffset() const { return m_shieldSwayRotOffset; }

	/// <summary>
	/// 盾UI画像ハンドル取得
	/// </summary>
	/// <returns>盾UI画像ハンドル</returns>
	int GetShieldImageHandle() const;

private:
	int m_shieldModelHandle;
	int m_shieldImageHandle;

	float m_shieldDurability;
	float m_shieldBarAnim;
	float m_maxShieldDurability;
	float m_shieldRegenRate;
	bool m_isShieldBroken;

	// ガード関連
	bool m_isGuarding;
	bool m_wasGuarding;
	float m_guardAnimTimer;
	float m_guardAnimDuration;
	int m_guardTimer;

	// 盾のアニメーション
	bool m_isShieldAnimating;
	bool m_isShieldRecovering;
	float m_shieldAnimTimer;
	float m_shieldAnimDuration;

	// エフェクト
	int m_guardEffectHandle;
	int m_sparkEffectHandle;
	int m_sparkEffectTimer;

	// Sway
	VECTOR m_shieldSwayOffset;
	VECTOR m_shieldSwayRotOffset;

	// 武器切り替え状態の記録（エフェクト再生用）
	bool m_wasSwitchingWeapon;
};

