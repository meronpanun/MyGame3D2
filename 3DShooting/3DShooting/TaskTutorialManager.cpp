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
    constexpr unsigned int kTaskTextColor = 0xFFFFFF; // 白色
    constexpr int kTaskFontThickness = 2;
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
    m_fontHandle(-1)
{
    // フォントの作成
    m_fontHandle = CreateFontToHandle("HGPｺﾞｼｯｸE", kTaskFontSize, kTaskFontThickness, DX_FONTTYPE_ANTIALIASING_EDGE_8X8);
}

TaskTutorialManager::~TaskTutorialManager()
{
    if (m_fontHandle != -1)
    {
        DeleteFontToHandle(m_fontHandle);
    }
}

void TaskTutorialManager::Init(WaveManager* pWaveManager, Player* pPlayer)
{
    m_pWaveManager = pWaveManager;
    m_pPlayer = pPlayer;
    m_step = TaskStep::Shoot; // 最初のタスクは射撃
    m_shootKills = 0;
    m_tackleKills = 0;
    
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


void TaskTutorialManager::Update()
{
    switch (m_step)
    {
    case TaskStep::Shoot:
        if (m_shootKills >= kShootKillGoal)
        {
            m_step = TaskStep::Tackle; // 次のステップへ
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
            m_step = TaskStep::Completed; // チュートリアル完了
            if (m_pPlayer)
            {
                m_pPlayer->SetAttackRestrictions(AttackType::None);
            }
        }
        break;
    case TaskStep::Completed:
        // 何か完了演出などを入れたい場合はここに書く
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
    case TaskStep::Shoot:
        taskText = std::to_string(m_shootKills) + " / " + std::to_string(kShootKillGoal);
        DrawStringToHandle(kTaskTextX, kTaskTextY, "射撃で敵を5体倒せ", kTaskTextColor, m_fontHandle);
        DrawStringToHandle(kTaskTextX, kTaskTextY + kTaskFontSize + 5, taskText.c_str(), kTaskTextColor, m_fontHandle);
        break;
    case TaskStep::Tackle:
        taskText = std::to_string(m_tackleKills) + " / " + std::to_string(kTackleKillGoal);
        DrawStringToHandle(kTaskTextX, kTaskTextY, "タックルで敵を5体倒せ", kTaskTextColor, m_fontHandle);
        DrawStringToHandle(kTaskTextX, kTaskTextY + kTaskFontSize + 5, taskText.c_str(), kTaskTextColor, m_fontHandle);
        break;
    case TaskStep::Completed:
        DrawStringToHandle(kTaskTextX, kTaskTextY, "チュートリアル完了！", kTaskTextColor, m_fontHandle);
        break;
    default:
        break;
    }
}