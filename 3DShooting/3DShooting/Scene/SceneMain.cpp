#include "SceneMain.h"
#include "SceneTitle.h"
#include "SceneResult.h"
#include "SceneOption.h"
#include "DxLib.h"
#include "Player.h"
#include "Mouse.h"
#include "Game.h"
#include "EnemyBase.h"
#include "EnemyNormal.h"
#include "Camera.h"
#include <cassert>

namespace
{
    constexpr int	kInitialCountdownValue = 3;	   // 初期カウントダウンの値
    constexpr int	kMainCountdownValue    = 10;   // メインカウントダウンの値
    constexpr int	kButtonWidth           = 200;  // ボタンの幅
    constexpr int	kButtonHeight          = 50;   // ボタンの幅
    constexpr int	kFontSize              = 48;   // フォントサイズ
    constexpr float kScreenCenterOffset    = 0.5f; // 画面中央のオフセット
    constexpr int   kButtonYOffset         = 70;   // ボタンのY軸オフセット
    constexpr int   kButtonSpacing         = 20;   // ボタン間のスペース

    // ボタンの位置を計算
    constexpr int kReturnButtonX = 210; // 戻るボタンの背景の左上X座標
    constexpr int kReturnButtonY = 290; // 戻るボタンの背景の左上Y座標
    constexpr int kOptionButtonX = 210; // オプションボタンの背景の左上X座標
    constexpr int kOptionButtonY = 120; // オプションボタンの背景の左上Y座標

    //カメラを回す速度
    constexpr float kCameraRotaSpeed = 0.001f;

    // スカイドームの位置
	constexpr float kSkyDomePosY = 200.0f; // スカイドームのY座標

    // スカイドームの大きさ
	constexpr float kSkyDomeScale = 10.0f; // スカイドームのスケール
}

SceneMain::SceneMain() :
    m_isPaused(false),
    m_isEscapePressed(false),
    m_isReturningFromOption(false),
    m_cameraSensitivity(Game::g_cameraSensitivity),
    m_pCamera(std::make_unique<Camera>()),
    m_skyDomeHandle(-1),
    m_dotHandle(-1)
{
	// スカイドームのモデルを読み込む
    m_skyDomeHandle = MV1LoadModel("data/model/Dome.mv1");
    assert(m_skyDomeHandle != -1);

	// ドット型の画像を読み込む
    m_dotHandle = LoadGraph("data/image/Dot.png");
	assert(m_dotHandle != -1);
}

SceneMain::~SceneMain()
{
    // スカイドームのモデルを削除
    MV1DeleteModel(m_skyDomeHandle);    

    // ドット型の画像を削除
	DeleteGraph(m_dotHandle);
}

void SceneMain::Init()
{
    // プレイヤーの初期化
    m_pPlayer = std::make_unique<Player>();
    m_pPlayer->Init();

	// 通常ゾンビの初期化
	m_pEnemyNormal = std::make_shared<EnemyNormal>();
	m_pEnemyNormal->Init();

    // グローバル変数からカメラ感度を取得して設定
    if (m_pPlayer->GetCamera())
    {
        m_pPlayer->GetCamera()->SetSensitivity(m_cameraSensitivity);
    }

    // ポーズ状態に応じてマウスカーソルの表示を切り替える
    SetMouseDispFlag(m_isPaused);

    //スカイドームの位置
    MV1SetPosition(m_skyDomeHandle, VGet(0, kSkyDomePosY, 0));

    // スカイドームの大きさを設定
    MV1SetScale(m_skyDomeHandle, VGet(kSkyDomeScale, kSkyDomeScale, kSkyDomeScale));

    // フラグをリセット
    m_isReturningFromOption = false;
}

