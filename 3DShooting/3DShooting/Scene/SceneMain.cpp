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
#include <cassert>
#include <algorithm>

namespace
{
    constexpr int	kInitialCountdownValue = 3;	  
    constexpr int	kMainCountdownValue    = 10;  
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

   
	constexpr float kSkyDomeScale = 10.0f; 

    constexpr float kDropInitialHeight = 140.0f; // アイテムドロップ時の初期上昇量
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

    m_skyDomeHandle = MV1LoadModel("data/model/Dome.mv1");
    assert(m_skyDomeHandle != -1);


    m_dotHandle = LoadGraph("data/image/Dot.png");
	assert(m_dotHandle != -1);
}

SceneMain::~SceneMain()
{

    MV1DeleteModel(m_skyDomeHandle);    

	DeleteGraph(m_dotHandle);
}

void SceneMain::Init()
{
    m_pPlayer = std::make_unique<Player>();
    m_pPlayer->Init();

	m_pEnemyNormal = std::make_shared<EnemyNormal>();
	m_pEnemyNormal->Init();

	m_pEnemyRunner = std::make_shared<EnemyRunner>();
	m_pEnemyRunner->Init();

	m_pEnemyAcid = std::make_shared<EnemyAcid>();
	m_pEnemyAcid->Init();

    if (m_pPlayer->GetCamera())
    {
        m_pPlayer->GetCamera()->SetSensitivity(m_cameraSensitivity);
    }

    SetMouseDispFlag(m_isPaused);

    MV1SetPosition(m_skyDomeHandle, VGet(0, kSkyDomePosY, 0));

    MV1SetScale(m_skyDomeHandle, VGet(kSkyDomeScale, kSkyDomeScale, kSkyDomeScale));

    m_isReturningFromOption = false;

    m_items.clear();

    // 敵の死亡時にアイテムをドロップするコールバックを設定
    m_pEnemyNormal->SetOnDropItemCallback([this](const VECTOR& pos) {
        auto dropItem = std::make_shared<FirstAidKitItem>();
        dropItem->Init();
        VECTOR dropPos = pos;
        dropPos.y += kDropInitialHeight;
        dropItem->SetPos(dropPos);
        m_items.push_back(dropItem);
    });

	m_pEnemyRunner->SetOnDropItemCallback([this](const VECTOR& pos) {
		auto dropItem = std::make_shared<FirstAidKitItem>();
		dropItem->Init();
		VECTOR dropPos = pos;
		dropPos.y += kDropInitialHeight;
		dropItem->SetPos(dropPos);
		m_items.push_back(dropItem);
	});

	m_pEnemyAcid->SetOnDropItemCallback([this](const VECTOR& pos) {
		auto dropItem = std::make_shared<FirstAidKitItem>();
		dropItem->Init();
		VECTOR dropPos = pos;
		dropPos.y += kDropInitialHeight;
		dropItem->SetPos(dropPos);
		m_items.push_back(dropItem);
	});
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

    m_pPlayer->Update(m_enemyList);

    if (m_pPlayer->GetHealth() <= 0.0f)
    {
        return new SceneGameOver();
    }

//	m_pEnemyNormal->Update(m_pPlayer->GetBullets(), m_pPlayer->GetTackleInfo(), *m_pPlayer);

//	m_pEnemyRunner->Update(m_pPlayer->GetBullets(), m_pPlayer->GetTackleInfo(), *m_pPlayer);

	m_pEnemyAcid->Update(m_pPlayer->GetBullets(), m_pPlayer->GetTackleInfo(), *m_pPlayer);

    for (auto& item : m_items)
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

    return this;
}

void SceneMain::Draw()
{
    int screenW, screenH;
    GetScreenState(&screenW, &screenH, nullptr);

	MV1DrawModel(m_skyDomeHandle); 

    for (auto& item : m_items) 
    {
        item->Draw();
    }

//  m_pEnemyNormal->Draw();

//	m_pEnemyRunner->Draw();

	m_pEnemyAcid->Draw();

    m_pPlayer->Draw();

    constexpr int kDotSize = 64;
    DrawGraph(screenW * 0.5f - kDotSize * 0.5f, screenH * 0.5f - kDotSize * 0.5f, m_dotHandle, true);

    if (m_isPaused)
    {
        DrawPauseMenu();
    }
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
