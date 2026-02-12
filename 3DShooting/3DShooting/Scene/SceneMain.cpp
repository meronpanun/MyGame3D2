#include "SceneMain.h"
#include "AmmoItem.h"
#include "AnimationManager.h"
#include "BossUI.h"
#include "Camera.h"
#include "DebugUtil.h"
#include "DirectionIndicator.h"
#include "Effect.h"
#include "EffekseerForDXLib.h"
#include "EnemyAcid.h"
#include "EnemyBase.h"
#include "EnemyNormal.h"
#include "EnemyRunner.h"
#include "FirstAidKitItem.h"
#include "Game.h"
#include "InputManager.h"
#include "Player.h"
#include "SceneGameOver.h"
#include "SceneOption.h"
#include "SceneResult.h"
#include "SceneTitle.h"
#include "ScoreManager.h"
#include "ShellCasing.h"
#include "Stage.h"
#include "TaskTutorialManager.h"
#include "TutorialManager.h"
#include "WaveManager.h"
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <string>

// static変数の定義
float SceneMain::s_elapsedTime = 0.0f;
bool SceneMain::s_isSkipTutorial = false;
bool SceneMain::s_isLowHealthTutorialShown = false;

namespace SceneMainConstants
{
    // UI関連の定数
    constexpr int kButtonWidth = 200;           // ボタンの幅
    constexpr int kButtonHeight = 50;           // ボタンの高さ
    constexpr int kFontSize = 48;               // フォントサイズ
    constexpr float kScreenCenterOffset = 0.5f; // 画面中央のオフセット
    constexpr int kButtonYOffset = 70;          // ボタンのY座標オフセット
    constexpr int kButtonSpacing = 20;          // ボタン間のスペース

    // ゲームクリアシーンへの遷移遅延フレーム数
    constexpr int kClearSceneDelayFrames = 60;

    // 戻るボタンとオプションボタンの座標
    constexpr int kReturnButtonX = 210; // 戻るボタンのX座標
    constexpr int kReturnButtonY = 290; // 戻るボタンのY座標
    constexpr int kOptionButtonX = 210; // オプションボタンのX座標
    constexpr int kOptionButtonY = 120; // オプションボタンのY座標

    // カメラの回転速度
    constexpr float kCameraRotaSpeed = 0.0001f;

    // スカイドーム関連
    constexpr float kSkyDomePosY = 200.0f;  // スカイドームのY座標
    constexpr float kSkyDomeScale = 150.0f; // スカイドームのスケール

    // アイテムドロップ時の初期上昇量
    constexpr float kDropInitialHeight = 140.0f;

    // 環境光設定
    constexpr float kAmbientLightR = 0.5f; // 環境光の赤成分
    constexpr float kAmbientLightG = 0.5f; // 環境光の緑成分
    constexpr float kAmbientLightB = 0.5f; // 環境光の青成分
    constexpr float kAmbientLightA = 1.0f; // 環境光のアルファ成分

    // ヒットマーク関連
    constexpr int kHitMarkLineLength = 8;       // ラインの長さ
    constexpr int kHitMarkCenterSpacing = 4;    // 中央の間隔幅
    constexpr int kHitMarkLineThickness = 2;    // ラインの太さ
    constexpr int kHitMarkDuration = 25;        // 表示時間
    constexpr int kHitMarkDoubleLineOffset = 2; // ダブルラインのオフセット

    // スコアポップアップ関連
    constexpr int kScorePopupX = 80;         // スコアポップアップのX座標
    constexpr int kScorePopupY = 60;         // スコアポップアップのY座標
    constexpr int kPopupOffsetY = 32;        // ポップアップのYオフセット
    constexpr int kPopupDuration = 60;       // 表示時間
    constexpr int kTotalScoreDuration = 120; // 合計スコアの表示時間

    // レティクル表示位置補正値
    constexpr int kReticleOffset = 64;

    // Road_floorオブジェトの範囲
    constexpr VECTOR kRoadFloorMin = { -500.0f, 0.0f, -500.0f }; // 床の最小座標
    constexpr VECTOR kRoadFloorMax = { 500.0f, 0.0f, 500.0f };   // 床の最大座標
}

using namespace SceneMainConstants;

SceneMain* g_sceneMainInstance = nullptr;

