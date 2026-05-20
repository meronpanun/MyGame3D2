#include "SceneManager.h"
#include "DebugUtil.h"
#include "Game.h"
#include "InputManager.h"
#include "SoundManager.h"
#include "SceneBase.h"
#include "SceneTitle.h"
#include <DxLib.h>
#include <string>

namespace
{
    constexpr int   kFadeSpeed       =   5;           // フェード速度（フレームあたりのアルファ変化量）
    constexpr int   kFadeAlphaMax    = 255;           // フェードアルファの最大値（完全不透明）
    constexpr int   kFadeAlphaMin    =   0;           // フェードアルファの最小値（完全透明）
    constexpr float kUpdateDeltaTime = 1.0f / 60.0f; // サウンド更新用デルタタイム
}

SceneManager::SceneManager()
    : m_pCurrentScene(nullptr)
    , m_pNextScene(nullptr)
    , m_pSceneToChange(nullptr)
    , m_loadingDotCount(0)
    , m_loadingAnimTimer(0)
    , m_fadeState(FadeState::Idle)
    , m_fadeAlpha(kFadeAlphaMin)
    , m_fadeSpeed(kFadeSpeed)
{
}

SceneManager::~SceneManager()
{
    // unique_ptr が m_pCurrentScene と m_pSceneToChange を自動解放する
    SoundManager::GetInstance()->Release();
}

void SceneManager::Init()
{
    SoundManager::GetInstance()->Load("data/CSV/SoundList.csv");

    m_pCurrentScene = std::make_unique<SceneTitle>();
    m_pCurrentScene->Init();
    m_fadeState = FadeState::FadingIn;
    m_fadeAlpha = kFadeAlphaMax;
}

void SceneManager::Update()
{
    SoundManager::GetInstance()->Update(kUpdateDeltaTime);
    InputManager::GetInstance()->Update();

    // フェードイン・アウト処理
    if (m_fadeState == FadeState::FadingIn)
    {
        m_fadeAlpha -= m_fadeSpeed;
        if (m_fadeAlpha <= kFadeAlphaMin)
        {
            m_fadeAlpha = kFadeAlphaMin;
            m_fadeState = FadeState::Idle;
        }
    }
    else if (m_fadeState == FadeState::FadingOut)
    {
        m_fadeAlpha += m_fadeSpeed;
        if (m_fadeAlpha >= kFadeAlphaMax)
        {
            m_fadeAlpha = kFadeAlphaMax;

            // エフェクト停止（SceneBase の virtual 呼び出し。エフェクトを持たないシーンは no-op）
            m_pCurrentScene->StopAllEffects();

            SoundManager::GetInstance()->StopAllSE();

            // 所有権を移譲して現在シーンを破棄し、遷移先シーンへ切り替える
            m_pCurrentScene = std::move(m_pSceneToChange);
            m_pCurrentScene->Init();

            m_fadeState = m_pCurrentScene->IsLoading() ? FadeState::Loading : FadeState::FadingIn;

            // シーン遷移直後の誤トリガーを防ぐために入力ログを更新する
            InputManager::GetInstance()->Update();
        }
    }

    // 現在のシーンを更新（フェードアウト中は更新しない）
    if (m_pCurrentScene && m_fadeState != FadeState::FadingOut)
    {
        m_pNextScene = m_pCurrentScene->Update();

        // ロード完了を監視（描画ラグを防ぐためシーン更新直後にチェック）
        if (m_fadeState == FadeState::Loading && !m_pCurrentScene->IsLoading())
        {
            m_fadeState = FadeState::FadingIn;
        }
    }

    // シーンが切り替わった場合はフェードアウトを開始する
    if (m_pNextScene != nullptr && m_pNextScene != m_pCurrentScene.get())
    {
        if (m_fadeState == FadeState::Idle)
        {
            m_pSceneToChange.reset(m_pNextScene);
            m_fadeState = FadeState::FadingOut;
        }
    }
}

void SceneManager::Draw()
{
    if (m_pCurrentScene) m_pCurrentScene->Draw();

    // ロード中はシーン側で描画するためフェード膜は重ねない
    if (m_fadeAlpha > 0 && m_fadeState != FadeState::Loading)
    {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, m_fadeAlpha);
        DrawBox(0, 0, Game::GetScreenWidth(), Game::GetScreenHeight(), 0x000000, true);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }

    DebugUtil::ShowDebugWindow();
}

void SceneManager::RequestChangeScene(SceneBase* newScene)
{
    if (m_fadeState == FadeState::Idle)
    {
        m_pSceneToChange.reset(newScene);
        m_fadeState = FadeState::FadingOut;
    }
}
