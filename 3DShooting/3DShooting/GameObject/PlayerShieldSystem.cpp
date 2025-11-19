#include "PlayerShieldSystem.h"
#include "Camera.h"
#include "Effect.h"
#include "Game.h"
#include "EffekseerForDXLib.h"
#include <cmath>
#include <cassert>

namespace
{
	// 盾関連
	constexpr float kShieldBaseScreenW = 640.0f;
	constexpr float kShieldBaseScreenH = 480.0f;
	constexpr float kShieldCamZ = -35.0f;
	constexpr float kShieldCamTargetFactor = 0.3f;
	constexpr float kShieldWaitX = -20.0f;
	constexpr float kShieldWaitY = -45.0f;
	constexpr float kShieldWaitZ = -10.0f;
	constexpr float kShieldPivotZ = -25.0f;
	constexpr float kShieldModelScale = 2.0f;
	constexpr float kGuardAnimDuration = 0.1f;
	constexpr float kGuardEffectOffsetZ = 60.0f;
	constexpr float kGuardEffectOffsetX = 10.0f;

	// 盾アニメーション関連
	constexpr float kShieldAnimRecoverStartYOffset = -200.0f;
	constexpr float kShieldAnimBreakEndYOffset = -200.0f;
	constexpr float kShieldAnimBreakRotY = DX_PI_F * 1.25f;
	constexpr float kShieldAnimBreakRotX = DX_PI_F * 0.25f;
	constexpr float kShieldAnimEasingPower = 3.0f;

	// カメラを左右に振った際の横揺れ関連の定数
	constexpr float kShieldSwayAmount = 4.0f;
	constexpr float kShieldSwayDamping = 0.9f;

	// Update関連
	constexpr float kDeltaTime = 1.0f / 60.0f;
	constexpr float kHpBarAnimSpeed = 1.5f;

	// パリィ受付フレーム数
	constexpr int kParryFrame = 60;

	// タックル関連
	constexpr float kTackleShieldThrust = 20.0f;
	constexpr float kGuardShakeAmount = 0.4f;
}

PlayerShieldSystem::PlayerShieldSystem() :
	m_shieldModelHandle(-1),
	m_shieldImageHandle(-1),
	m_shieldDurability(0.0f),
	m_shieldBarAnim(0.0f),
	m_maxShieldDurability(0.0f),
	m_shieldRegenRate(0.0f),
	m_isShieldBroken(false),
	m_isGuarding(false),
	m_wasGuarding(false),
	m_guardAnimTimer(0.0f),
	m_guardAnimDuration(kGuardAnimDuration),
	m_guardTimer(0),
	m_isShieldAnimating(false),
	m_isShieldRecovering(false),
	m_shieldAnimTimer(0.0f),
	m_shieldAnimDuration(1.0f),
	m_guardEffectHandle(-1),
	m_sparkEffectHandle(-1),
	m_sparkEffectTimer(0),
	m_shieldSwayOffset(VGet(0, 0, 0)),
	m_shieldSwayRotOffset(VGet(0, 0, 0)),
	m_wasSwitchingWeapon(false)
{
	// 盾モデルの読み込み
	m_shieldModelHandle = MV1LoadModel("data/model/Shield.mv1");
	assert(m_shieldModelHandle != -1);

	// 盾UI画像の読み込み
	m_shieldImageHandle = LoadGraph("data/image/ShieldUI.png");
	assert(m_shieldImageHandle != -1);
}

PlayerShieldSystem::~PlayerShieldSystem()
{
	MV1DeleteModel(m_shieldModelHandle);
	DeleteGraph(m_shieldImageHandle);
}

void PlayerShieldSystem::Init(float maxDurability, float regenRate)
{
	m_shieldDurability = maxDurability;
	m_shieldBarAnim = maxDurability;
	m_maxShieldDurability = maxDurability;
	m_shieldRegenRate = regenRate;
	m_isShieldBroken = false;
}

