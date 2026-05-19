#pragma once
#include "Vec2.h"

/// <summary>
/// マウス・キーボード入力を一元管理するシングルトンクラス。
/// ボタンの押下・トリガー・リリース判定およびカメラ回転の更新を提供する。
/// </summary>
class InputManager
{
public:
    /// <summary>
    /// シングルトンインスタンスを返す
    /// </summary>
    /// <returns>InputManager インスタンスへのポインタ</returns>
    static InputManager* GetInstance();

    /// <summary>
    /// 毎フレームの入力状態を更新する（ボタンログ・ホイール量を取得）
    /// </summary>
    void Update();

    /// <summary>
    /// 現在のマウスカーソルのスクリーン座標を返す
    /// </summary>
    /// <returns>マウスカーソルの X, Y 座標</returns>
    Vec2 GetMousePos() const;

    /// <summary>
    /// マウス左ボタンが現在押されているかを返す
    /// </summary>
    /// <returns>押されていれば true</returns>
    bool IsPressMouseLeft() const;

    /// <summary>
    /// マウス左ボタンが押された瞬間かを返す
    /// </summary>
    /// <returns>押された瞬間のフレームなら true</returns>
    bool IsTriggerMouseLeft() const;

    /// <summary>
    /// マウス左ボタンが離された瞬間かを返す
    /// </summary>
    /// <returns>離された瞬間のフレームなら true</returns>
    bool IsReleaseMouseLeft() const;

    /// <summary>
    /// マウス右ボタンが押された瞬間かを返す
    /// </summary>
    /// <returns>押された瞬間のフレームなら true</returns>
    bool IsTriggerMouseRight() const;

    /// <summary>
    /// マウス右ボタンが現在押されているかを返す
    /// </summary>
    /// <returns>押されていれば true</returns>
    bool IsPressMouseRight() const;

    /// <summary>
    /// マウスの移動量に基づいてカメラのヨー・ピッチ角を更新する。
    /// ウィンドウ非アクティブ時および PrintScreen キー押下時は処理をスキップする。
    /// </summary>
    /// <param name="cameraYaw">更新するカメラのヨー角（ラジアン）への参照</param>
    /// <param name="cameraPitch">更新するカメラのピッチ角（ラジアン）への参照</param>
    /// <param name="sensitivity">マウス感度</param>
    void UpdateCameraRotation(float& cameraYaw, float& cameraPitch, float sensitivity);

    /// <summary>
    /// マウスホイールの回転量を返す
    /// </summary>
    /// <returns>回転量（正：上方向、負：下方向）</returns>
    int GetMouseWheelRotVol() const;

private:
    /// <summary>
    /// シングルトンパターンのためコンストラクタを private にする
    /// </summary>
    InputManager();

    InputManager(const InputManager&)            = delete; // コピー禁止
    InputManager& operator=(const InputManager&) = delete; // 代入禁止

    static constexpr int kLogNum = 16; // ボタン入力ログのフレーム数

    int m_mouseLog[kLogNum];      // 左ボタンの入力ログ（[0] が最新）
    int m_mouseRightLog[kLogNum]; // 右ボタンの入力ログ（[0] が最新）
    int m_mouseWheelRot;          // マウスホイールの回転量
};
