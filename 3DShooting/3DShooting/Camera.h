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
	VECTOR GetPosition() const { return m_pos; }
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
	/// カメラの位置を設定
	/// </summary>
	/// <param name="pos">カメラの位置</param>
	void SetPosition(const VECTOR& pos) { m_pos = pos; }
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

private:
	VECTOR m_pos;	    // カメラの位置
	VECTOR m_target;    // カメラの注視点
	VECTOR m_offset;    // カメラのオフセット
	VECTOR m_playerPos; // プレイヤーの位置

	// カメラの回転角度
	float m_yaw;
	float m_pitch;
	float m_sensitivity;
};

