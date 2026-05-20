#include "PlayerMovement.h"
#include "Camera.h"
#include "CapsuleCollider.h"
#include "Collision.h"
#include "EffekseerWarningSuppress.h"
#include "Game.h"
#include <algorithm>
#include <cmath>

namespace PlayerMovementConstants
{
    // 重力とジャンプ関連
    constexpr float kGravity      = 0.3f;
    constexpr float kJumpPower    = 7.0f;
    constexpr float kRunJumpPower = 12.5f;

    // 飛行モード関連
    constexpr float kFlightAscendSpeed      = 8.0f;  // 上昇速度
    constexpr float kFlightDescendSpeed     = 8.0f;  // 降下速度
    constexpr float kFlightAccelMultiplier  = 3.0f;  // 飛行モード中の加速倍率

    // カプセルコライダーのサイズ
    constexpr float kCapsuleHeight = 100.0f;
    constexpr float kCapsuleRadius = 50.0f;

    // 移動・判定関連
    constexpr float kPlayerColliderYOffset  = 60.0f;  // コライダーの Y オフセット
    constexpr float kGroundCheckTolerance   = 1.0f;   // 地面判定の許容誤差
    constexpr float kCoyoteTimeDuration     = 0.2f;   // コヨーテタイムの持続時間（秒）
    constexpr float kDeltaTimeEpsilon       = 0.0001f; // 速度計算用 deltaTime の最小値
    constexpr float kSlopeNormalYThreshold  = 0.6f;   // 傾斜と判定する法線 Y の閾値
    constexpr float kRunBlockPushBackSq     = 1.0f;   // ダッシュ解除とみなす押し返し量の二乗

    // 着地判定関連
    constexpr float kAirborneTimeThreshold    = 0.15f; // 着地SEを鳴らすための最低滞空時間（秒）
    constexpr float kLandingVelocityThreshold = -1.5f; // 着地とみなす最低落下速度
    constexpr float kHeavyLandingThreshold    = -5.0f; // 重着地とみなす落下速度

    // カメラ揺れ関連
    constexpr float kJumpSwayPower              = 5.0f;  // ジャンプ時の揺れの強さ
    constexpr float kLandingSwayPower           = 5.0f;  // 着地時の揺れの強さ
    constexpr float kRunLandingSwayPower        = 20.0f; // ダッシュ着地時の揺れの強さ
    constexpr float kLandingVelocityFactor      = 0.5f;  // 着地時の衝撃（速度）による揺れの補正係数
    constexpr float kRunLandingShakeIntensity   = 1.0f;  // ダッシュ着地時のシェイク強度（画面全体の振動）
    constexpr float kRunLandingShakeVelocityFactor = 0.2f; // 速度によるシェイク加算係数
    constexpr int   kRunLandingShakeDuration    = 5;     // シェイク持続時間（フレーム数）

    // ノックバック関連
    constexpr float kKnockbackDamping       = 0.9f;  // ノックバック速度の減衰率
    constexpr float kKnockbackStopThreshold = 0.1f;  // ノックバック停止とみなす速度

    // 空中制御関連
    constexpr float kAirControlFactor      = 1.0f;  // 空中での操作の効き具合
    constexpr float kAirBrakeFactor        = 0.01f; // 空中でのブレーキの効き具合（Lerp 係数）
    constexpr float kAirAccelFactor        = 0.05f; // 空中での加速の効き具合（Lerp 係数）
    constexpr float kAirSteerFactor        = 0.1f;  // 空中での前方ステアリングの強さ
    constexpr float kInertiaSpeedThreshold = 0.1f;  // 慣性速度が小さいとみなす閾値
    constexpr float kJumpVelocityMinSize   = 0.001f; // ジャンプ速度ベクトルの最小長
}