SceneMain* SceneMain::Instance() { return g_sceneMainInstance; }

SceneMain::SceneMain(bool isReturningFromOtherScene)
    : m_isPaused(false)
    , m_isEscapePressed(false)
    , m_isReturningFromOption(false)
    , m_cameraSensitivity(Game::g_cameraSensitivity)
    , m_hitDistance(0.0f)
    , m_pCamera(std::make_unique<Camera>())
    , m_skyDomeHandle(-1)
    , m_dotDefaultHandle(-1)
    , m_dotOnTargetHandle(-1)
    , m_sgDefaultReticleHandle(-1)
    , m_sgOnTargetReticleHandle(-1)
    , m_hitMarkTimer(0)
    , m_isWave1FirstAidDropped(false)
    , m_isWave1AmmoDropped(false)
    , m_wave1DropCount(0)
    , m_totalScorePopupTimer(0)
    , m_lastTotalScorePopupValue(0)
    , m_bgmHandle(-1)
    , m_isBGMStarted(false)
    , m_isLoading(true)
    , m_isReturningFromOtherScene(isReturningFromOtherScene)
    , m_clearSceneDelayTimer(-1)
    , m_scoreFontHandle(-1)
    , m_isPlayerInit(false)
    , m_isTaskTutorialInit(false)
    , m_pEffect(std::make_unique<Effect>())
    , m_pAnimManager(std::make_unique<AnimationManager>())
    , m_gameOverDelayTimer(-1)
    , m_isTutorialStage(false)
    , m_loadingFrameCount(0)
    , m_loadingDotCount(0)
    , m_loadingAnimTimer(0)
    , m_loadingModelHandle(-1)
    , m_loadingModelPos(VGet(0, 0, 0))
    , m_loadingModelAnimTime(0.0f)
    , m_isShowDebugHUD(false)
    , m_lastDeltaTime(0.0f)
    , m_prevTimeCount(GetNowHiPerformanceCount())
{
    g_sceneMainInstance = this;

    // スコアポップアップ用フォントの作成
    m_scoreFontHandle =
        CreateFontToHandle("Abadi MT", 24, 3, DX_FONTTYPE_ANTIALIASING_EDGE_8X8);
    assert(m_scoreFontHandle != -1);
}

SceneMain::~SceneMain()
{
    // モデルやリソースの解放
    if (m_skyDomeHandle != -1)
    {
        MV1DeleteModel(m_skyDomeHandle);
        m_skyDomeHandle = -1;
    }
    if (m_dotDefaultHandle != -1)
    {
        DeleteGraph(m_dotDefaultHandle);
        m_dotDefaultHandle = -1;
    }
    if (m_dotOnTargetHandle != -1)
    {
        DeleteGraph(m_dotOnTargetHandle);
        m_dotOnTargetHandle = -1;
    }
    if (m_sgDefaultReticleHandle != -1)
    {
        DeleteGraph(m_sgDefaultReticleHandle);
        m_sgDefaultReticleHandle = -1;
    }
    if (m_sgOnTargetReticleHandle != -1)
    {
        DeleteGraph(m_sgOnTargetReticleHandle);
        m_sgOnTargetReticleHandle = -1;
    }

    // アイテムモデルの解放
    FirstAidKitItem::DeleteModel();
    AmmoItem::DeleteModel();
    ShellCasing::DeleteModel();

    // インジケーター画像の解放
    DirectionIndicator::DeleteResources();

    // BGMの解放
    if (m_bgmHandle != -1)
    {
        DeleteSoundMem(m_bgmHandle);
        m_bgmHandle = -1;
    }
    // フォントの解放
    if (m_scoreFontHandle != -1)
    {
        DeleteFontToHandle(m_scoreFontHandle);
        m_scoreFontHandle = -1;
    }

    // ローディング用モデルの解放
    if (m_loadingModelHandle != -1)
    {
        MV1DeleteModel(m_loadingModelHandle);
        m_loadingModelHandle = -1;
    }
}

