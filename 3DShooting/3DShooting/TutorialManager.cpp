#include "TutorialManager.h"
#include "EffekseerForDXLib.h"
#include "SceneMain.h" 
#include "Mouse.h"
#include "WaveManager.h"
#include <cmath>
#include <cassert>

namespace
{
	constexpr float kFrameTime = 1.0f / 60.0f; // フレーム時間
	constexpr int kFontSize = 22; // チュートリアルメッセージのフォントサイズ
	constexpr int kMessageOffsetX = 420; // メッセージのXオフセット
}

TutorialManager::TutorialManager() : 
    m_step(Step::None),
    m_isMoveDone(false),
    m_isViewDone(false),
    m_checkMarkHandle(-1),
    m_prevMousePos{0,0},
    m_moveAccumTime(0.0f),
    m_viewAccumTime(0.0f),
    m_completeWaitTime(0.0f),
    m_isCompletedDisplay(false),
    m_isMoveCheckAnim(false),
    m_isMoveCheckAnimTime(0.0f),
    m_isViewCheckAnim(false),
    m_isViewCheckAnimTime(0.0f),
    m_isJumpDone(false),
    m_isRunDone(false),
    m_jumpAccumTime(0.0f),
    m_runAccumTime(0.0f),
    m_isJumpCheckAnim(false),
    m_isJumpCheckAnimTime(0.0f),
    m_isRunCheckAnim(false),
    m_isRunCheckAnimTime(0.0f)
{
	// チェックマーク画像の読み込み
    m_checkMarkHandle = LoadGraph("data/image/CheckMark.png");
    assert(m_checkMarkHandle != -1);
}

TutorialManager::~TutorialManager()
{
	// チェックマーク画像の解放
    DeleteGraph(m_checkMarkHandle);
}

void TutorialManager::Init()
{
    m_step = Step::Move;
}

void TutorialManager::Update()
{
    // チュートリアル完了後の待機演出
    if (m_isCompletedDisplay)
    {
        m_completeWaitTime += kFrameTime;
        if (m_completeWaitTime >= 1.0f) 
        {
            m_isCompletedDisplay = false;
            m_step = Step::Completed;
        }
        // アニメタイマーも進める
        if (m_isMoveCheckAnim) m_isMoveCheckAnimTime += kFrameTime;
        if (m_isViewCheckAnim) m_isViewCheckAnimTime += kFrameTime;
        if (m_isJumpCheckAnim) m_isJumpCheckAnimTime += kFrameTime;
        if (m_isRunCheckAnim)  m_isRunCheckAnimTime  += kFrameTime;
        return;
    }
    if (m_step == Step::Completed) return;

    // 1.WASD移動
    if (!m_isMoveDone) 
    {
        bool isMoving = CheckHitKey(KEY_INPUT_W) || CheckHitKey(KEY_INPUT_A) ||
                        CheckHitKey(KEY_INPUT_S) || CheckHitKey(KEY_INPUT_D);
        if (isMoving) 
        {
            m_moveAccumTime += kFrameTime;
        }
        if (m_moveAccumTime >= 2.0f) 
        {
            m_isMoveDone          = true;
            m_isMoveCheckAnim     = true;
            m_isMoveCheckAnimTime = 0.0f;
            m_step = Step::View;
        }
    }
    // 2.視点操作
    else if (!m_isViewDone) 
    {
        Vec2 now = Mouse::GetPos();
        float dx = now.x - m_prevMousePos.x;
        float dy = now.y - m_prevMousePos.y;
        bool isViewing = (std::abs(dx) > 2 || std::abs(dy) > 2);
        if (isViewing) 
        {
            m_viewAccumTime += kFrameTime;
        }
        if (m_viewAccumTime >= 1.0f) 
        {
            m_isViewDone          = true;
            m_isViewCheckAnim     = true;
            m_isViewCheckAnimTime = 0.0f;
            m_step = Step::Jump;
        }
        m_prevMousePos = now;
    }
    // 3.ジャンプ
    else if (!m_isJumpDone) 
    {
        if (CheckHitKey(KEY_INPUT_SPACE)) 
        {
            m_jumpAccumTime += kFrameTime;
        }
        if (m_jumpAccumTime >= 0.2f) // 0.2秒間押下でOK
        {
            m_isJumpDone          = true;
            m_isJumpCheckAnim     = true;
            m_isJumpCheckAnimTime = 0.0f;
            m_step = Step::Run;
        }
    }
    // 4.走る（シフト+W）
    else if (!m_isRunDone) 
    {
        if (CheckHitKey(KEY_INPUT_W) && CheckHitKey(KEY_INPUT_LSHIFT)) 
        {
            m_runAccumTime += kFrameTime;
        }
        if (m_runAccumTime >= 1.0f) 
        {
            m_isRunDone          = true;
            m_isRunCheckAnim     = true;
            m_isRunCheckAnimTime = 0.0f;
            m_isCompletedDisplay = true;
            m_completeWaitTime   = 0.0f;
        }
    }
    // アニメタイマー進行
    if (m_isMoveCheckAnim) m_isMoveCheckAnimTime += kFrameTime;
    if (m_isViewCheckAnim) m_isViewCheckAnimTime += kFrameTime;
    if (m_isJumpCheckAnim) m_isJumpCheckAnimTime += kFrameTime;
    if (m_isRunCheckAnim)  m_isRunCheckAnimTime  += kFrameTime;
}

