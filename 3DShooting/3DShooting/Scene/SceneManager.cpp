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
        // このフレームでは m_pCurrentScene->Update() を実行せず、
        // m_pCurrentScene は既に RequestChangeScene で設定されているため、
        // 以下のシーン切り替えロジックも発動しない。
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
        // ここで古いシーンを delete するロジックは、SceneManagerが所有するシーンのみを対象としています。
        // SceneManagerのメンバーではない動的にnewされたシーンは、ここで delete されずメモリリークする可能性があります。
        // これは既存の設計上の問題であり、今回の修正の範囲外とします。
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
    // ここでの delete 処理は削除します。
    // シーンの解放は SceneManager::Update() またはデストラクタで行われるべきです。
    // 現在の設計では、SceneManagerがnewしたシーン（m_pTitleなど）以外はメモリリークする可能性がありますが、
    // これは既存の設計上の問題であり、今回のデバッグ機能の範囲外とします。

    m_pCurrentScene = newScene;
    m_pCurrentScene->Init();
    m_pNextScene = m_pCurrentScene; // Update()で上書きされないように設定
    m_isExternalSceneChange = true; // 外部からの変更要求があったことを示す
}
