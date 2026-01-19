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
	constexpr float kGroundCheckTolerance  = 1.0f;  // 地面判定の許容誤差 (0.01f から緩和)
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
	constexpr float kAirBrakeFactor   = 0.01f; // 空中でのブレーキの強さ
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

	// コヨーテタイムの更新
	if (m_coyoteTimeTimer > 0.0f)
	{
		m_coyoteTimeTimer -= deltaTime;
	}

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

	// コヨーテタイムの設定
	if (isOnGround)
	{
		m_coyoteTimeTimer = kCoyoteTimeDuration;
	}

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
		// 走るキー入力（シフトキーでダッシュモード切替、コントロールキーは飛行モード専用）
		// ShiftキーかつWキー（前方）入力時のみダッシュ開始
		if (CheckHitKey(KEY_INPUT_LSHIFT) && CheckHitKey(KEY_INPUT_W))
		{
			m_isRunMode = true;
		}

		// Wキーが離されていたらダッシュ解除（ASDのみではダッシュ不可）
		if (!CheckHitKey(KEY_INPUT_W))
		{
			m_isRunMode = false;
		}

		bool isRunning = m_isRunMode;

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

		// 移動方向の正規化をここで行う（斜め移動の速度修正）
		if (moveDir.x != 0.0f || moveDir.z != 0.0f)
		{
			float len = sqrtf(moveDir.x * moveDir.x + moveDir.z * moveDir.z);
			moveDir.x /= len;
			moveDir.z /= len;
		}

		// スペースキーを押した瞬間のみジャンプ（死亡中はジャンプ不可、飛行モード中は無効）
		// 接地判定にコヨーテタイムを使用
		bool canJump = (m_coyoteTimeTimer > 0.0f);
		if (!isDead && keyState[KEY_INPUT_SPACE] && !prevKeyState[KEY_INPUT_SPACE] && canJump && !m_isJumping && !isTackling)
		{
			m_jumpVelocity = isRunning ? kRunJumpPower : kJumpPower;
			m_isJumping = true;
			m_isRunJumping = isRunning;
			m_coyoteTimeTimer = 0.0f; // ジャンプしたらコヨーテタイム終了

			// 移動入力がある場合は慣性移動を有効化（歩き・ダッシュ共通）
			if (moveDir.x != 0.0f || moveDir.z != 0.0f)
			{
				m_isJumpInertiaActive = true;
				float currentSpeed = isRunning ? m_runSpeed : m_moveSpeed;
				m_jumpMoveVelocity = VScale(moveDir, currentSpeed);

				if (pCamera)
				{
					m_jumpStartYaw = pCamera->GetYaw();
				}
				m_jumpSpeedScalar = VSize(m_jumpMoveVelocity);
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
					// moveDirは既に正規化されている
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
						// 角度差分の計算（±90度以内でステアリング有効）
						float diffYaw = yaw - m_jumpStartYaw;
						while (diffYaw <= -DX_PI_F) diffYaw += DX_TWO_PI_F;
						while (diffYaw > DX_PI_F) diffYaw -= DX_TWO_PI_F;

						// 慣性方向の単位ベクトル
						VECTOR inertiaDir = VScale(m_jumpMoveVelocity, 1.0f / inertiaSpeed);

						if (fabsf(diffYaw) < DX_PI_F * 0.5f)
						{
							// ステアリング有効エリア
							// W入力（dotFwd > 0）の場合、慣性ベクトル自体を回転させる
							if (dotFwd > 0.0f)
							{
								// 旋回力を加算 (係数は調整)
								VECTOR steerForce = VScale(camFwd, dotFwd * currentSpeed * kAirControlFactor * 0.1f);
								m_jumpMoveVelocity = VAdd(m_jumpMoveVelocity, steerForce);

								// 速度（スカラー）を維持して方向のみ更新
								if (VSize(m_jumpMoveVelocity) > 0.001f)
								{
									m_jumpMoveVelocity = VScale(VNorm(m_jumpMoveVelocity), m_jumpSpeedScalar);
								}
							}
						}

						// 前方入力ベクトル
						VECTOR fwdForceRaw = VScale(camFwd, dotFwd * currentSpeed * kAirControlFactor);
						float fwdProjDot = VDot(fwdForceRaw, inertiaDir);
						VECTOR fwdForceProj = VScale(inertiaDir, fwdProjDot);

						// 減速（慣性方向と逆向きの入力成分がある）場合に適用
						if (fwdProjDot < 0.0f)
						{
							// m_jumpMoveVelocity の現在の速さを取得
							float currentInertiaSpeed = VSize(m_jumpMoveVelocity);
							
							// 目標速度（逆入力の強さに応じて減衰）
							// fwdProjDot は負の値なので、加算することで減速させる
							float targetInertiaSpeed = currentInertiaSpeed + fwdProjDot;
							if (targetInertiaSpeed < 0.0f) targetInertiaSpeed = 0.0f;

							// Lerpを用いて速度を更新
							float newInertiaSpeed = currentInertiaSpeed + (targetInertiaSpeed - currentInertiaSpeed) * kAirBrakeFactor;
							
							if (currentInertiaSpeed > 0.001f)
							{
								m_jumpMoveVelocity = VScale(inertiaDir, newInertiaSpeed);
							}
							else
							{
								m_jumpMoveVelocity = VGet(0,0,0);
							}
							
							// 速度スカラーの同期
							m_jumpSpeedScalar = newInertiaSpeed;
						}
					}
					else
					{
						// 慣性がない場合は自由移動
						finalControl = VScale(moveDir, currentSpeed * kAirControlFactor);
					}

					m_modelPos = VAdd(m_modelPos, finalControl);
				}
				isMoving = true;
			}
			else if (moveDir.x != 0.0f || moveDir.z != 0.0f)
			{
				// moveDirは既に正規化されている
				// 移動前の位置を保存
				VECTOR prevPos = m_modelPos;

				m_modelPos = VAdd(m_modelPos, VScale(moveDir, moveSpeed));

				// ステージ衝突判定（簡易）: 壁にぶつかったらダッシュ解除
				// 厳密な判定はCollision::CheckStageCollisionで行うが、押し戻し量をチェックすることで判定
				if (m_isRunMode)
				{
					// 仮の移動後の位置で衝突判定を予見的に行うのはコストが高いので、
					// Update末尾の衝突解決後と比較するのが正確だが、移動処理内での簡易チェックとして
					// ここでは何もしない（下部の衝突解決後に行う）
				}

				isMoving = true;
			}
			else
			{
				// 移動入力がない場合はダッシュ解除（オプション次第だが、今回は維持する仕様？）
				// 「押し込み続けなくても走り続ける」なので維持が正しい。
				// しかし、停止したら歩きに戻るのが一般的であれば解除。
				// リクエストは「押し込み続けなくても走り続ける」なので維持。ただし壁衝突等で解除。
			}
		}

		m_isMoving = isMoving;
		m_isWasRunning = isRunning;
		m_wasJumping = m_isJumping;

		// Head Bobbing状態をカメラに設定
		if (pCamera)
		{
			// 接地している場合のみ Head Bobbing を有効化
			bool enableHeadBobbing = isMoving && isOnGround;
			pCamera->SetHeadBobbingState(enableHeadBobbing, isRunning);
		}

		// 移動後の位置で再度当たり判定を行い、めり込みを解消
		VECTOR posBeforeCollision = m_modelPos;
		CollisionResult postCollisionResult = Collision::CheckStageCollision(m_modelPos, kCapsuleHeight, kCapsuleRadius, kPlayerColliderYOffset, collisionData);
		m_isGroundedOnStage = postCollisionResult.isGrounded;

		// 壁衝突判定: ダッシュ中に大きく押し戻されたらダッシュ解除
		// ただし、スロープを登っている場合の押し戻しは許容する
		if (m_isRunMode && !m_isJumping && isMoving)
		{
			// 水平方向の押し戻し量をチェック
			float pushBackDistSq = (m_modelPos.x - posBeforeCollision.x) * (m_modelPos.x - posBeforeCollision.x) + (m_modelPos.z - posBeforeCollision.z) * (m_modelPos.z - posBeforeCollision.z);

			// 接地法線が上向き（スロープ）でない、または接地していない場合にのみ解除判定を行う
			bool isOnSlope = postCollisionResult.isGrounded && postCollisionResult.groundNormal.y > 0.6f;
			if (pushBackDistSq > 1.0f && !isOnSlope) // 壁にぶつかったと判断
			{
				m_isRunMode = false;
			}
		}

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

					// 着地時の角度チェック（ダッシュ維持判定）
					if (m_isRunJumping)
					{
						float currentYaw = pCamera->GetYaw();
						float diffYaw = currentYaw - m_jumpStartYaw;
						while (diffYaw <= -DX_PI_F) diffYaw += DX_TWO_PI_F;
						while (diffYaw > DX_PI_F) diffYaw -= DX_TWO_PI_F;

						if (fabsf(diffYaw) >= DX_PI_F * 0.5f) // 差分が90度以上の場合
						{
							m_isRunMode = false; // ダッシュ解除
						}
						// 90度未満なら維持（何もしない）
					}
				}
			}
			m_isRunJumping = false;
			m_isJumpInertiaActive = false;
			m_jumpVelocity = 0.0f;
			m_isJumping = false;
		}
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
