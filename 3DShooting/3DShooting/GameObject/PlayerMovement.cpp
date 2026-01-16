#include "PlayerMovement.h"
#include "Camera.h"
#include "CapsuleCollider.h"
#include "EffekseerForDXLib.h"
#include "Collision.h"
#include <cmath>
#include <algorithm>

namespace
{
	// 重力とジャンプ関連
	constexpr float kGravity   = 0.25f;
	constexpr float kJumpPower = 6.0f;
	constexpr float kRunJumpPower = 10.0f;

	// 飛行モード関連
	constexpr float kFlightAscendSpeed  = 8.0f;  // 上昇速度
	constexpr float kFlightDescendSpeed = 8.0f;  // 下降速度
	constexpr float kFlightAccelMultiplier = 3.0f; // 飛行モード中の加速倍率

	// カプセルコライダーのサイズ
	constexpr float kCapsuleHeight = 100.0f;
	constexpr float kCapsuleRadius = 50.0f;

	// Update関連
	constexpr float kPlayerColliderYOffset = 60.0f; // コライダーのYオフセット
	constexpr float kGroundCheckTolerance  = 0.01f; // 地面判定の許容誤差
	constexpr float kJumpSwayPower		   = 5.0f;  // ジャンプ時の揺れの強さ
	constexpr float kLandingSwayPower	   = 5.0f;  // 着地時の揺れの強さ
	constexpr float kRunLandingSwayPower   = 20.0f; // ダッシュ着地時の揺れの強さ
	constexpr float kLandingVelocityFactor = 0.5f;  // 着地時の衝撃（速度）による揺れの補正係数

	// ダッシュ着地時のシェイク（画面全体の振動）
	constexpr float kRunLandingShakeIntensity      = 1.0f; // ベースシェイク強度
	constexpr float kRunLandingShakeVelocityFactor = 0.2f; // 速度によるシェイク加算係数
	constexpr int   kRunLandingShakeDuration       = 5;   // シェイク持続時間（フレーム）

	// 空中制御
	constexpr float kAirControlFactor = 1.0f; // 空中での操作の効き具合
}

PlayerMovement::PlayerMovement() :
	m_modelPos(VGet(0, 0, 0)),
	m_scale(VGet(1, 1, 1)),
	m_moveSpeed(0.0f),
	m_runSpeed(0.0f),
	m_isMoving(false),
	m_isJumping(false),
	m_wasJumping(false),
	m_isWasRunning(false),
	m_isGroundedOnStage(false),
	m_isRunJumping(false),
	m_isJumpInertiaActive(false),
	m_jumpVelocity(0.0f),
	m_jumpMoveVelocity(VGet(0, 0, 0)),
	m_pBodyCollider(std::make_shared<CapsuleCollider>())
{
}

void PlayerMovement::Init(const VECTOR& pos, float moveSpeed, float runSpeed, float scale)
{
	m_modelPos = pos;
	m_moveSpeed = moveSpeed;
	m_runSpeed = runSpeed;
	m_scale = VGet(scale, scale, scale);
}

