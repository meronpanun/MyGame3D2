#include "SceneMain.h"
#include "SceneTitle.h"
#include "SceneOption.h"
#include "SceneResult.h"
#include "SceneGameOver.h" 
#include "DxLib.h"
#include "Player.h"
#include "Mouse.h"
#include "Game.h"
#include "EnemyBase.h"
#include "EnemyNormal.h"
#include "EnemyRunner.h"
#include "EnemyAcid.h"
#include "DebugUtil.h"
#include "Camera.h"
#include "FirstAidKitItem.h"
#include "Stage.h"
#include "WaveManager.h"
#include <cassert>
#include <algorithm>

namespace
{
	// UI関連の定数
	constexpr int	kButtonWidth        = 200;  // ボタンの幅 
	constexpr int	kButtonHeight       = 50;   // ボタンの高さ
	constexpr int	kFontSize           = 48;   // フォントサイズ
	constexpr float kScreenCenterOffset = 0.5f; // 画面中央のオフセット
	constexpr int   kButtonYOffset      = 70;   // ボタンのY座標オフセット
	constexpr int   kButtonSpacing      = 20;   // ボタン間のスペース

	// 戻るボタンとオプションボタンの座標
	constexpr int kReturnButtonX = 210; // 戻るボタンのX座標
	constexpr int kReturnButtonY = 290; // 戻るボタンのY座標
	constexpr int kOptionButtonX = 210; // オプションボタンのX座標
	constexpr int kOptionButtonY = 120; // オプションボタンのY座標

    // カメラの回転速度
	constexpr float kCameraRotaSpeed = 0.001f; 

    // スカイドームのY座標
	constexpr float kSkyDomePosY = 200.0f;  

    // スカイドームのスケール
	constexpr float kSkyDomeScale = 100.0f; 

    // アイテムドロップ時の初期上昇量
    constexpr float kDropInitialHeight = 140.0f; 

	// Road_floorオブジェトの範囲
	constexpr VECTOR kRoadFloorMin = { -500.0f, 0.0f, -500.0f }; // 床の最小座標
	constexpr VECTOR kRoadFloorMax = { 500.0f, 0.0f, 500.0f };   // 床の最大座標

    // 環境光設定
	constexpr float kAmbientLightR = 0.5f; // 環境光の赤成分
	constexpr float kAmbientLightG = 0.5f; // 環境光の緑成分
	constexpr float kAmbientLightB = 0.5f; // 環境光の青成分
	constexpr float kAmbientLightA = 1.0f; // 環境光のアルファ成分

    // ヒットマーク関連
	constexpr int kHitMarkLineLength    = 8; // ラインの長さ
	constexpr int kHitMarkCenterSpacing = 4; // 中央の間隔幅
	constexpr int kHitMarkLineThickness = 2; // ラインの太さ
}

SceneMain::SceneMain() :
    m_isPaused(false),
    m_isEscapePressed(false),
    m_isReturningFromOption(false),
    m_cameraSensitivity(Game::g_cameraSensitivity),
    m_pCamera(std::make_unique<Camera>()),
    m_skyDomeHandle(-1),
    m_dotHandle(-1),
    m_hitMarkTimer(0)
{
    // モデルの読み込み
    m_skyDomeHandle = MV1LoadModel("data/model/Dome.mv1");
    assert(m_skyDomeHandle != -1);

    // レティクル画像の読み込み
    m_dotHandle = LoadGraph("data/image/Dot.png");
    assert(m_dotHandle != -1);
}

SceneMain::~SceneMain()
{
	// モデルやリソースの解放
    MV1DeleteModel(m_skyDomeHandle);    
	DeleteGraph(m_dotHandle);
}

