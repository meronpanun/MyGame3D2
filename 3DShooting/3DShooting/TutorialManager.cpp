#include "TutorialManager.h"
#include "InputManager.h"
#include <algorithm>
#include <cmath>

namespace
{
    // 時間関連
    constexpr float kFrameTime = 1.0f / 60.0f;    // 1フレームの時間
    constexpr float kCompleteWaitTime = 2.0f;     // チュートリアル完了後の待機時間
    constexpr float kStepCompleteWaitTime = 1.5f; // ステップ完了後の待機時間
    constexpr float kMoveAccumGoalTime = 2.0f; // 移動チュートリアルの目標累積時間
    constexpr float kViewAccumGoalTime = 1.0f; // 視点チュートリアルの目標累積時間
    constexpr float kJumpAccumGoalTime = 0.2f; // ジャンプチュートリアルの目標累積時間
    constexpr float kRunAccumGoalTime = 1.0f;  // 走行チュートリアルの目標累積時間

    // マウスの移動量閾値
    constexpr float kMouseMovementThreshold = 2.0f;
}

TutorialManager::TutorialManager()
    : m_step(Step::None)
    , m_prevMousePos{ 0, 0 }
    , m_moveAccumTime(0.0f)
    , m_viewAccumTime(0.0f)
    , m_completeWaitTime(0.0f)
    , m_isDisplayingCompletion(false)
    , m_hasCompletedJump(false)
    , m_hasCompletedRun(false)
    , m_jumpAccumTime(0.0f)
    , m_runAccumTime(0.0f)
    , m_stepCompleteWaitTime(0.0f)
    , m_isStepCompleted(false)
    , m_hasCompletedMove(false)
    , m_hasCompletedView(false)
{
}

TutorialManager::~TutorialManager()
{
}

void TutorialManager::Init()
{
    m_step = Step::Move;
}

void TutorialManager::Update()
{
    UpdateMessages();

    // チュートリアル完了後の待機演出
    if (m_isDisplayingCompletion)
    {
        m_completeWaitTime += kFrameTime;
        if (m_completeWaitTime >= kCompleteWaitTime)
        {
            m_isDisplayingCompletion = false;
            m_step = Step::Completed;
        }
        return; 
    }

    // ステップ完了後の待機処理
    if (m_isStepCompleted)
    {
        m_stepCompleteWaitTime += kFrameTime;
        if (m_stepCompleteWaitTime >= kStepCompleteWaitTime)
        {
            m_isStepCompleted = false;
            
            // 最後のステップだったら完了演出へ
            if (m_step == Step::Run)
            {
                m_isDisplayingCompletion = true;
                m_completeWaitTime = 0.0f;
            }
            else
            {
                // 次のステップへ
                m_step = static_cast<Step>(static_cast<int>(m_step) + 1);
            }
        }
        return; 
    }

    switch (m_step)
    {
    case Step::Move:
        if (!m_hasCompletedMove)
        {
            bool isMoving = CheckHitKey(KEY_INPUT_W) || CheckHitKey(KEY_INPUT_A) || CheckHitKey(KEY_INPUT_S) || CheckHitKey(KEY_INPUT_D);
            if (isMoving) m_moveAccumTime += kFrameTime;
            if (m_moveAccumTime >= kMoveAccumGoalTime)
            {
                m_hasCompletedMove = true;
            }
        }
        else
        {
            m_isStepCompleted = true;
            m_stepCompleteWaitTime = 0.0f;
        }
        break;
    case Step::View:
        if (!m_hasCompletedView)
        {
            Vec2 now = InputManager::GetInstance()->GetMousePos();
            float dx = now.x - m_prevMousePos.x;
            float dy = now.y - m_prevMousePos.y;

            if (std::abs(dx) > kMouseMovementThreshold || std::abs(dy) > kMouseMovementThreshold)
            {
                m_viewAccumTime += kFrameTime;
            }

            if (m_viewAccumTime >= kViewAccumGoalTime)
            {
                m_hasCompletedView = true;
            }
            m_prevMousePos = now;
        }
        else
        {
            m_isStepCompleted = true;
            m_stepCompleteWaitTime = 0.0f;
        }
        break;
    case Step::Jump:
        if (!m_hasCompletedJump)
        {
            if (CheckHitKey(KEY_INPUT_SPACE)) m_jumpAccumTime += kFrameTime;
            if (m_jumpAccumTime >= kJumpAccumGoalTime)
            {
                m_hasCompletedJump = true;
            }
        }
        else
        {
            m_isStepCompleted = true;
            m_stepCompleteWaitTime = 0.0f;
        }
        break;
    case Step::Run:
        if (!m_hasCompletedRun)
        {
            if (CheckHitKey(KEY_INPUT_W) && CheckHitKey(KEY_INPUT_LSHIFT))
            {
                m_runAccumTime += kFrameTime;
            }
            if (m_runAccumTime >= kRunAccumGoalTime)
            {
                m_hasCompletedRun = true;
            }
        }
        else
        {
            m_isStepCompleted = true;
            m_stepCompleteWaitTime = 0.0f;
        }
        break;
    }
}

void TutorialManager::UpdateMessages()
{
    for (auto it = m_messages.begin(); it != m_messages.end(); )
    {
        it->displayTimer += kFrameTime;
        ++it;
    }
}

bool TutorialManager::IsActive() const
{
    return m_step != Step::None && m_step != Step::Completed;
}

bool TutorialManager::IsCompleted() const
{
    return m_step == Step::Completed;
}

void TutorialManager::Skip()
{
    m_step = Step::Completed;
    m_isDisplayingCompletion = false;
    m_hasCompletedMove = true;
    m_hasCompletedView = true;
    m_hasCompletedJump = true;
    m_hasCompletedRun = true;
}

void TutorialManager::AddMessage(const std::string& title, const std::string& detail)
{
    // 初期値設定
    m_messages.push_back({ title, detail, 750.0f, 0.0f, UIState::Entering });
}
