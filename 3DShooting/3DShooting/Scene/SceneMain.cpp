#include "SceneMain.h"
#include "SceneTitle.h"
#include "SceneOption.h"
#include "SceneResult.h"
#include "SceneGameOver.h" 
#include "EffekseerForDXLib.h"
#include "Player.h"
#include "InputManager.h"
#include "Game.h"
#include "EnemyBase.h"
#include "EnemyNormal.h"
#include "EnemyRunner.h"
#include "EnemyAcid.h"
#include "DebugUtil.h"
#include "Camera.h"
#include "FirstAidKitItem.h"
#include "AmmoItem.h"
#include "Stage.h"
#include "WaveManager.h"
#include "ScoreManager.h"
#include "TutorialManager.h"
#include "TaskTutorialManager.h"
#include "Effect.h"
#include "DirectionIndicator.h"
#include "ShellCasing.h"
#include "AnimationManager.h"
#include <cstdio>
#include <cassert>
#include <algorithm>
#include <string>

// static変数の定義
float SceneMain::s_elapsedTime = 0.0f;
bool  SceneMain::s_isSkipTutorial = false;
bool  SceneMain::s_isLowHealthTutorialShown = false;

namespace
{
	// UI関連の定数
	constexpr int	kButtonWidth        = 200;  // ボタンの幅 
	constexpr int	kButtonHeight       = 50;   // ボタンの高さ
	constexpr int	kFontSize           = 48;   // フォントサイズ
	constexpr float kScreenCenterOffset = 0.5f; // 画面中央のオフセット
	constexpr int   kButtonYOffset      = 70;   // ボタンのY座標オフセット
	constexpr int   kButtonSpacing      = 20;   // ボタン間のスペース

	// ゲームクリアシーンへの遷移遅延フレーム数
	constexpr int   kClearSceneDelayFrames = 60; 

	// 戻るボタンとオプションボタンの座標
	constexpr int   kReturnButtonX = 210; // 戻るボタンのX座標
	constexpr int   kReturnButtonY = 290; // 戻るボタンのY座標
	constexpr int   kOptionButtonX = 210; // オプションボタンのX座標
	constexpr int   kOptionButtonY = 120; // オプションボタンのY座標

    // カメラの回転速度
	constexpr float kCameraRotaSpeed = 0.0001f; 

    // スカイドーム関連
	constexpr float kSkyDomePosY  = 200.0f; // スカイドームのY座標
	constexpr float kSkyDomeScale = 100.0f; // スカイドームのスケール

    // アイテムドロップ時の初期上昇量
    constexpr float kDropInitialHeight = 140.0f; 
	
    // 環境光設定
	constexpr float kAmbientLightR = 0.5f; // 環境光の赤成分
	constexpr float kAmbientLightG = 0.5f; // 環境光の緑成分
	constexpr float kAmbientLightB = 0.5f; // 環境光の青成分
	constexpr float kAmbientLightA = 1.0f; // 環境光のアルファ成分

    // ヒットマーク関連
	constexpr int   kHitMarkLineLength       = 8;  // ラインの長さ
	constexpr int   kHitMarkCenterSpacing    = 4;  // 中央の間隔幅
	constexpr int   kHitMarkLineThickness    = 2;  // ラインの太さ
    constexpr int   kHitMarkDuration         = 25; // 表示時間
	constexpr int   kHitMarkDoubleLineOffset = 2;  // ダブルラインのオフセット

	// スコアポップアップ関連
	constexpr int   kScorePopupX        = 80;  // スコアポップアップのX座標
	constexpr int   kScorePopupY        = 60;  // スコアポップアップのY座標
	constexpr int   kPopupOffsetY       = 32;  // ポップアップのYオフセット
    constexpr int   kPopupDuration      = 60;  // 表示時間
    constexpr int   kTotalScoreDuration = 120; // 合計スコアの表示時間

	// レティクル表示位置補正値
	constexpr int   kReticleOffset = 64; 

    // 画面中央サイズ
	constexpr int   kScreenCenterX = Game::kScreenWidth * 0.5f;  
	constexpr int   kScreenCenterY = Game::kScreenHeigth * 0.5f; 

    // Road_floorオブジェトの範囲
    constexpr VECTOR kRoadFloorMin = { -500.0f, 0.0f, -500.0f }; // 床の最小座標
    constexpr VECTOR kRoadFloorMax = { 500.0f, 0.0f, 500.0f };   // 床の最大座標
}

