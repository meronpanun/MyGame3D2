#include "Mouse.h"
#include "DxLib.h"
#include "Game.h"

namespace
{
    // マウスの入力ログの数
    constexpr int kLogNum = 16;
    // 入力ログ0が最新の状態
    int mouseLog[kLogNum];
}

namespace Mouse
{
    // マウスの入力状態を取得
    void Update()
    {
        // ログの更新
        for (int i = kLogNum - 1; i >= 1; i--)
        {
            mouseLog[i] = mouseLog[i - 1];
        }
        // 最新の状態を取得
        if (GetMouseInput() & MOUSE_INPUT_LEFT)
        {
            mouseLog[0] = 1;
        }
        else
        {
            mouseLog[0] = 0;
        }
    }

    // 現在のマウスの位置を取得
    Vec2 GetPos()
    {
        Vec2 mousePos{ 0,0 };
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

    // 押し下げ判定
    bool IsPressLeft()
    {
        return (mouseLog[0]);
    }
    // トリガー判定
    bool IsTriggerLeft()
    {
        bool isNow = mouseLog[0]; // 現在の状態
        bool isLast = mouseLog[1]; // 1フレーム前の状態
        return (isNow && !isLast);
    }
    // 離した瞬間判定
    bool IsReleaseLeft()
    {
        bool isNow = mouseLog[0]; // 現在の状態
        bool isLast = mouseLog[1]; // 1フレーム前の状態
        return (!isNow && isLast);
    }

    // カメラの回転角度を更新
    void UpdateCameraRotation(float& cameraYaw, float& cameraPitch, float sensitivity)
    {
        // マウスの移動量を取得
        Vec2 mousePos = GetPos();

        // マウスの移動量に基づいてカメラの回転角度を更新
        cameraYaw += (mousePos.x - Game::kScreenWidth * 0.5f) * sensitivity;
        cameraPitch -= (mousePos.y - Game::kScreenHeigth * 0.5f) * sensitivity;

        // カメラのピッチ角度を制限
        if (cameraPitch > DX_PI_F * 0.5f) cameraPitch = DX_PI_F * 0.5f;
        if (cameraPitch < -DX_PI_F * 0.5f) cameraPitch = -DX_PI_F * 0.5f;

        // マウスの位置を中央に戻す
        SetMousePoint(Game::kScreenWidth * 0.5f, Game::kScreenHeigth * 0.5f);
    }
}