PlayerMovement::PlayerMovement()
    : m_modelPos(VGet(0, 0, 0))
    , m_scale(VGet(1, 1, 1))
    , m_moveSpeed(0.0f)
    , m_runSpeed(0.0f)
    , m_isMoving(false)
    , m_isJumping(false)
    , m_wasJumping(false)
    , m_wasRunning(false)
    , m_isGroundedOnStage(false)
    , m_isRunMode(false)
    , m_isRunJumping(false)
    , m_isJumpInertiaActive(false)
    , m_justLanded(false)
    , m_impactVelocity(0.0f)
    , m_jumpVelocity(0.0f)
    , m_jumpStartYaw(0.0f)
    , m_jumpSpeedScalar(0.0f)
    , m_coyoteTimeTimer(0.0f)
    , m_jumpMoveVelocity(VGet(0, 0, 0))
    , m_airSideControlVelocity(VGet(0, 0, 0))
    , m_knockbackVelocity(VGet(0, 0, 0))
    , m_pBodyCollider(std::make_shared<CapsuleCollider>())
    , m_currentSpeed(0.0f)
    , m_airborneTime(0.0f)
{
}

void PlayerMovement::Init(const VECTOR& pos, float moveSpeed, float runSpeed, float scale)
{
    m_modelPos  = pos;
    m_moveSpeed = moveSpeed;
    m_runSpeed  = runSpeed;
    m_scale     = VGet(scale, scale, scale);

    // 速度・慣性・状態フラグをすべてリセットする。
    // チュートリアルからメインステージへ移行するとき等に
    // 前の移動慣性がそのまま引き継がれないようにする。
    m_jumpMoveVelocity       = VGet(0.0f, 0.0f, 0.0f);
    m_airSideControlVelocity = VGet(0.0f, 0.0f, 0.0f);
    m_knockbackVelocity      = VGet(0.0f, 0.0f, 0.0f);
    m_jumpVelocity           = 0.0f;
    m_jumpStartYaw           = 0.0f;
    m_jumpSpeedScalar        = 0.0f;
    m_coyoteTimeTimer        = 0.0f;
    m_currentSpeed           = 0.0f;
    m_airborneTime           = 0.0f;
    m_impactVelocity         = 0.0f;
    m_isMoving               = false;
    m_isJumping              = false;
    m_wasJumping             = false;
    m_wasRunning             = false;
    m_isGroundedOnStage      = false;
    m_isRunMode              = false;
    m_isRunJumping           = false;
    m_isJumpInertiaActive    = false;
    m_justLanded             = false;
}

void PlayerMovement::Update(float deltaTime, Camera* pCamera, bool isDead, bool isTackling,
    bool isFlightMode, const std::vector<Stage::StageCollisionData>& collisionData, bool isInputDisabled)
{
    m_justLanded = false;
    VECTOR prevPos = m_modelPos;

    UpdateCollider();

    if (m_coyoteTimeTimer > 0.0f) m_coyoteTimeTimer -= deltaTime;

    if (isTackling)
    {
        // タックル中の速度計算を更新（移動処理はタックル側で行うためスキップ）
        float dist = VSize(VSub(m_modelPos, prevPos));
        m_currentSpeed = (deltaTime > PlayerMovementConstants::kDeltaTimeEpsilon) ? dist / deltaTime : 0.0f;
        return;
    }

    // 接地判定（移動前）
    CollisionResult preCollisionResult = Collision::CheckStageCollision(
        m_modelPos,
        PlayerMovementConstants::kCapsuleHeight,
        PlayerMovementConstants::kCapsuleRadius,
        PlayerMovementConstants::kPlayerColliderYOffset,
        collisionData);
    m_isGroundedOnStage = preCollisionResult.isGrounded;
    m_groundedObjectName = preCollisionResult.isGrounded ? preCollisionResult.groundedObjectName : "";

    if (m_isGroundedOnStage && m_jumpVelocity < 0.0f)
    {
        m_jumpVelocity = 0.0f;
        m_isJumping    = false;
        m_airborneTime = 0.0f;
    }

    m_airborneTime = IsOnGround() ? 0.0f : m_airborneTime + deltaTime;

    if (isFlightMode && !isDead)
    {
        UpdateFlightMode(deltaTime, pCamera, isDead, isInputDisabled);
    }
    else
    {
        UpdateNormalMode(deltaTime, pCamera, isDead, isTackling, collisionData, isInputDisabled);
    }

    // 速度計算
    float dist = VSize(VSub(m_modelPos, prevPos));
    m_currentSpeed = (deltaTime > PlayerMovementConstants::kDeltaTimeEpsilon) ? dist / deltaTime : 0.0f;
}

