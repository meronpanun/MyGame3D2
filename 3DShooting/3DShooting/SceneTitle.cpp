#include "SceneTitle.h"
#include "DxLib.h"
#include "Game.h"
#include "SceneMain.h"
#include "Mouse.h"
#include <cassert>

namespace
{
    // タイトルロゴの幅と高さ
	constexpr int kLogoWidth  = 1920;
    constexpr int kLogoHeight = 1080;

	// タイトルロゴの表示位置
	constexpr int kLogoX = Game::kScreenWidth * -0.5f;
	constexpr int kLogoY = (Game::kScreenHeigth - kLogoHeight) * 0.5f;

	constexpr int kFadeDuration = 60; // フェードイン・フェードアウトのフレーム数
	constexpr int kWaitDuration = 60; // フェードイン後の待機時間（フレーム数）

    // パネルの幅と高さ
    constexpr int kPanelWidth  = 350; // パネルの幅
    constexpr int kPanelHeight = 240; // パネルの高さ

    // スタートボタンの範囲を定数化
    constexpr int kStartButtonX1 = (Game::kScreenWidth - kPanelWidth) * 0.5f; // スタートボタンの左上X座標
    constexpr int kStartButtonY1 = 10;                                        // スタートボタンの左上Y座標
    constexpr int kStartButtonX2 = kStartButtonX1 + kPanelWidth;              // スタートボタンの右下X座標
    constexpr int kStartButtonY2 = 300;                                       // スタートボタンの右下Y座標
}

SceneTitle::SceneTitle(bool skipLogo):
    m_logoHandle(-1),
	m_fadeAlpha(0),
	m_fadeFrame(0),
	m_sceneFadeAlpha(0),
	m_waitFrame(0),
	m_isFadeComplete(false),
	m_isFadeOut(false),
	m_skipLogo(skipLogo),
	m_isSceneFadeIn(false)
{
    // タイトルロゴ画像を読み込む
    m_logoHandle = LoadGraph("data/image/TitleLogo.png");
    assert(m_logoHandle != -1);

    // ロゴをスキップする場合、フェード処理と待機時間をスキップし、ボタン操作を有効化
    if (m_skipLogo)
    {
        m_isFadeComplete = true;
        m_isFadeOut      = true;
        m_waitFrame      = kWaitDuration; // 待機時間をスキップ
        m_fadeAlpha      = 0;             // フェードアウト済みの状態に設定
        m_isSceneFadeIn  = true;          // タイトルシーンのフェードインを開始
        m_sceneFadeAlpha = 255;           // フェードインを完全に表示
    }
}

SceneTitle::~SceneTitle()
{
    // タイトルロゴ画像を解放する
	DeleteGraph(m_logoHandle);
}

void SceneTitle::Init()
{
    // マウスカーソルを表示する
    SetMouseDispFlag(true);
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
            m_fadeAlpha      = 255;
            m_isFadeComplete = true; // フェードインが完了
            m_fadeFrame      = 0;    // フェードアウト用にリセット
        }
        return this;
    }

    // フェードイン後の待機時間をカウント
    if (m_waitFrame < kWaitDuration)
    {
        m_waitFrame++;
        return this; // 待機時間中はシーン遷移しない
    }

    // タイトルロゴのフェードアウト処理
    if (!m_isFadeOut)
    {
        if (m_fadeFrame < kFadeDuration)
        {
            m_fadeAlpha = static_cast<int>(255.0f * (1.0f - (m_fadeFrame / static_cast<float>(kFadeDuration))));
            m_fadeFrame++;
        }
        else
        {
            m_fadeAlpha = 0;
            m_isFadeOut = true; // フェードアウトが完了
            m_fadeFrame = 0;    // フェードイン用にリセット
        }
        return this;
    }

    // タイトルシーンのフェードイン処理
    if (!m_isSceneFadeIn)
    {
        if (m_sceneFadeAlpha < 255)
        {
            m_sceneFadeAlpha += 5; // フェードインの速度を調整
        }
        else
        {
            m_sceneFadeAlpha = 255; // フェードイン完了
            m_isSceneFadeIn  = true;
        }
        return this;
    }


    // マウスの左クリックをチェック
    if (Mouse::IsTriggerLeft())
    {
        // マウスの位置を取得
        Vec2 mousePos = Mouse::GetPos();

        if (mousePos.x >= kStartButtonX1 && mousePos.x <= kStartButtonX2 &&
            mousePos.y >= kStartButtonY1 && mousePos.y <= kStartButtonY2)
        {
            return new SceneMain();
        }
        //// マウスがオプションボタンとパネルを囲む背景の範囲内にあるかチェック
        //if (mousePos.x >= kBackgroundX1 && mousePos.x <= kBackgroundX2 &&
        //    mousePos.y >= kBackgroundY1 && mousePos.y <= kBackgroundY2)
        //{
        //    return new SceneOption(this, m_currentReticleType);
        //}
    }
    // 何もしなければシーン遷移しない(タイトル画面のまま)
    return this;
}

void SceneTitle::Draw()
{
    // マウスの位置を取得
    Vec2 mousePos = Mouse::GetPos();

    // タイトルロゴのフェードイン・フェードアウト描画
    if (!m_isFadeOut)
    {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, m_fadeAlpha);
        DrawRectExtendGraph(
            0, 0,                       
            Game::kScreenWidth, Game::kScreenHeigth,
            0, 0,                       
            kLogoWidth, kLogoHeight,    
            m_logoHandle, true          
        );
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

        // フェード処理が終わっていない場合はここで描画を終了
        return;
    }

    // ボタンの描画
    if (m_isFadeOut || m_skipLogo)
    {
        // スタートボタンの描画
        unsigned int buttonColor = 0xadadad; // 通常時のボタン色（白）
        if (mousePos.x >= kStartButtonX1 && mousePos.x <= kStartButtonX2 &&
            mousePos.y >= kStartButtonY1 && mousePos.y <= kStartButtonY2)
        {
            buttonColor = 0xffffff; // ホバー時のボタン色（グレー）
        }

        // ボタンの背景を描画
        DrawBox(kStartButtonX1, kStartButtonY1, kStartButtonX2, kStartButtonY2, buttonColor, true);

        // ボタンのテキストを描画
        const char* buttonText = "START";
        int textWidth = GetDrawStringWidth(buttonText, strlen(buttonText));
        int textX     = (kStartButtonX1 + kStartButtonX2) * 0.5f - textWidth * 0.5f;
        int textY     = (kStartButtonY1 + kStartButtonY2) * 0.5f - 10; // テキストの高さを調整
        DrawString(textX, textY, buttonText, GetColor(0, 0, 0)); // テキスト色は黒
    }
}
