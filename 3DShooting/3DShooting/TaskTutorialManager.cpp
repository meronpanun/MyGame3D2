#include "TaskTutorialManager.h"
#include "WaveManager.h"
#include "Player.h"
#include "DxLib.h"
#include <string>

namespace
{
    // タスクの目標キル数
    constexpr int kShootKillGoal = 5;
    constexpr int kTackleKillGoal = 5;

    // UI関連
    constexpr int kTaskTextX = 20;
    constexpr int kTaskTextY = 20;
    constexpr int kTaskFontSize = 24;
    constexpr int kTaskFontThickness = 2;
    constexpr unsigned int kTaskTextColor = 0xFFFFFF;
}

// 静的メンバ変数の実体を定義
TaskTutorialManager* TaskTutorialManager::m_instance = nullptr;

TaskTutorialManager* TaskTutorialManager::GetInstance()
{
    if (m_instance == nullptr)
    {
        m_instance = new TaskTutorialManager();
    }
    return m_instance;
}

TaskTutorialManager::TaskTutorialManager() :
    m_pWaveManager(nullptr),
    m_pPlayer(nullptr),
    m_step(TaskStep::None),
    m_shootKills(0),
    m_tackleKills(0),
    m_titleFontHandle(-1),
    m_taskFontHandle(-1),
    m_diamondImg(-1),
    m_mouseLeftImg(-1),
    m_mouseRightImg(-1),
    m_titlePosX(0.0f),
    m_titleAnimSpeed(5.0f),
    m_isTitleAnimFinished(false),
    m_taskAlpha(0),
    m_taskFadeSpeed(5.0f),
    m_animationWaitTimer(0),
    m_displayedShootProgress(0.0f),
    m_displayedTackleProgress(0.0f),
    m_progressAnimSpeed(0.02f),
    m_transitionDelayTimer(0)
{
    // フォントの作成
    m_titleFontHandle = CreateFontToHandle("HGPｺﾞｼｯｸE", 32, kTaskFontThickness, DX_FONTTYPE_ANTIALIASING_EDGE_8X8);
    m_taskFontHandle = CreateFontToHandle("HGPｺﾞｼｯｸE", 20, kTaskFontThickness, DX_FONTTYPE_ANTIALIASING_EDGE_8X8);
    m_diamondImg = LoadGraph("data/image/Diamond.png");
    m_mouseLeftImg = LoadGraph("data/image/MouseLeft.png");
    m_mouseRightImg = LoadGraph("data/image/MouseRight.png");
}

TaskTutorialManager::~TaskTutorialManager()
{
    DeleteFontToHandle(m_titleFontHandle);
    DeleteFontToHandle(m_taskFontHandle);
    DeleteGraph(m_diamondImg);
    DeleteGraph(m_mouseLeftImg);
    DeleteGraph(m_mouseRightImg);
}

void TaskTutorialManager::Init(WaveManager* pWaveManager, Player* pPlayer)
{
    m_pWaveManager = pWaveManager;
    m_pPlayer = pPlayer;
    m_step = TaskStep::Shoot; // 最初のタスクは射撃
    m_shootKills = 0;
    m_tackleKills = 0;

	// アニメション初期化
	m_titlePosX = -300.0f; // 画面外からスタート
    m_isTitleAnimFinished = false;
    m_taskAlpha = 0;
    m_animationWaitTimer = 0;
    m_displayedShootProgress = 0.0f;
    m_displayedTackleProgress = 0.0f;
    m_transitionDelayTimer = 0;

    if (m_pWaveManager)
    {
        m_pWaveManager->SpawnTutorialWave(1);
    }
    if (m_pPlayer)
    {
        m_pPlayer->SetAttackRestrictions(AttackType::Shoot);
    }
}

void TaskTutorialManager::NotifyEnemyKilled(AttackType attackType)
{
    switch (m_step)
    {
    case TaskStep::Shoot:
        if (attackType == AttackType::Shoot)
        {
            m_shootKills++;
        }
        break;
    case TaskStep::Tackle:
        if (attackType == AttackType::Tackle)
        {
            m_tackleKills++;
        }
        break;
    default:
        break;
    }
}

void TaskTutorialManager::Reset()
{
    m_step = TaskStep::None;
    m_shootKills = 0;
    m_tackleKills = 0;
    m_pWaveManager = nullptr;
    m_pPlayer = nullptr;
}

