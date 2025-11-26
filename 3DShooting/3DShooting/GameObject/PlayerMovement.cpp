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

	// 飛行モード関連
	constexpr float kFlightAscendSpeed  = 8.0f;  // 上昇速度
	constexpr float kFlightDescendSpeed = 8.0f;  // 下降速度
	constexpr float kFlightAccelMultiplier = 2.0f; // 飛行モード中の加速倍率

	// カプセルコライダーのサイズ
	constexpr float kCapsuleHeight = 100.0f;
	constexpr float kCapsuleRadius = 50.0f;

	// Update関連
	constexpr float kPlayerColliderYOffset = 60.0f;
	constexpr float kGroundCheckTolerance  = 0.01f;
	constexpr float kJumpSwayPower         = 5.0f;
	constexpr float kLandingSwayPower      = 5.0f;

	// トライアングル上の最近接点を求める関数
	VECTOR GetClosestPointOnTriangle(const VECTOR& p, const VECTOR& a, const VECTOR& b, const VECTOR& c)
	{
		VECTOR ab = VSub(b, a);
		VECTOR ac = VSub(c, a);
		VECTOR ap = VSub(p, a);

		float d1 = VDot(ab, ap);
		float d2 = VDot(ac, ap);

		if (d1 <= 0.0f && d2 <= 0.0f) return a;

		VECTOR bp = VSub(p, b);
		float d3 = VDot(ab, bp);
		float d4 = VDot(ac, bp);

		if (d3 >= 0.0f && d4 <= d3) return b;

		float vc = d1 * d4 - d3 * d2;
		if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f)
		{
			float v = d1 / (d1 - d3);
			return VAdd(a, VScale(ab, v));
		}

		VECTOR cp = VSub(p, c);
		float d5 = VDot(ab, cp);
		float d6 = VDot(ac, cp);

		if (d6 >= 0.0f && d5 <= d6) return c;

		float vb = d5 * d2 - d1 * d6;
		if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f)
		{
			float w = d2 / (d2 - d6);
			return VAdd(a, VScale(ac, w));
		}

		float va = d3 * d6 - d5 * d4;
		if (va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f)
		{
			float w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
			return VAdd(b, VScale(VSub(c, b), w));
		}

		float denom = 1.0f / (va + vb + vc);
		float v = vb * denom;
		float w = vc * denom;
		return VAdd(a, VAdd(VScale(ab, v), VScale(ac, w)));
	}
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

	// ステージ接地フラグをリセット
	m_isGroundedOnStage = false;

	// 当たり判定（移動前に行うことで、前フレームのめり込みを解消し、接地判定を行う）
	// すり抜け防止のために複数回判定を行う
	CheckCollision(collisionData);

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

			// 着地判定（Y=0平面）
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

	// 移動後の位置で再度当たり判定を行い、めり込みを解消
	CheckCollision(collisionData);

	// 移動範囲制限
	m_modelPos.x = std::clamp(m_modelPos.x, -kLimitMoveX, kLimitMoveX);
	m_modelPos.z = std::clamp(m_modelPos.z, -kLimitMoveZ, kLimitMoveZ);
}

void PlayerMovement::Jump(Camera* pCamera)
{
	// constexpr float kGroundY = 0.0f; // PlayerMovement::kGroundYを使用
	constexpr float kGroundCheckTolerance = 0.01f;
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

void PlayerMovement::CheckCollision(const std::vector<Stage::StageCollisionData>& collisionData)
{
	// 反復回数（すり抜け防止のため複数回行う）
	const int kIterations = 4;
	// 接地判定の許容誤差（押し出し後も接地扱いにするため）
	const float kGroundTolerance = 0.5f;
	const float kGroundToleranceSq = (kCapsuleRadius + kGroundTolerance) * (kCapsuleRadius + kGroundTolerance);

	for (int i = 0; i < kIterations; ++i)
	{
		// プレイヤーの足元から少し上の位置を判定の基準にする
		// ループ内で位置が更新されるため、毎回計算し直す
		VECTOR checkPos = VAdd(m_modelPos, VGet(0.0f, kPlayerColliderYOffset, 0.0f));
		
		// カプセルの半径
		float radius = kCapsuleRadius;

		// カプセルの線分も毎回計算し直す
		VECTOR capA = VAdd(checkPos, VGet(0, -kCapsuleHeight * 0.5f, 0));
		VECTOR capB = VAdd(checkPos, VGet(0, kCapsuleHeight * 0.5f, 0));

		for (const auto& data : collisionData)
		{
			// 足元、中心、頭上の3点で判定を行う
			
			// 判定点リスト
			VECTOR points[] = { capA, checkPos, capB };
			
			for (const auto& p : points)
			{
				// トライアングル上の最近接点を求める
				VECTOR closest = GetClosestPointOnTriangle(p, data.v1, data.v2, data.v3);
				
				// 距離を計算
				VECTOR diff = VSub(p, closest);
				float distSq = VDot(diff, diff);
				
				// 接地判定（許容誤差込み）
				if (distSq < kGroundToleranceSq)
				{
					float dist = sqrtf(distSq);
					
					// 法線計算
					VECTOR normal;
					if (dist > 0.0001f)
					{
						normal = VScale(diff, 1.0f / dist);
					}
					else
					{
						VECTOR v12 = VSub(data.v2, data.v1);
						VECTOR v13 = VSub(data.v3, data.v1);
						normal = VNorm(VCross(v12, v13));
					}

					// 床判定（法線が上向き）
					if (normal.y > 0.7f)
					{
						m_isGroundedOnStage = true;
						if (m_jumpVelocity < 0.0f)
						{
							m_jumpVelocity = 0.0f;
							m_isJumping = false;
						}
					}

					// 実際の押し出し処理（半径内のみ）
					if (distSq < radius * radius)
					{
						float penetration = radius - dist;
						
						if (penetration > 0.001f)
						{
							// 押し出し
							m_modelPos = VAdd(m_modelPos, VScale(normal, penetration));
							
							// 座標更新
							checkPos = VAdd(m_modelPos, VGet(0.0f, kPlayerColliderYOffset, 0.0f));
							capA = VAdd(checkPos, VGet(0, -kCapsuleHeight * 0.5f, 0));
							capB = VAdd(checkPos, VGet(0, kCapsuleHeight * 0.5f, 0));
						}
					}
				}
			}
		}
	}
}