void PlayerShieldSystem::Update(float deltaTime, Camera* pCamera, const VECTOR& playerPos, bool isGuarding, bool isTackling, bool isSwitchingWeapon, float weaponSwitchTimer, float weaponSwitchDuration, float yawDelta)
{
	// パリィ判定のために、更新前に前フレームのガード状態を保存
	m_wasGuarding = m_isGuarding;
	m_isGuarding = isGuarding;

	// 盾のアニメーションタイマー更新
	if (m_isShieldAnimating)
	{
		m_shieldAnimTimer += deltaTime;
		if (m_shieldAnimTimer >= m_shieldAnimDuration)
		{
			m_isShieldAnimating = false;
			m_shieldAnimTimer = 0.0f;
		}
	}

	// Swayの計算
	m_shieldSwayOffset.x -= yawDelta * kShieldSwayAmount;
	m_shieldSwayOffset.x *= kShieldSwayDamping;

	// ガードアニメーションタイマーの更新
	if (m_isGuarding)
	{
		m_guardAnimTimer += deltaTime;
		if (m_guardAnimTimer > m_guardAnimDuration)
		{
			m_guardAnimTimer = m_guardAnimDuration;
		}
		m_guardTimer++;
	}
	else
	{
		m_guardAnimTimer -= deltaTime;
		if (m_guardAnimTimer < 0.0f)
		{
			m_guardAnimTimer = 0.0f;
		}
		m_guardTimer = 0;

		// 盾が壊れていない場合のみ回復
		if (!m_isShieldBroken)
		{
			m_shieldDurability += m_shieldRegenRate * deltaTime;
			if (m_shieldDurability > m_maxShieldDurability)
			{
				m_shieldDurability = m_maxShieldDurability;
			}
		}
		// 盾が壊れている場合は、回復しきったらアニメーションを開始
		else
		{
			m_shieldDurability += m_shieldRegenRate * deltaTime;
			if (m_shieldDurability >= m_maxShieldDurability)
			{
				m_shieldDurability = m_maxShieldDurability;
				m_isShieldBroken = false;

				// 回復アニメーション開始
				m_isShieldAnimating = true;
				m_isShieldRecovering = true;
				m_shieldAnimTimer = 0.0f;
			}
		}
	}

	// 盾バーアニメーション
	if (m_shieldBarAnim != m_shieldDurability)
	{
		if (m_shieldBarAnim > m_shieldDurability)
		{
			m_shieldBarAnim -= kHpBarAnimSpeed;
			if (m_shieldBarAnim < m_shieldDurability)
			{
				m_shieldBarAnim = m_shieldDurability;
			}
		}
		else
		{
			m_shieldBarAnim += kHpBarAnimSpeed;
			if (m_shieldBarAnim > m_shieldDurability)
			{
				m_shieldBarAnim = m_shieldDurability;
			}
		}
	}
}

