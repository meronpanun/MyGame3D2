#pragma once
#include "SceneBase.h"
#include <memory>
#include <chrono>
#include <vector>

class Player;
class Camera;

/// <summary>
/// ゲームシーンクラス
/// </summary>
class SceneMain : public SceneBase
{
public:
	SceneMain();
	virtual~SceneMain();

	void Init() override;
	SceneBase* Update() override;
	void Draw() override;

	/// <summary>
	/// ポーズ状態を設定する
	/// </summary>
	/// <param name="paused">ポーズ状態</param>
	void SetPaused(bool paused);

	// カメラを取得するメソッドを追加
	Camera* GetCamera() const { return m_pCamera.get(); }
	
	/// <summary>
	/// カメラの感度を設定する
	/// </summary>
	/// <param name="sensitivity">感度</param>
	void SetCameraSensitivity(float sensitivity);

private:
	/// <summary>
	/// ポーズメニューを描画
	/// </summary>
	void DrawPauseMenu();

private:
	std::unique_ptr<Player> m_pPlayer;
	std::shared_ptr<Camera> m_pCamera;

	// ポーズに入ったタイミングの時間を記録する
	std::chrono::steady_clock::time_point m_pauseStartTime;

	bool  m_isPaused;               // ポーズ状態を管理するフラグ
	bool  m_isReturningFromOption;  // オプションシーンから戻ったかどうかを判定するフラグ
	bool  m_isEscapePressed;        // エスケープキーの押下状態を管理するフラグ
	int   m_skyDomeHandle;          // スカイドームのハンドル
	int   m_skyDomeTextureHandle;   // スカイドームのテクスチャハンドル
	float m_cameraSensitivity;      // カメラの感度
};

