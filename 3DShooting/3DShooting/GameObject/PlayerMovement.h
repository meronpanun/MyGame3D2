#pragma once
#include "EffekseerWarningSuppress.h"
#include "Stage.h"
#include <memory>
#include <vector>
#include <string>

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

    /// <summary>
    /// 初期化処理
    /// </summary>
    /// <param name="pos">初期位置</param>
    /// <param name="moveSpeed">歩き速度</param>
    /// <param name="runSpeed">走り速度</param>
    /// <param name="scale">スケール</param>
    void Init(const VECTOR& pos, float moveSpeed, float runSpeed, float scale);

    /// <summary>
    /// 毎フレームの更新処理
    /// </summary>
    /// <param name="deltaTime">経過時間</param>
    /// <param name="pCamera">カメラのポインタ</param>
    /// <param name="isDead">死亡中かどうか</param>
    /// <param name="isTackling">タックル中かどうか</param>
    /// <param name="isFlightMode">飛行モードかどうか</param>
    /// <param name="collisionData">ステージコリジョンデータ</param>
    /// <param name="isInputDisabled">入力無効かどうか</param>
    void Update(float deltaTime, Camera* pCamera, bool isDead, bool isTackling,
        bool isFlightMode,
        const std::vector<Stage::StageCollisionData>& collisionData,
        bool isInputDisabled = false);

    /// <summary>
    /// 位置を取得する
    /// </summary>
    /// <returns>位置ベクトル</returns>
    VECTOR GetPos() const { return m_modelPos; }

    /// <summary>
    /// 位置を設定する
    /// </summary>
    /// <param name="pos">位置ベクトル</param>
    void SetPos(const VECTOR& pos) { m_modelPos = pos; }

    /// <summary>
    /// 移動中かどうかを返す
    /// </summary>
    /// <returns>移動中ならtrue</returns>
    bool IsMoving() const { return m_isMoving; }

    /// <summary>
    /// 現在の移動速度を取得する
    /// </summary>
    /// <returns>移動速度</returns>
    float GetCurrentSpeed() const { return m_currentSpeed; }

    /// <summary>
    /// ジャンプ中かどうかを返す
    /// </summary>
    /// <returns>ジャンプ中ならtrue</returns>
    bool IsJumping() const { return m_isJumping; }

    /// <summary>
    /// 前フレームでジャンプ状態だったかどうかを返す
    /// </summary>
    /// <returns>ジャンプ状態だったならtrue</returns>
    bool WasJumping() const { return m_wasJumping; }

    /// <summary>
    /// 前フレームで走っていたかどうかを返す
    /// </summary>
    /// <returns>走っていたならtrue</returns>
    bool WasRunning() const { return m_wasRunning; }

    /// <summary>
    /// 地面に接地しているかどうかを返す
    /// </summary>
    /// <returns>接地しているならtrue</returns>
    bool IsOnGround() const;

    /// <summary>
    /// 着地した瞬間かどうかを返す
    /// </summary>
    bool JustLanded() const { return m_justLanded; }

    /// <summary>
    /// 着地時の衝撃速度を取得する
    /// </summary>
    float GetImpactVelocity() const { return m_impactVelocity; }

    /// <summary>
    /// ボディカプセルコライダーを取得する
    /// </summary>
    /// <returns>コライダーの共有ポインタ</returns>
    std::shared_ptr<CapsuleCollider> GetBodyCollider() const { return m_pBodyCollider; }

    /// <summary>
    /// 接地しているオブジェクト名を取得する
    /// </summary>
    /// <returns>接地しているオブジェクト名</returns>
    std::string GetGroundedObjectName() const { return m_groundedObjectName; }

    /// <summary>
    /// ジャンプする
    /// </summary>
    /// <param name="pCamera">カメラのポインタ</param>
    void Jump(Camera* pCamera);

    /// <summary>
    /// 地面の Y 座標を取得する
    /// </summary>
    /// <returns>地面の Y 座標</returns>
    static float GetGroundY() { return 0.0f; }

    /// <summary>
    /// 地面接地判定の許容誤差を取得する
    /// </summary>
    /// <returns>許容誤差</returns>
    static float GetGroundCheckTolerance() { return 1.0f; }

    /// <summary>
    /// ジャンプ時の慣性速度ベクトルを取得する
    /// </summary>
    /// <returns>ジャンプ慣性速度ベクトル</returns>
    VECTOR GetJumpMoveVelocity() const { return m_jumpMoveVelocity; }

    /// <summary>
    /// ダッシュモードを解除する
    /// </summary>
    void CancelRunMode() { m_isRunMode = false; }

    /// <summary>
    /// ダッシュモード中かどうかを返す
    /// </summary>
    bool IsRunMode() const { return m_isRunMode; }

    /// <summary>
    /// ノックバック力を適用する
    /// </summary>
    /// <param name="velocity">ノックバック速度ベクトル</param>
    void ApplyKnockback(const VECTOR& velocity);

    /// <summary>
    /// 鉛直速度を強制リセットする（ボスシーンへの移動時など）
    /// </summary>
    void ResetVerticalVelocity() { m_jumpVelocity = 0.0f; }

    static constexpr float kGroundY = 0.0f; // 地面の Y 座標

