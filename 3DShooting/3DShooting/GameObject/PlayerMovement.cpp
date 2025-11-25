#include "PlayerMovement.h"
#include "Camera.h"
#include "CapsuleCollider.h"
#include "EffekseerForDXLib.h"
#include <cmath>
#include <algorithm>

namespace
{
	// 重力とジャンプ関連
	constexpr float kGravity   = 0.35f;
	constexpr float kJumpPower = 7.0f;
	constexpr float kGroundY   = 0.0f;

	// 飛行モード関連
	constexpr float kFlightAscendSpeed  = 8.0f;  // 上昇速度
	constexpr float kFlightDescendSpeed = 8.0f;  // 下降速度
	constexpr float kFlightAccelMultiplier = 2.0f; // 飛行モード中の加速倍率

	// カプセルコライダーのサイズ
	constexpr float kCapsuleHeight = 100.0f;
	constexpr float kCapsuleRadius = 50.0f;

	// X,Z座標の移動範囲制限
	constexpr float kLimitMoveX = 2800.0f;
	constexpr float kLimitMoveZ = 2800.0f;

	// Update関連
	constexpr float kPlayerColliderYOffset = 60.0f;
	constexpr float kGroundCheckTolerance  = 0.01f;
	constexpr float kJumpSwayPower         = 5.0f;
	constexpr float kLandingSwayPower      = 5.0f;
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
	m_jumpVelocity(0.0f),
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

void PlayerMovement::Update(float deltaTime, Camera* pCamera, bool isDead, bool isTackling, bool isFlightMode)
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

	// 地面にいるかどうかの判定
	bool isOnGround = (m_modelPos.y <= kGroundY + kGroundCheckTolerance);

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
			m_jumpVelocity = kJumpPower;
			m_isJumping = true;
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

			// 着地判定
			if (m_modelPos.y <= kGroundY)
			{
				m_modelPos.y = kGroundY;
				m_jumpVelocity = 0.0f;
				m_isJumping = false;

				// 着地した瞬間の処理
				if (m_wasJumping && pCamera)
				{
					pCamera->ApplyLandingSway(kLandingSwayPower);
				}
			}
		}

		// 移動方向がある場合（死亡中は移動不可）
		if (!isDead && (moveDir.x != 0.0f || moveDir.z != 0.0f))
		{
			// 移動方向の長さを計算
			float len = sqrtf(moveDir.x * moveDir.x + moveDir.z * moveDir.z);
			moveDir.x /= len;
			moveDir.z /= len;
			m_modelPos = VAdd(m_modelPos, VScale(moveDir, moveSpeed));
			isMoving = true;
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

	// X,Z座標の移動範囲制限
	if (m_modelPos.x < -kLimitMoveX) m_modelPos.x = -kLimitMoveX;
	if (m_modelPos.x > kLimitMoveX) m_modelPos.x = kLimitMoveX;
	if (m_modelPos.z < -kLimitMoveZ) m_modelPos.z = -kLimitMoveZ;
	if (m_modelPos.z > kLimitMoveZ) m_modelPos.z = kLimitMoveZ;
}

void PlayerMovement::Jump(Camera* pCamera)
{
	constexpr float kGroundY = 0.0f;
	constexpr float kGroundCheckTolerance = 0.01f;
	bool isOnGround = (m_modelPos.y <= kGroundY + kGroundCheckTolerance);

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

