using UnityEngine;

/// <summary>
/// DxLib版 PlayerMovement.cpp の挙動を忠実に再現したUnity用スクリプト
/// </summary>
[RequireComponent(typeof(CharacterController))]
public class PlayerMovement : MonoBehaviour
{
    [Header("Settings")]
    public float m_moveSpeed = 5.0f;        // DxLib版 moveSpeed 相当
    public float m_runSpeed = 10.0f;         // DxLib版 runSpeed 相当
    public float m_gravity = 20.0f;          // Unity用に調整した重力 (kGravity 0.25f 相当)
    public float m_jumpPower = 8.0f;         // kJumpPower 6.0f 相当
    public float m_runJumpPower = 12.0f;     // kRunJumpPower 10.0f 相当
    public float m_airControlFactor = 1.0f;  // kAirControlFactor 1.0f 相当

    [Header("Components")]
    public Transform m_pCameraTransform;      // カメラの向きを参照するため

    // 内部状態
    private CharacterController m_pController;
    private Vector3 m_velocity = Vector3.zero;
    private bool m_is_runMode = false;
    private bool m_is_jumping = false;
    private bool m_is_runJumping = false;
    private bool m_is_jumpInertiaActive = false;
    private Vector3 m_jumpMoveVelocity = Vector3.zero;
    private float m_jumpStartYaw = 0.0f;
    private float m_jumpSpeedScalar = 0.0f;

    [Header("Buffer/Coyote Settings")]
    public float m_coyoteTimeDuration = 0.15f; // 地面を離れてからジャンプ可能な時間
    public float m_jumpBufferDuration = 0.15f; // 着地前に先行入力を受け付ける時間
    private float m_coyoteTimeTimer = 0.0f;
    private float m_jumpBufferTimer = 0.0f;

    void Awake()
    {
        m_pController = GetComponent<CharacterController>();
        if (m_pCameraTransform == null) m_pCameraTransform = Camera.main.transform;
    }

    void Update()
    {
        UpdateMovement();
    }