private:
    /// <summary>
    /// ボディコライダーの位置・サイズを更新する
    /// </summary>
    void UpdateCollider();

    /// <summary>
    /// 飛行モード時の更新処理
    /// </summary>
    void UpdateFlightMode(float deltaTime, Camera* pCamera, bool isDead, bool isInputDisabled);

    /// <summary>
    /// 通常移動モード時の更新処理
    /// </summary>
    void UpdateNormalMode(float deltaTime, Camera* pCamera, bool isDead, bool isTackling,
        const std::vector<Stage::StageCollisionData>& collisionData, bool isInputDisabled);

    /// <summary>
    /// 空中での慣性・横移動制御を処理する
    /// </summary>
    void UpdateAirControl(const VECTOR& moveDir, Camera* pCamera, float timeScale);

    /// <summary>
    /// キー入力からワールド空間での移動方向ベクトルを計算する
    /// </summary>
    VECTOR CalculateMoveDirection(Camera* pCamera, const unsigned char* keyState);

    /// <summary>
    /// ジャンプ入力の処理
    /// </summary>
    void HandleJump(const unsigned char* keyState, const unsigned char* prevKeyState, bool isDead,
        bool isTackling, const VECTOR& moveDir, Camera* pCamera);

    /// <summary>
    /// 重力・ジャンプ速度など物理演算を処理する
    /// </summary>
    void HandlePhysics(bool isOnGround, Camera* pCamera);

    /// <summary>
    /// ステージとの衝突解決処理
    /// </summary>
    void ResolveCollisions(const std::vector<Stage::StageCollisionData>& collisionData,
        Camera* pCamera, bool isMoving);

private:
    VECTOR m_modelPos;
    VECTOR m_scale;
    VECTOR m_jumpMoveVelocity;
    VECTOR m_airSideControlVelocity; // 空中での左右操作速度
    VECTOR m_knockbackVelocity;      // ノックバック速度

    float m_moveSpeed;
    float m_runSpeed;
    float m_jumpVelocity;
    float m_jumpStartYaw;
    float m_jumpSpeedScalar;
    float m_coyoteTimeTimer;  // コヨーテタイム用のタイマー
    float m_currentSpeed;

    // 状態フラグ
    bool m_isMoving;
    bool m_isJumping;
    bool m_wasJumping;
    bool m_wasRunning;
    bool m_isGroundedOnStage;
    bool m_isRunMode;
    bool m_isRunJumping;
    bool m_isJumpInertiaActive;
    bool  m_justLanded;      // 着地した瞬間フラグ
    float m_impactVelocity;  // 着地時の衝撃速度
    float m_airborneTime;    // 滞空時間

    std::shared_ptr<CapsuleCollider> m_pBodyCollider;

    std::string m_groundedObjectName;
};
