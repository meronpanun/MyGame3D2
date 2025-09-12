#include "SceneManager.h"
#include <string>
#include "SceneTitle.h"
#include "SceneMain.h"
#include "SceneResult.h"
#include "SceneOption.h"
#include "SceneGameOver.h"
#include "DebugUtil.h"
#include "Mouse.h"

SceneManager::SceneManager() :
	m_pTitle(nullptr),
	m_pOption(nullptr),
	m_pSceneMain(nullptr),
	m_pResult(nullptr),
	m_pGameOver(nullptr),
	m_pCurrentScene(nullptr),
	m_pNextScene(nullptr),
    m_isExternalSceneChange(false),
	m_loadingDotCount(0),
	m_loadingAnimTimer(0)
{
}

SceneManager::~SceneManager()
{
	if (m_pTitle != nullptr)
	{
		delete m_pTitle;
		m_pTitle = nullptr;
	}
	if (m_pOption != nullptr)
	{
		delete m_pOption;
		m_pOption = nullptr;
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
	if (m_pGameOver != nullptr)
	{
		delete m_pGameOver;
		m_pGameOver = nullptr;
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

    // 現在のシーンを更新
    if (m_pCurrentScene != nullptr)
    {
        m_pNextScene = m_pCurrentScene->Update();
    }

    // シーンが変わった場合、初期化処理とローディング画面表示を行う
    if (m_pNextScene != nullptr && m_pNextScene != m_pCurrentScene)
    {
        // ローディング画面表示（SceneMainの場合のみ）
        if (dynamic_cast<SceneMain*>(m_pNextScene))
        {
            for (int i = 0; i < 360; ++i)
            {
                ClearDrawScreen();
                int screenW, screenH;
                GetScreenState(&screenW, &screenH, nullptr);
                DrawBox(0, 0, screenW, screenH, 0x000000, true);
                SetFontSize(48);
                std::string loadingText = "Now Loading";
                m_loadingAnimTimer++;
                if (m_loadingAnimTimer > 30)
                {
                    m_loadingAnimTimer = 0;
                    m_loadingDotCount++;
                    if (m_loadingDotCount > 3)
                    {
                        m_loadingDotCount = 0;
                    }
                }
                for (int j = 0; j < m_loadingDotCount; ++j)
                {
                    loadingText += ".";
                }
                int textWidth = GetDrawStringWidth(loadingText.c_str(), -1);
                int textX = (screenW - textWidth) * 0.5f;
                int textY = screenH * 0.5f - 30;
                DrawString(textX, textY, loadingText.c_str(), 0xffffff);
                SetFontSize(16);
                ScreenFlip();
                if (ProcessMessage() == -1)
                {
                    break;
                }
            }
        }

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

	// デバッグウィンドウを表示
	DebugUtil::ShowDebugWindow();
}

void SceneManager::RequestChangeScene(SceneBase* newScene)
{
    m_pCurrentScene = newScene;
    m_pCurrentScene->Init();
    m_pNextScene = m_pCurrentScene; // Update()で上書きされないように設定
    m_isExternalSceneChange = true; // 外部からの変更要求があったことを示す
}