void PlayerShieldSystem::Draw(Camera* pCamera, const VECTOR& playerPos, bool isTackling, bool isSwitchingWeapon, float weaponSwitchTimer, float weaponSwitchDuration)
{
	if (!pCamera) return;

	int screenW, screenH;
	GetScreenState(&screenW, &screenH, NULL);

	/*盾の描画*/

	// 画面サイズに応じてスケーリング
	float scaleW = screenW / kShieldBaseScreenW;
	float scaleH = screenH / kShieldBaseScreenH;
	float scaleAvg = (scaleW + scaleH) * 0.5f;

	// カメラオフセット設定
	VECTOR totalCameraOffset = VGet(0, 0, 0);
	if (pCamera)
	{
		VECTOR shakeOffset = pCamera->GetShakeOffset();
		VECTOR headBobOffset = pCamera->GetHeadBobOffset();
		VECTOR landingSwayOffset = pCamera->GetLandingSwayOffset();
		VECTOR jumpSwayOffset = pCamera->GetJumpSwayOffset();
		totalCameraOffset = VAdd(shakeOffset, headBobOffset);
		totalCameraOffset = VAdd(totalCameraOffset, landingSwayOffset);
		totalCameraOffset = VAdd(totalCameraOffset, jumpSwayOffset);
	}

	VECTOR shieldCamPos = VGet(0, 0, kShieldCamZ * scaleAvg);
	shieldCamPos.x += totalCameraOffset.x;
	shieldCamPos.y += totalCameraOffset.y;
	VECTOR shieldCamTarget = VGet(totalCameraOffset.x * kShieldCamTargetFactor, totalCameraOffset.y * kShieldCamTargetFactor, 0);
	SetCameraPositionAndTarget_UpVecY(shieldCamPos, shieldCamTarget);

	// ガードアニメーションの進行度を計算
	float guardAnimProgress = m_guardAnimTimer / m_guardAnimDuration;
	float easeProgress = 1.0f - cosf(guardAnimProgress * DX_PI_F * 0.5f); // イージング

	// 待機位置とガード位置を定義
	VECTOR waitPos = VAdd(VGet(kShieldWaitX * scaleW, kShieldWaitY * scaleH, kShieldWaitZ), m_shieldSwayOffset);
	VECTOR guardPos = VGet(0.0f, kShieldWaitY * scaleH, -15.0f); // 中央の位置

	// 進行度に応じて位置を補間
	VECTOR currentPos = VAdd(waitPos, VScale(VSub(guardPos, waitPos), easeProgress));

	// 待機回転とガード回転を定義
	constexpr float kShieldWaitAngleY = -0.3f; // 待機時のY軸回転角度
	VECTOR waitRot = VGet(0.0f, DX_PI_F + kShieldWaitAngleY, 0.0f);
	VECTOR guardRot = VGet(0.0f, DX_PI_F, 0.0f);

	// 進行度に応じて回転を補間
	VECTOR currentRot = VAdd(waitRot, VScale(VSub(guardRot, waitRot), easeProgress));

	// タックル中の盾アニメーション
	if (isTackling)
	{
		currentPos = guardPos; // ガード位置（中央）を基準にする
		currentPos.z += kTackleShieldThrust;
		currentRot = guardRot; // 回転もガード状態（正面）にする
	}

	// 武器切り替え中の盾アニメーション
	if (isSwitchingWeapon && weaponSwitchDuration > 0.0f)
	{
		float halfDuration = weaponSwitchDuration / 2.0f;
		
		// 前半：盾を下に隠す
		if (weaponSwitchTimer < halfDuration)
		{
			float progress = weaponSwitchTimer / halfDuration;
			float easeOut = 1.0f - powf(1.0f - progress, 3.0f);
			float yOffset = easeOut * 300.0f;
			currentPos.y -= yOffset;
		}
		// 後半：盾を元の位置に戻す
		else
		{
			float progress = (weaponSwitchTimer - halfDuration) / halfDuration;
			float easeOut = 1.0f - powf(1.0f - progress, 3.0f);
			float yOffset = (1.0f - easeOut) * 100.0f;
			currentPos.y -= yOffset;
		}
	}

	// ガード中は小刻みに揺らす
	if (m_isGuarding)
	{
		currentPos.x += ((float)rand() / RAND_MAX - 0.5f) * kGuardShakeAmount;
		currentPos.y += ((float)rand() / RAND_MAX - 0.5f) * kGuardShakeAmount;
	}

	// 盾のアニメーション処理
	if (m_isShieldAnimating)
	{
		float animProgress = m_shieldAnimTimer / m_shieldAnimDuration;

		// イージング適用
		float easedProgress;

		// 回復アニメーション
		if (m_isShieldRecovering)
		{
			easedProgress = 1.0f - powf(1.0f - animProgress, kShieldAnimEasingPower);
			// 下から元の位置へ
			float startY = kShieldAnimRecoverStartYOffset;
			float endY = currentPos.y;
			currentPos.y = startY + (endY - startY) * easedProgress;
		}
		// 破壊アニメーション
		else
		{
			easedProgress = powf(animProgress, kShieldAnimEasingPower);
			// 左斜め上を向いて下に消える
			float targetRotY = kShieldAnimBreakRotY; // 左斜め上
			float targetRotX = kShieldAnimBreakRotX;
			currentRot.y = currentRot.y + (targetRotY - currentRot.y) * easedProgress;
			currentRot.x = currentRot.x + (targetRotX - currentRot.x) * easedProgress;
			float endY = kShieldAnimBreakEndYOffset;
			currentPos.y = currentPos.y + (endY - currentPos.y) * easedProgress;
		}
	}

	// モデルの位置と回転を直接設定
	MV1SetPosition(m_shieldModelHandle, currentPos);
	MV1SetRotationXYZ(m_shieldModelHandle, currentRot);
	MV1SetScale(m_shieldModelHandle, VGet(kShieldModelScale * scaleAvg, kShieldModelScale * scaleAvg, kShieldModelScale * scaleAvg));

	// 盾が壊れていない、またはアニメーション中のみ描画
	if (!m_isShieldBroken || m_isShieldAnimating)
	{
		MV1DrawModel(m_shieldModelHandle);
	}

	// メインカメラに戻す
	pCamera->SetCameraToDxLib();
}

float PlayerShieldSystem::TakeDamage(float damage, Effect* pEffect, Camera* pCamera, const VECTOR& playerPos)
{
	if (m_isShieldBroken)
	{
		return damage; // 盾が壊れている場合は全ダメージを返す
	}

	m_shieldDurability -= damage;
	float remainingDamage = 0.0f;

	// スパークエフェクトを再生
	if (pEffect && pCamera)
	{
		VECTOR forward = VNorm(VSub(pCamera->GetTarget(), pCamera->GetPos()));
		VECTOR effectPos = VAdd(playerPos, VScale(forward, 80.0f));
		m_sparkEffectHandle = pEffect->PlaySparkEffect(effectPos.x, effectPos.y, effectPos.z);
		m_sparkEffectTimer = 30;
	}

	if (m_shieldDurability <= 0.0f)
	{
		remainingDamage = -m_shieldDurability;
		m_shieldDurability = 0.0f;
		m_isShieldBroken = true;

		// 盾破壊アニメーション開始
		m_isShieldAnimating = true;
		m_isShieldRecovering = false;
		m_shieldAnimTimer = 0.0f;
	}

	return remainingDamage;
}

