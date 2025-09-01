#include "SceneManager.h"
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
    m_isExternalSceneChange(false)
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

    // 外部からのシーン変更要求があった場合
    if (m_isExternalSceneChange)
    {
        m_isExternalSceneChange = false; // フラグをリセット
    }
    else // 通常のシーン更新
    {
        // 現在のシーンを更新
        if (m_pCurrentScene != nullptr)
        {
            m_pNextScene = m_pCurrentScene->Update();
        }
    }

	// シーンが変わった場合、初期化処理を行う
	if (m_pNextScene != nullptr && m_pNextScene != m_pCurrentScene)
	{
        if (m_pCurrentScene == m_pTitle) 
		{
            delete m_pTitle; m_pTitle = nullptr;
        } 
		else if (m_pCurrentScene == m_pOption) 
		{
            delete m_pOption; m_pOption = nullptr;
        }
		else if (m_pCurrentScene == m_pSceneMain) 
		{
            delete m_pSceneMain; m_pSceneMain = nullptr;
        }
		else if (m_pCurrentScene == m_pResult) 
		{
            delete m_pResult; m_pResult = nullptr;
        }
		else if (m_pCurrentScene == m_pGameOver) 
		{
            delete m_pGameOver; m_pGameOver = nullptr;
        }

		m_pCurrentScene = m_pNextScene;
		
		// 新しいシーンがSceneMainの場合、ローディング画面を即座に表示
		if (dynamic_cast<SceneMain*>(m_pCurrentScene))
		{
			// ローディング画面を表示
			ClearDrawScreen();
			int screenW, screenH;
			GetScreenState(&screenW, &screenH, nullptr);
			
			// 背景を黒で塗りつぶし
			DrawBox(0, 0, screenW, screenH, 0x000000, true);
			
			// ローディングテキストを中央に表示
			SetFontSize(48);
			const char* loadingText = "Now Loading...";
			int textWidth = GetDrawStringWidth(loadingText, 12);
			int textX = (screenW - textWidth) / 2;
			int textY = screenH / 2 - 30;
			DrawString(textX, textY, loadingText, 0xffffff);

			// プログレスバー風の表示（点々をアニメーション）
			static int loadingDots = 0;
			static int loadingTimer = 0;
			loadingTimer++;
			if (loadingTimer >= 20)
			{
				loadingTimer = 0;
				loadingDots = (loadingDots + 1) % 4;
			}

			std::string dots = std::string(loadingDots, '.');
			DrawString(textX + textWidth + 10, textY, dots.c_str(), 0x888888);
			
			SetFontSize(16);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
			ScreenFlip(); // ローディング画面を即座に表示
		}
		
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