void PlayerMovement::UpdateCollider()
{
    VECTOR center = m_modelPos;
    center.y += PlayerMovementConstants::kPlayerColliderYOffset;
    VECTOR capA = VAdd(center, VGet(0, -PlayerMovementConstants::kCapsuleHeight * 0.5f, 0));
    VECTOR capB = VAdd(center, VGet(0,  PlayerMovementConstants::kCapsuleHeight * 0.5f, 0));
    m_pBodyCollider->SetSegment(capA, capB);
    m_pBodyCollider->SetRadius(PlayerMovementConstants::kCapsuleRadius);
}

void PlayerMovement::UpdateFlightMode(float deltaTime, Camera* pCamera, bool isDead, bool isInputDisabled)
{
    m_jumpVelocity = 0.0f;
    m_isJumping    = false;

    unsigned char keyState[256];
    if (isInputDisabled)
        memset(keyState, 0, sizeof(keyState));
    else
        GetHitKeyStateAll(reinterpret_cast<char*>(keyState));

    float timeScale = Game::GetTimeScale();

    if (keyState[KEY_INPUT_SPACE])  m_modelPos.y += PlayerMovementConstants::kFlightAscendSpeed  * timeScale;
    if (keyState[KEY_INPUT_LSHIFT]) m_modelPos.y -= PlayerMovementConstants::kFlightDescendSpeed * timeScale;

    bool isAccelerating = (keyState[KEY_INPUT_LCONTROL] != 0);
    float speed = (isAccelerating ? m_runSpeed * PlayerMovementConstants::kFlightAccelMultiplier : m_moveSpeed) * timeScale;

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

    m_wasRunning = isAccelerating;
    m_wasJumping = false;
    if (pCamera) pCamera->SetHeadBobbingState(m_isMoving, isAccelerating);
}

void PlayerMovement::UpdateNormalMode(float deltaTime, Camera* pCamera, bool isDead, bool isTackling,
    const std::vector<Stage::StageCollisionData>& collisionData, bool isInputDisabled)
{
    bool isOnGround = (m_modelPos.y <= kGroundY + PlayerMovementConstants::kGroundCheckTolerance) || m_isGroundedOnStage;
    if (isOnGround) m_coyoteTimeTimer = PlayerMovementConstants::kCoyoteTimeDuration;

    unsigned char keyState[256];
    if (isInputDisabled)
        memset(keyState, 0, sizeof(keyState));
    else
        GetHitKeyStateAll(reinterpret_cast<char*>(keyState));

    static unsigned char prevKeyState[256] = {};

    if (keyState[KEY_INPUT_LSHIFT] && keyState[KEY_INPUT_W]) m_isRunMode = true;
    if (!keyState[KEY_INPUT_W]) m_isRunMode = false;

    VECTOR moveDir = CalculateMoveDirection(pCamera, keyState);
    HandleJump(keyState, prevKeyState, isDead, isTackling, moveDir, pCamera);

    std::copy(std::begin(keyState), std::end(keyState), std::begin(prevKeyState));

    HandlePhysics(isOnGround, pCamera);

    float timeScale = Game::GetTimeScale();

    // ノックバック処理
    if (VSize(m_knockbackVelocity) > 0.0f)
    {
        m_modelPos = VAdd(m_modelPos, VScale(m_knockbackVelocity, timeScale));
        m_knockbackVelocity = VScale(m_knockbackVelocity, PlayerMovementConstants::kKnockbackDamping);
        if (VSize(m_knockbackVelocity) < PlayerMovementConstants::kKnockbackStopThreshold)
        {
            m_knockbackVelocity = VGet(0, 0, 0);
        }
    }

    if (!isDead)
    {
        if (m_isJumpInertiaActive)
        {
            UpdateAirControl(moveDir, pCamera, timeScale);
        }
        else if (VSize(moveDir) > 0.0f)
        {
            float currentSpeed = (m_isRunMode ? m_runSpeed : m_moveSpeed) * timeScale;
            m_modelPos = VAdd(m_modelPos, VScale(moveDir, currentSpeed));
            m_isMoving = true;
        }
        else
        {
            m_isMoving = false;
        }
    }

    m_wasRunning = m_isRunMode;
    m_wasJumping = m_isJumping;
    if (pCamera) pCamera->SetHeadBobbingState(m_isMoving && isOnGround, m_isRunMode);

    ResolveCollisions(collisionData, pCamera, m_isMoving);
}

