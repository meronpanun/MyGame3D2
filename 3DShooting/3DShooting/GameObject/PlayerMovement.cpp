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
	constexpr float kGravity   = 0.3f;
	constexpr float kJumpPower = 7.0f;
	constexpr float kRunJumpPower = 12.5f;

	// 飛行モード関連
	constexpr float kFlightAscendSpeed  = 8.0f;  // 上昇速度
	constexpr float kFlightDescendSpeed = 8.0f;  // 下降速度
	constexpr float kFlightAccelMultiplier = 3.0f; // 飛行モード中の加速倍率

	// カプセルコライダーのサイズ
	constexpr float kCapsuleHeight = 100.0f;
	constexpr float kCapsuleRadius = 50.0f;

	// Update関連
	constexpr float kPlayerColliderYOffset = 60.0f; // コライダーのYオフセット
	constexpr float kGroundCheckTolerance  = 1.0f;  // 地面判定の許容誤差
	constexpr float kCoyoteTimeDuration    = 0.2f;  // コヨーテタイムの持続時間 (秒)
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
	constexpr float kAirBrakeFactor   = 0.01f; // 空中でのブレーキの強さ（Lerp係数）
	constexpr float kAirAccelFactor   = 0.05;  // 空中での加速の強さ（Lerp係数）
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
	m_isRunMode(false),
	m_isRunJumping(false),
	m_isJumpInertiaActive(false),
	m_jumpVelocity(0.0f),
	m_jumpStartYaw(0.0f),
	m_jumpSpeedScalar(0.0f),
	m_jumpMoveVelocity(VGet(0, 0, 0)),
	m_airSideControlVelocity(VGet(0, 0, 0)),
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
	UpdateCollider();

	if (m_coyoteTimeTimer > 0.0f) m_coyoteTimeTimer -= deltaTime;
	if (isTackling) return;

	// 接地判定（移動前）
	CollisionResult preCollisionResult = Collision::CheckStageCollision(m_modelPos, kCapsuleHeight, kCapsuleRadius, kPlayerColliderYOffset, collisionData);
	m_isGroundedOnStage = preCollisionResult.isGrounded;
	if (m_isGroundedOnStage && m_jumpVelocity < 0.0f)
	{
		m_jumpVelocity = 0.0f;
		m_isJumping = false;
	}

	if (isFlightMode && !isDead)
	{
		UpdateFlightMode(deltaTime, pCamera, isDead);
	}
	else
	{
		UpdateNormalMode(deltaTime, pCamera, isDead, isTackling, collisionData);
	}
}

void PlayerMovement::UpdateCollider()
{
	VECTOR center = m_modelPos;
	center.y += kPlayerColliderYOffset;
	VECTOR capA = VAdd(center, VGet(0, -kCapsuleHeight * 0.5f, 0));
	VECTOR capB = VAdd(center, VGet(0, kCapsuleHeight * 0.5f, 0));
	m_pBodyCollider->SetSegment(capA, capB);
	m_pBodyCollider->SetRadius(kCapsuleRadius);
}

void PlayerMovement::UpdateFlightMode(float deltaTime, Camera* pCamera, bool isDead)
{
	m_jumpVelocity = 0.0f;
	m_isJumping = false;

	unsigned char keyState[256];
	GetHitKeyStateAll(reinterpret_cast<char*>(keyState));

	if (keyState[KEY_INPUT_SPACE]) m_modelPos.y += kFlightAscendSpeed;
	if (keyState[KEY_INPUT_LSHIFT]) m_modelPos.y -= kFlightDescendSpeed;

	bool isAccelerating = (keyState[KEY_INPUT_LCONTROL] != 0);
	float speed = isAccelerating ? m_runSpeed * kFlightAccelMultiplier : m_moveSpeed;

	VECTOR moveDir = CalculateMoveDirection(pCamera, keyState);

	if (VSize(moveDir) > 0.0f)
	{
		m_modelPos = VAdd(m_modelPos, VScale(moveDir, speed));
		m_isMoving = true;
	}
	else
	{
		m_isMoving = false;
	}

	m_isWasRunning = isAccelerating;
	m_wasJumping = false;
	if (pCamera) pCamera->SetHeadBobbingState(m_isMoving, isAccelerating);
}

