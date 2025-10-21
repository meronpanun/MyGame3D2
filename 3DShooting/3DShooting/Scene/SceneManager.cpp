#include "SceneManager.h"
#include <string>
#include "SceneTitle.h"
#include "SceneMain.h"
#include "SceneResult.h"
#include "SceneOption.h"
#include "SceneGameOver.h"
#include "DebugUtil.h"
#include "Mouse.h"
#include <DxLib.h>

SceneManager::SceneManager() :
	m_pTitle(nullptr),
	m_pOption(nullptr),
	m_pSceneMain(nullptr),
	m_pResult(nullptr),
	m_pGameOver(nullptr),
	m_pCurrentScene(nullptr),
	m_pNextScene(nullptr),
	m_pSceneToChange(nullptr),
    m_isExternalSceneChange(false),
	m_loadingDotCount(0),
	m_loadingAnimTimer(0),
	m_fadeState(FadeState::Idle),
	m_fadeAlpha(0),
	m_fadeSpeed(5)
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
	m_fadeState = FadeState::FadingIn;
	m_fadeAlpha = 255;
}

void SceneManager::Update()
{
    // マウスの入力状態を更新
    Mouse::Update();

	// フェードイン・アウト処理
	if (m_fadeState == FadeState::FadingIn)
	{
		m_fadeAlpha -= m_fadeSpeed;
		if (m_fadeAlpha <= 0)
		{
			m_fadeAlpha = 0;
			m_fadeState = FadeState::Idle;
		}
	}
	else if (m_fadeState == FadeState::FadingOut)
	{
		m_fadeAlpha += m_fadeSpeed;
		if (m_fadeAlpha >= 255)
		{
			m_fadeAlpha = 255;
			
			// ローディング画面表示（SceneMainの場合のみ）
			if (dynamic_cast<SceneMain*>(m_pSceneToChange))
			{
				for (int i = 0; i < 180; ++i)
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

			m_pCurrentScene = m_pSceneToChange;
			m_pCurrentScene->Init();
			m_pSceneToChange = nullptr;
			m_fadeState = FadeState::FadingIn;
		}
	}

    // 現在のシーンを更新 (フェードアウト中は更新しない)
    if (m_pCurrentScene != nullptr && m_fadeState != FadeState::FadingOut)
    {
        m_pNextScene = m_pCurrentScene->Update();
    }

    // シーンが変わった場合、フェードアウトを開始
    if (m_pNextScene != nullptr && m_pNextScene != m_pCurrentScene)
    {
		if (m_fadeState == FadeState::Idle)
		{
			m_pSceneToChange = m_pNextScene;
			m_fadeState = FadeState::FadingOut;
		}
    }
}

void SceneManager::Draw()
{
	// 現在のシーンを描画
	if (m_pCurrentScene != nullptr)
	{
		m_pCurrentScene->Draw();
	}

	// フェード処理
	if (m_fadeAlpha > 0)
	{
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, m_fadeAlpha);
		DrawBox(0, 0, 1920, 1080, 0x000000, true);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	// デバッグウィンドウを表示
	DebugUtil::ShowDebugWindow();
}

// 外部からシーン変更をリクエストする関数
void SceneManager::RequestChangeScene(SceneBase* newScene)
{
	if (m_fadeState == FadeState::Idle)
	{
		m_pSceneToChange = newScene;
		m_fadeState = FadeState::FadingOut;
		m_isExternalSceneChange = true; // 外部からの変更要求があったことを示す
	}
}