void PlayerMovement::Update(float deltaTime, Camera* pCamera, bool isDead, bool isTackling, bool isFlightMode, const std::vector<Stage::StageCollisionData>& collisionData)
{
	// プレイヤーのカプセルコライダーを毎フレーム更新（タックル中も必要）
	VECTOR center = m_modelPos;
	center.y += kPlayerColliderYOffset;
	VECTOR capA = VAdd(center, VGet(0, -kCapsuleHeight * 0.5f, 0));
	VECTOR capB = VAdd(center, VGet(0, kCapsuleHeight * 0.5f, 0));
	m_pBodyCollider->SetSegment(capA, capB);
	m_pBodyCollider->SetRadius(kCapsuleRadius);

	// タックル中は移動処理をスキップ
	if (isTackling)
	{
		return;
	}

	// 当たり判定（移動前に行うことで、前フレームのめり込みを解消し、接地判定を行う）
	CollisionResult preCollisionResult = Collision::CheckStageCollision(m_modelPos, kCapsuleHeight, kCapsuleRadius, kPlayerColliderYOffset, collisionData);
	m_isGroundedOnStage = preCollisionResult.isGrounded;
    if (m_isGroundedOnStage && m_jumpVelocity < 0.0f)
    {
        m_jumpVelocity = 0.0f;
        m_isJumping = false;
    }


	// 地面にいるかどうかの判定（Y=0平面 または ステージ上）
	bool isOnGround = (m_modelPos.y <= kGroundY + kGroundCheckTolerance) || m_isGroundedOnStage;

	// キー入力の取得
	unsigned char keyState[256];
	GetHitKeyStateAll(reinterpret_cast<char*>(keyState));
	static unsigned char prevKeyState[256] = {};

	// 飛行モードの処理
	if (isFlightMode && !isDead)
	{
		// 飛行モード中は重力を無効化
		m_jumpVelocity = 0.0f;
		m_isJumping = false;

		// スペースキーで上昇
		if (keyState[KEY_INPUT_SPACE])
		{
			m_modelPos.y += kFlightAscendSpeed;
		}

		// シフトキーで下降
		if (keyState[KEY_INPUT_LSHIFT])
		{
			m_modelPos.y -= kFlightDescendSpeed;
		}

		// コントロールキーで加速
		bool isAccelerating = CheckHitKey(KEY_INPUT_LCONTROL);
		float currentMoveSpeed = isAccelerating ? m_moveSpeed * kFlightAccelMultiplier : m_moveSpeed;
		float currentRunSpeed = isAccelerating ? m_runSpeed * kFlightAccelMultiplier : m_runSpeed;

		// 移動方向の初期化
		VECTOR moveDir = VGet(0, 0, 0);

		// キー入力による移動方向の設定
		if (CheckHitKey(KEY_INPUT_W))
		{
			moveDir.x += sinf(pCamera->GetYaw());
			moveDir.z += cosf(pCamera->GetYaw());
		}
		if (CheckHitKey(KEY_INPUT_S))
		{
			moveDir.x -= sinf(pCamera->GetYaw());
			moveDir.z -= cosf(pCamera->GetYaw());
		}
		if (CheckHitKey(KEY_INPUT_A))
		{
			moveDir.x += sinf(pCamera->GetYaw() - DX_PI_F * 0.5f);
			moveDir.z += cosf(pCamera->GetYaw() - DX_PI_F * 0.5f);
		}
		if (CheckHitKey(KEY_INPUT_D))
		{
			moveDir.x += sinf(pCamera->GetYaw() + DX_PI_F * 0.5f);
			moveDir.z += cosf(pCamera->GetYaw() + DX_PI_F * 0.5f);
		}

		// 移動方向がある場合
		if (moveDir.x != 0.0f || moveDir.z != 0.0f)
		{
			// 移動方向の長さを計算
			float len = sqrtf(moveDir.x * moveDir.x + moveDir.z * moveDir.z);
			moveDir.x /= len;
			moveDir.z /= len;
			float speed = isAccelerating ? currentRunSpeed : currentMoveSpeed;
			m_modelPos = VAdd(m_modelPos, VScale(moveDir, speed));
			m_isMoving = true;
		}
		else
		{
			m_isMoving = false;
		}

		m_isWasRunning = isAccelerating;
		m_wasJumping = false;

		// Head Bobbing状態をカメラに設定
		if (pCamera)
		{
			pCamera->SetHeadBobbingState(m_isMoving, isAccelerating);
		}
	}
	else
	{
		// 通常モードの処理
		// 走るキー入力（シフトキーのみでダッシュ、コントロールキーは飛行モード専用）
		const bool wantRun = CheckHitKey(KEY_INPUT_W) && CheckHitKey(KEY_INPUT_LSHIFT);
		bool isRunning = wantRun;

		float moveSpeed = isRunning ? m_runSpeed : m_moveSpeed;
		bool isMoving = false;

		// 移動方向の初期化
		VECTOR moveDir = VGet(0, 0, 0);

		// キー入力による移動方向の設定
		if (CheckHitKey(KEY_INPUT_W))
		{
			moveDir.x += sinf(pCamera->GetYaw());
			moveDir.z += cosf(pCamera->GetYaw());
		}
		if (CheckHitKey(KEY_INPUT_S))
		{
			moveDir.x -= sinf(pCamera->GetYaw());
			moveDir.z -= cosf(pCamera->GetYaw());
		}
		if (CheckHitKey(KEY_INPUT_A))
		{
			moveDir.x += sinf(pCamera->GetYaw() - DX_PI_F * 0.5f);
			moveDir.z += cosf(pCamera->GetYaw() - DX_PI_F * 0.5f);
		}
		if (CheckHitKey(KEY_INPUT_D))
		{
			moveDir.x += sinf(pCamera->GetYaw() + DX_PI_F * 0.5f);
			moveDir.z += cosf(pCamera->GetYaw() + DX_PI_F * 0.5f);
		}

		// スペースキーを押した瞬間のみジャンプ（死亡中はジャンプ不可、飛行モード中は無効）
		if (!isDead && keyState[KEY_INPUT_SPACE] && !prevKeyState[KEY_INPUT_SPACE] && isOnGround && !m_isJumping && !isTackling)
		{
			m_jumpVelocity = isRunning ? kRunJumpPower : kJumpPower;
			m_isJumping = true;
			m_isRunJumping = isRunning;
			
			// 移動入力がある場合は慣性移動を有効化（歩き・ダッシュ共通）
			if (moveDir.x != 0.0f || moveDir.z != 0.0f)
			{
				m_isJumpInertiaActive = true;
				float currentSpeed = isRunning ? m_runSpeed : m_moveSpeed;
				m_jumpMoveVelocity = VScale(moveDir, currentSpeed);
			}

			if (pCamera)
			{
				pCamera->ApplyJumpSway(kJumpSwayPower);
			}
		}

		std::copy(std::begin(keyState), std::end(keyState), std::begin(prevKeyState));

		// ジャンプ中または空中なら重力適用
		if (m_isJumping || !isOnGround)
		{
			m_modelPos.y += m_jumpVelocity;
			m_jumpVelocity -= kGravity;

			// 着地判定（Y=0平面）
			if (m_modelPos.y <= kGroundY)
			{
				m_modelPos.y = kGroundY;
				float lastJumpVelocity = m_jumpVelocity;
				m_jumpVelocity = 0.0f;
				m_isJumping = false;

				// 着地した瞬間の処理
				if (m_wasJumping && pCamera)
				{
					float swayPower = m_isRunJumping ? kRunLandingSwayPower : kLandingSwayPower;
					swayPower += fabsf(lastJumpVelocity) * kLandingVelocityFactor;
					pCamera->ApplyLandingSway(swayPower);

					// ダッシュジャンプ時は画面全体もシェイクさせる
					if (m_isRunJumping)
					{
						float shakeIntensity = kRunLandingShakeIntensity + (fabsf(lastJumpVelocity) * kRunLandingShakeVelocityFactor);
						pCamera->Shake(shakeIntensity, kRunLandingShakeDuration);
					}
				}
				m_isRunJumping = false;
				m_isJumpInertiaActive = false;
			}
		}

		// 移動方向がある場合（死亡中は移動不可）
		if (!isDead)
		{
			// ジャンプ慣性移動中はキー入力を無視して移動
			if (m_isJumpInertiaActive)
			{
				// 慣性移動
				m_modelPos = VAdd(m_modelPos, m_jumpMoveVelocity);

				// 空中操作（Air Control）
				if ((moveDir.x != 0.0f || moveDir.z != 0.0f) && pCamera)
				{
					// 正規化
					float len = sqrtf(moveDir.x * moveDir.x + moveDir.z * moveDir.z);
					if (len > 0.0f)
					{
						moveDir.x /= len;
						moveDir.z /= len;

						float currentSpeed = (isRunning || m_isRunJumping) ? m_runSpeed : m_moveSpeed;
						float inertiaSpeed = VSize(m_jumpMoveVelocity);

						VECTOR finalControl = VGet(0, 0, 0);

						// 慣性がある場合
						if (inertiaSpeed > 0.1f)
						{
							// カメラの前方・右ベクトルを取得
							float yaw = pCamera->GetYaw();
							VECTOR camFwd = VGet(sinf(yaw), 0.0f, cosf(yaw));
							VECTOR camRight = VGet(sinf(yaw + DX_PI_F * 0.5f), 0.0f, cosf(yaw + DX_PI_F * 0.5f));

							// 入力を成分分解
							// moveDirは既に正規化されている
							float dotFwd = VDot(moveDir, camFwd);
							float dotRight = VDot(moveDir, camRight);

							// 1. 横入力（A/D）はそのまま適用（Strafing）
							VECTOR sideForce = VScale(camRight, dotRight * currentSpeed * kAirControlFactor);
							finalControl = VAdd(finalControl, sideForce);

							// 2. 前方入力（W/S）の処理
							// 慣性方向の単位ベクトル
							VECTOR inertiaDir = VScale(m_jumpMoveVelocity, 1.0f / inertiaSpeed);

							// 前方入力ベクトル（ステアリング成分含む）
							VECTOR fwdForceRaw = VScale(camFwd, dotFwd * currentSpeed * kAirControlFactor);

							// 前方入力を慣性方向に射影（平行成分のみ抽出、垂直成分＝ステアリングを破棄）
							float fwdProjDot = VDot(fwdForceRaw, inertiaDir);
							VECTOR fwdForceProj = VScale(inertiaDir, fwdProjDot);

							// 加速（同方向）は無視、減速（逆方向）のみ適用
							if (fwdProjDot < 0.0f)
							{
								finalControl = VAdd(finalControl, fwdForceProj);
							}
						}
						else
						{
							// 慣性がない場合は自由移動
							finalControl = VScale(moveDir, currentSpeed * kAirControlFactor);
						}

						m_modelPos = VAdd(m_modelPos, finalControl);
					}
				}
				isMoving = true;
			}
			else if (moveDir.x != 0.0f || moveDir.z != 0.0f)
			{
				// 移動方向の長さを計算
				float len = sqrtf(moveDir.x * moveDir.x + moveDir.z * moveDir.z);
				moveDir.x /= len;
				moveDir.z /= len;
				m_modelPos = VAdd(m_modelPos, VScale(moveDir, moveSpeed));
				isMoving = true;
			}
		}

		m_isMoving = isMoving;
		m_isWasRunning = isRunning;
		m_wasJumping = m_isJumping;

		// Head Bobbing状態をカメラに設定
		if (pCamera)
		{
			pCamera->SetHeadBobbingState(isMoving, isRunning);
		}
	}

	// 移動後の位置で再度当たり判定を行い、めり込みを解消
	CollisionResult postCollisionResult = Collision::CheckStageCollision(m_modelPos, kCapsuleHeight, kCapsuleRadius, kPlayerColliderYOffset, collisionData);
    m_isGroundedOnStage = postCollisionResult.isGrounded;
    if (m_isGroundedOnStage && m_jumpVelocity < 0.0f)
    {
		if (m_wasJumping || m_jumpVelocity < -5.0f)
		{
			if (pCamera)
			{
				float swayPower = m_isRunJumping ? kRunLandingSwayPower : kLandingSwayPower;
				swayPower += fabsf(m_jumpVelocity) * kLandingVelocityFactor;
				pCamera->ApplyLandingSway(swayPower);

				// ダッシュジャンプ時は画面全体もシェイクさせる
				if (m_isRunJumping)
				{
					float shakeIntensity = kRunLandingShakeIntensity + (fabsf(m_jumpVelocity) * kRunLandingShakeVelocityFactor);
					pCamera->Shake(shakeIntensity, kRunLandingShakeDuration);
				}
			}
		}
		m_isRunJumping = false;
		m_isJumpInertiaActive = false;
        m_jumpVelocity = 0.0f;
        m_isJumping = false;
    }


}

void PlayerMovement::Jump(Camera* pCamera)
{
	bool isOnGround = (m_modelPos.y <= kGroundY + kGroundCheckTolerance) || m_isGroundedOnStage;

	if (!m_isJumping && isOnGround)
	{
		m_jumpVelocity = kJumpPower;
		m_isJumping = true;
		if (pCamera)
		{
			pCamera->ApplyJumpSway(kJumpSwayPower);
		}
	}
}
