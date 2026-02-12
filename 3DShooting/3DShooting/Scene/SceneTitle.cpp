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
    : m_fontHandle(-1)
    , m_titleLogo(-1)
    , m_bannerHandle(-1)
    , m_bgmHandle(-1)
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
    // タイトルロゴ画像を読み込む
    m_titleLogo = LoadGraph("data/image/TitleLogo.png");
    assert(m_titleLogo != -1);

    // タイトルBGMを読み込む
    m_bgmHandle = LoadSoundMem("data/sound/BGM/TitleBGM.wav");
    assert(m_bgmHandle != -1);

    // 決定ボタンSEを読み込む
    m_confirmSEHandle = LoadSoundMem("data/sound/SE/ConfirmButton.mp3");
    assert(m_confirmSEHandle != -1);

    // フォントの作成
    m_fontHandle = CreateFontToHandle("Arial Black", 30, 3, DX_FONTTYPE_ANTIALIASING_EDGE_8X8);
}

SceneTitle::~SceneTitle()
{
    // フォントハンドルの解放
    if (m_fontHandle != -1)
    {
        DeleteFontToHandle(m_fontHandle);
        m_fontHandle = -1;
    }

    // 画像の解放
    if (m_titleLogo != -1)
    {
        DeleteGraph(m_titleLogo);
        m_titleLogo = -1;
    }

    // サウンドの解放
    if (m_bgmHandle != -1)
    {
        DeleteSoundMem(m_bgmHandle);
        m_bgmHandle = -1;
    }
    if (m_confirmSEHandle != -1)
    {
        DeleteSoundMem(m_confirmSEHandle);
        m_confirmSEHandle = -1;
    }
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
        if (CheckSoundMem(m_bgmHandle) == 0)
        {
            PlaySoundMem(m_bgmHandle, DX_PLAYTYPE_LOOP);
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
        StopSoundMem(m_bgmHandle);
        PlaySoundMem(m_confirmSEHandle, DX_PLAYTYPE_BACK);
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
        int textWidth = GetDrawStringWidthToHandle(gameStartText, -1, m_fontHandle);

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, m_gameStartTextAlpha);
        DrawFormatStringToHandle((Game::GetScreenWidth() - textWidth) * 0.5f, Game::GetScreenHeight() - 180, 0xffffff, m_fontHandle, gameStartText);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }
}

