#pragma once
#include "DxLib.h"

/// <summary>
/// カメラクラス
/// </summary>
class Camera
{
public:
	Camera();
	virtual ~Camera();

	void Init();
	void Update();

	/// <summary>
	/// カメラの感度を設定 
	/// </summary>
	/// <param name="sensitivity">感度</param> 
	void SetSensitivity(float sensitivity);

	/// <summary>
	/// カメラの位置を取得
	/// </summary>
	/// <returns>カメラの位置</returns>
	VECTOR GetPos() const { return m_pos; }

	/// <summary>
	/// カメラの注視点を取得
	/// </summary>
	/// <returns>カメラの注視点</returns>
	VECTOR GetTarget() const { return m_target; }

	/// <summary>
	/// カメラのオフセットを取得
	/// </summary>
	/// <returns>カメラのオフセット</returns>
	VECTOR GetOffset() const { return m_offset; }

	/// <summary>
	/// カメラのオフセットを設定
	/// </summary>
	/// <param name="offset">カメラのオフセット</param>
	void SetOffset(const VECTOR& offset) { m_offset = offset; }

	/// <summary>
	/// カメラの位置を設定
	/// </summary>
	/// <param name="pos">カメラの位置</param>
	void SetPos(const VECTOR& pos) { m_pos = pos; }
	/// <summary>
	/// カメラの注視点を設定
	/// </summary>
	/// <param name="target">カメラの注視点</param>
	void SetTarget(const VECTOR& target) { m_target = target; }

	/// <summary>
	/// プレイヤーの位置を設定
	/// </summary>
	/// <param name="playerPos">プレイヤーの位置</param>
	void SetPlayerPos(const VECTOR& playerPos) { m_playerPos = playerPos; }

	/// <summary>
	/// カメラの回転角度を取得
	/// </summary>
	/// <returns>カメラの回転角度</returns>
	float GetYaw()   const { return m_yaw; }
	/// <summary>
	/// カメラのピッチ角度を取得
	/// </summary>
	/// <returns>カメラのピッチ角度</returns>
	float GetPitch() const { return m_pitch; }

	/// <summary>
	/// カメラの位置と注視点をDxLibに設定
	/// </summary>
	void SetCameraToDxLib(); 

	/// <summary>
	/// カメラの視野角(FOV)を設定
	/// </summary>
	/// <param name="fov">視野角</param>
	void SetFOV(float fov);

	/// <summary>
	/// 現在の視野角(FOV)を取得
	/// </summary>
	/// <returns>現在の視野角(FOV)</returns>
	float GetFOV() const;

	/// <summary>
	/// カメラの視野角(FOV)をデフォルトに戻す
	/// </summary>
	void ResetFOV();

	/// <summary>
	/// カメラのオフセットをデフォルトに戻す
	/// </summary>
	void ResetOffset();

	/// <summary>
	/// 目標FOVをセット
	/// </summary>
	/// <param name="fov">目標FOV</param>
	void SetTargetFOV(float fov);

private:
	VECTOR m_pos;			// カメラの位置
	VECTOR m_target;		// カメラの注視点
	VECTOR m_offset;		// カメラのオフセット
	VECTOR m_defaultOffset; // デフォルトのオフセット
	VECTOR m_playerPos;		// プレイヤーの位置

	float m_yaw;		  // ヨー角度
	float m_pitch;		  // ピッチ角度
	float m_sensitivity;  // カメラの感度
	float m_fov;		  // カメラの視野角
	float m_defaultFov;   // デフォルトのFOV
	float m_targetFov;    // 目標FOV
	float m_fovLerpSpeed; // FOVの補間速度
};

