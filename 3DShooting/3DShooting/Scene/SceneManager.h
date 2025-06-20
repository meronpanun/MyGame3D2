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
	SceneManager();
	~SceneManager();

	void Init();
	void Update();
	void Draw();

private:
	SceneBase* m_pCurrentScene;
	SceneBase* m_pNextScene;

	// SceneManagerで管理するシーン
	SceneTitle*    m_pTitle;
	SceneMain*     m_pSceneMain;
	SceneResult*   m_pResult;
	SceneOption*   m_pOption;
	SceneGameOver* m_pGameOver;
};