    private void UpdateMovement()
    {
        bool is_grounded = m_pController.isGrounded;

        // 接地判定とタイマーの更新
        if (is_grounded)
        {
            m_coyoteTimeTimer = m_coyoteTimeDuration;
            if (m_velocity.y < 0)
            {
                m_velocity.y = -2f; // Unityの接地判定を安定させるため少し下向きに力をかける
                if (m_is_jumping)
                {
                    OnLanding();
                }
            }
        }
        else
        {
            m_coyoteTimeTimer -= Time.deltaTime;
        }

        // ジャンプバッファタイマーの更新
        if (m_jumpBufferTimer > 0)
        {
            m_jumpBufferTimer -= Time.deltaTime;
        }

        if (Input.GetKeyDown(KeyCode.Space))
        {
            m_jumpBufferTimer = m_jumpBufferDuration;
        }

        // 入力の取得
        float horizontal = Input.GetAxisRaw("Horizontal");
        float vertical = Input.GetAxisRaw("Vertical");

        // カメラ基準の移動方向を計算
        float yaw = m_pCameraTransform.eulerAngles.y * Mathf.Deg2Rad;
        Vector3 camFwd = new Vector3(Mathf.Sin(yaw), 0, Mathf.Cos(yaw));
        Vector3 camRight = new Vector3(Mathf.Sin(yaw + Mathf.PI * 0.5f), 0, Mathf.Cos(yaw + Mathf.PI * 0.5f));
        Vector3 moveDir = (camFwd * vertical + camRight * horizontal).normalized;

        // ダッシュ判定 (Shift + W)
        if (Input.GetKey(KeyCode.LeftShift) && Input.GetKey(KeyCode.W))
        {
            m_is_runMode = true;
        }
        else if (!Input.GetKey(KeyCode.W))
        {
            m_is_runMode = false;
        }

        bool is_running = m_is_runMode;
        float currentTargetSpeed = is_running ? m_runSpeed : m_moveSpeed;

        // ジャンプ処理 (バッファとコヨーテタイムを使用)
        if (m_jumpBufferTimer > 0 && m_coyoteTimeTimer > 0 && !m_is_jumping)
        {
            m_is_jumping = true;
            m_is_runJumping = is_running;
            m_velocity.y = is_running ? m_runJumpPower : m_jumpPower;

            m_jumpBufferTimer = 0; // 消費
            m_coyoteTimeTimer = 0; // 消費

            // ジャンプ慣性の設定
            if (moveDir.sqrMagnitude > 0.001f)
            {
                m_is_jumpInertiaActive = true;
                m_jumpMoveVelocity = moveDir * currentTargetSpeed;
                m_jumpStartYaw = yaw;
                m_jumpSpeedScalar = m_jumpMoveVelocity.magnitude;
            }
        }

        // 移動計算
        Vector3 finalMovement = Vector3.zero;

        if (m_is_jumpInertiaActive)
        {
            // 空中慣性移動
            finalMovement = m_jumpMoveVelocity;

            // 空中操作 (Air Control)
            if (moveDir.sqrMagnitude > 0.001f)
            {
                float currentSpeed = (is_running || m_is_runJumping) ? m_runSpeed : m_moveSpeed;
                float inertiaSpeed = m_jumpMoveVelocity.magnitude;

                Vector3 controlForce = Vector3.zero;

                if (inertiaSpeed > 0.1f)
                {
                    // 1. 横入力 (A/D)
                    float dotRight = Vector3.Dot(moveDir, camRight);
                    controlForce += camRight * (dotRight * currentSpeed * m_airControlFactor);

                    // 2. 前方入力 (W/S) 旋回とブレーキ
                    float dotFwd = Vector3.Dot(moveDir, camFwd);
                    float diffYaw = Mathf.DeltaAngle(yaw * Mathf.Rad2Deg, m_jumpStartYaw * Mathf.Rad2Deg) * Mathf.Deg2Rad;

                    Vector3 inertiaDir = m_jumpMoveVelocity.normalized;

                    // 旋回（ステアリング）
                    if (Mathf.Abs(diffYaw) < Mathf.PI * 0.5f)
                    {
                        if (dotFwd > 0.1f)
                        {
                            m_jumpMoveVelocity += camFwd * (dotFwd * currentSpeed * m_airControlFactor * 0.1f);
                            if (m_jumpMoveVelocity.sqrMagnitude > 0.001f)
                            {
                                m_jumpMoveVelocity = m_jumpMoveVelocity.normalized * m_jumpSpeedScalar;
                            }
                        }
                    }

                    // 【直感的なブレーキ】
                    // 慣性方向と逆向きの入力成分がある場合に減速を適用
                    Vector3 fwdForceRaw = camFwd * (dotFwd * currentSpeed * m_airControlFactor);
                    float fwdProjDot = Vector3.Dot(fwdForceRaw, inertiaDir);
                    if (fwdProjDot < 0.0f)
                    {
                        controlForce += inertiaDir * fwdProjDot;
                    }
                }
                else
                {
                    // 慣性がない場合は自由移動
                    controlForce = moveDir * (currentTargetSpeed * m_airControlFactor);
                }

                finalMovement += controlForce;
            }
        }
        else if (is_grounded)
        {
            // 地上移動
            finalMovement = moveDir * currentTargetSpeed;
        }

        // キャラクターの移動実行 (Time.deltaTimeを掛ける)
        m_pController.Move(finalMovement * Time.deltaTime);

        // 重力の適用
        m_velocity.y -= m_gravity * Time.deltaTime;
        m_pController.Move(m_velocity * Time.deltaTime);
    }

    private void OnLanding()
    {
        m_is_jumping = false;
        m_is_runJumping = false;
        m_is_jumpInertiaActive = false;
        
        // 着地時の角度チェックによるダッシュ解除
        float currentYaw = m_pCameraTransform.eulerAngles.y * Mathf.Deg2Rad;
        float diffYaw = Mathf.DeltaAngle(currentYaw * Mathf.Rad2Deg, m_jumpStartYaw * Mathf.Rad2Deg) * Mathf.Deg2Rad;
        if (Mathf.Abs(diffYaw) >= Mathf.PI * 0.5f)
        {
            m_is_runMode = false;
        }
    }
}