void PlayerMovement::UpdateAirControl(const VECTOR& moveDir, Camera* pCamera, float timeScale)
{
    m_isMoving = true;

    // 慣性速度ベクトルにタイムスケールを適用してプレイヤー座標を更新
    m_modelPos = VAdd(m_modelPos, VScale(m_jumpMoveVelocity, timeScale));

    // 入力がない、またはカメラが存在しない場合は以降の修正処理をスキップ
    if (VSize(moveDir) <= 0.0f || !pCamera) return;

    float currentSpeed = ((m_isRunMode || m_isRunJumping) ? m_runSpeed : m_moveSpeed) * timeScale;
    float inertiaSpeed = VSize(m_jumpMoveVelocity);

    // 慣性速度が極めて小さい場合は直接位置を加算して終了
    if (inertiaSpeed <= PlayerMovementConstants::kInertiaSpeedThreshold)
    {
        m_modelPos = VAdd(m_modelPos, VScale(moveDir, currentSpeed * PlayerMovementConstants::kAirControlFactor));
        return;
    }

    // 空中制御
    float yaw = pCamera->GetYaw();
    VECTOR camFwd   = VGet(sinf(yaw), 0.0f, cosf(yaw));
    VECTOR camRight = VGet(sinf(yaw + DX_PI_F * 0.5f), 0.0f, cosf(yaw + DX_PI_F * 0.5f));
    float dotFwd    = VDot(moveDir, camFwd);
    float dotRight  = VDot(moveDir, camRight);

    // [ストレイフ] 目標の横移動速度を算出し、Lerp で滑らかに加速
    VECTOR targetSideVelocity = VScale(camRight, dotRight * currentSpeed * PlayerMovementConstants::kAirControlFactor);
    m_airSideControlVelocity.x += (targetSideVelocity.x - m_airSideControlVelocity.x) * PlayerMovementConstants::kAirAccelFactor * timeScale;
    m_airSideControlVelocity.z += (targetSideVelocity.z - m_airSideControlVelocity.z) * PlayerMovementConstants::kAirAccelFactor * timeScale;
    m_modelPos = VAdd(m_modelPos, m_airSideControlVelocity);

    // [前方追従] ジャンプ開始からのカメラ回転量を -π〜π に正規化
    float diffYaw = yaw - m_jumpStartYaw;
    while (diffYaw <= -DX_PI_F) diffYaw += DX_TWO_PI_F;
    while (diffYaw >   DX_PI_F) diffYaw -= DX_TWO_PI_F;

    if (fabsf(diffYaw) < DX_PI_F * 0.5f && dotFwd > 0.0f)
    {
        VECTOR steerForce = VScale(camFwd, dotFwd * currentSpeed * PlayerMovementConstants::kAirControlFactor * PlayerMovementConstants::kAirSteerFactor);
        m_jumpMoveVelocity = VAdd(m_jumpMoveVelocity, steerForce);
        if (VSize(m_jumpMoveVelocity) > PlayerMovementConstants::kJumpVelocityMinSize)
        {
            m_jumpMoveVelocity = VScale(VNorm(m_jumpMoveVelocity), m_jumpSpeedScalar);
        }
    }

    // [空中ブレーキ] 慣性と逆方向への入力がある場合、Lerp で自然に減速
    VECTOR inertiaDir = VNorm(m_jumpMoveVelocity);
    float fwdProjDot  = VDot(VScale(camFwd, dotFwd * currentSpeed * PlayerMovementConstants::kAirControlFactor), inertiaDir);

    if (fwdProjDot < 0.0f)
    {
        float curSpeed    = VSize(m_jumpMoveVelocity);
        float targetSpeed = (std::max)(0.0f, curSpeed + fwdProjDot);
        float newSpeed    = curSpeed + (targetSpeed - curSpeed) * PlayerMovementConstants::kAirBrakeFactor * timeScale;

        m_jumpMoveVelocity = VScale(inertiaDir, newSpeed);
        m_jumpSpeedScalar  = newSpeed;
    }
}

