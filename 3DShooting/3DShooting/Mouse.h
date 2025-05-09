#pragma once
#include "Vec2.h"

/* マウスの入力情報を取得する */
namespace Mouse
{
	// マウスの入力状態取得
	void Update();

	/// <summary>
	/// マウスの位置を取得
	/// </summary>
	/// <returns>マウスの位置</returns>
	Vec2 GetPos();

	/// <summary>
	/// マウスの左ボタンが押されているかの判定
	/// </summary>
	/// <returns>押されているならtrue</returns>
	bool IsPressLeft();
	/// <summary>
	/// マウスの左ボタンがトリガーされた瞬間の判定
	/// </summary>
	/// <returns>トリガーされた瞬間ならtrue</returns>
	bool IsTriggerLeft();
	/// <summary>
	/// マウスの左ボタンが離された瞬間の判定
	/// </summary>
	/// <returns>離された瞬間ならtrue</returns>
	bool IsReleaseLeft();

	/// <summary>
	/// カメラの回転角度を更新
	/// </summary>
	/// <param name="cameraYaw">カメラのヨー角度</param>
	/// <param name="cameraPitch">カメラのピッチ角度</param>
	void UpdateCameraRotation(float& cameraYaw, float& cameraPitch, float sensitivity);
}

