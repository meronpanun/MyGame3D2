#include "TutorialManager.h"
#include "InputManager.h"
#include <algorithm>
#include <cmath>

namespace
{
    constexpr float kFrameTime           = 1.0f / 60.0f; // 1 フレームの時間（秒）
    constexpr float kCompleteWaitTime    = 2.0f;          // 完了演出の表示時間（秒）
    constexpr float kStepCompleteWait   = 1.5f;           // ステップ完了後の次ステップまでの待機時間（秒）
    constexpr float kMoveAccumGoalTime  = 2.0f;           // 移動チュートリアルのクリア累積時間（秒）
    constexpr float kViewAccumGoalTime  = 1.0f;           // 視点チュートリアルのクリア累積時間（秒）
    constexpr float kJumpAccumGoalTime  = 0.2f;           // ジャンプチュートリアルのクリア累積時間（秒）
    constexpr float kRunAccumGoalTime   = 1.0f;           // 走行チュートリアルのクリア累積時間（秒）
    constexpr float kMouseMoveThreshold = 2.0f;           // 視点移動と判定するマウス移動量の閾値（px）
    constexpr float kTitleStartXOffset  = 750.0f;         // メッセージのスライドイン開始 X オフセット（px）
    constexpr float kUIAnimationSpeed   = 15.0f;          // メッセージのスライドアニメーション速度（px/frame）
    constexpr float kMessageDisplayTime =  5.0f;          // メッセージの表示時間（秒）
}

TutorialManager::TutorialManager()
    : m_step(Step::None)
    , m_prevMousePos{ 0, 0 }
    , m_completeWaitTime(0.0f)
    , m_moveAccumTime(0.0f)
    , m_viewAccumTime(0.0f)
    , m_jumpAccumTime(0.0f)
    , m_runAccumTime(0.0f)
    , m_isDisplayingCompletion(false)
    , m_hasCompletedMove(false)
    , m_hasCompletedView(false)
    , m_hasCompletedJump(false)
    , m_hasCompletedRun(false)
    , m_stepCompleteWaitTime(0.0f)
    , m_isStepCompleted(false)
{
}

void TutorialManager::Init()
{
    m_step = Step::Move;
}

void TutorialManager::Update()
{
    UpdateMessages();

    // 完了演出中
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

    // ステップ完了後の次ステップ移行待機中
    if (m_isStepCompleted)
    {
        m_stepCompleteWaitTime += kFrameTime;
        if (m_stepCompleteWaitTime >= kStepCompleteWait)
        {
            m_isStepCompleted = false;

            if (m_step == Step::Run)
            {
                // 最後のステップ完了 → 完了演出へ
                m_isDisplayingCompletion = true;
                m_completeWaitTime = 0.0f;
            }
            else
            {
                // 次のステップへ進む
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
            bool isMoving = CheckHitKey(KEY_INPUT_W) || CheckHitKey(KEY_INPUT_A)
                         || CheckHitKey(KEY_INPUT_S) || CheckHitKey(KEY_INPUT_D);
            if (isMoving) m_moveAccumTime += kFrameTime;
            if (m_moveAccumTime >= kMoveAccumGoalTime) m_hasCompletedMove = true;
        }
        else
        {
            m_isStepCompleted      = true;
            m_stepCompleteWaitTime = 0.0f;
        }
        break;

    case Step::View:
        if (!m_hasCompletedView)
        {
            Vec2  now = InputManager::GetInstance()->GetMousePos();
            float dx  = now.x - m_prevMousePos.x;
            float dy  = now.y - m_prevMousePos.y;
            if (std::abs(dx) > kMouseMoveThreshold || std::abs(dy) > kMouseMoveThreshold)
            {
                m_viewAccumTime += kFrameTime;
            }
            if (m_viewAccumTime >= kViewAccumGoalTime) m_hasCompletedView = true;
            m_prevMousePos = now;
        }
        else
        {
            m_isStepCompleted      = true;
            m_stepCompleteWaitTime = 0.0f;
        }
        break;

    case Step::Jump:
        if (!m_hasCompletedJump)
        {
            if (CheckHitKey(KEY_INPUT_SPACE)) m_jumpAccumTime += kFrameTime;
            if (m_jumpAccumTime >= kJumpAccumGoalTime) m_hasCompletedJump = true;
        }
        else
        {
            m_isStepCompleted      = true;
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
            if (m_runAccumTime >= kRunAccumGoalTime) m_hasCompletedRun = true;
        }
        else
        {
            m_isStepCompleted      = true;
            m_stepCompleteWaitTime = 0.0f;
        }
        break;

    default:
        break;
    }
}

void TutorialManager::UpdateMessages()
{
    for (auto& msg : m_messages)
    {
        msg.displayTimer += kFrameTime;

        // スライドイン・表示・スライドアウトのアニメーション状態機械
        switch (msg.state)
        {
        case UIState::Entering:
            msg.xOffset -= kUIAnimationSpeed;
            if (msg.xOffset <= 0.0f) { msg.xOffset = 0.0f; msg.state = UIState::OnScreen; }
            break;
        case UIState::OnScreen:
            if (msg.displayTimer >= kMessageDisplayTime) msg.state = UIState::Exiting;
            break;
        case UIState::Exiting:
            msg.xOffset += kUIAnimationSpeed;
            if (msg.xOffset >= kTitleStartXOffset) msg.state = UIState::Hidden;
            break;
        default:
            break;
        }
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
    m_step                   = Step::Completed;
    m_isDisplayingCompletion = false;
    m_hasCompletedMove       = true;
    m_hasCompletedView       = true;
    m_hasCompletedJump       = true;
    m_hasCompletedRun        = true;
}

void TutorialManager::AddMessage(const std::string& title, const std::string& detail)
{
    m_messages.push_back({ title, detail, kTitleStartXOffset, 0.0f, UIState::Entering });
}
