#include "SceneManager.h"
#include "SceneTitle.h"
#include "SceneMain.h"
#include "SceneResult.h"
#include "Mouse.h"

SceneManager::SceneManager() :
	m_pTitle(nullptr),
	m_pSceneMain(nullptr),
	m_pResult(nullptr),
	m_pCurrentScene(nullptr),
	m_pNextScene(nullptr)
{
}

SceneManager::~SceneManager()
{
	if (m_pTitle != nullptr)
	{
		delete m_pTitle;
		m_pTitle = nullptr;
	}
	if (m_pSceneMain != nullptr)
	{
		delete m_pSceneMain;
		m_pSceneMain = nullptr;
	}
	if (m_pResult != nullptr)
	{
		delete m_pResult;
		m_pResult = nullptr;
	}
}

void SceneManager::Init()
{
	// 初期シーンをタイトルシーンに設定
	m_pTitle = new SceneTitle();
	m_pTitle->Init();
	m_pCurrentScene = m_pTitle;
}

void SceneManager::Update()
{
	// マウスの入力状態を更新
	Mouse::Update();

	// 現在のシーンを更新a
	if (m_pCurrentScene != nullptr)
	{
		m_pNextScene = m_pCurrentScene->Update();
	}

	// シーンが変わった場合、初期化処理を行う
	if (m_pNextScene != nullptr && m_pNextScene != m_pCurrentScene)
	{
		m_pCurrentScene = m_pNextScene;
		m_pCurrentScene->Init();
	}
}

void SceneManager::Draw()
{
	// 現在のシーンを描画
	if (m_pCurrentScene != nullptr)
	{
		m_pCurrentScene->Draw();
	}
}