void SceneMain::Init()
{
    // 経過時間リセット
    s_elapsedTime = 0.0f;

    // ローディング用モデルの読み込み（非同期ロードの前に同期で読み込む）
    m_loadingModelHandle = MV1LoadModel("data/model/NormalZombie.mv1");
    // 初期設定
    // 歩行アニメーションを名前で検索
    int walkAnimIndex = MV1GetAnimIndex(m_loadingModelHandle, "WALK");
    if (walkAnimIndex == -1) walkAnimIndex = 0; // 見つからなければ0番

    // アニメーションをアタッチ (slot 0)
    MV1AttachAnim(m_loadingModelHandle, walkAnimIndex, -1, FALSE);

    m_loadingModelPos = VGet(-200.0f, -750.0f, 600.0f); // 画面左外側、カメラスペースでの位置 (さらに下げる)
    m_loadingModelAnimTime = 0.0f;
    // スケール調整
    MV1SetScale(m_loadingModelHandle, VGet(1.0f, 1.0f, 1.0f));
    // 回転 (右向きに修正: 90度で左だったため-90度に変更)
    MV1SetRotationXYZ(m_loadingModelHandle, VGet(0.0f, -90.0f * DX_PI_F / 180.0f, 0.0f));

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
    m_dotDefaultHandle = LoadGraph("data/image/DotDefault.png");
    m_dotOnTargetHandle = LoadGraph("data/image/DotOnTarget.png");
    m_sgDefaultReticleHandle = LoadGraph("data/image/SGDefaultReticl.png");
    m_sgOnTargetReticleHandle = LoadGraph("data/image/SGOnTargetReticle.png");
    m_bgmHandle = LoadSoundMem("data/sound/BGM/GameSceneBGM.mp3");

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

    // ボスUIの作成
    m_pBossUI = std::make_unique<BossUI>();

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
    m_pWaveManager->SetOnEnemyDeathCallback([this](const VECTOR &pos)
    {
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

    // ヒットマーク用コールバックをWaveManagerに  // 敵ヒット時のコールバック設定
    m_pWaveManager->SetOnEnemyHitCallback(
        [this](EnemyBase::HitPart part, float distance)
        {
            OnPlayerBulletHitEnemy(part, distance);
        });

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
}

// スコアポップアップを追加する
void SceneMain::AddScorePopup(int score, bool isHeadShot, int combo) 
{
  m_scorePopups.push_back(
      {score, combo, kPopupDuration, static_cast<bool>(isHeadShot)});

  if (m_scorePopups.size() > 5) m_scorePopups.pop_front(); // 最大5件まで

  m_totalScorePopupTimer = kTotalScoreDuration; // 合計スコアタイマーリセット

  // 直近の倍率適用済みスコアを保存
  int totalScore = ScoreManager::Instance().GetScore();
  float lastComboRate = ScoreManager::Instance().GetLastComboRate();
  m_lastTotalScorePopupValue = static_cast<int>(totalScore * lastComboRate);
}

void SceneMain::SwitchToMainStage()
{
  // チュートリアルのアイテムを消去
  m_items.clear();

  // WaveManagerをリセット (敵の消去とWave情報の初期化)
  m_pWaveManager->Reset();

  // エフェクトを全て停止
  if (m_pEffect) {
    m_pEffect->StopAllEffects();
  }

  // メインステージをロード
  m_pStage->LoadStage(false);
  m_isTutorialStage = false;

  // プレイヤーの再初期化（位置などをCSVから再取得）
  m_pPlayer->Init(false);
}

SceneBase* SceneMain::Update()
{
    // ローディング中は他の処理を行わない
    if (m_isLoading)
    {
        // ローディングアニメーション更新
        m_loadingAnimTimer++;
        if (m_loadingAnimTimer > 30)
        {
            m_loadingAnimTimer = 0;
            m_loadingDotCount++;
            if (m_loadingDotCount > 3)
            {
                m_loadingDotCount = 0;
            }
        }

        // ローディングモデルの更新 (歩行アニメーション)
        if (m_loadingModelHandle != -1)
        {
            // アニメーション進行
            m_loadingModelAnimTime += 1.0f; // スピード調整
            float totalTime = MV1GetAttachAnimTotalTime(m_loadingModelHandle, 0);
            if (m_loadingModelAnimTime >= totalTime)
            {
                m_loadingModelAnimTime = fmodf(m_loadingModelAnimTime, totalTime);
            }
            MV1SetAttachAnimTime(m_loadingModelHandle, 0, m_loadingModelAnimTime);

            // 移動 (左から右へ)
            m_loadingModelPos.x += 3.0f;
            if (m_loadingModelPos.x > 350.0f) // 画面右端を超えたらループ
            {
                m_loadingModelPos.x = -350.0f;
            }

            // 位置設定 (カメラ固定と仮定して簡易的に配置)
        }

        // 非同期読み込みが完了しているかチェック
        if (GetASyncLoadNum() == 0)
        {
            // 最低限のローディング時間を確保
            m_loadingFrameCount++;
            if (m_loadingFrameCount >= 80)
            {
                m_isLoading = false;
                m_loadingFrameCount = 0;

                // 非同期ロード完了後に1回だけ呼ぶ
                if (!m_isPlayerInit && m_pPlayer)
                {
                    m_pPlayer->Init(m_isTutorialStage);
                    m_isPlayerInit = true;

                    // 非同期読み込みを無効化
                    SetUseASyncLoadFlag(false);
                }
            }
            else
            {
                // グレース期間中もローディング中とみなし、処理を返して真っ暗な画面での進行を防ぐ
                // Draw()でのローディング表示を有効にするためにここを通す
                return this;
            }
        }
        else
        {
            return this;
        }
    }

    // BGM再生
    if (!m_isBGMStarted && !m_isLoading) // ロード完了後に再生開始
    {
        if (CheckSoundMem(m_bgmHandle) == 0)
        {
            ChangeVolumeSoundMem(200, m_bgmHandle);
            PlaySoundMem(m_bgmHandle, DX_PLAYTYPE_LOOP);
        }
        m_isBGMStarted = true;
    }

    // タイムスケールの更新
    Game::UpdateTimeScale();

    // 経過時間を加算
    long long now = GetNowHiPerformanceCount();
    m_lastDeltaTime = (now - m_prevTimeCount) / 1000000.0f;
    m_prevTimeCount = now;

    float dt = (1.0f / 60.0f) * Game::GetTimeScale();
    s_elapsedTime += dt;

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

            if (mousePos.x >= kReturnButtonX &&
                mousePos.x <= kReturnButtonX + kButtonWidth &&
                mousePos.y >= kReturnButtonY &&
                mousePos.y <= kReturnButtonY + kButtonHeight)
            {
                // BGMを停止
                StopSoundMem(m_bgmHandle);
                return new SceneTitle(true);
            }

            if (mousePos.x >= kOptionButtonX &&
                mousePos.x <= kOptionButtonX + kButtonWidth &&
                mousePos.y >= kOptionButtonY &&
                mousePos.y <= kOptionButtonY + kButtonHeight)
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
            m_pTutorialManager->AddMessage("回復アイテム",
                                           "敵を倒すと回復アイテムをドロップする。\n"
                                           "生き残りたければ積極的に行動せよ");
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

        // タスクチュートリアル中もWaveManagerの更新（スポーン処理のみ）を行う
        m_pWaveManager->Update();

        // タスクチュートリアル中はWaveManagerの通常の更新は行わない
        // ただし、敵の更新とプレイヤーの更新は必要

        // WaveManagerからアクティブな敵のリストを取得してプレイヤーを更新
        std::vector<std::shared_ptr<EnemyBase>>& enemyList = m_pWaveManager->GetEnemyList();
        std::vector<EnemyBase*> enemyPtrList;
        for (std::shared_ptr<EnemyBase>& enemy : enemyList)
        {
            enemyPtrList.push_back(enemy.get());
        }
        // プレイヤーは敵を撃ったりタックルしたりするために敵で更新する必要がある
        m_pPlayer->Update(enemyPtrList, m_pStage->GetCollisionData());
        
        // 敵も更新する必要がある
        m_pWaveManager->UpdateEnemies(m_pPlayer->GetBullets(), m_pPlayer->GetTackleInfo(), *m_pPlayer,
            m_pStage->GetCollisionData(), m_pEffect.get());

        // アイテム、スコアポップアップなどの更新はタスクチュートリアル中でも実行
        for (std::shared_ptr<ItemBase>& item : m_items)
        {
            item->Update(m_pPlayer.get(), m_pStage->GetCollisionData());
        }
        m_items.erase(std::remove_if(m_items.begin(), m_items.end(),
            [](const std::shared_ptr<ItemBase>& item) { return item->IsUsed(); }), m_items.end());

        if (m_hitMarkTimer > 0)
        {
            --m_hitMarkTimer;
        }
        for (auto& popup : m_scorePopups)
        {
            popup.timer--;
        }
        while (!m_scorePopups.empty() && m_scorePopups.front().timer <= 0)
        {
            m_scorePopups.pop_front();
        }
        if (m_totalScorePopupTimer > 0)
        {
            m_totalScorePopupTimer--;
        }
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

    // ウェーブ5終了後の遅延処理
    if (m_pWaveManager->GetCurrentWave() > 5)
    {
        if (m_clearSceneDelayTimer == -1) // 遅延がまだ開始されていない場合
        {
            m_clearSceneDelayTimer = kClearSceneDelayFrames; // 遅延タイマーを開始
            StopSoundMem(m_bgmHandle);                       // BGMを停止
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
    m_pWaveManager->UpdateEnemies(m_pPlayer->GetBullets(), m_pPlayer->GetTackleInfo(),
        *m_pPlayer, m_pStage->GetCollisionData(), m_pEffect.get());

    for (std::shared_ptr<ItemBase>& item : m_items)
    {
        item->Update(m_pPlayer.get(), m_pStage->GetCollisionData());
    }
    m_items.erase(std::remove_if(m_items.begin(), m_items.end(),
        [](const std::shared_ptr<ItemBase>& item) { return item->IsUsed(); }), m_items.end());

    // ヒットマークタイマー更新
    if (m_hitMarkTimer > 0)
    {
        --m_hitMarkTimer;
    }

    // スコアポップアップタイマー更新
    for (auto& popup : m_scorePopups)
    {
        popup.timer--;
    }

    // タイマー切れのポップアップ削除
    while (!m_scorePopups.empty() && m_scorePopups.front().timer <= 0)
    {
        m_scorePopups.pop_front();
    }

    // 合計スコアポップアップタイマー更新
    m_pDirectionIndicator->Update(m_pWaveManager->GetEnemyList()); // 方向インジケータも更新
    ScoreManager::Instance().Update();
    return this;
}

bool SceneMain::IsLoading() const { return m_isLoading; }

void SceneMain::Draw()
{
    int screenW, screenH;
    GetScreenState(&screenW, &screenH, nullptr);

    EnemyBase::ResetDrawCount();

    m_pStage->Draw();

    MV1DrawModel(m_skyDomeHandle);

    for (std::shared_ptr<ItemBase>& item : m_items)
    {
        item->Draw();
    }

    // ローディング中の表示
    if (m_isLoading)
    {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
        DrawBox(0, 0, screenW, screenH, 0x000000, true);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

        SetFontSize(static_cast<int>(48 * Game::GetUIScale()));
        std::string loadingText = "Now Loading";
        for (int i = 0; i < m_loadingDotCount; ++i)
        {
            loadingText += ".";
        }

        int textWidth = GetDrawStringWidth(loadingText.c_str(), -1);
        int textX = (screenW - textWidth) / 2;
        float scale = Game::GetUIScale();
        int textY = (screenH - static_cast<int>(48 * scale)) / 2 - static_cast<int>(50 * scale); // テキストを少し上にずらす
        DrawString(textX, textY, loadingText.c_str(), 0xffffff);
        SetFontSize(16);

        // ローディングモデルの描画
        if (m_loadingModelHandle != -1)
        {
            // カメラ設定をローディング専用にする
            VECTOR camPos = VGet(0.0f, -550.0f, 0.0f);

            VECTOR camTarget = VGet(0.0f, -550.0f, 1000.0f);
            SetupCamera_Perspective(60.0f * DX_PI_F / 180.0f);
            SetCameraPositionAndTarget_UpVecY(camPos, camTarget);

            // モデル位置設定
            MV1SetPosition(m_loadingModelHandle, m_loadingModelPos);
            MV1DrawModel(m_loadingModelHandle);

            // カメラ設定を戻す (次のフレームの描画に影響しないように推奨されるが、
            // 実際にはメインループで毎フレーム設定されるため、ここでは簡易的で良い)
        }

        return; // ローディング中はこれ以降描画しない
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
        m_pWaveManager->DrawEnemies(m_isTutorialStage);
    }
    // メインゲームループ中の敵描画 (両方のチュートリアルが完了した場合)
    else
    {
        m_pWaveManager->DrawEnemies(m_isTutorialStage);
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

        // ターゲットに照準が合っているかに応じてハンドルを決定（色はブレンドモードで付ける）
        int currentReticleHandle = m_pPlayer->IsAimingAtEnemy() ? onTargetHandle : defaultHandle;

        // レティクルの描画
        // 中心座標を動的に計算
        int centerX = screenW / 2;
        int centerY = screenH / 2;
        float scale = Game::GetUIScale();

        // 拡大描画時のジャギー対策としてバイリニア補間を有効にする
        SetDrawMode(DX_DRAWMODE_BILINEAR);

        // 反動によるスケール計算
        // GetRecoilScaleは0.0~1.0を返す。これを1.0~1.5倍程度の拡大率に変換
        float recoil = m_pPlayer->GetWeaponManager().GetRecoilScale();
        float recoilScale = 1.0f + (recoil * 0.5f); // 最大1.5倍

        if (currentReticleHandle != -1)
        {
            int reticleWidth = 0;
            int reticleHeight = 0;
            GetGraphSize(currentReticleHandle, &reticleWidth, &reticleHeight);

            int scaledReticleW = static_cast<int>(reticleWidth * scale * recoilScale);
            int scaledReticleH = static_cast<int>(reticleHeight * scale * recoilScale);

            // 敵に照準が合っている場合は赤くする
            bool isAiming = m_pPlayer->IsAimingAtEnemy();
            if (isAiming)
            {
                SetDrawBlendMode(DX_BLENDMODE_ADD, 255); // 加算ブレンドで光らせる
                SetDrawBright(255, 100, 100); // 赤みを帯びさせる
            }

            DrawExtendGraph(centerX - scaledReticleW / 2, centerY - scaledReticleH / 2,
                centerX + scaledReticleW / 2, centerY + scaledReticleH / 2, currentReticleHandle, true);

            if (isAiming)
            {
                SetDrawBright(255, 255, 255);
                SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
            }
        }

        // ドットレティクルを常に描画（こちらは反動の影響を受けず、常に中心）
        int dotReticleHandle = m_pPlayer->IsAimingAtEnemy() ? m_dotOnTargetHandle : m_dotDefaultHandle;
        if (dotReticleHandle != -1)
        {
            int dotReticleWidth = 0;
            int dotReticleHeight = 0;
            GetGraphSize(dotReticleHandle, &dotReticleWidth, &dotReticleHeight);

            int scaledDotW = static_cast<int>(dotReticleWidth * scale);
            int scaledDotH = static_cast<int>(dotReticleHeight * scale);

            DrawExtendGraph(centerX - scaledDotW / 2, centerY - scaledDotH / 2,
                centerX + scaledDotW / 2, centerY + scaledDotH / 2, dotReticleHandle, true);
        }

        // 描画モードをデフォルト(Nearest)に戻す
        SetDrawMode(DX_DRAWMODE_NEAREST);
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
        float scale = Game::GetUIScale();
        int popupBaseX = Game::GetScreenWidth() / 2 + static_cast<int>(kScorePopupX * scale);
        int popupBaseY = Game::GetScreenHeight() / 2 + static_cast<int>(kScorePopupY * scale);
        int scaledPopupOffsetY = static_cast<int>(kPopupOffsetY * scale);
        int idx = 0;
        int totalScore = ScoreManager::Instance().GetScore();
        int combo = ScoreManager::Instance().GetCombo();
        float comboRate = std::pow(1.1f, combo > 0 ? combo - 1 : 0);
        int comboScore = static_cast<int>(totalScore * comboRate);
        float lastComboRate = ScoreManager::Instance().GetLastComboRate();
        int displayCombo = (ScoreManager::Instance().GetCombo() > 1) ? ScoreManager::Instance().GetCombo() : 1;

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
                DrawFormatStringToHandle(popupBaseX, popupBaseY, 0x00ffcc, m_scoreFontHandle,
                    "%d ×%.2f", m_lastTotalScorePopupValue, lastComboRate);
            }
            else
            {
                DrawFormatStringToHandle(popupBaseX, popupBaseY, 0x00ffcc, m_scoreFontHandle,
                    "%d", m_lastTotalScorePopupValue);
            }
        }
        else if (showScorePopup)
        {
            // 合計スコア
            if (lastComboRate > 1.0f)
            {
                DrawFormatStringToHandle(popupBaseX, popupBaseY + idx * scaledPopupOffsetY,
                    0x00ffcc, m_scoreFontHandle, "%d ×%.2f", comboScore, lastComboRate);
            }
            else
            {
                DrawFormatStringToHandle(popupBaseX, popupBaseY + idx * scaledPopupOffsetY,
                    0x00ffcc, m_scoreFontHandle, "%d", comboScore);
            }
            idx++;
            int lastIsHeadShot = -1;
            for (const auto& popup : m_scorePopups)
            {
                if (lastIsHeadShot == -1 || lastIsHeadShot != static_cast<int>(popup.isHeadShot))
                {
                    if (popup.isHeadShot)
                    {
                        DrawFormatStringToHandle(popupBaseX, popupBaseY + idx * scaledPopupOffsetY,
                            0xffe000, m_scoreFontHandle, "%dpt ヘッドショットキル×%d", 200, displayCombo);
                    }
                    else
                    {
                        DrawFormatStringToHandle(popupBaseX, popupBaseY + idx * scaledPopupOffsetY,
                            0xffffff, m_scoreFontHandle, "%dpt ゾンビキル×%d", 100, displayCombo);
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
        // 距離減衰の計算
        // 基準距離(3m)以内は減衰なし、消失距離(20m)以上は最小アルファ
        constexpr float kMinDistance = 300.0f;
        constexpr float kMaxDistance = 2000.0f;
        constexpr float kMaxRatio = 1.0f;
        constexpr float kMinRatio = 0.2f;

        float ratio = 1.0f;
        if (m_hitDistance > kMinDistance)
        {
            float t = (m_hitDistance - kMinDistance) / (kMaxDistance - kMinDistance);
            t = (std::min)(t, 1.0f); // Clamp 0~1
            ratio = kMaxRatio * (1.0f - t) + kMinRatio * t;
        }

        // アルファ値を計算
        int alpha = static_cast<int>((255 * m_hitMarkTimer * ratio) / kHitMarkDuration);
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

        // 赤 or 黄色
        unsigned int color = (m_hitMarkType == EnemyBase::HitPart::Head) ? 0xffd700 : 0xff4500;

        // 通常のヒットマーク描画
        // 左上→右下
        float scale = Game::GetUIScale();
        int scaledLineLength = static_cast<int>(kHitMarkLineLength * scale);
        int scaledCenterSpacing = static_cast<int>(kHitMarkCenterSpacing * scale);
        int scaledThickness = (std::max)(1, static_cast<int>(kHitMarkLineThickness * scale));
        int centerX = Game::GetScreenWidth() / 2;
        int centerY = Game::GetScreenHeight() / 2;
        
        DrawLine(centerX - scaledLineLength, centerY - scaledLineLength,
            centerX - scaledCenterSpacing, centerY - scaledCenterSpacing, color, scaledThickness);
        DrawLine(centerX + scaledCenterSpacing, centerY + scaledCenterSpacing,
            centerX + scaledLineLength, centerY + scaledLineLength, color, scaledThickness);
        // 左下→右上
        DrawLine(centerX - scaledLineLength, centerY + scaledLineLength,
            centerX - scaledCenterSpacing, centerY + scaledCenterSpacing, color, scaledThickness);
        DrawLine(centerX + scaledCenterSpacing, centerY - scaledCenterSpacing,
            centerX + scaledLineLength, centerY - scaledLineLength, color, scaledThickness);

        // ヘッドショットの場合は二重線を描画
        if (m_hitMarkType == EnemyBase::HitPart::Head)
        {
            int offset = static_cast<int>(kHitMarkDoubleLineOffset * scale);
            DrawLine(centerX - scaledLineLength - offset, centerY - scaledLineLength + offset,
                centerX - scaledCenterSpacing - offset, centerY - scaledCenterSpacing + offset, color, scaledThickness);
            DrawLine(centerX + scaledCenterSpacing + offset, centerY + scaledCenterSpacing - offset,
                centerX + scaledLineLength + offset, centerY + scaledLineLength - offset, color, scaledThickness);
            DrawLine(centerX - scaledLineLength - offset, centerY + scaledLineLength - offset,
                centerX - scaledCenterSpacing - offset, centerY + scaledCenterSpacing - offset, color, scaledThickness);
            DrawLine(centerX + scaledCenterSpacing + offset, centerY - scaledCenterSpacing + offset,
                centerX + scaledLineLength + offset, centerY - scaledLineLength + offset, color, scaledThickness);
        }

        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }

    // ボスUIの描画
    if (m_pBossUI)
    {
        m_pBossUI->Draw(m_pWaveManager->GetEnemyList());
    }

    if (m_isPaused)
    {
        DrawPauseMenu();
    }

    // タスクチュートリアルUI描画
    TaskTutorialManager::GetInstance()->Draw();

    // デバッグHUD描画
    if (m_isShowDebugHUD)
    {
        DrawDebugHUD();
    }
}

void SceneMain::SetShowDebugHUD(bool show)
{
    m_isShowDebugHUD = show;
    // EnemyBase側のフラグも連動させる
    EnemyBase::SetShowDamage(show);
}

void SceneMain::DrawDebugHUD()
{
    int screenW = Game::GetScreenWidth();
    int screenH = Game::GetScreenHeight();

    // 画面全体を半透明の黒で覆う
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);
    DrawBox(0, 0, screenW, screenH, 0x000000, true);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    // 情報収集
    VECTOR playerPos = m_pPlayer ? m_pPlayer->GetPos() : VGet(0, 0, 0);

    float fps = GetFPS();
    // WaveManager.hでは GetAliveEnemyCount() となっていた
    int aliveEnemyCount = m_pWaveManager ? m_pWaveManager->GetAliveEnemyCount() : 0;
    int drawnEnemyCount = EnemyBase::GetDrawCount();

    // Last Damage
    float lastDamage = EnemyBase::GetDebugLastDamage();
    std::string hitInfo = EnemyBase::GetDebugHitInfo();
    int damageTimer = EnemyBase::GetDebugDamageTimer();
    std::string damageStr = (damageTimer > 0) ? std::to_string((int)lastDamage) + (hitInfo.empty() ? "" : " " + hitInfo) : "-";

    // テキスト描画 (左上)
    int x = 20;
    int y = 20;
    int lineHeight = 20;
    unsigned int color = 0xFFFFFF;

    DrawFormatString(x, y, color, "FPS: %.1f", fps);
    y += lineHeight;
    DrawFormatString(x, y, color, "Delta Time: %.4f", m_lastDeltaTime);
    y += lineHeight;
    DrawFormatString(x, y, color, "Player Pos: (%.1f, %.1f, %.1f)", playerPos.x,
                     playerPos.y, playerPos.z);
    y += lineHeight;
    if (m_pPlayer)
    {
        DrawFormatString(x, y, color, "Speed: %.2f", m_pPlayer->GetCurrentSpeed());
        y += lineHeight;
    }

    DrawFormatString(x, y, color, "Active Enemy Count: %d", aliveEnemyCount);
    y += lineHeight;
    DrawFormatString(x, y, color, "Drawn Enemy Count: %d", drawnEnemyCount);
    y += lineHeight;
    DrawFormatString(x, y, color, "Total Defeated: %d", ScoreManager::Instance().GetTotalDefeatedCount());
    y += lineHeight;
    DrawFormatString(x, y, color, "Last Damage: %s", damageStr.c_str());
    y += lineHeight;
}

void SceneMain::DrawPauseMenu()
{
    Vec2 mousePos = InputManager::GetInstance()->GetMousePos();

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);
    DrawBox(50, 50, Game::GetScreenWidth() - 50, Game::GetScreenHeight() - 50, 0x000000, true);
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

/// <summary>
/// プレイヤーの弾が敵にヒットした際に呼ばれる
/// </summary>
void SceneMain::OnPlayerBulletHitEnemy(EnemyBase::HitPart part, float distance)
{
    // ヒット距離を保存
    m_hitDistance = distance;

    // ヒット部位によって処理を分ける（例：ヘッドショット時のSEなど）
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