void PlayerMovement::ApplyKnockback(const VECTOR& velocity)
{
    m_knockbackVelocity = velocity;
}

VECTOR PlayerMovement::CalculateMoveDirection(Camera* pCamera, const unsigned char* keyState)
{
    if (!pCamera) return VGet(0, 0, 0);

    VECTOR moveDir = VGet(0, 0, 0);
    float yaw = pCamera->GetYaw();

    if (keyState[KEY_INPUT_W])
    {
        moveDir.x += sinf(yaw);
        moveDir.z += cosf(yaw);
    }
    if (keyState[KEY_INPUT_S])
    {
        moveDir.x -= sinf(yaw);
        moveDir.z -= cosf(yaw);
    }
    if (keyState[KEY_INPUT_A])
    {
        moveDir.x += sinf(yaw - DX_PI_F * 0.5f);
        moveDir.z += cosf(yaw - DX_PI_F * 0.5f);
    }
    if (keyState[KEY_INPUT_D])
    {
        moveDir.x += sinf(yaw + DX_PI_F * 0.5f);
        moveDir.z += cosf(yaw + DX_PI_F * 0.5f);
    }

    if (VSize(moveDir) > 0.0f) moveDir = VNorm(moveDir);
    return moveDir;
}

void PlayerMovement::HandleJump(const unsigned char* keyState,
    const unsigned char* prevKeyState, bool isDead,
    bool isTackling, const VECTOR& moveDir,
    Camera* pCamera)
{
    if (isDead || isTackling) return;

    if (keyState[KEY_INPUT_SPACE] && !prevKeyState[KEY_INPUT_SPACE] && m_coyoteTimeTimer > 0.0f && !m_isJumping)
    {
        m_jumpVelocity = m_isRunMode ? PlayerMovementConstants::kRunJumpPower : PlayerMovementConstants::kJumpPower;
        m_isJumping    = true;
        m_isRunJumping = m_isRunMode;
        m_coyoteTimeTimer = 0.0f;

        if (VSize(moveDir) > 0.0f)
        {
            m_isJumpInertiaActive = true;
            m_jumpMoveVelocity    = VScale(moveDir, m_isRunMode ? m_runSpeed : m_moveSpeed);
            if (pCamera) m_jumpStartYaw = pCamera->GetYaw();
            m_jumpSpeedScalar = VSize(m_jumpMoveVelocity);
        }
        if (pCamera) pCamera->ApplyJumpSway(PlayerMovementConstants::kJumpSwayPower);
    }
}

void PlayerMovement::HandlePhysics(bool isOnGround, Camera* pCamera)
{
    float timeScale = Game::GetTimeScale();

    if (m_isJumping || !isOnGround)
    {
        m_modelPos.y += m_jumpVelocity * timeScale;
        float lastJumpVelocity = m_jumpVelocity;
        m_jumpVelocity -= PlayerMovementConstants::kGravity * timeScale;

        if (m_modelPos.y <= kGroundY)
        {
            m_modelPos.y = kGroundY;

            // ジャンプ状態、または一定以上の滞空時間・速度で落下して着地した場合のみ「着地」とみなす
            // 斜面の滑り止めなどの微細な浮き沈みによる SE 再生を防止（0.15s 以上の滞空が必要）
            if (m_isJumping || (m_airborneTime > PlayerMovementConstants::kAirborneTimeThreshold && m_jumpVelocity < PlayerMovementConstants::kLandingVelocityThreshold))
            {
                m_justLanded     = true;
                m_impactVelocity = m_jumpVelocity;
            }

            m_jumpVelocity = 0.0f;
            m_isJumping    = false;

            if (m_wasJumping && pCamera)
            {
                float swayPower = (m_isRunJumping ? PlayerMovementConstants::kRunLandingSwayPower : PlayerMovementConstants::kLandingSwayPower) +
                    fabsf(lastJumpVelocity) * PlayerMovementConstants::kLandingVelocityFactor;
                pCamera->ApplyLandingSway(swayPower);
                if (m_isRunJumping)
                {
                    pCamera->Shake(
                        PlayerMovementConstants::kRunLandingShakeIntensity + fabsf(lastJumpVelocity) * PlayerMovementConstants::kRunLandingShakeVelocityFactor,
                        PlayerMovementConstants::kRunLandingShakeDuration);
                }
            }
            m_isRunJumping        = false;
            m_isJumpInertiaActive = false;
            m_airSideControlVelocity = VGet(0, 0, 0);
        }
    }
}