bool PlayerShieldSystem::IsJustGuarded() const
{
	// パリィ判定：ガード開始からkParryFrameフレーム以内
	// 過去のコードを参考に、m_guardTimer > 0 && m_guardTimer <= kParryFrame の条件を使用
	return m_isGuarding && (m_guardTimer > 0 && m_guardTimer <= kParryFrame);
}

void PlayerShieldSystem::UpdateGuardEffect(Effect* pEffect, Camera* pCamera, const VECTOR& playerPos, bool isSwitchingWeapon)
{
	if (!pEffect || !pCamera) return;

	// 武器切り替えが終了したタイミングを検知（前フレームで切り替え中、今フレームで終了）
	bool weaponSwitchJustFinished = m_wasSwitchingWeapon && !isSwitchingWeapon;

	// 武器切り替え中はガードエフェクトを再生しない（アニメーション終了後に再生）
	if (isSwitchingWeapon)
	{
		m_wasSwitchingWeapon = true;
		return;
	}

	// 前フレームの武器切り替え状態を更新
	m_wasSwitchingWeapon = false;

	// ガード開始時にエフェクトを再生（通常のガード開始、または武器切り替え終了後にガード中の場合）
	if (m_isGuarding && (!m_wasGuarding || weaponSwitchJustFinished) && !m_isShieldBroken && !m_isShieldAnimating)
	{
		// 既にエフェクトが再生されている場合は停止
		if (m_guardEffectHandle != -1)
		{
			StopEffekseer3DEffect(m_guardEffectHandle);
			m_guardEffectHandle = -1;
		}

		float pitch = -pCamera->GetPitch();
		float yaw = pCamera->GetYaw();
		VECTOR forward = VNorm(VSub(pCamera->GetTarget(), pCamera->GetPos()));
		VECTOR right = VGet(sinf(yaw + DX_PI_F * 0.5f), 0, cosf(yaw + DX_PI_F * 0.5f));
		VECTOR effectPos = VAdd(playerPos, VAdd(VScale(forward, kGuardEffectOffsetZ), VScale(right, kGuardEffectOffsetX)));
		m_guardEffectHandle = pEffect->PlayGuardEffect(effectPos.x, effectPos.y, effectPos.z, pitch, yaw, 0.0f);
	}
	// ガード終了時（解除された場合）
	else if (!m_isGuarding && m_wasGuarding)
	{
		if (m_guardEffectHandle != -1)
		{
			StopEffekseer3DEffect(m_guardEffectHandle);
			m_guardEffectHandle = -1;
		}
	}
	// ガード中はエフェクトを追従させる
	else if (m_isGuarding && m_guardEffectHandle != -1 && !m_isShieldBroken)
	{
		float yaw = pCamera->GetYaw();
		VECTOR forward = VNorm(VSub(pCamera->GetTarget(), pCamera->GetPos()));
		VECTOR right = VGet(sinf(yaw + DX_PI_F * 0.5f), 0, cosf(yaw + DX_PI_F * 0.5f));
		VECTOR effectPos = VAdd(playerPos, VAdd(VScale(forward, kGuardEffectOffsetZ), VScale(right, kGuardEffectOffsetX)));
		SetPosPlayingEffekseer3DEffect(m_guardEffectHandle, effectPos.x, effectPos.y, effectPos.z);

		float pitch = -pCamera->GetPitch();
		SetRotationPlayingEffekseer3DEffect(m_guardEffectHandle, pitch, yaw, 0.0f);
	}
}

void PlayerShieldSystem::UpdateSparkEffect(Effect* pEffect, const VECTOR& playerPos, Camera* pCamera)
{
	if (!pEffect) return;

	if (m_sparkEffectTimer > 0)
	{
		m_sparkEffectTimer--;
		if (m_sparkEffectTimer <= 0 && m_sparkEffectHandle != -1)
		{
			StopEffekseer3DEffect(m_sparkEffectHandle);
			m_sparkEffectHandle = -1;
		}
	}
}

int PlayerShieldSystem::GetShieldImageHandle() const
{
	return m_shieldImageHandle;
}

