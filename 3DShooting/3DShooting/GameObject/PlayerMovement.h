#pragma once
#include "EffekseerForDXLib.h"
#include "Stage.h"
#include <memory>
#include <vector>

class Camera;
class CapsuleCollider;

/// <summary>
/// プレイヤーの移動・物理管理クラス
/// </summary>
class PlayerMovement
{
public:
	PlayerMovement();
	~PlayerMovement() = default;

	void Init(const VECTOR& pos, float moveSpeed, float runSpeed, float scale);
	void Update(float deltaTime, Camera* pCamera, bool isDead, bool isTackling, bool isFlightMode, const std::vector<Stage::StageCollisionData>& collisionData);

	/// <summary>
	/// 位置取得
	/// </summary>
	/// <returns>位置ベクトル</returns>
	VECTOR GetPos() const { return m_modelPos; }

	/// <summary>
	/// 位置設定
	/// </summary>
	/// <param name="pos">位置ベクトル</param>
	void SetPos(const VECTOR& pos) { m_modelPos = pos; }

	/// <summary>
	/// 移動中かどうか
	/// </summary>
	/// <returns>移動中ならtrue</returns>
	bool IsMoving() const { return m_isMoving; }

	/// <summary>
	/// ジャンプ中かどうか
	/// </summary>
	/// <returns>ジャンプ中ならtrue</returns>
	bool IsJumping() const { return m_isJumping; }

	/// <summary>
	/// 前フレームでジャンプ中だったかどうか
	/// </summary>
	/// <returns>true: ジャンプ中だった, false: ジャンプ中でなかった</returns>
	bool WasJumping() const { return m_wasJumping; }

	/// <summary>
	/// 前フレームで走っていたかどうか
	/// </summary>
	/// <returns>true: 走っていた, false: 走っていなかった</returns>
	bool IsWasRunning() const { return m_isWasRunning; }

	/// <summary>
	/// コライダーを取得
	/// </summary>
	/// <returns>コライダーの共有ポインタ</returns>
	std::shared_ptr<CapsuleCollider> GetBodyCollider() const { return m_pBodyCollider; }

	/// <summary>
	/// ジャンプ処理
	/// </summary>
	/// <param name="pCamera">カメラのポインタ</param>
	void Jump(Camera* pCamera);

	/// <summary>
	/// 地面のY座標を取得
	/// </summary>
	/// <returns>地面のY座標</returns>
	static float GetGroundY() { return 0.0f; }

	/// <summary>
	/// 地面接地判定の許容値
	/// </summary>
	/// <returns>許容値</returns>
	static float GetGroundCheckTolerance() { return 1.0f; }

	VECTOR GetJumpMoveVelocity() const { return m_jumpMoveVelocity; }

	// ダッシュモード管理
	void CancelRunMode() { m_isRunMode = false; }
	bool IsRunMode() const { return m_isRunMode; }

	static constexpr float kGroundY = 0.0f;

private:
	VECTOR m_modelPos;
	VECTOR m_scale;

	float m_moveSpeed;
	float m_runSpeed;

	// フラグ
	bool m_isMoving;
	bool m_isJumping;
	bool m_wasJumping;
	bool m_isWasRunning;
	bool m_isGroundedOnStage;
	bool m_isRunMode;
	bool m_isRunJumping;
	bool m_isJumpInertiaActive;

	float m_jumpVelocity;
	float m_jumpStartYaw;
	float m_jumpSpeedScalar;
	VECTOR m_jumpMoveVelocity;

	float m_coyoteTimeTimer; // コヨーテタイム用のタイマー

	std::shared_ptr<CapsuleCollider> m_pBodyCollider;
};