void SceneMain::Init()
{
    SetWaitVSyncFlag(true); // VSync有効化で描画負荷を安定化
    m_pPlayer = std::make_unique<Player>();
    m_pPlayer->Init();

	m_pEnemyNormal = std::make_shared<EnemyNormal>();
	m_pEnemyNormal->Init();

	m_pEnemyRunner = std::make_shared<EnemyRunner>();
	m_pEnemyRunner->Init();

	m_pEnemyAcid = std::make_shared<EnemyAcid>();
	m_pEnemyAcid->Init();

	m_pStage = std::make_shared<Stage>();
	m_pStage->Init();

	m_pWaveManager = std::make_shared<WaveManager>();
	m_pWaveManager->Init();
	
	// Road_floorオブジェクトの範囲を設定(マップ全体の範囲)
	m_pWaveManager->SetRoadFloorBounds(kRoadFloorMin, kRoadFloorMax);

	// カメラの初期化
    if (m_pPlayer->GetCamera())
    {
        m_pPlayer->GetCamera()->SetSensitivity(m_cameraSensitivity);
    }

    // マウスカーソルの表示/非表示を設定
	SetMouseDispFlag(m_isPaused); 

    // スカイドームのY座標を設定
	MV1SetPosition(m_skyDomeHandle, VGet(0, kSkyDomePosY, 0)); 

    // スカイドームのスケールを設定
	MV1SetScale(m_skyDomeHandle, VGet(kSkyDomeScale, kSkyDomeScale, kSkyDomeScale)); 

    m_isReturningFromOption = false;

    m_items.clear();

    // WaveManagerの敵の死亡時にアイテムをドロップするコールバックを設定
    m_pWaveManager->SetOnEnemyDeathCallback([this](const VECTOR& pos) {
        auto dropItem = std::make_shared<FirstAidKitItem>();
        dropItem->Init();
        VECTOR dropPos = pos;
        dropPos.y += kDropInitialHeight;
        dropItem->SetPos(dropPos);
        m_items.push_back(dropItem);
    });

    // ヒットマーク用コールバックをWaveManagerに設定
    m_pWaveManager->SetOnEnemyHitCallback([this](EnemyBase::HitPart part) { OnPlayerBulletHitEnemy(part); });

	// 環境光の設定
    SetLightAmbColor(GetColorF(kAmbientLightR, kAmbientLightG, kAmbientLightB, kAmbientLightA));
}

SceneBase* SceneMain::Update()
{
	// デバックウィンドウが表示されている場合は、更新をスキップ
    if (DebugUtil::IsDebugWindowVisible())
    {
        return this;
    }

    // スカイドームの回転
	MV1SetRotationXYZ(m_skyDomeHandle, VGet(0, MV1GetRotationXYZ(m_skyDomeHandle).y + kCameraRotaSpeed, 0)); 

    // エスケープキーが押されたかチェック
	if (CheckHitKey(KEY_INPUT_ESCAPE)) 
    {
        if (!m_isEscapePressed)
        {
            m_isPaused = !m_isPaused;
            SetMouseDispFlag(m_isPaused);
            m_isEscapePressed = true;

            if (m_isPaused)
            {
                
                m_pauseStartTime = std::chrono::steady_clock::now();
            }
            else
            {
                auto now = std::chrono::steady_clock::now();
                auto pauseDuration = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_pauseStartTime).count();
            }
        }
    }
    else
    {
        m_isEscapePressed = false;
    }

    if (m_isPaused)
    {
        if (Mouse::IsTriggerLeft())
        {
            Vec2 mousePos = Mouse::GetPos();

            if (mousePos.x >= kReturnButtonX && mousePos.x <= kReturnButtonX + kButtonWidth &&
                mousePos.y >= kReturnButtonY && mousePos.y <= kReturnButtonY + kButtonHeight)
            {
                return new SceneTitle(true);
            }

            if (mousePos.x >= kOptionButtonX && mousePos.x <= kOptionButtonX + kButtonWidth &&
                mousePos.y >= kOptionButtonY && mousePos.y <= kOptionButtonY + kButtonHeight)
            {
                m_isReturningFromOption = true; 
                return new SceneOption(this);
            }
        }
        return this;
    }

    // WaveManagerの敵リストを取得してプレイヤーに渡す
    std::vector<std::shared_ptr<EnemyBase>>& enemyList = m_pWaveManager->GetEnemyList();
    std::vector<EnemyBase*> enemyPtrList;

	// 敵のポインタをプレイヤーに渡すためのリストを作成
    for (std::shared_ptr<EnemyBase>& enemy : enemyList)
    {
        enemyPtrList.push_back(enemy.get());
    }
    
    m_pPlayer->Update(enemyPtrList);

    if (m_pPlayer->GetHealth() <= 0.0f)
    {
        return new SceneGameOver();
    }

    // WaveManagerの更新
    m_pWaveManager->Update();

    // WaveManagerの敵を一括更新
    m_pWaveManager->UpdateEnemies(m_pPlayer->GetBullets(), m_pPlayer->GetTackleInfo(), *m_pPlayer);

    for (std::shared_ptr<ItemBase>& item : m_items)
    {
        item->Update(m_pPlayer.get());
    }
    // 取得済みアイテムを削除
    m_items.erase(
        std::remove_if(m_items.begin(), m_items.end(), [](const std::shared_ptr<ItemBase>& item) {
            return item->IsUsed();
        }),
        m_items.end()
    );

    if (m_hitMarkTimer > 0) --m_hitMarkTimer;

    return this;
}