void TaskTutorialManager::Skip(WaveManager* pWaveManager)
{
    m_step = TaskStep::Completed;
    m_pWaveManager = pWaveManager;
}

void TaskTutorialManager::Update()
{
    // プログレスバーのアニメーション
    if (m_step == TaskStep::Shoot || m_step == TaskStep::ShootCompleteDelay)
    {
        float targetProgress = static_cast<float>(m_shootKills) / kShootKillGoal;
        if (m_displayedShootProgress < targetProgress)
        {
            m_displayedShootProgress += m_progressAnimSpeed;
            if (m_displayedShootProgress > targetProgress)
            {
                m_displayedShootProgress = targetProgress;
            }
        }
    }
    else if (m_step == TaskStep::Tackle || m_step == TaskStep::TackleCompleteDelay)
    {
        float targetProgress = static_cast<float>(m_tackleKills) / kTackleKillGoal;
        if (m_displayedTackleProgress < targetProgress)
        {
            m_displayedTackleProgress += m_progressAnimSpeed;
            if (m_displayedTackleProgress > targetProgress)
            {
                m_displayedTackleProgress = targetProgress;
            }
        }
    }

    if (!m_isTitleAnimFinished)
    {
        m_titlePosX += m_titleAnimSpeed;
        if (m_titlePosX >= kTaskTextX)
        {
            m_titlePosX = kTaskTextX;
            m_isTitleAnimFinished = true;
            m_animationWaitTimer = 60; // 60フレーム待機
        }
    }
    else
    {
        if (m_animationWaitTimer > 0)
        {
            m_animationWaitTimer--;
        }
        else
        {
            // Task fade-in
            if (m_taskAlpha < 255)
            {
                m_taskAlpha += m_taskFadeSpeed;
                if (m_taskAlpha > 255)
                {
                    m_taskAlpha = 255;
                }
            }
        }
    }

    switch (m_step)
    {
    case TaskStep::Shoot:
        if (m_shootKills >= kShootKillGoal)
        {
            m_step = TaskStep::ShootCompleteDelay;
            m_transitionDelayTimer = 120; // 120フレーム待機
        }
        break;
    case TaskStep::ShootCompleteDelay:
        m_transitionDelayTimer--;
        if (m_transitionDelayTimer <= 0)
        {
            m_step = TaskStep::Tackle; // 次のステップへ

            m_titlePosX = -300.0f;
            m_isTitleAnimFinished = false;
            m_taskAlpha = 0;
            m_animationWaitTimer = 0;
            m_displayedShootProgress = 0.0f;
            m_displayedTackleProgress = 0.0f;

            if (m_pWaveManager)
            {
                m_pWaveManager->SpawnTutorialWave(2);
            }
            if (m_pPlayer)
            {
                m_pPlayer->SetAttackRestrictions(AttackType::Tackle);
            }
        }
        break;
    case TaskStep::Tackle:
        if (m_tackleKills >= kTackleKillGoal)
        {
            m_step = TaskStep::TackleCompleteDelay;
            m_transitionDelayTimer = 120; // 120フレーム待機
        }
        break;
    case TaskStep::TackleCompleteDelay:
        m_transitionDelayTimer--;
        if (m_transitionDelayTimer <= 0)
        {
            m_step = TaskStep::Completed; // チュートリアル完了
            if (m_pPlayer)
            {
                m_pPlayer->SetAttackRestrictions(AttackType::None);
            }
        }
        break;
    case TaskStep::Completed:
        break;
    default:
        break;
    }
}

bool TaskTutorialManager::IsCompleted() const
{
    return m_step == TaskStep::Completed;
}