void PlayerMovement::UpdateNormalMode(float deltaTime, Camera* pCamera, bool isDead, bool isTackling, const std::vector<Stage::StageCollisionData>& collisionData)
{
	bool isOnGround = (m_modelPos.y <= kGroundY + kGroundCheckTolerance) || m_isGroundedOnStage;
	if (isOnGround) m_coyoteTimeTimer = kCoyoteTimeDuration;

	unsigned char keyState[256];
	GetHitKeyStateAll(reinterpret_cast<char*>(keyState));
	static unsigned char prevKeyState[256] = {};

	if (keyState[KEY_INPUT_LSHIFT] && keyState[KEY_INPUT_W]) m_isRunMode = true;
	if (!keyState[KEY_INPUT_W]) m_isRunMode = false;

	VECTOR moveDir = CalculateMoveDirection(pCamera, keyState);
	HandleJump(keyState, prevKeyState, isDead, isTackling, moveDir, pCamera);
	
	std::copy(std::begin(keyState), std::end(keyState), std::begin(prevKeyState));

	HandlePhysics(isOnGround, pCamera);

	if (!isDead)
	{
		if (m_isJumpInertiaActive)
		{
			m_modelPos = VAdd(m_modelPos, m_jumpMoveVelocity);
			if (VSize(moveDir) > 0.0f && pCamera)
			{
				float currentSpeed = (m_isRunMode || m_isRunJumping) ? m_runSpeed : m_moveSpeed;
				float inertiaSpeed = VSize(m_jumpMoveVelocity);
				
				if (inertiaSpeed > 0.1f)
				{
					float yaw = pCamera->GetYaw();
					VECTOR camFwd = VGet(sinf(yaw), 0.0f, cosf(yaw));
					VECTOR camRight = VGet(sinf(yaw + DX_PI_F * 0.5f), 0.0f, cosf(yaw + DX_PI_F * 0.5f));
					float dotFwd = VDot(moveDir, camFwd);
					float dotRight = VDot(moveDir, camRight);
					
					VECTOR targetSideVelocity = VScale(camRight, dotRight * currentSpeed * kAirControlFactor);
					m_airSideControlVelocity.x += (targetSideVelocity.x - m_airSideControlVelocity.x) * kAirAccelFactor;
					m_airSideControlVelocity.z += (targetSideVelocity.z - m_airSideControlVelocity.z) * kAirAccelFactor;
					m_modelPos = VAdd(m_modelPos, m_airSideControlVelocity);

					float diffYaw = yaw - m_jumpStartYaw;
					while (diffYaw <= -DX_PI_F) diffYaw += DX_TWO_PI_F;
					while (diffYaw > DX_PI_F) diffYaw -= DX_TWO_PI_F;
					
					if (fabsf(diffYaw) < DX_PI_F * 0.5f && dotFwd > 0.0f)
					{
						VECTOR steerForce = VScale(camFwd, dotFwd * currentSpeed * kAirControlFactor * 0.1f);
						m_jumpMoveVelocity = VAdd(m_jumpMoveVelocity, steerForce);
						if (VSize(m_jumpMoveVelocity) > 0.001f) m_jumpMoveVelocity = VScale(VNorm(m_jumpMoveVelocity), m_jumpSpeedScalar);
					}

					VECTOR inertiaDir = VNorm(m_jumpMoveVelocity);
					float fwdProjDot = VDot(VScale(camFwd, dotFwd * currentSpeed * kAirControlFactor), inertiaDir);
					if (fwdProjDot < 0.0f)
					{
						float currentInertiaSpeed = VSize(m_jumpMoveVelocity);
						float targetInertiaSpeed = (std::max)(0.0f, currentInertiaSpeed + fwdProjDot);
						float newInertiaSpeed = currentInertiaSpeed + (targetInertiaSpeed - currentInertiaSpeed) * kAirBrakeFactor;
						m_jumpMoveVelocity = VScale(inertiaDir, newInertiaSpeed);
						m_jumpSpeedScalar = newInertiaSpeed;
					}
				}
				else
				{
					m_modelPos = VAdd(m_modelPos, VScale(moveDir, currentSpeed * kAirControlFactor));
				}
			}
			m_isMoving = true;
		}
		else if (VSize(moveDir) > 0.0f)
		{
			m_modelPos = VAdd(m_modelPos, VScale(moveDir, m_isRunMode ? m_runSpeed : m_moveSpeed));
			m_isMoving = true;
		}
		else
		{
			m_isMoving = false;
		}
	}

	m_isWasRunning = m_isRunMode;
	m_wasJumping = m_isJumping;
	if (pCamera) pCamera->SetHeadBobbingState(m_isMoving && isOnGround, m_isRunMode);

	ResolveCollisions(collisionData, pCamera, m_isMoving);
}

VECTOR PlayerMovement::CalculateMoveDirection(Camera* pCamera, const unsigned char* keyState)
{
	if (!pCamera) return VGet(0, 0, 0);
	VECTOR moveDir = VGet(0, 0, 0);
	float yaw = pCamera->GetYaw();
	if (keyState[KEY_INPUT_W]) { moveDir.x += sinf(yaw); moveDir.z += cosf(yaw); }
	if (keyState[KEY_INPUT_S]) { moveDir.x -= sinf(yaw); moveDir.z -= cosf(yaw); }
	if (keyState[KEY_INPUT_A]) { moveDir.x += sinf(yaw - DX_PI_F * 0.5f); moveDir.z += cosf(yaw - DX_PI_F * 0.5f); }
	if (keyState[KEY_INPUT_D]) { moveDir.x += sinf(yaw + DX_PI_F * 0.5f); moveDir.z += cosf(yaw + DX_PI_F * 0.5f); }
	if (VSize(moveDir) > 0.0f) moveDir = VNorm(moveDir);
	return moveDir;
}

