using UnityEngine;

/// <summary>
/// マウス操作による一人称視点の回転を制御するスクリプト
/// </summary>
public class FirstPersonCamera : MonoBehaviour
{
    [Header("Settings")]
    public float m_mouseSensitivity = 2.0f; // マウス感度
    public float m_verticalRange = 80.0f;    // 上下の回転制限（度）

    [Header("References")]
    public Transform m_pPlayerBody;           // プレイヤー本体のTransform（左右回転用）

    private float m_verticalRotation = 0f;

    void Start()
    {
        // マウスカーソルを非表示にして中央に固定
        Cursor.lockState = CursorLockMode.Locked;
        Cursor.visible = false;

        if (m_pPlayerBody == null)
        {
            // 設定されていない場合は親オブジェクトをプレイヤー本体とみなす
            m_pPlayerBody = transform.parent;
        }
    }

    void Update()
    {
        // マウスの移動量を取得
        float mouseX = Input.GetAxis("Mouse X") * m_mouseSensitivity;
        float mouseY = Input.GetAxis("Mouse Y") * m_mouseSensitivity;

        // 左右の回転（プレイヤー本体を回す）
        if (m_pPlayerBody != null)
        {
            m_pPlayerBody.Rotate(Vector3.up * mouseX);
        }

        // 上下の回転（カメラ自身を回す、かつ範囲制限をかける）
        m_verticalRotation -= mouseY;
        m_verticalRotation = Mathf.Clamp(m_verticalRotation, -m_verticalRange, m_verticalRange);

        transform.localRotation = Quaternion.Euler(m_verticalRotation, 0f, 0f);
    }
}
