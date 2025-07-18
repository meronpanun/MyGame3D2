#include "TutorialManager.h"
#include "DxLib.h"
#include <cmath>
#include <cassert>

TutorialManager::TutorialManager() : 
    m_step(Step::None),
    m_moveDone(false),
    m_viewDone(false),
    m_checkMarkHandle(-1),
    m_prevMousePos{0,0},
    m_moveAccumTime(0.0f),
    m_viewAccumTime(0.0f)
{
    m_checkMarkHandle = LoadGraph("data/image/CheckMark.png");
    assert(m_checkMarkHandle != -1);
}

TutorialManager::~TutorialManager()
{
    DeleteGraph(m_checkMarkHandle);
}

void TutorialManager::Init()
{
    m_step = Step::Move;
    m_moveDone = false;
    m_viewDone = false;
    m_jumpDone = false;
    m_runDone = false;
    m_moveAccumTime = 0.0f;
    m_viewAccumTime = 0.0f;
    m_jumpAccumTime = 0.0f;
    m_runAccumTime = 0.0f;
    m_completeWaitTime = 0.0f;
    m_isCompletedDisplay = false;
    m_prevMousePos = Mouse::GetPos();
    m_moveCheckAnim = false;
    m_moveCheckAnimTime = 0.0f;
    m_viewCheckAnim = false;
    m_viewCheckAnimTime = 0.0f;
    m_jumpCheckAnim = false;
    m_jumpCheckAnimTime = 0.0f;
    m_runCheckAnim = false;
    m_runCheckAnimTime = 0.0f;
}

void TutorialManager::Update()
{
    // チュートリアル完了後の待機演出
    if (m_isCompletedDisplay) {
        m_completeWaitTime += 1.0f / 60.0f;
        if (m_completeWaitTime >= 1.0f) {
            m_isCompletedDisplay = false;
            m_step = Step::Completed;
        }
        // アニメタイマーも進める
        if (m_moveCheckAnim) m_moveCheckAnimTime += 1.0f / 60.0f;
        if (m_viewCheckAnim) m_viewCheckAnimTime += 1.0f / 60.0f;
        if (m_jumpCheckAnim) m_jumpCheckAnimTime += 1.0f / 60.0f;
        if (m_runCheckAnim) m_runCheckAnimTime += 1.0f / 60.0f;
        return;
    }
    if (m_step == Step::Completed) return;

    // 1. WASD移動
    if (!m_moveDone) {
        bool isMoving = CheckHitKey(KEY_INPUT_W) || CheckHitKey(KEY_INPUT_A) ||
                        CheckHitKey(KEY_INPUT_S) || CheckHitKey(KEY_INPUT_D);
        if (isMoving) {
            m_moveAccumTime += 1.0f / 60.0f;
        }
        if (m_moveAccumTime >= 2.0f) {
            m_moveDone = true;
            m_moveCheckAnim = true;
            m_moveCheckAnimTime = 0.0f;
            m_step = Step::View;
        }
    }
    // 2. 視点操作
    else if (!m_viewDone) {
        Vec2 now = Mouse::GetPos();
        float dx = now.x - m_prevMousePos.x;
        float dy = now.y - m_prevMousePos.y;
        bool isViewing = (std::abs(dx) > 2 || std::abs(dy) > 2);
        if (isViewing) {
            m_viewAccumTime += 1.0f / 60.0f;
        }
        if (m_viewAccumTime >= 1.0f) {
            m_viewDone = true;
            m_viewCheckAnim = true;
            m_viewCheckAnimTime = 0.0f;
            m_step = Step::Jump;
        }
        m_prevMousePos = now;
    }
    // 3. ジャンプ
    else if (!m_jumpDone) {
        if (CheckHitKey(KEY_INPUT_SPACE)) {
            m_jumpAccumTime += 1.0f / 60.0f;
        }
        if (m_jumpAccumTime >= 0.2f) { // 0.2秒間押下でOK
            m_jumpDone = true;
            m_jumpCheckAnim = true;
            m_jumpCheckAnimTime = 0.0f;
            m_step = Step::Run;
        }
    }
    // 4. 走る（シフト+W）
    else if (!m_runDone) {
        if (CheckHitKey(KEY_INPUT_W) && CheckHitKey(KEY_INPUT_LSHIFT)) {
            m_runAccumTime += 1.0f / 60.0f;
        }
        if (m_runAccumTime >= 1.0f) {
            m_runDone = true;
            m_runCheckAnim = true;
            m_runCheckAnimTime = 0.0f;
            m_isCompletedDisplay = true;
            m_completeWaitTime = 0.0f;
        }
    }
    // アニメタイマー進行
    if (m_moveCheckAnim) m_moveCheckAnimTime += 1.0f / 60.0f;
    if (m_viewCheckAnim) m_viewCheckAnimTime += 1.0f / 60.0f;
    if (m_jumpCheckAnim) m_jumpCheckAnimTime += 1.0f / 60.0f;
    if (m_runCheckAnim) m_runCheckAnimTime += 1.0f / 60.0f;
}

