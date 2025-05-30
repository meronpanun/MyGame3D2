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
	// �����V�[�����^�C�g���V�[���ɐݒ�
	m_pTitle = new SceneTitle();
	m_pTitle->Init();
	m_pCurrentScene = m_pTitle;
}

void SceneManager::Update()
{
	// �}�E�X�̓��͏�Ԃ��X�V
	Mouse::Update();

	// ���݂̃V�[�����X�Va
	if (m_pCurrentScene != nullptr)
	{
		m_pNextScene = m_pCurrentScene->Update();
	}

	// �V�[�����ς�����ꍇ�A�������������s��
	if (m_pNextScene != nullptr && m_pNextScene != m_pCurrentScene)
	{
		m_pCurrentScene = m_pNextScene;
		m_pCurrentScene->Init();
	}
}

void SceneManager::Draw()
{
	// ���݂̃V�[����`��
	if (m_pCurrentScene != nullptr)
	{
		m_pCurrentScene->Draw();
	}
}