void TaskTutorialManager::Draw()
{
    std::string taskText = "";

    switch (m_step)
    {
    case TaskStep::ShootCompleteDelay:
    case TaskStep::Shoot:
    {
        int displayedKills = static_cast<int>(m_displayedShootProgress * kShootKillGoal);
        if (displayedKills > kShootKillGoal) { displayedKills = kShootKillGoal; }
        taskText = std::to_string(displayedKills) + " / " + std::to_string(kShootKillGoal);

        constexpr int kTitleFontSize = 32;
        constexpr int kTaskFontSize = 20;
        constexpr int kDiamondSize = 20;
        constexpr int kMouseImgSize = 24;
        constexpr int kSpacing = 5;

        // タイトル
        DrawStringToHandle(m_titlePosX, kTaskTextY, "射撃訓練", kTaskTextColor, m_titleFontHandle);

        // タスク内容
        if (m_isTitleAnimFinished)
        {
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, m_taskAlpha);

            int taskY = kTaskTextY + kTitleFontSize + 10;
            int currentX = kTaskTextX;

            DrawExtendGraph(currentX, taskY, currentX + kDiamondSize, taskY + kDiamondSize, m_diamondImg, true);
            currentX += kDiamondSize + kSpacing;

            DrawExtendGraph(currentX, taskY, currentX + kMouseImgSize, taskY + kMouseImgSize, m_mouseLeftImg, true);
            currentX += kMouseImgSize + kSpacing;

            DrawStringToHandle(currentX, taskY, "でゾンビを倒す", kTaskTextColor, m_taskFontHandle);

            // 進捗
            constexpr int kBarMaxWidth = 150;
            constexpr int kBarHeight = 15;
            int barY = taskY + kTaskFontSize + 10;
            float progress = m_displayedShootProgress;
            int currentBarWidth = static_cast<int>(kBarMaxWidth * progress);

            unsigned int barColor = (progress >= 1.0f) ? GetColor(0, 255, 128) : GetColor(100, 150, 255);

            DrawBox(kTaskTextX, barY, kTaskTextX + kBarMaxWidth, barY + kBarHeight, GetColor(100, 100, 100), true);
            DrawBox(kTaskTextX, barY, kTaskTextX + currentBarWidth, barY + kBarHeight, barColor, true);

            DrawStringToHandle(kTaskTextX + kBarMaxWidth + 5, barY, taskText.c_str(), kTaskTextColor, m_taskFontHandle);

            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        }
    }
    break;
    case TaskStep::TackleCompleteDelay:
    case TaskStep::Tackle:
    {
        int displayedKills = static_cast<int>(m_displayedTackleProgress * kTackleKillGoal);
        if (displayedKills > kTackleKillGoal) { displayedKills = kTackleKillGoal; }
        taskText = std::to_string(displayedKills) + " / " + std::to_string(kTackleKillGoal);

        constexpr int kTitleFontSize = 32;
        constexpr int kTaskFontSize = 20;
        constexpr int kDiamondSize = 20;
        constexpr int kMouseImgSize = 24;
        constexpr int kSpacing = 5;

        // タイトル
        DrawStringToHandle(m_titlePosX, kTaskTextY, "タックル訓練", kTaskTextColor, m_titleFontHandle);

        // タスク内容
        if (m_isTitleAnimFinished)
        {
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, m_taskAlpha);

            int taskY = kTaskTextY + kTitleFontSize + 10;
            int currentX = kTaskTextX;

            DrawExtendGraph(currentX, taskY, currentX + kDiamondSize, taskY + kDiamondSize, m_diamondImg, true);
            currentX += kDiamondSize + kSpacing;

            DrawExtendGraph(currentX, taskY, currentX + kMouseImgSize, taskY + kMouseImgSize, m_mouseRightImg, true);
            currentX += kMouseImgSize + kSpacing;

            DrawStringToHandle(currentX, taskY, "でゾンビを倒す", kTaskTextColor, m_taskFontHandle);

            // 進捗
            constexpr int kBarMaxWidth = 150;
            constexpr int kBarHeight = 15;
            int barY = taskY + kTaskFontSize + 10;
            float progress = m_displayedTackleProgress;
            int currentBarWidth = static_cast<int>(kBarMaxWidth * progress);

            unsigned int barColor = (progress >= 1.0f) ? GetColor(0, 255, 128) : GetColor(100, 150, 255);

            DrawBox(kTaskTextX, barY, kTaskTextX + kBarMaxWidth, barY + kBarHeight, GetColor(100, 100, 100), true);
            DrawBox(kTaskTextX, barY, kTaskTextX + currentBarWidth, barY + kBarHeight, barColor, true);

            DrawStringToHandle(kTaskTextX + kBarMaxWidth + 5, barY, taskText.c_str(), kTaskTextColor, m_taskFontHandle);

            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        }
    }
    break;
    case TaskStep::Completed:
        break;
    default:
        break;
    }
}