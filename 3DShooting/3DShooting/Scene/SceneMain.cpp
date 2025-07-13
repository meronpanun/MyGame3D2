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
    constexpr int	kButtonWidth           = 200; 
    constexpr int	kButtonHeight          = 50;   
    constexpr int	kFontSize              = 48;   
    constexpr float kScreenCenterOffset    = 0.5f; 
    constexpr int   kButtonYOffset         = 70;   
    constexpr int   kButtonSpacing         = 20;   

    constexpr int kReturnButtonX = 210; 
    constexpr int kReturnButtonY = 290; 
    constexpr int kOptionButtonX = 210; 
    constexpr int kOptionButtonY = 120; 

   
    constexpr float kCameraRotaSpeed = 0.001f;

   
	constexpr float kSkyDomePosY = 200.0f; 

   
	constexpr float kSkyDomeScale = 100.0f; 

    constexpr float kDropInitialHeight = 140.0f; // アイテムドロップ時の初期上昇量
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
    SetWaitVSyncFlag(TRUE); // VSync有効化で描画負荷を安定化
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

	// WaveManagerを初期化
	m_pWaveManager = std::make_shared<WaveManager>();
	m_pWaveManager->Init();
	
	// Road_floorオブジェクトの範囲を設定（マップ全体の範囲）
	m_pWaveManager->SetRoadFloorBounds(VGet(-500.0f, 0.0f, -500.0f), VGet(500.0f, 0.0f, 500.0f));

    if (m_pPlayer->GetCamera())
    {
        m_pPlayer->GetCamera()->SetSensitivity(m_cameraSensitivity);
    }

    SetMouseDispFlag(m_isPaused);

    MV1SetPosition(m_skyDomeHandle, VGet(0, kSkyDomePosY, 0));

    MV1SetScale(m_skyDomeHandle, VGet(kSkyDomeScale, kSkyDomeScale, kSkyDomeScale));

    m_isReturningFromOption = false;

    m_items.clear();

    // WaveManagerの敵の死亡時にアイテムをドロップするコールバックを設定
    m_pWaveManager->SetOnEnemyDeathCallback([this](const VECTOR& pos) {
        printf("Creating item at position: (%.2f, %.2f, %.2f)\n", pos.x, pos.y, pos.z);
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
    SetLightAmbColor(GetColorF(0.5f, 0.5f, 0.5f, 1.0f));
}

SceneBase* SceneMain::Update()
{
    if (DebugUtil::IsDebugWindowVisible())
    {
        return this;
    }

    MV1SetRotationXYZ(m_skyDomeHandle, VGet(0, MV1GetRotationXYZ(m_skyDomeHandle).y + kCameraRotaSpeed, 0));

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
    for (std::shared_ptr<EnemyBase>& enemy : enemyList)
    {
        enemyPtrList.push_back(enemy.get());
    }
    
    m_pPlayer->Update(enemyPtrList);

    if (m_pPlayer->GetHealth() <= 0.0f)
    {
        return new SceneGameOver();
    }

    // WaveManagerの更新（敵の生成と管理）
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
    if (m_hitMarkTimer > 0) {
        // 赤 or 黄色
        unsigned int color = (m_hitMarkType == EnemyBase::HitPart::Head) ? GetColor(255, 255, 64) : GetColor(255, 32, 32);
        int len = 8; // ラインの長さ（端から中心までの距離）
        int gap = 4; // 中央の空白幅
        int thick = 2; // ラインの太さ
        // 左上→右下
        DrawLine(cx - len, cy - len, cx - gap, cy - gap, color, thick);
        DrawLine(cx + gap, cy + gap, cx + len, cy + len, color, thick);
        // 左下→右上
        DrawLine(cx - len, cy + len, cx - gap, cy + gap, color, thick);
        DrawLine(cx + gap, cy - gap, cx + len, cy - len, color, thick);
    }

    // デバッグ情報を表示
    m_pWaveManager->DrawDebugInfo();

    if (m_isPaused)
    {
        DrawPauseMenu();
    }
}

void SceneMain::DrawShadowCasters()
{
    // 影を落とすものだけ描画
    m_pStage->Draw();
    m_pWaveManager->Draw();
    m_pPlayer->Draw();
    // アイテムや空（スカイドーム）は影を落とさないので描画しない
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