SceneBase* SceneMain::Update()
{
    // スカイドームの回転を更新
    MV1SetRotationXYZ(m_skyDomeHandle, VGet(0, MV1GetRotationXYZ(m_skyDomeHandle).y + kCameraRotaSpeed, 0));

    // エスケープキーが押されたらポーズ状態を切り替える
    if (CheckHitKey(KEY_INPUT_ESCAPE))
    {
        if (!m_isEscapePressed)
        {
            m_isPaused = !m_isPaused;
            SetMouseDispFlag(m_isPaused); // ポーズ状態に応じてマウスカーソルの表示を切り替える
            m_isEscapePressed = true;

            if (m_isPaused)
            {
                // ポーズに入ったタイミングで現在の時間を記録
                m_pauseStartTime = std::chrono::steady_clock::now();
            }
            else
            {
                // ポーズから戻ったタイミングで経過時間を計算し、カウントダウンの開始時間を調整
                auto now = std::chrono::steady_clock::now();
                auto pauseDuration = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_pauseStartTime).count();
            }
        }
    }
    else
    {
        m_isEscapePressed = false;
    }

    // ポーズ中は更新処理を行わない
    if (m_isPaused)
    {
        // マウスの左クリックをチェック
        if (Mouse::IsTriggerLeft())
        {
            // マウスの位置を取得
            Vec2 mousePos = Mouse::GetPos();

            // マウスがタイトルに戻るボタンの範囲内にあるかチェック
            if (mousePos.x >= kReturnButtonX && mousePos.x <= kReturnButtonX + kButtonWidth &&
                mousePos.y >= kReturnButtonY && mousePos.y <= kReturnButtonY + kButtonHeight)
            {
                return new SceneTitle(true); // ロゴスキップフラグを有効にしてタイトルシーンに戻る
            }

            // マウスがオプションボタンの範囲内にあるかチェック
            if (mousePos.x >= kOptionButtonX && mousePos.x <= kOptionButtonX + kButtonWidth &&
                mousePos.y >= kOptionButtonY && mousePos.y <= kOptionButtonY + kButtonHeight)
            {
                m_isReturningFromOption = true; // オプションシーンから戻るフラグを設定
                return new SceneOption(this);
            }
        }
        return this;
    }

    // プレイヤーの更新
    m_pPlayer->Update();

	// 通常ゾンビの更新
	m_pEnemyNormal->Update(m_pPlayer->GetBullets());

    // 何もしなければシーン遷移しない(ゲーム画面のまま)
    return this;
}

void SceneMain::Draw()
{
    // 画面中央座標を取得
    int screenW, screenH;
    GetScreenState(&screenW, &screenH, nullptr);

    // スカイドームを描画
	MV1DrawModel(m_skyDomeHandle); 

	// 通常ゾンビの描画
	m_pEnemyNormal->Draw();

    // プレイヤーの描画
    m_pPlayer->Draw();

    // ドット型のレティクルを描画
    constexpr int kDotSize = 64;
    DrawGraph(screenW * 0.5f - kDotSize * 0.5f, screenH * 0.5f - kDotSize * 0.5f, m_dotHandle, true);

    // ポーズ中はポーズメニューを描画する
    if (m_isPaused)
    {
        DrawPauseMenu();
    }
}

void SceneMain::DrawPauseMenu()
{
    // マウスの位置を取得
    Vec2 mousePos = Mouse::GetPos();

    // 半透明な板を描画
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128); // 透明度を設定
    DrawBox(50, 50, Game::kScreenWidth - 50, Game::kScreenHeigth - 50, 0x000000, true);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0); // 透明度をリセット
}

// ポーズ状態を設定する
void SceneMain::SetPaused(bool paused)
{
    m_isPaused = paused;
    SetMouseDispFlag(m_isPaused); // ポーズ状態に応じてマウスカーソルの表示を切り替える
}

// カメラの感度を設定する
void SceneMain::SetCameraSensitivity(float sensitivity)
{
    m_cameraSensitivity = sensitivity;
    if (m_pCamera)
    {
        m_pCamera->SetSensitivity(sensitivity);
    }
}