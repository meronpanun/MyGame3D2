#pragma once
#include "SceneBase.h"

class SceneTitle;
class SceneMain;
class SceneResult;

/// <summary>
/// �V�[���Ǘ��N���X
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

	// SceneManager�ŊǗ�����V�[��
	SceneTitle* m_pTitle;
	SceneMain* m_pSceneMain;
	SceneResult* m_pResult;
};
