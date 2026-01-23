#include "PlayerShieldSystem.h"
#include "Camera.h"
#include "Effect.h"
#include "Game.h"
#include "EffekseerForDXLib.h"
#include "EnemyBase.h"
#include "CapsuleCollider.h"
#include "PlayerMovement.h"
#include <cmath>
#include <cassert>

namespace
{
	// 盾関連
	constexpr float kShieldBaseScreenW	   = 640.0f;
	constexpr float kShieldBaseScreenH     = 480.0f;
	constexpr float kShieldCamZ			   = -35.0f;
	constexpr float kShieldCamTargetFactor = 0.3f;
	constexpr float kShieldWaitX		   = -20.0f;
	constexpr float kShieldWaitY		   = -45.0f;
	constexpr float kShieldWaitZ		   = -10.0f;
	constexpr float kShieldPivotZ		   = -25.0f;
	constexpr float kShieldModelScale	   = 2.0f;
	constexpr float kGuardAnimDuration	   = 0.1f;
	constexpr float kGuardEffectOffsetZ    = 40.0f;
	constexpr float kGuardEffectOffsetX    = 2.0f;
	constexpr float kGuardEffectOffsetY    = -90.0f;

	// 待機時の揺れ
	constexpr float kIdleSwaySpeed  = 1.5f; // 揺れの速さ
	constexpr float kIdleSwayAmount = 0.04f; // 揺れの量 

	// 盾アニメーション関連
	constexpr float kShieldAnimRecoverStartYOffset = -200.0f;
	constexpr float kShieldAnimBreakEndYOffset     = -200.0f;
	constexpr float kShieldAnimBreakRotY           = DX_PI_F * 1.25f;
	constexpr float kShieldAnimBreakRotX           = DX_PI_F * 0.25f;
	constexpr float kShieldAnimEasingPower         = 3.0f;

	// カメラを左右に振った際の横揺れ関連の定数
	constexpr float kShieldSwayAmount  = 4.0f;
	constexpr float kShieldSwayDamping = 0.9f;

	// Update関連
	constexpr float kDeltaTime = 1.0f / 60.0f;
	constexpr float kHpBarAnimSpeed = 1.5f;

	// パリィ受付フレーム数
	constexpr int kParryFrame = 60;

	// タックル関連
	constexpr float kTackleShieldThrust = 20.0f; // タックル時の盾の前方突き出し量
	constexpr float kGuardShakeAmount   = 0.4f;  // ガード時のカメラシェイク量

	// シールドソー関連
	constexpr float kShieldThrowSpeed         = 1000.0f; // シールドの移動速度
	constexpr float kShieldThrowMaxRange      = 1200.0f; // 最大投げ距離
	constexpr float kShieldThrowDamage		  = 50.0f;   // シールドソーのダメージ
	constexpr float kShieldThrowRadius        = 50.0f;   // シールドの当たり判定半径
	constexpr float kShieldThrowHeight		  = 100.0f;  // シールドの当たり判定の高さ
	constexpr float kShieldThrowRotationSpeed = 20.0f;   // シールドの回転速度
	constexpr float kShieldReflectAngleCos    = 0.3f;  // 反射を許可する入射角のコサイン閾値
	constexpr float kShieldThrowCooldown      = 1.0f;  // シールド投げのクールタイム（秒）