void PlayerMovement::ResolveCollisions(const std::vector<Stage::StageCollisionData>& collisionData,
    Camera* pCamera, bool isMoving)
{
    VECTOR posBefore = m_modelPos;
    CollisionResult res = Collision::CheckStageCollision(
        m_modelPos,
        PlayerMovementConstants::kCapsuleHeight,
        PlayerMovementConstants::kCapsuleRadius,
        PlayerMovementConstants::kPlayerColliderYOffset,
        collisionData);
    m_isGroundedOnStage  = res.isGrounded;
    m_groundedObjectName = res.isGrounded ? res.groundedObjectName : "";

    if (m_isRunMode && !m_isJumping && isMoving)
    {
        float pushBackSq = powf(m_modelPos.x - posBefore.x, 2) + powf(m_modelPos.z - posBefore.z, 2);
        bool isOnSlope   = res.isGrounded && res.groundNormal.y > PlayerMovementConstants::kSlopeNormalYThreshold;
        if (pushBackSq > PlayerMovementConstants::kRunBlockPushBackSq && !isOnSlope) m_isRunMode = false;
    }

    // ジャンプ状態、または一定以上の滞空時間・速度で落下してステージに着地した場合のみ着地とする
    // 斜面の滑り止めなどの微細な接地判定によるトリガーの誤発火を防止
    if (m_isGroundedOnStage && (m_isJumping || (m_airborneTime > PlayerMovementConstants::kAirborneTimeThreshold && m_jumpVelocity < PlayerMovementConstants::kLandingVelocityThreshold)))
    {
        m_justLanded     = true;
        m_impactVelocity = m_jumpVelocity;

        if ((m_wasJumping || m_jumpVelocity < PlayerMovementConstants::kHeavyLandingThreshold) && pCamera)
        {
            float sway = (m_isRunJumping ? PlayerMovementConstants::kRunLandingSwayPower : PlayerMovementConstants::kLandingSwayPower) +
                fabsf(m_jumpVelocity) * PlayerMovementConstants::kLandingVelocityFactor;
            pCamera->ApplyLandingSway(sway);
            if (m_isRunJumping)
            {
                pCamera->Shake(
                    PlayerMovementConstants::kRunLandingShakeIntensity + fabsf(m_jumpVelocity) * PlayerMovementConstants::kRunLandingShakeVelocityFactor,
                    PlayerMovementConstants::kRunLandingShakeDuration);
                float diffYaw = pCamera->GetYaw() - m_jumpStartYaw;
                while (diffYaw <= -DX_PI_F) diffYaw += DX_TWO_PI_F;
                while (diffYaw >   DX_PI_F) diffYaw -= DX_TWO_PI_F;
                if (fabsf(diffYaw) >= DX_PI_F * 0.5f) m_isRunMode = false;
            }
        }
        m_isRunJumping        = false;
        m_isJumpInertiaActive = false;
        m_airSideControlVelocity = VGet(0, 0, 0);
        m_jumpVelocity = 0.0f;
        m_isJumping    = false;
    }
}

void PlayerMovement::Jump(Camera* pCamera)
{
    if (!m_isJumping && m_coyoteTimeTimer > 0.0f)
    {
        m_jumpVelocity    = PlayerMovementConstants::kJumpPower;
        m_isJumping       = true;
        m_coyoteTimeTimer = 0.0f;
        if (pCamera)
        {
            pCamera->ApplyJumpSway(PlayerMovementConstants::kJumpSwayPower);
        }
    }
}

bool PlayerMovement::IsOnGround() const
{
    return m_isGroundedOnStage || m_modelPos.y <= kGroundY + PlayerMovementConstants::kGroundCheckTolerance;
}