SceneMain* g_sceneMainInstance = nullptr;

SceneMain* SceneMain::Instance() 
{
    return g_sceneMainInstance;
}

SceneMain::SceneMain(bool isReturningFromOtherScene) :
    m_isPaused(false),
    m_isEscapePressed(false),
    m_isReturningFromOption(false),
    m_cameraSensitivity(Game::g_cameraSensitivity),
    m_pCamera(std::make_unique<Camera>()),
    m_skyDomeHandle(-1),
    m_dotDefaultHandle(-1),
    m_dotOnTargetHandle(-1),
	m_sgDefaultReticleHandle(-1),
	m_sgOnTargetReticleHandle(-1),
    m_hitMarkTimer(0),
	m_isWave1FirstAidDropped(false),
	m_isWave1AmmoDropped(false),
	m_wave1DropCount(0),
	m_totalScorePopupTimer(0),
	m_lastTotalScorePopupValue(0),
    m_bgmHandle(-1),
    m_isBGMStarted(false),
    m_isLoading(true),  
	m_isReturningFromOtherScene(isReturningFromOtherScene),
	m_clearSceneDelayTimer(-1),
	m_scoreFontHandle(-1),
	m_isPlayerInit(false),
	m_isTaskTutorialInit(false),
	m_pEffect(std::make_unique<Effect>()),
	m_pAnimManager(std::make_unique<AnimationManager>()),
	m_gameOverDelayTimer(-1),
	m_isTutorialStage(false)
{
    g_sceneMainInstance = this;

    // スコアポップアップ用フォントの作成
    m_scoreFontHandle = CreateFontToHandle("Abadi MT", 24, 3, DX_FONTTYPE_ANTIALIASING_EDGE_8X8);
    assert(m_scoreFontHandle != -1);
}


SceneMain::~SceneMain()
{
	// モデルやリソースの解放
    MV1DeleteModel(m_skyDomeHandle);    
	DeleteGraph(m_dotDefaultHandle);
	DeleteGraph(m_dotOnTargetHandle);
	DeleteGraph(m_sgDefaultReticleHandle);
	DeleteGraph(m_sgOnTargetReticleHandle);

	// アイテムモデルの解放
	FirstAidKitItem::DeleteModel();
	AmmoItem::DeleteModel();
	ShellCasing::DeleteModel();
	
	// インジケーター画像の解放
	DirectionIndicator::DeleteResources();
	
	// BGMの解放
	DeleteSoundMem(m_bgmHandle);
    // フォントの解放
    DeleteFontToHandle(m_scoreFontHandle);
}

