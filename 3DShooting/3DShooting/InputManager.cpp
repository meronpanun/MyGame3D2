#include "InputManager.h"
#include "EffekseerWarningSuppress.h"
#include "Game.h"
#include <DxLib.h>

namespace
{
    constexpr float kScreenCenter    = 0.5f;  // 画面中央の比率
    constexpr float kPitchLimitDeg   = 89.0f; // カメラピッチの最大角度（度）。ジンバルロック防止のため 90 度未満
}

InputManager* InputManager::GetInstance()
{
    static InputManager instance;
    return &instance;
}

InputManager::InputManager()
    : m_mouseWheelRot(0)
{
    for (int i = 0; i < kLogNum; ++i)
    {
        m_mouseLog[i]      = 0;
        m_mouseRightLog[i] = 0;
    }
}

void InputManager::Update()
{
    for (int i = kLogNum - 1; i >= 1; i--)
    {
        m_mouseLog[i]      = m_mouseLog[i - 1];
        m_mouseRightLog[i] = m_mouseRightLog[i - 1];
    }
    m_mouseLog[0]      = (GetMouseInput() & MOUSE_INPUT_LEFT)  ? 1 : 0;
    m_mouseRightLog[0] = (GetMouseInput() & MOUSE_INPUT_RIGHT) ? 1 : 0;
    m_mouseWheelRot    = ::GetMouseWheelRotVol();
}

Vec2 InputManager::GetMousePos() const
{
    Vec2 mousePos{ 0, 0 };
    int mouseX = 0;
    int mouseY = 0;
    if (GetMousePoint(&mouseX, &mouseY) == -1)
    {
        return mousePos;
    }
    mousePos.x = static_cast<float>(mouseX);
    mousePos.y = static_cast<float>(mouseY);
    return mousePos;
}

bool InputManager::IsPressMouseLeft() const
{
    return (m_mouseLog[0] != 0);
}

bool InputManager::IsTriggerMouseLeft() const
{
    bool isNow  = (m_mouseLog[0] != 0); // 現在の状態
    bool isLast = (m_mouseLog[1] != 0); // 1 フレーム前の状態
    return (isNow && !isLast);
}

bool InputManager::IsReleaseMouseLeft() const
{
    bool isNow  = (m_mouseLog[0] != 0); // 現在の状態
    bool isLast = (m_mouseLog[1] != 0); // 1 フレーム前の状態
    return (!isNow && isLast);
}

bool InputManager::IsTriggerMouseRight() const
{
    bool isNow  = (m_mouseRightLog[0] != 0); // 現在の状態
    bool isLast = (m_mouseRightLog[1] != 0); // 1 フレーム前の状態
    return (isNow && !isLast);
}

bool InputManager::IsPressMouseRight() const
{
    return (m_mouseRightLog[0] != 0);
}

void InputManager::UpdateCameraRotation(float& cameraYaw, float& cameraPitch, float sensitivity)
{
    // ウィンドウ非アクティブ時・PrintScreen キー押下時はマウス固定を解除するためスキップ
    if (GetWindowActiveFlag() == FALSE || CheckHitKey(KEY_INPUT_SYSRQ))
    {
        return;
    }

    Vec2 mousePos = GetMousePos();

    cameraYaw   += (mousePos.x - Game::GetScreenWidth()  * kScreenCenter) * sensitivity;
    cameraPitch -= (mousePos.y - Game::GetScreenHeight() * kScreenCenter) * sensitivity;

    // ピッチ角を制限（ジンバルロック防止のため 89 度止まり）
    constexpr float kPitchLimit = kPitchLimitDeg * (DX_PI_F / 180.0f);
    if (cameraPitch >  kPitchLimit) cameraPitch =  kPitchLimit;
    if (cameraPitch < -kPitchLimit) cameraPitch = -kPitchLimit;

    // マウスを画面中央に固定
    SetMousePoint(
        static_cast<int>(Game::GetScreenWidth()  * kScreenCenter),
        static_cast<int>(Game::GetScreenHeight() * kScreenCenter));
}

int InputManager::GetMouseWheelRotVol() const
{
    return m_mouseWheelRot;
}