void PlayerMovement::HandleJump(const unsigned char* keyState, const unsigned char* prevKeyState, bool isDead, bool isTackling, const VECTOR& moveDir, Camera* pCamera)
{
	if (isDead || isTackling) return;
	if (keyState[KEY_INPUT_SPACE] && !prevKeyState[KEY_INPUT_SPACE] && m_coyoteTimeTimer > 0.0f && !m_isJumping)
	{
		m_jumpVelocity = m_isRunMode ? kRunJumpPower : kJumpPower;
		m_isJumping = true;
		m_isRunJumping = m_isRunMode;
		m_coyoteTimeTimer = 0.0f;

		if (VSize(moveDir) > 0.0f)
		{
			m_isJumpInertiaActive = true;
			m_jumpMoveVelocity = VScale(moveDir, m_isRunMode ? m_runSpeed : m_moveSpeed);
			if (pCamera) m_jumpStartYaw = pCamera->GetYaw();
			m_jumpSpeedScalar = VSize(m_jumpMoveVelocity);
		}
		if (pCamera) pCamera->ApplyJumpSway(kJumpSwayPower);
	}
}

void PlayerMovement::HandlePhysics(bool isOnGround, Camera* pCamera)
{
	if (m_isJumping || !isOnGround)
	{
		m_modelPos.y += m_jumpVelocity;
		float lastJumpVelocity = m_jumpVelocity;
		m_jumpVelocity -= kGravity;

		if (m_modelPos.y <= kGroundY)
		{
			m_modelPos.y = kGroundY;
			m_jumpVelocity = 0.0f;
			m_isJumping = false;

			if (m_wasJumping && pCamera)
			{
				float swayPower = (m_isRunJumping ? kRunLandingSwayPower : kLandingSwayPower) + fabsf(lastJumpVelocity) * kLandingVelocityFactor;
				pCamera->ApplyLandingSway(swayPower);
				if (m_isRunJumping) pCamera->Shake(kRunLandingShakeIntensity + fabsf(lastJumpVelocity) * kRunLandingShakeVelocityFactor, kRunLandingShakeDuration);
			}
			m_isRunJumping = false;
			m_isJumpInertiaActive = false;
			m_airSideControlVelocity = VGet(0,0,0);
		}
	}
}

void PlayerMovement::ResolveCollisions(const std::vector<Stage::StageCollisionData>& collisionData, Camera* pCamera, bool isMoving)
{
	VECTOR posBefore = m_modelPos;
	CollisionResult res = Collision::CheckStageCollision(m_modelPos, kCapsuleHeight, kCapsuleRadius, kPlayerColliderYOffset, collisionData);
	m_isGroundedOnStage = res.isGrounded;

	if (m_isRunMode && !m_isJumping && isMoving)
	{
		float pushBackSq = powf(m_modelPos.x - posBefore.x, 2) + powf(m_modelPos.z - posBefore.z, 2);
		bool isOnSlope = res.isGrounded && res.groundNormal.y > 0.6f;
		if (pushBackSq > 1.0f && !isOnSlope) m_isRunMode = false;
	}

	if (m_isGroundedOnStage && m_jumpVelocity < 0.0f)
	{
		if ((m_wasJumping || m_jumpVelocity < -5.0f) && pCamera)
		{
			float sway = (m_isRunJumping ? kRunLandingSwayPower : kLandingSwayPower) + fabsf(m_jumpVelocity) * kLandingVelocityFactor;
			pCamera->ApplyLandingSway(sway);
			if (m_isRunJumping)
			{
				pCamera->Shake(kRunLandingShakeIntensity + fabsf(m_jumpVelocity) * kRunLandingShakeVelocityFactor, kRunLandingShakeDuration);
				float diffYaw = pCamera->GetYaw() - m_jumpStartYaw;
				while (diffYaw <= -DX_PI_F) diffYaw += DX_TWO_PI_F;
				while (diffYaw > DX_PI_F) diffYaw -= DX_TWO_PI_F;
				if (fabsf(diffYaw) >= DX_PI_F * 0.5f) m_isRunMode = false;
			}
		}
		m_isRunJumping = false;
		m_isJumpInertiaActive = false;
		m_airSideControlVelocity = VGet(0,0,0);
		m_jumpVelocity = 0.0f;
		m_isJumping = false;
	}
}

void PlayerMovement::Jump(Camera* pCamera)
{
	bool canJump = (m_coyoteTimeTimer > 0.0f);

	if (!m_isJumping && canJump)
	{
		m_jumpVelocity = kJumpPower;
		m_isJumping = true;
		m_coyoteTimeTimer = 0.0f;
		if (pCamera)
		{
			pCamera->ApplyJumpSway(kJumpSwayPower);
		}
	}
}