void TutorialManager::Draw(int screenW, int screenH)
{
    // 完了演出中も含めて表示
    if (m_step == Step::None) return;
    if (m_step == Step::Completed && !m_isCompletedDisplay) return;
	int x = screenW - kMessageOffsetX;
	int y = 40;
    SetFontSize(kFontSize);
    // 1. WASD
    DrawFormatString(x, y, 0xffffff, "WASDで移動してください");
    if (m_isMoveDone && m_checkMarkHandle >= 0) {
        float scale = 1.0f;
        if (m_isMoveCheckAnim && m_isMoveCheckAnimTime < 0.3f) {
            float t = m_isMoveCheckAnimTime / 0.3f;
            scale = 2.0f - t;
            if (scale < 1.0f) scale = 1.0f;
        } else {
            m_isMoveCheckAnim = false;
        }
        int size = static_cast<int>(40 * scale);
        int cx = x + 260 + 20;
        int cy = y + 20;
        DrawExtendGraph(cx - size/2, cy - size/2, cx + size/2, cy + size/2, m_checkMarkHandle, true);
    }
    y += 40;
    // 2. 視点
    DrawFormatString(x, y, 0xffffff, "マウスで視点を動かしてください");
    if (m_isViewDone && m_checkMarkHandle >= 0) {
        float scale = 1.0f;
        if (m_isViewCheckAnim && m_isViewCheckAnimTime < 0.3f) {
            float t = m_isViewCheckAnimTime / 0.3f;
            scale = 2.0f - t;
            if (scale < 1.0f) scale = 1.0f;
        } else {
            m_isViewCheckAnim = false;
        }
        int size = static_cast<int>(40 * scale);
        int cx = x + 320 + 20;
        int cy = y + 20;
        DrawExtendGraph(cx - size/2, cy - size/2, cx + size/2, cy + size/2, m_checkMarkHandle, true);
    }
    y += 40;
    // 3. ジャンプ
    DrawFormatString(x, y, 0xffffff, "スペースキーでジャンプしてください");
    if (m_isJumpDone && m_checkMarkHandle >= 0) {
        float scale = 1.0f;
        if (m_isJumpCheckAnim && m_isJumpCheckAnimTime < 0.3f) {
            float t = m_isJumpCheckAnimTime / 0.3f;
            scale = 2.0f - t;
            if (scale < 1.0f) scale = 1.0f;
        } else {
            m_isJumpCheckAnim = false;
        }
        int size = static_cast<int>(40 * scale);
        int cx = x + 320 + 20;
        int cy = y + 20;
        DrawExtendGraph(cx - size/2, cy - size/2, cx + size/2, cy + size/2, m_checkMarkHandle, true);
    }
    y += 40;
    // 4. 走る
    DrawFormatString(x, y, 0xffffff, "Shift+Wで走ってください");
    if (m_isRunDone && m_checkMarkHandle >= 0) {
        float scale = 1.0f;
        if (m_isRunCheckAnim && m_isRunCheckAnimTime < 0.3f) {
            float t = m_isRunCheckAnimTime / 0.3f;
            scale = 2.0f - t;
            if (scale < 1.0f) scale = 1.0f;
        } else {
            m_isRunCheckAnim = false;
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