void SceneMain::Draw()
{
    int screenW, screenH;
    GetScreenState(&screenW, &screenH, nullptr);

    m_pStage->Draw();

    MV1DrawModel(m_skyDomeHandle); 

    for (std::shared_ptr<ItemBase>& item : m_items) 
    {
        item->Draw();
    }

    // WaveManagerの敵を一括描画
    m_pWaveManager->DrawEnemies();

    m_pPlayer->Draw();

    constexpr int kDotSize = 64;
    int cx = screenW / 2;
    int cy = screenH / 2;
    DrawGraph(cx - kDotSize / 2, cy - kDotSize / 2, m_dotHandle, true);

    // ヒットマーク描画
    if (m_hitMarkTimer > 0)
    {
        // 赤 or 黄色
        unsigned int color = (m_hitMarkType == EnemyBase::HitPart::Head) ? 0xffff40 : 0xff2020;

        // 左上→右下
        DrawLine(cx - kHitMarkLineLength, cy - kHitMarkLineLength, 
            cx - kHitMarkCenterSpacing, cy - kHitMarkCenterSpacing, 
            color, kHitMarkLineThickness);

        DrawLine(cx + kHitMarkCenterSpacing, cy + kHitMarkCenterSpacing, 
            cx + kHitMarkLineLength, cy + kHitMarkLineLength, 
            color, kHitMarkLineThickness);

        // 左下→右上
        DrawLine(cx - kHitMarkLineLength, cy + kHitMarkLineLength, 
            cx - kHitMarkCenterSpacing, cy + kHitMarkCenterSpacing, 
            color, kHitMarkLineThickness);

        DrawLine(cx + kHitMarkCenterSpacing, cy - kHitMarkCenterSpacing,
            cx + kHitMarkLineLength, cy - kHitMarkLineLength, 
            color, kHitMarkLineThickness);
    }

#ifdef _DEBUG
    // デバッグ情報を表示
    m_pWaveManager->DrawDebugInfo();
#endif // DEBUG
    
    if (m_isPaused)
    {
        DrawPauseMenu();
    }
}

void SceneMain::DrawShadowCasters()
{
    // 影を落とすものだけ描画
    m_pStage->Draw();
    m_pPlayer->Draw();
}

void SceneMain::DrawPauseMenu()
{
    Vec2 mousePos = Mouse::GetPos();

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128); 
    DrawBox(50, 50, Game::kScreenWidth - 50, Game::kScreenHeigth - 50, 0x000000, true);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0); 
}

void SceneMain::SetPaused(bool paused)
{
    m_isPaused = paused;
    SetMouseDispFlag(m_isPaused); 
}


void SceneMain::SetCameraSensitivity(float sensitivity)
{
    m_cameraSensitivity = sensitivity;
    if (m_pCamera)
    {
        m_pCamera->SetSensitivity(sensitivity);
    }
}

void SceneMain::OnPlayerBulletHitEnemy(EnemyBase::HitPart part)
{
    m_hitMarkTimer = kHitMarkDuration;
    m_hitMarkType = part;
}
