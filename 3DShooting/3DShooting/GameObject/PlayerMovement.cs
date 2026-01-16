using UnityEngine;

/// <summary>
/// DxLib版 PlayerMovement.cpp の挙動を忠実に再現したUnity用スクリプト
/// </summary>
[RequireComponent(typeof(CharacterController))]
public class PlayerMovement : MonoBehaviour
{
    [Header("Settings")]
    public float moveSpeed = 5.0f;        // DxLib版 m_moveSpeed 相当
    public float runSpeed = 10.0f;         // DxLib版 m_runSpeed 相当
    public float gravity = 20.0f;          // Unity用に調整した重力 (kGravity 0.25f 相当)
    public float jumpPower = 8.0f;         // kJumpPower 6.0f 相当
    public float runJumpPower = 12.0f;     // kRunJumpPower 10.0f 相当
    public float airControlFactor = 1.0f;  // kAirControlFactor 1.0f 相当

    [Header("Components")]
    public Transform cameraTransform;      // カメラの向きを参照するため

    // 内部状態
    private CharacterController _controller;
    private Vector3 _velocity = Vector3.zero;
    private bool _isRunMode = false;
    private bool _isJumping = false;
    private bool _isRunJumping = false;
    private bool _isJumpInertiaActive = false;
    private Vector3 _jumpMoveVelocity = Vector3.zero;
    private float _jumpStartYaw = 0.0f;
    private float _jumpSpeedScalar = 0.0f;

    void Awake()
    {
        _controller = GetComponent<CharacterController>();
        if (cameraTransform == null) cameraTransform = Camera.main.transform;
    }

    void Update()
    {
        UpdateMovement();
    }

    private void UpdateMovement()
    {
        bool isGrounded = _controller.isGrounded;

        // 接地時の初期化
        if (isGrounded && _velocity.y < 0)
        {
            _velocity.y = -2f; // Unityの接地判定を安定させるため少し下向きに力をかける
            if (_isJumping)
            {
                OnLanding();
            }
        }

        // 入力の取得
        float horizontal = Input.GetAxisRaw("Horizontal");
        float vertical = Input.GetAxisRaw("Vertical");
        Vector3 inputDir = new Vector3(horizontal, 0, vertical).normalized;

        // カメラ基準の移動方向を計算
        float yaw = cameraTransform.eulerAngles.y * Mathf.Deg2Rad;
        Vector3 camFwd = new Vector3(Mathf.Sin(yaw), 0, Mathf.Cos(yaw));
        Vector3 camRight = new Vector3(Mathf.Sin(yaw + Mathf.PI * 0.5f), 0, Mathf.Cos(yaw + Mathf.PI * 0.5f));
        Vector3 moveDir = (camFwd * vertical + camRight * horizontal).normalized;

        // ダッシュ判定 (Shift + W)
        if (Input.GetKey(KeyCode.LeftShift) && Input.GetKey(KeyCode.W))
        {
            _isRunMode = true;
        }
        else if (!Input.GetKey(KeyCode.W))
        {
            _isRunMode = false;
        }

        bool isRunning = _isRunMode;
        float currentTargetSpeed = isRunning ? runSpeed : moveSpeed;

        // ジャンプ処理
        if (Input.GetKeyDown(KeyCode.Space) && isGrounded && !_isJumping)
        {
            _isJumping = true;
            _isRunJumping = isRunning;
            _velocity.y = isRunning ? runJumpPower : jumpPower;

            // ジャンプ慣性の設定
            if (moveDir.sqrMagnitude > 0.001f)
            {
                _isJumpInertiaActive = true;
                _jumpMoveVelocity = moveDir * currentTargetSpeed;
                _jumpStartYaw = yaw;
                _jumpSpeedScalar = _jumpMoveVelocity.magnitude;
            }
        }

        // 移動計算
        Vector3 finalMovement = Vector3.zero;

        if (_isJumpInertiaActive)
        {
            // 空中慣性移動
            finalMovement = _jumpMoveVelocity;

            // 空中操作 (Air Control)
            if (moveDir.sqrMagnitude > 0.001f)
            {
                float currentSpeed = (isRunning || _isRunJumping) ? runSpeed : moveSpeed;
                float inertiaSpeed = _jumpMoveVelocity.magnitude;

                Vector3 controlForce = Vector3.zero;

                if (inertiaSpeed > 0.1f)
                {
                    // 1. 横入力 (A/D)
                    float dotRight = Vector3.Dot(moveDir, camRight);
                    controlForce += camRight * (dotRight * currentSpeed * airControlFactor);

                    // 2. 前方入力 (W/S) 旋回とブレーキ
                    float dotFwd = Vector3.Dot(moveDir, camFwd);
                    float diffYaw = Mathf.DeltaAngle(yaw * Mathf.Rad2Deg, _jumpStartYaw * Mathf.Rad2Deg) * Mathf.Deg2Rad;

                    Vector3 inertiaDir = _jumpMoveVelocity.normalized;

                    // 旋回（ステアリング）
                    if (Mathf.Abs(diffYaw) < Mathf.PI * 0.5f)
                    {
                        if (dotFwd > 0.1f)
                        {
                            _jumpMoveVelocity += camFwd * (dotFwd * currentSpeed * airControlFactor * 0.1f);
                            if (_jumpMoveVelocity.sqrMagnitude > 0.001f)
                            {
                                _jumpMoveVelocity = _jumpMoveVelocity.normalized * _jumpSpeedScalar;
                            }
                        }
                    }

                    // 【直感的なブレーキ】
                    // 慣性方向と逆向きの入力成分がある場合に減速を適用
                    Vector3 fwdForceRaw = camFwd * (dotFwd * currentSpeed * airControlFactor);
                    float fwdProjDot = Vector3.Dot(fwdForceRaw, inertiaDir);
                    if (fwdProjDot < 0.0f)
                    {
                        controlForce += inertiaDir * fwdProjDot;
                    }
                }
                else
                {
                    // 慣性がない場合は自由移動
                    controlForce = moveDir * (currentTargetSpeed * airControlFactor);
                }

                finalMovement += controlForce;
            }
        }
        else if (isGrounded)
        {
            // 地上移動
            finalMovement = moveDir * currentTargetSpeed;
        }

        // キャラクターの移動実行 (Time.deltaTimeを掛ける)
        _controller.Move(finalMovement * Time.deltaTime);

        // 重力の適用
        _velocity.y -= gravity * Time.deltaTime;
        _controller.Move(_velocity * Time.deltaTime);
    }

    private void OnLanding()
    {
        _isJumping = false;
        _isRunJumping = false;
        _isJumpInertiaActive = false;
        
        // 着地時の角度チェックによるダッシュ解除
        float currentYaw = cameraTransform.eulerAngles.y * Mathf.Deg2Rad;
        float diffYaw = Mathf.DeltaAngle(currentYaw * Mathf.Rad2Deg, _jumpStartYaw * Mathf.Rad2Deg) * Mathf.Deg2Rad;
        if (Mathf.Abs(diffYaw) >= Mathf.PI * 0.5f)
        {
            _isRunMode = false;
        }
    }
}