void SceneMain::Init()
{
    SetWaitVSyncFlag(true); // VSync有効化で描画負荷を安定化

    // 経過時間リセット
    s_elapsedTime = 0.0f;

    // 非同期読み込みを有効化
    SetUseASyncLoadFlag(true);

    // アイテムモデルの読み込み
	FirstAidKitItem::LoadModel();
	AmmoItem::LoadModel();
	ShellCasing::LoadModel();

    // インジケーター画像の読み込み
    DirectionIndicator::LoadResources();

    // 重いリソースの非同期読み込みを開始
    m_skyDomeHandle = MV1LoadModel("data/model/Dome.mv1");
    m_dotDefaultHandle     = LoadGraph("data/image/DotDefault.png");
    m_dotOnTargetHandle    = LoadGraph("data/image/DotOnTarget.png");
	m_sgDefaultReticleHandle = LoadGraph("data/image/SGDefaultReticl.png");
	m_sgOnTargetReticleHandle = LoadGraph("data/image/SGOnTargetReticle.png");
    m_bgmHandle     = LoadSoundMem("data/sound/BGM/GameSceneBGM.mp3");

    m_pPlayer = std::make_unique<Player>();
	m_pPlayer->SetEffect(m_pEffect.get());
	m_pPlayer->SetAnimationManager(m_pAnimManager.get());
    Game::m_pPlayer = m_pPlayer.get();

	m_pEnemyNormal = std::make_shared<EnemyNormal>();
	m_pEnemyNormal->Init();

	m_pEnemyRunner = std::make_shared<EnemyRunner>();
	m_pEnemyRunner->Init();

	m_pEnemyAcid = std::make_shared<EnemyAcid>();
	m_pEnemyAcid->Init();

	m_pStage = std::make_shared<Stage>();
	
	// チュートリアルスキップフラグに応じてステージをロード
	if (s_isSkipTutorial || m_isReturningFromOtherScene)
	{
		m_pStage->LoadStage(false); // メインステージ
		m_isTutorialStage = false;
	}
	else
	{
		m_pStage->LoadStage(true); // チュートリアルステージ
		m_isTutorialStage = true;
	}

	m_pWaveManager = std::make_shared<WaveManager>();
	m_pWaveManager->Init();
	
	m_pDirectionIndicator = std::make_unique<DirectionIndicator>();
	m_pDirectionIndicator->Init(m_pPlayer.get());
	
	// プレイヤーに方向インジケーターを設定
	m_pPlayer->SetDirectionIndicator(m_pDirectionIndicator.get());
	
	// Road_floorオブジェクトの範囲を設定（マップ全体の範囲）
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

    // wave1開始時にフラグとカウントをリセット
    m_isWave1FirstAidDropped = false;
    m_isWave1AmmoDropped = false;
    m_wave1DropCount = 0;

    m_clearSceneDelayTimer = -1; // 遅延タイマーをリセット

    // WaveManagerの敵の死亡時にアイテムをドロップするコールバックを設定
    m_pWaveManager->SetOnEnemyDeathCallback([this](const VECTOR& pos) {
        static VECTOR lastDropPos = { -99999, -99999, -99999 };
        // 直前と同じ座標なら何もしない
        if (pos.x == lastDropPos.x && pos.y == lastDropPos.y && pos.z == lastDropPos.z) return;
        lastDropPos = pos;
        if (m_pWaveManager->GetCurrentWave() == 1) 
        {
            if (m_wave1DropCount >= 2) return; // 2体分だけドロップ

            if (!m_isWave1FirstAidDropped && !m_isWave1AmmoDropped) 
            {
                int randValue = GetRand(99);
                if (randValue < 50) 
                {
                    auto firstAid = std::make_shared<FirstAidKitItem>();
                    firstAid->Init();
                    VECTOR dropPos = pos;
                    dropPos.y += kDropInitialHeight;
                    firstAid->SetPos(dropPos);
                    m_items.push_back(firstAid);
                    m_isWave1FirstAidDropped = true;
                } 
                else 
                {
                    auto ammo = std::make_shared<AmmoItem>();
                    ammo->Init();
                    VECTOR dropPos = pos;
                    dropPos.y += kDropInitialHeight;
                    ammo->SetPos(dropPos);
                    m_items.push_back(ammo);
                    m_isWave1AmmoDropped = true;
                }
                m_wave1DropCount++;
            }
            else if (!m_isWave1FirstAidDropped) 
            {
                auto firstAid = std::make_shared<FirstAidKitItem>();
                firstAid->Init();
                VECTOR dropPos = pos;
                dropPos.y += kDropInitialHeight;
                firstAid->SetPos(dropPos);
                m_items.push_back(firstAid);
                m_isWave1FirstAidDropped = true;
                m_wave1DropCount++;
            }
            else if (!m_isWave1AmmoDropped) 
            {
                auto ammo = std::make_shared<AmmoItem>();
                ammo->Init();
                VECTOR dropPos = pos;
                dropPos.y += kDropInitialHeight;
                ammo->SetPos(dropPos);
                m_items.push_back(ammo);
                m_isWave1AmmoDropped = true;
                m_wave1DropCount++;
            }
            // 両方ドロップ済み or 2体分超えたら何も落とさない
        } 
        else
        {
            // wave2以降はどちらか一方のみドロップ
            int randValue = GetRand(99);
            std::shared_ptr<ItemBase> dropItem;
            if (randValue < 50) 
            {
                dropItem = std::make_shared<FirstAidKitItem>();
            }
            else
            {
                dropItem = std::make_shared<AmmoItem>();
            }
            dropItem->Init();
            VECTOR dropPos = pos;
            dropPos.y += kDropInitialHeight;
            dropItem->SetPos(dropPos);
            m_items.push_back(dropItem);
        }
    });

    // チュートリアルマネージャ生成・初期化
    m_pTutorialManager = std::make_unique<TutorialManager>();
    if (!m_isReturningFromOption && !m_isReturningFromOtherScene && !s_isSkipTutorial)
    {
        m_pTutorialManager->Init();
    }

    // ヒットマーク用コールバックをWaveManagerに設定
    m_pWaveManager->SetOnEnemyHitCallback([this](EnemyBase::HitPart part) { OnPlayerBulletHitEnemy(part); });

	// 環境光の設定
    SetLightAmbColor(GetColorF(kAmbientLightR, kAmbientLightG, kAmbientLightB, kAmbientLightA));

    // BGM再生フラグをリセット
    m_isBGMStarted = false;

    // チュートリアルマネージャーをリセットまたはスキップ
    if (m_isReturningFromOtherScene)
    {
        TaskTutorialManager::GetInstance()->Skip(m_pWaveManager.get());
    }
    else
    {
        TaskTutorialManager::GetInstance()->Reset();
    }
    
    // 非同期読み込みを無効化
    SetUseASyncLoadFlag(false);
}