void TutorialManager::Draw(int screenW, int screenH)
{
    // 完了演出中も含めて表示
    if (m_step == Step::None) return;
    if (m_step == Step::Completed && !m_isCompletedDisplay) return;
    int x = screenW - 420;
    int y = 40;
    SetFontSize(22);
    // 1. WASD
    DrawFormatString(x, y, 0xffffff, "WASDで移動してください");
    if (m_moveDone && m_checkMarkHandle >= 0) {
        float scale = 1.0f;
        if (m_moveCheckAnim && m_moveCheckAnimTime < 0.3f) {
            float t = m_moveCheckAnimTime / 0.3f;
            scale = 2.0f - t;
            if (scale < 1.0f) scale = 1.0f;
        } else {
            m_moveCheckAnim = false;
        }
        int size = static_cast<int>(40 * scale);
        int cx = x + 260 + 20;
        int cy = y + 20;
        DrawExtendGraph(cx - size/2, cy - size/2, cx + size/2, cy + size/2, m_checkMarkHandle, true);
    }
    y += 40;
    // 2. 視点
    DrawFormatString(x, y, 0xffffff, "マウスで視点を動かしてください");
    if (m_viewDone && m_checkMarkHandle >= 0) {
        float scale = 1.0f;
        if (m_viewCheckAnim && m_viewCheckAnimTime < 0.3f) {
            float t = m_viewCheckAnimTime / 0.3f;
            scale = 2.0f - t;
            if (scale < 1.0f) scale = 1.0f;
        } else {
            m_viewCheckAnim = false;
        }
        int size = static_cast<int>(40 * scale);
        int cx = x + 320 + 20;
        int cy = y + 20;
        DrawExtendGraph(cx - size/2, cy - size/2, cx + size/2, cy + size/2, m_checkMarkHandle, true);
    }
    y += 40;
    // 3. ジャンプ
    DrawFormatString(x, y, 0xffffff, "スペースキーでジャンプしてください");
    if (m_jumpDone && m_checkMarkHandle >= 0) {
        float scale = 1.0f;
        if (m_jumpCheckAnim && m_jumpCheckAnimTime < 0.3f) {
            float t = m_jumpCheckAnimTime / 0.3f;
            scale = 2.0f - t;
            if (scale < 1.0f) scale = 1.0f;
        } else {
            m_jumpCheckAnim = false;
        }
        int size = static_cast<int>(40 * scale);
        int cx = x + 320 + 20;
        int cy = y + 20;
        DrawExtendGraph(cx - size/2, cy - size/2, cx + size/2, cy + size/2, m_checkMarkHandle, true);
    }
    y += 40;
    // 4. 走る
    DrawFormatString(x, y, 0xffffff, "Shift+Wで走ってください");
    if (m_runDone && m_checkMarkHandle >= 0) {
        float scale = 1.0f;
        if (m_runCheckAnim && m_runCheckAnimTime < 0.3f) {
            float t = m_runCheckAnimTime / 0.3f;
            scale = 2.0f - t;
            if (scale < 1.0f) scale = 1.0f;
        } else {
            m_runCheckAnim = false;
        }
        int size = static_cast<int>(40 * scale);
        int cx = x + 320 + 20;
        int cy = y + 20;
        DrawExtendGraph(cx - size/2, cy - size/2, cx + size/2, cy + size/2, m_checkMarkHandle, true);
    }
    SetFontSize(16);
}

bool TutorialManager::IsActive() const
{
    return m_step != Step::None && m_step != Step::Completed;
}

bool TutorialManager::IsCompleted() const
{
    return m_step == Step::Completed;
}
