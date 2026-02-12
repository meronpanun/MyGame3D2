#include "SceneTitle.h"
#include "EffekseerForDXLib.h"
#include "Game.h"
#include "SceneMain.h"
#include "InputManager.h"
#include "DebugUtil.h"
#include <cassert>

namespace
{
    // タイトルロゴの幅と高さ
    constexpr int kLogoWidth = 1050;
    constexpr int kLogoHeight = 1080;

    // フェード関連
    constexpr int kFadeDuration = 60; // フェードイン・フェードアウトのフレーム数
    constexpr int kWaitDuration = 60; // フェードイン後の待機時間（フレーム数）

    // ゲームスタートテキストの点滅速度
    constexpr int kGameStartTextBlinkSpeed = 4;
}

SceneTitle::SceneTitle(bool isReturningFromOtherScene)
    : m_font("Arial Black", 30, 3, DX_FONTTYPE_ANTIALIASING_EDGE_8X8)
    , m_titleLogo("data/image/TitleLogo.png")
    , m_bgm("data/sound/BGM/TitleBGM.wav")
    , m_confirmSE("data/sound/SE/ConfirmButton.mp3")
    , m_fadeAlpha(0)
    , m_fadeFrame(0)
    , m_sceneFadeAlpha(0)
    , m_waitFrame(0)
    , m_isFadeComplete(false)
    , m_isFadeOut(false)
    , m_isSceneFadeIn(false)
    , m_isBGMStarted(false)
    , m_gameStartTextAlpha(0)
    , m_gameStartTextAlphaDir(1)
{
    // ロード確認
    assert(m_titleLogo.IsValid());
    assert(m_bgm.IsValid());
    assert(m_confirmSE.IsValid());
}

SceneTitle::~SceneTitle()
{
    // 自動解放されるため処理不要
}

void SceneTitle::Init()
{
}

SceneBase* SceneTitle::Update()
{
    // タイトルロゴのフェードイン処理
    if (!m_isFadeComplete)
    {
        if (m_fadeFrame < kFadeDuration)
        {
            m_fadeAlpha = static_cast<int>(255.0f * (m_fadeFrame / static_cast<float>(kFadeDuration)));
            m_fadeFrame++;
        }
        else
        {
            m_fadeAlpha = 255;
            m_isFadeComplete = true; // フェードインが完了
            m_fadeFrame = 0;    // フェードアウト用にリセット
        }
        return this;
    }

    // フェードイン後の待機時間をカウント
    if (m_waitFrame < kWaitDuration)
    {
        m_waitFrame++;
        return this; // 待機時間中はシーン遷移しない
    }

    // BGM再生（ロゴ演出が終わった直後に一度だけ）
    if (!m_isBGMStarted)
    {
        if (!m_bgm.IsPlaying())
        {
            m_bgm.Play(DX_PLAYTYPE_LOOP);
        }
        m_isBGMStarted = true;
    }

    // 「ゲームスタート」文字の点滅処理
    m_gameStartTextAlpha += m_gameStartTextAlphaDir * kGameStartTextBlinkSpeed;
    if (m_gameStartTextAlpha > 255)
    {
        m_gameStartTextAlpha = 255;
        m_gameStartTextAlphaDir = -1;
    }
    else if (m_gameStartTextAlpha < 0)
    {
        m_gameStartTextAlpha = 0;
        m_gameStartTextAlphaDir = 1;
    }

    // マウスの左クリックをチェック
    // デバッグメニューが表示されていない場合のみ有効
    if (!DebugUtil::IsDebugWindowVisible() && InputManager::GetInstance()->IsTriggerMouseLeft())
    {
        // BGMを停止
        m_bgm.Stop();
        m_confirmSE.Play(DX_PLAYTYPE_BACK);
        return new SceneMain();
    }
    // 何もしなければシーン遷移しない(タイトル画面のまま)
    return this;
}

void SceneTitle::Draw()
{
    // マウスの位置を取得
    Vec2 mousePos = InputManager::GetInstance()->GetMousePos();

    // タイトルロゴの描画
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, m_fadeAlpha);
    DrawRectExtendGraph(
        0, 0,
        Game::GetScreenWidth(), Game::GetScreenHeight(),
        0, 0,
        kLogoWidth, kLogoHeight,
        m_titleLogo, true
    );
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    if (m_isFadeComplete)
    {
        const char* gameStartText = "Press Left Click to Start Game";
        int textWidth = GetDrawStringWidthToHandle(gameStartText, -1, m_font);

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, m_gameStartTextAlpha);
        DrawFormatStringToHandle((Game::GetScreenWidth() - textWidth) * 0.5f, Game::GetScreenHeight() - 180, 0xffffff, m_font, gameStartText);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }
}