// スコアポップアップを追加する
void SceneMain::AddScorePopup(int score, bool isHeadShot, int combo)
{
    m_scorePopups.push_back({ score, combo, kPopupDuration, static_cast<bool>(isHeadShot) });

    if (m_scorePopups.size() > 5) m_scorePopups.pop_front(); // 最大5件まで
    m_totalScorePopupTimer = kTotalScoreDuration; // 合計スコアタイマーリセット

    // 直近の倍率適用済みスコアを保存
    int totalScore = ScoreManager::Instance().GetScore();
    float lastComboRate = ScoreManager::Instance().GetLastComboRate();
    m_lastTotalScorePopupValue = static_cast<int>(totalScore * lastComboRate);
}

void SceneMain::SwitchToMainStage()
{
	// メインステージをロード
	m_pStage->LoadStage(false);
	m_isTutorialStage = false;

	// プレイヤーの位置をリセット（必要であれば）
	// m_pPlayer->SetPos(VGet(0, 0, 0)); 
	// m_pPlayer->Init(); // Initを呼ぶとTransformDataから再読み込みされるので、CSVの初期位置に戻るはず

	// プレイヤーの再初期化（位置などをCSVから再取得）
	m_pPlayer->Init(false);
}

SceneBase* SceneMain::Update()
{
    // ローディング中は他の処理を行わない
    if (m_isLoading) 
    {
        // 非同期読み込みが完了しているかチェック
        if (GetASyncLoadNum() == 0) 
        {
            // 最低限のローディング時間を確保
            static int loadingFrameCount = 0;
            loadingFrameCount++;
            if (loadingFrameCount >= 80) 
            {
                m_isLoading = false;
                loadingFrameCount = 0;
            }

            // 非同期ロード完了後に1回だけ呼ぶ
            if (!m_isPlayerInit && m_pPlayer)
            {
                m_pPlayer->Init(m_isTutorialStage);
                m_isPlayerInit = true;
            }
        } 
        else 
        {
            return this;
        }
    }

    // BGM再生
    if (!m_isBGMStarted)
    {
        if (CheckSoundMem(m_bgmHandle) == 0)
        {
            ChangeVolumeSoundMem(200, m_bgmHandle);
            PlaySoundMem(m_bgmHandle, DX_PLAYTYPE_LOOP);
        }
        m_isBGMStarted = true;
    }

    // 経過時間を加算
    s_elapsedTime += 1.0f / 60.0f;

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
        if (InputManager::GetInstance()->IsTriggerMouseLeft())
        {
            Vec2 mousePos = InputManager::GetInstance()->GetMousePos();

            if (mousePos.x >= kReturnButtonX && mousePos.x <= kReturnButtonX + kButtonWidth &&
                mousePos.y >= kReturnButtonY && mousePos.y <= kReturnButtonY + kButtonHeight)
            {
                // BGMを停止
                StopSoundMem(m_bgmHandle);
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

    // チュートリアルマネージャの更新
    if (m_pTutorialManager)
    {
        m_pTutorialManager->Update();

        // 低体力チュートリアルの表示
        if (m_pPlayer && m_pPlayer->IsLowHealth() && !s_isLowHealthTutorialShown)
        {
            m_pTutorialManager->AddMessage("回復アイテム", "敵を倒すと回復アイテムをドロップする。\n生き残りたければ積極的に行動せよ");
            s_isLowHealthTutorialShown = true;
        }
    }

    // 基本操作チュートリアル中の処理
    if (m_pTutorialManager && m_pTutorialManager->IsActive())
    {
        m_pPlayer->Update({}, m_pStage->GetCollisionData()); // プレイヤーはチュートリアル中も移動可能
        return this;
    }
    // 基本操作チュートリアルが完了したら、タスクチュートリアルを初期化
    else if (m_pTutorialManager && m_pTutorialManager->IsCompleted() && !m_isTaskTutorialInit)
    {
        TaskTutorialManager::GetInstance()->Init(m_pWaveManager.get(), m_pPlayer.get());
        m_isTaskTutorialInit = true;
    }

    // タスクチュートリアルが完了していない間
    if (!TaskTutorialManager::GetInstance()->IsCompleted())
    {
        TaskTutorialManager::GetInstance()->Update();
        // タスクチュートリアル中はWaveManagerの通常の更新は行わない
        // ただし、敵の更新とプレイヤーの更新は必要
        
        // WaveManagerからアクティブな敵のリストを取得してプレイヤーを更新
        std::vector<std::shared_ptr<EnemyBase>>& enemyList = m_pWaveManager->GetEnemyList();
        std::vector<EnemyBase*> enemyPtrList;
        for (std::shared_ptr<EnemyBase>& enemy : enemyList)
        {
            enemyPtrList.push_back(enemy.get());
        }
        m_pPlayer->Update(enemyPtrList, m_pStage->GetCollisionData()); // プレイヤーは敵を撃ったりタックルしたりするために敵で更新する必要がある
        m_pWaveManager->UpdateEnemies(m_pPlayer->GetBullets(), m_pPlayer->GetTackleInfo(), *m_pPlayer, m_pStage->GetCollisionData(), m_pEffect.get()); // 敵も更新する必要がある
        
        // アイテム、スコアポップアップなどの更新はタスクチュートリアル中でも実行
        for (std::shared_ptr<ItemBase>& item : m_items) { item->Update(m_pPlayer.get()); }
        m_items.erase(
            std::remove_if(m_items.begin(), m_items.end(), [](const std::shared_ptr<ItemBase>& item) { return item->IsUsed(); }),
            m_items.end()
        );
        if (m_hitMarkTimer > 0) { --m_hitMarkTimer; }
        for (auto& popup : m_scorePopups) { popup.timer--; }
        while (!m_scorePopups.empty() && m_scorePopups.front().timer <= 0) { m_scorePopups.pop_front(); }
        if (m_totalScorePopupTimer > 0) { m_totalScorePopupTimer--; }
        ScoreManager::Instance().Update();

        // ゲームオーバーチェックもここで実行
        if (m_pPlayer->IsDead())
        {
            if (m_gameOverDelayTimer == -1)
            {
                m_gameOverDelayTimer = 180; // 3秒の遅延
            }
            else if (m_gameOverDelayTimer > 0)
            {
                m_gameOverDelayTimer--;
            }
            else
            {
                int wave = m_pWaveManager->GetCurrentWave();
                int killCount = ScoreManager::Instance().GetBodyKillCount() + ScoreManager::Instance().GetHeadKillCount();
                int score = ScoreManager::Instance().GetTotalScore();
                StopSoundMem(m_bgmHandle);
                return new SceneGameOver(wave, killCount, score);
            }
        }
                
        m_pDirectionIndicator->Update(m_pWaveManager->GetEnemyList()); // 方向インジケータも更新
        
        return this; // タスクチュートリアル中に留まる
    }
        
    // ここから通常進行 (両方のチュートリアルが完了した場合のみ)

	// チュートリアルステージにいる場合はメインステージに切り替え
	if (m_isTutorialStage)
	{
		SwitchToMainStage();
	}

    m_pWaveManager->Update(); // メインのウェーブマネージャを更新
        
    std::vector<std::shared_ptr<EnemyBase>>& enemyList = m_pWaveManager->GetEnemyList();
    std::vector<EnemyBase*> enemyPtrList;
    for (std::shared_ptr<EnemyBase>& enemy : enemyList) 
    {
        enemyPtrList.push_back(enemy.get());
    }
    m_pPlayer->Update(enemyPtrList, m_pStage->GetCollisionData());

    if (m_pPlayer->IsDead()) 
    {
        if (m_gameOverDelayTimer == -1)
        {
            m_gameOverDelayTimer = 180;
        }
        else if (m_gameOverDelayTimer > 0)
        {
            m_gameOverDelayTimer--;
        }
        else
        {
            int wave = m_pWaveManager->GetCurrentWave();
            int killCount = ScoreManager::Instance().GetBodyKillCount() + ScoreManager::Instance().GetHeadKillCount();
            int score = ScoreManager::Instance().GetTotalScore();
            StopSoundMem(m_bgmHandle);
            return new SceneGameOver(wave, killCount, score);
        }
    }
        
    // ウェーブ3終了後の遅延処理
    if (m_pWaveManager->GetCurrentWave() > 3)
    {
        if (m_clearSceneDelayTimer == -1) // 遅延がまだ開始されていない場合
        {
            m_clearSceneDelayTimer = kClearSceneDelayFrames; // 遅延タイマーを開始
            StopSoundMem(m_bgmHandle); // BGMを停止
        }
        else if (m_clearSceneDelayTimer > 0) // 遅延中の場合
        {
            m_clearSceneDelayTimer--; // タイマーを減らす
        }
        else // 遅延が終了した場合
        {
            return new SceneResult(); // シーン遷移
        }
    }

    // 遅延中は他の処理をスキップ
    if (m_clearSceneDelayTimer != -1) return this;
    m_pWaveManager->UpdateEnemies(m_pPlayer->GetBullets(), m_pPlayer->GetTackleInfo(), *m_pPlayer, m_pStage->GetCollisionData(), m_pEffect.get());
    for (std::shared_ptr<ItemBase>& item : m_items) { item->Update(m_pPlayer.get()); }
    m_items.erase(
        std::remove_if(m_items.begin(), m_items.end(), [](const std::shared_ptr<ItemBase>& item) { return item->IsUsed(); }),
        m_items.end()
    );

    // ヒットマークタイマー更新
    if (m_hitMarkTimer > 0) { --m_hitMarkTimer; }

    // スコアポップアップタイマー更新
    for (auto& popup : m_scorePopups) { popup.timer--; } 

    // タイマー切れのポップアップ削除
    while (!m_scorePopups.empty() && m_scorePopups.front().timer <= 0) { m_scorePopups.pop_front(); } 

    // 合計スコアポップアップタイマー更新
    if (m_totalScorePopupTimer > 0) { m_totalScorePopupTimer--; } 
    m_pDirectionIndicator->Update(m_pWaveManager->GetEnemyList()); // 方向インジケータも更新
    ScoreManager::Instance().Update();
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

    // 古いチュートリアルUI描画
    if (m_pTutorialManager)
    {
        m_pTutorialManager->Draw(screenW, screenH);
    }

    // 基本操作チュートリアル中は敵などを描画しない
    if (m_pTutorialManager && m_pTutorialManager->IsActive())
    {
        // 何もしない（プレイヤー描画は下で行う）
    }
    // タスクチュートリアルUI描画
    else if (!TaskTutorialManager::GetInstance()->IsCompleted())
    {
        TaskTutorialManager::GetInstance()->Draw();
        // タスクチュートリアル中は敵を描画する
        m_pWaveManager->DrawEnemies();
    }
    // メインゲームループ中の敵描画 (両方のチュートリアルが完了した場合)
    else
    {
        m_pWaveManager->DrawEnemies();
    }

    m_pPlayer->Draw3D();

    m_pEffect->Draw();

    // ここからUI描画
    m_pPlayer->DrawShield();

    if (!m_pPlayer->IsDead())
    {
        int defaultHandle = -1;
        int onTargetHandle = -1;

        // 武器の種類に応じてハンドルを切り替える
        switch (m_pPlayer->GetCurrentWeaponType())
        {
        case WeaponType::AssaultRifle:
            defaultHandle = m_dotDefaultHandle;
            onTargetHandle = m_dotOnTargetHandle;
            break;
        case WeaponType::Shotgun:
            defaultHandle = m_sgDefaultReticleHandle;
            onTargetHandle = m_sgOnTargetReticleHandle;
            break;
        }

        // ターゲットに照準が合っているかに応じてハンドルを決定
        int currentReticleHandle = m_pPlayer->IsAimingAtEnemy() ? onTargetHandle : defaultHandle;
        
        // レティクルを描画
        if (currentReticleHandle != -1)
        {
            int reticleWidth = 0;
            int reticleHeight = 0;
            GetGraphSize(currentReticleHandle, &reticleWidth, &reticleHeight);
            DrawGraph(kScreenCenterX - reticleWidth * 0.5f, kScreenCenterY - reticleHeight * 0.5f, currentReticleHandle, true);
        }

        // ドットレティクルを常に描画
        int dotReticleHandle = m_pPlayer->IsAimingAtEnemy() ? m_dotOnTargetHandle : m_dotDefaultHandle;
        if (dotReticleHandle != -1)
        {
            int dotReticleWidth = 0;
            int dotReticleHeight = 0;
            GetGraphSize(dotReticleHandle, &dotReticleWidth, &dotReticleHeight);
            DrawGraph(kScreenCenterX - dotReticleWidth * 0.5f, kScreenCenterY - dotReticleHeight * 0.5f, dotReticleHandle, true);
        }
    }

    // ウェーブUIの描画
    m_pWaveManager->DrawWaveUI();

    m_pPlayer->DrawUI();

    // 方向インジケーターUIの描画
    if (!m_pPlayer->IsDead())
    {
        m_pDirectionIndicator->Draw();
    }

    // スコアポップアップ描画
    bool showScorePopup = !m_scorePopups.empty();
    bool showTotalScoreOnly = (m_totalScorePopupTimer > 0);
    if (showScorePopup || showTotalScoreOnly) 
    {
        int popupBaseX = kScreenCenterX + kScorePopupX;
        int popupBaseY = kScreenCenterY + kScorePopupY;
        int idx = 0;
        int totalScore = ScoreManager::Instance().GetScore();
        int combo      = ScoreManager::Instance().GetCombo();
        float comboRate = std::pow(1.1f, combo > 0 ? combo - 1 : 0);
        int comboScore  = static_cast<int>(totalScore * comboRate);
        float lastComboRate = ScoreManager::Instance().GetLastComboRate();
        int displayCombo    = (ScoreManager::Instance().GetCombo() > 1) ? ScoreManager::Instance().GetCombo() : 1;

        // フェードアウト用アルファ値(最も古いポップアップのtimer値を利用)
        int fadeTimer = showScorePopup ? m_scorePopups.front().timer : m_totalScorePopupTimer;
        int alpha = 255;
        if (fadeTimer < 30) 
        {
            alpha = 128 + (127 * fadeTimer / 30);
        }
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

        // 合計スコアのみ
        if (showTotalScoreOnly && !showScorePopup)
        {
            float lastComboRate = ScoreManager::Instance().GetLastComboRate();
            if (lastComboRate > 1.0f)
            {
                DrawFormatStringToHandle(popupBaseX, popupBaseY, 0x00ffcc, m_scoreFontHandle, "%d ×%.2f", m_lastTotalScorePopupValue, lastComboRate);
            }
            else 
            {
                DrawFormatStringToHandle(popupBaseX, popupBaseY, 0x00ffcc, m_scoreFontHandle, "%d", m_lastTotalScorePopupValue);
            }
        }
        else if (showScorePopup) 
        {
            // 合計スコア
            if (lastComboRate > 1.0f) 
            {
                DrawFormatStringToHandle(popupBaseX, popupBaseY + idx * kPopupOffsetY, 0x00ffcc, m_scoreFontHandle, "%d ×%.2f", comboScore, lastComboRate);
            }
            else 
            {
                DrawFormatStringToHandle(popupBaseX, popupBaseY + idx * kPopupOffsetY, 0x00ffcc, m_scoreFontHandle, "%d", comboScore);
            }
            idx++;
            int lastIsHeadShot = -1;
            for (const auto& popup : m_scorePopups) 
            {
                if (lastIsHeadShot == -1 || lastIsHeadShot != static_cast<int>(popup.isHeadShot)) 
                {
                    if (popup.isHeadShot) 
                    {
                        DrawFormatStringToHandle(popupBaseX, popupBaseY + idx * kPopupOffsetY, 0xffe000, m_scoreFontHandle, "%dpt ヘッドショットキル×%d", 200, displayCombo);
                    }
                    else 
                    {
                        DrawFormatStringToHandle(popupBaseX, popupBaseY + idx * kPopupOffsetY, 0xffffff, m_scoreFontHandle, "%dpt ゾンビキル×%d", 100, displayCombo);
                    }
                    idx++;
                }
                lastIsHeadShot = static_cast<int>(popup.isHeadShot);
            }
        }
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }

    // ヒットマーク描画
    if (m_hitMarkTimer > 0)
    {
        // アルファ値を計算
        int alpha = (255 * m_hitMarkTimer) / kHitMarkDuration;
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

        // 赤 or 黄色
        unsigned int color = (m_hitMarkType == EnemyBase::HitPart::Head) ? 0xffd700 : 0xff4500;

        // 通常のヒットマーク描画
        // 左上→右下
        DrawLine(kScreenCenterX - kHitMarkLineLength, kScreenCenterY - kHitMarkLineLength,
            kScreenCenterX - kHitMarkCenterSpacing, kScreenCenterY - kHitMarkCenterSpacing,
            color, kHitMarkLineThickness);
        DrawLine(kScreenCenterX + kHitMarkCenterSpacing, kScreenCenterY + kHitMarkCenterSpacing,
            kScreenCenterX + kHitMarkLineLength, kScreenCenterY + kHitMarkLineLength,
            color, kHitMarkLineThickness);
        // 左下→右上
        DrawLine(kScreenCenterX - kHitMarkLineLength, kScreenCenterY + kHitMarkLineLength,
            kScreenCenterX - kHitMarkCenterSpacing, kScreenCenterY + kHitMarkCenterSpacing,
            color, kHitMarkLineThickness);
        DrawLine(kScreenCenterX + kHitMarkCenterSpacing, kScreenCenterY - kHitMarkCenterSpacing,
            kScreenCenterX + kHitMarkLineLength, kScreenCenterY - kHitMarkLineLength,
            color, kHitMarkLineThickness);

        // ヘッドショットの場合は二重線を描画
        if (m_hitMarkType == EnemyBase::HitPart::Head)
        {
            int offset = kHitMarkDoubleLineOffset;
            DrawLine(kScreenCenterX - kHitMarkLineLength - offset, kScreenCenterY - kHitMarkLineLength + offset,
                kScreenCenterX - kHitMarkCenterSpacing - offset, kScreenCenterY - kHitMarkCenterSpacing + offset,
                color, kHitMarkLineThickness);
            DrawLine(kScreenCenterX + kHitMarkCenterSpacing + offset, kScreenCenterY + kHitMarkCenterSpacing - offset,
                kScreenCenterX + kHitMarkLineLength + offset, kScreenCenterY + kHitMarkLineLength - offset,
                color, kHitMarkLineThickness);
            DrawLine(kScreenCenterX - kHitMarkLineLength - offset, kScreenCenterY + kHitMarkLineLength - offset,
                kScreenCenterX - kHitMarkCenterSpacing - offset, kScreenCenterY + kHitMarkCenterSpacing - offset,
                color, kHitMarkLineThickness);
            DrawLine(kScreenCenterX + kHitMarkCenterSpacing + offset, kScreenCenterY - kHitMarkCenterSpacing + offset,
                kScreenCenterX + kHitMarkLineLength + offset, kScreenCenterY - kHitMarkLineLength + offset,
                color, kHitMarkLineThickness);
        }

        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }

#ifdef _DEBUG
    // デバッグ情報を表示
    m_pWaveManager->DrawDebugInfo();
#endif // DEBUG
    
    if (m_isPaused)
    {
        DrawPauseMenu();
    }

    // タスクチュートリアルUI描画
    TaskTutorialManager::GetInstance()->Draw();
}

void SceneMain::DrawPauseMenu()
{
    Vec2 mousePos = InputManager::GetInstance()->GetMousePos();

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128); 
    DrawBox(50, 50, Game::kScreenWidth - 50, Game::kScreenHeigth - 50, 0x000000, true);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0); 
}


void SceneMain::SetPaused(bool paused)
{
    m_isPaused = paused;
    SetMouseDispFlag(m_isPaused); 
}

// カメラの感度を設定
void SceneMain::SetCameraSensitivity(float sensitivity)
{
    m_cameraSensitivity = sensitivity;
    if (m_pCamera)
    {
        m_pCamera->SetSensitivity(sensitivity);
    }
}

// プレイヤーの弾が敵にヒットした際に呼ばれる（ヒットマーク表示用）
void SceneMain::OnPlayerBulletHitEnemy(EnemyBase::HitPart part)
{
    m_hitMarkTimer = kHitMarkDuration;
    m_hitMarkType = part;
}

void SceneMain::StopAllEffects()
{
	if (m_pEffect)
	{
		m_pEffect->StopAllEffects();
	}
}