	// 盾投げ失敗アニメーション
	constexpr float kShieldThrowFailedAnimDuration = 0.2f;  // 失敗アニメーションの持続時間（秒）
	constexpr float kShieldThrowFailedAnimOffset   = 15.0f; // 失敗アニメーションの突き出し距離
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
	m_wasSwitchingWeapon(false),
	m_shieldThrowState(ShieldThrowState::Idle),
	m_isShieldThrown(false),
	m_shieldThrowPos(VGet(0, 0, 0)),
	m_shieldThrowDir(VGet(0, 0, 0)),
	m_shieldThrowStartPos(VGet(0, 0, 0)),
	m_shieldThrowDistance(0.0f),
	m_shieldThrowSpeed(kShieldThrowSpeed),
	m_shieldThrowMaxRange(kShieldThrowMaxRange),
	m_shieldThrowDamage(kShieldThrowDamage),
	m_shieldThrowHitEnemyId(-1),
	m_shieldReflectCount(0),
	m_shieldThrowRotationTimer(0.0f),
	m_shieldThrowCooldownTimer(0.0f),
	m_isShieldThrowFailedAnimating(false),
	m_shieldThrowFailedAnimTimer(0.0f),
	m_isTutorial(false),
	m_idleSwayTimer(0.0f)
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

void PlayerShieldSystem::Update(float deltaTime, Camera* pCamera, const VECTOR& playerPos, bool isGuarding, bool isTackling, bool isSwitchingWeapon, float weaponSwitchTimer, float weaponSwitchDuration, float yawDelta, bool isMoving)
{
	// パリィ判定のために、更新前に前フレームのガード状態を保存
	m_wasGuarding = m_isGuarding;
	m_isGuarding = isGuarding;

	// シールド投げのクールタイムタイマー減算
	if (m_shieldThrowCooldownTimer > 0.0f)
	{
		m_shieldThrowCooldownTimer -= deltaTime;
		if (m_shieldThrowCooldownTimer < 0.0f)
		{
			m_shieldThrowCooldownTimer = 0.0f;
		}
	}

	// 盾投げ失敗アニメーションの更新
	if (m_isShieldThrowFailedAnimating)
	{
		m_shieldThrowFailedAnimTimer += deltaTime;
		if (m_shieldThrowFailedAnimTimer >= kShieldThrowFailedAnimDuration)
		{
			m_isShieldThrowFailedAnimating = false;
			m_shieldThrowFailedAnimTimer = 0.0f;
		}
	}

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

	if (!isMoving)
	{
		m_idleSwayTimer += deltaTime;
		VECTOR idleSway = VGet(
			sinf(m_idleSwayTimer * kIdleSwaySpeed * 2.0f) * kIdleSwayAmount,
			cosf(m_idleSwayTimer * kIdleSwaySpeed) * kIdleSwayAmount,
			0.0f
		);
		m_shieldSwayOffset = VAdd(m_shieldSwayOffset, idleSway);
	}

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

	// シールドソーを投げている場合は通常のシールドを非表示にする
	if (m_isShieldThrown)
	{
		return;
	}

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

	// 盾投げ失敗アニメーション処理
	if (m_isShieldThrowFailedAnimating)
	{
		float animProgress = m_shieldThrowFailedAnimTimer / kShieldThrowFailedAnimDuration;
		// ease-out イージング（減速しながら戻る）
		float easedProgress = 1.0f - powf(1.0f - animProgress, 3.0f);
		// 前に突き出して戻る（0 → max → 0）
		float thrustOffset = sinf(easedProgress * DX_PI_F) * kShieldThrowFailedAnimOffset;
		currentPos.z += thrustOffset;
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

		// カメラ基準の座標系を作成
		MATRIX rotYaw = MGetRotY(yaw);
		MATRIX rotPitch = MGetRotX(pitch);
		MATRIX cameraRot = MMult(rotPitch, rotYaw);

		VECTOR forward = VTransform(VGet(0, 0, 1), cameraRot);
		VECTOR right = VTransform(VGet(1, 0, 0), cameraRot);
		VECTOR up = VTransform(VGet(0, 1, 0), cameraRot);

		// カメラ位置基準でエフェクト位置を計算
		VECTOR effectPos = VAdd(pCamera->GetPos(), VScale(forward, kGuardEffectOffsetZ));
		effectPos = VAdd(effectPos, VScale(right, kGuardEffectOffsetX));
		effectPos = VAdd(effectPos, VScale(up, kGuardEffectOffsetY));

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
		float pitch = -pCamera->GetPitch();

		// カメラ基準の座標系を作成
		MATRIX rotYaw = MGetRotY(yaw);
		MATRIX rotPitch = MGetRotX(pitch);
		MATRIX cameraRot = MMult(rotPitch, rotYaw);

		VECTOR forward = VTransform(VGet(0, 0, 1), cameraRot);
		VECTOR right = VTransform(VGet(1, 0, 0), cameraRot);
		VECTOR up = VTransform(VGet(0, 1, 0), cameraRot);

		// カメラ位置基準でエフェクト位置を計算
		VECTOR effectPos = VAdd(pCamera->GetPos(), VScale(forward, kGuardEffectOffsetZ));
		effectPos = VAdd(effectPos, VScale(right, kGuardEffectOffsetX));
		effectPos = VAdd(effectPos, VScale(up, kGuardEffectOffsetY));

		SetPosPlayingEffekseer3DEffect(m_guardEffectHandle, effectPos.x, effectPos.y, effectPos.z);
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

bool PlayerShieldSystem::ThrowShield(Camera* pCamera, const VECTOR& playerPos)
{
	// クールタイム中は投げられない
	if (m_shieldThrowCooldownTimer > 0.0f)
	{
		// 失敗アニメーションを開始
		m_isShieldThrowFailedAnimating = true;
		m_shieldThrowFailedAnimTimer = 0.0f;
		return false;
	}

	// 既に投げられている場合は投げられない
	if (m_isShieldThrown)
	{
		return false;
	}

	// ガード中やタックル中は投げられない
	if (m_isGuarding || m_isShieldBroken)
	{
		return false;
	}

	if (!pCamera) return false;

	// カメラの前方方向を取得（レティクルの方角）
	// カメラの位置から注視点への方向がレティクル方向
	VECTOR camPos = pCamera->GetPos();
	VECTOR camTarget = pCamera->GetTarget();
	VECTOR cameraForward = VSub(camTarget, camPos);
	m_shieldThrowDir = VNorm(cameraForward); // カメラの前方方向ベクトル（レティクル方向）

	// シールドの初期位置を設定
	constexpr float kShieldThrowStartYOffset = 80.0f; 
	VECTOR throwStartPos = playerPos;
	throwStartPos.y += kShieldThrowStartYOffset; 
	m_shieldThrowStartPos = throwStartPos;
	m_shieldThrowPos = m_shieldThrowStartPos;
	m_shieldThrowDistance = 0.0f;
	m_shieldThrowState = ShieldThrowState::Throwing;
	m_isShieldThrown = true;
	m_shieldThrowHitEnemyId = -1;
	m_shieldReflectCount = 0; // 反射回数をリセット
	m_shieldThrowRotationTimer = 0.0f; // 回転タイマーをリセット

	return true;
}

void PlayerShieldSystem::UpdateShieldThrow(float deltaTime, Camera* pCamera, const VECTOR& playerPos, const std::vector<EnemyBase*>& enemyList, const std::vector<Stage::StageCollisionData>& collisionData, Effect* pEffect, bool isGuarding, bool wasGuarding)
{
	if (!m_isShieldThrown) return;

	// ガード開始時に盾を即座にプレイヤーの位置に戻す
	if (isGuarding && !wasGuarding && m_isShieldThrown)
	{
		ImmediateReturnShield(playerPos);
		return;
	}

	// シールドの回転タイマーを更新
	m_shieldThrowRotationTimer += deltaTime * kShieldThrowRotationSpeed;

	if (m_shieldThrowState == ShieldThrowState::Throwing)
	{
		// カメラの前方方向（レティクルの方角）に向かって移動
		VECTOR moveDelta = VScale(m_shieldThrowDir, m_shieldThrowSpeed * deltaTime);
		m_shieldThrowPos = VAdd(m_shieldThrowPos, moveDelta);
		m_shieldThrowDistance += VSize(moveDelta);

		// 最大距離に達したら戻りモードに切り替え
		if (m_shieldThrowDistance >= m_shieldThrowMaxRange)
		{
			m_shieldThrowState = ShieldThrowState::Returning;
			m_shieldThrowHitEnemyId = -1; // 戻り時に再度ヒットできるようにリセット
		}
	}
	else if (m_shieldThrowState == ShieldThrowState::Returning)
	{
		// プレイヤーの位置にYオフセットを加えた位置（投げ始めた位置と同じ高さ）に向かって移動
		constexpr float kShieldThrowStartYOffset = 80.0f;
		VECTOR returnTargetPos = playerPos;
		returnTargetPos.y += kShieldThrowStartYOffset; // 投げ始めた位置と同じ高さにする
		
		VECTOR toReturnTarget = VSub(returnTargetPos, m_shieldThrowPos);
		float distToReturnTarget = VSize(toReturnTarget);
		
		if (distToReturnTarget < 50.0f)
		{
			// 戻り位置に到達したら待機モードに戻る
			m_shieldThrowState = ShieldThrowState::Idle;
			m_isShieldThrown = false;
			m_shieldThrowPos = VGet(0, 0, 0);
			m_shieldThrowDistance = 0.0f;
			m_shieldThrowHitEnemyId = -1;
			m_shieldThrowRotationTimer = 0.0f;
			// クールタイムを開始
			m_shieldThrowCooldownTimer = kShieldThrowCooldown;
			return;
		}

		VECTOR returnDir = VNorm(toReturnTarget);
		VECTOR moveDelta = VScale(returnDir, m_shieldThrowSpeed * deltaTime);
		m_shieldThrowPos = VAdd(m_shieldThrowPos, moveDelta);
	}

	// ステージとの当たり判定
	if (m_shieldThrowState == ShieldThrowState::Throwing)
	{
		// 移動範囲制限と地面との当たり判定
		if (m_shieldThrowPos.y <= PlayerMovement::kGroundY)
		{
			m_shieldThrowState = ShieldThrowState::Returning;
			m_shieldThrowHitEnemyId = -1;
		}

		for (const auto& col : collisionData)
		{
			if (HitCheck_Sphere_Triangle(m_shieldThrowPos, kShieldThrowRadius, col.v1, col.v2, col.v3))
			{
				// 法線の計算
				VECTOR edge1 = VSub(col.v2, col.v1);
				VECTOR edge2 = VSub(col.v3, col.v1);
				VECTOR normal = VNorm(VCross(edge1, edge2));

				// 入射ベクトルと法線の内積
				float dot = VDot(m_shieldThrowDir, normal);
				
				// 反射回数が残っており、かつ入射角度が浅い（法線に対して30度以上）場合のみ反射
				if (m_shieldReflectCount < 1 && fabsf(dot) < kShieldReflectAngleCos)
				{
					// 反射ベクトルの計算: R = I - 2(I・N)N
					VECTOR reflectDir = VSub(m_shieldThrowDir, VScale(normal, 2.0f * dot));
					m_shieldThrowDir = VNorm(reflectDir);
					m_shieldReflectCount++;
					
					// 反射エフェクトの再生
					if (pEffect)
					{
						pEffect->PlaySparkEffect2(m_shieldThrowPos.x, m_shieldThrowPos.y, m_shieldThrowPos.z);
					}

					// めり込み防止（少しだけ法線方向にずらす）
					m_shieldThrowPos = VAdd(m_shieldThrowPos, VScale(normal, 2.0f));
				}
				else
				{
					// 反射しない、または既に反射済みの場合は戻りモードに切り替え
					m_shieldThrowState = ShieldThrowState::Returning;
					m_shieldThrowHitEnemyId = -1; // 戻り時に再度ヒットできるようにリセット
				}

				break; // 1つでも当たればOK
			}
		}
	}

	// 敵との当たり判定
	if (m_shieldThrowState == ShieldThrowState::Throwing || m_shieldThrowState == ShieldThrowState::Returning)
	{
		// シールドの当たり判定（カプセル形状）
		VECTOR shieldCapA = VAdd(m_shieldThrowPos, VGet(0, -kShieldThrowHeight * 0.5f, 0));
		VECTOR shieldCapB = VAdd(m_shieldThrowPos, VGet(0, kShieldThrowHeight * 0.5f, 0));

		for (EnemyBase* enemy : enemyList)
		{
			if (!enemy || !enemy->IsAlive()) continue;

			// 敵のコライダーを取得
			auto enemyCollider = enemy->GetBodyCollider();
			if (!enemyCollider) continue;

			// カプセル同士の当たり判定
			VECTOR enemyCapA = enemyCollider->GetSegmentA();
			VECTOR enemyCapB = enemyCollider->GetSegmentB();
			float enemyRadius = enemyCollider->GetRadius();

			// 簡易的な当たり判定（カプセル間の最短距離を計算）
			VECTOR shieldCenter = VAdd(shieldCapA, VScale(VSub(shieldCapB, shieldCapA), 0.5f));
			VECTOR enemyCenter = VAdd(enemyCapA, VScale(VSub(enemyCapB, enemyCapA), 0.5f));
			VECTOR toEnemy = VSub(enemyCenter, shieldCenter);
			float dist = VSize(toEnemy);
			float minDist = kShieldThrowRadius + enemyRadius + kShieldThrowHeight * 0.5f;

			if (dist < minDist)
			{
				// 同じ敵に連続でヒットしないようにする
				// 簡易的なIDとして敵のポインタアドレスを使用
				int enemyId = reinterpret_cast<intptr_t>(enemy);
				if (m_shieldThrowHitEnemyId != enemyId)
				{
					m_shieldThrowHitEnemyId = enemyId;
					
					// ダメージを与える
					enemy->TakeDamage(m_shieldThrowDamage, AttackType::Shoot);

					// エフェクトを再生
					if (pEffect)
					{
						pEffect->PlaySparkEffect(m_shieldThrowPos.x, m_shieldThrowPos.y, m_shieldThrowPos.z);
					}

					// 投げ中に敵に当たった場合は戻りモードに切り替え
					if (m_shieldThrowState == ShieldThrowState::Throwing)
					{
						m_shieldThrowState = ShieldThrowState::Returning;
					}
				}
			}
		}
	}
}

void PlayerShieldSystem::DrawShieldThrow(Camera* pCamera, const VECTOR& playerPos) const
{
	if (!m_isShieldThrown || !pCamera) return;

	// メインカメラを設定
	pCamera->SetCameraToDxLib();

	// シールドの位置と回転を計算
	VECTOR shieldPos = m_shieldThrowPos;
	
	// シールドの向きを計算（移動方向に合わせる）
	VECTOR forward = m_shieldThrowDir;
	if (m_shieldThrowState == ShieldThrowState::Returning)
	{
		// 戻り中はプレイヤー方向を向く（水平方向のみ）
		constexpr float kShieldThrowStartYOffset = 80.0f;
		VECTOR returnTargetPos = playerPos;
		returnTargetPos.y += kShieldThrowStartYOffset;
		VECTOR toPlayer = VSub(returnTargetPos, m_shieldThrowPos);
		forward = VNorm(toPlayer);
	}

	// 回転を計算
	// forward方向を向く回転行列を作成
	float forwardYaw = atan2f(forward.x, forward.z);
	float forwardPitch = -asinf(forward.y);
	
	// forward方向を向く回転行列（Yaw → Pitch）
	MATRIX rotYaw = MGetRotY(forwardYaw);
	MATRIX rotPitch = MGetRotX(forwardPitch);
	MATRIX rotToForward = MMult(rotPitch, rotYaw);
	
	// 横向きにする回転（X軸90度回転）
	MATRIX rotX = MGetRotX(DX_PI_F * 0.5f);
	
	// forward方向を軸として回転させる（横向きの状態を維持）
	// forward方向を軸として回転させるには、forward方向をローカルZ軸として扱い、
	// そのZ軸周りに回転させる
	// 回転の順序：forward方向を向く → 横向きにする → forward方向を軸として回転
	MATRIX rotAroundForward = MGetRotZ(m_shieldThrowRotationTimer); // Z軸周りの回転（横向きの状態）
	
	// 合成：forward方向を向く回転 → 横向きにする回転 → forward方向を軸とした回転
	// 順序：rotToForward → rotX → rotAroundForward
	MATRIX tempMatrix = MMult(rotX, rotToForward);
	MATRIX rotMatrix = MMult(rotAroundForward, tempMatrix);

	// シールドモデルを描画
	MV1SetPosition(m_shieldModelHandle, shieldPos);
	MV1SetRotationMatrix(m_shieldModelHandle, rotMatrix);
	MV1SetScale(m_shieldModelHandle, VGet(kShieldModelScale, kShieldModelScale, kShieldModelScale));
	MV1DrawModel(m_shieldModelHandle);
}

void PlayerShieldSystem::ImmediateReturnShield(const VECTOR& playerPos)
{
	if (!m_isShieldThrown) return;

	// 盾を即座にプレイヤーの位置（Yオフセット付き）に戻す
	constexpr float kShieldThrowStartYOffset = 80.0f;
	VECTOR returnTargetPos = playerPos;
	returnTargetPos.y += kShieldThrowStartYOffset;
	m_shieldThrowPos = returnTargetPos;

	// 待機モードに戻す
	m_shieldThrowState = ShieldThrowState::Idle;
	m_isShieldThrown = false;
	m_shieldThrowDistance = 0.0f;
	m_shieldThrowHitEnemyId = -1;
	m_shieldThrowRotationTimer = 0.0f;
	// クールタイムを開始
	m_shieldThrowCooldownTimer = kShieldThrowCooldown;
}
