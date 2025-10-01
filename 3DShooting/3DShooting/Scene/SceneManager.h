#pragma once
#include "SceneBase.h"

class SceneTitle;
class SceneMain;
class SceneResult;
class SceneOption;
class SceneGameOver;

/// <summary>
/// シーン管理クラス
/// </summary>
class SceneManager
{
public:
	enum class FadeState
	{
		Idle,
		FadingOut,
		FadingIn
	};

	SceneManager();
	~SceneManager();

	void Init();
	void Update();
	void Draw();

	/// <summary>
	/// シーンを変更するリクエストを行う
	/// </summary>
	/// <param name="newScene">新しいシーンのポインタ</param>
	void RequestChangeScene(SceneBase* newScene);

	/// <summary>
	/// 現在のシーンを取得
	/// </summary>
	/// <returns>現在のシーンのポインタ</returns>
	SceneBase* GetCurrentScene() const { return m_pCurrentScene; }

private:
	SceneBase* m_pCurrentScene;
	SceneBase* m_pNextScene;
	SceneBase* m_pSceneToChange; // 遷移先のシーンを一時的に保持

	// SceneManagerで管理するシーン
	SceneTitle*    m_pTitle;
	SceneMain*     m_pSceneMain;
	SceneResult*   m_pResult;
	SceneOption*   m_pOption;
	SceneGameOver* m_pGameOver;

	int m_loadingDotCount;    // ロード中のドットの数
	int m_loadingAnimTimer;   // ロードアニメーションのタイマー

	bool m_isExternalSceneChange; // 外部からのシーン変更要求フラグ

	// フェード処理用
	FadeState m_fadeState;
	int m_fadeAlpha;
	int m_fadeSpeed;
};