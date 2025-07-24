#include "TutorialManager.h"
#include "EffekseerForDXLib.h"
#include "SceneMain.h" 
#include "Mouse.h"
#include "WaveManager.h"
#include <cmath>
#include <cassert>

namespace
{
    // 時間関連
    constexpr float kFrameTime         = 1.0f / 60.0f; // 1フレームの時間
    constexpr float kCompleteWaitTime  = 1.0f; // チュートリアル完了後の待機時間
    constexpr float kMoveAccumGoalTime = 2.0f; // 移動チュートリアルの目標累積時間
    constexpr float kViewAccumGoalTime = 1.0f; // 視点チュートリアルの目標累積時間
    constexpr float kJumpAccumGoalTime = 0.2f; // ジャンプチュートリアルの目標累積時間
    constexpr float kRunAccumGoalTime  = 1.0f; // 走行チュートリアルの目標累積時間
    constexpr float kCheckAnimDuration = 0.3f; // チェックマークのアニメーション時間

    // UI関連
    constexpr int kFontSize               = 22;   // チュートリアルメッセージのフォントサイズ
    constexpr int kDefaultFontSize        = 16;   // デフォルトのフォントサイズ
    constexpr int kMessageOffsetX         = 420;  // メッセージのXオフセット
    constexpr int kInitialYPos            = 40;   // メッセージの初期Y座標
    constexpr int kLineSpacing            = 40;   // メッセージの行間
    constexpr int kCheckMarkBaseSize      = 40;   // チェックマークの基本サイズ
    constexpr int kCheckMarkOffsetXMove   = 280;  // 移動チェックマークのXオフセット
    constexpr int kCheckMarkOffsetXOthers = 340;  // それ以外のチェックマークのXオフセット
    constexpr int kCheckMarkOffsetY       = 20;   // チェックマークのYオフセット
    constexpr float kCheckMarkAnimScale   = 2.0f; // チェックマークアニメーションの最大スケール

    // UIボックス関連
    constexpr int kBoxPaddingX = 20;  // ボックスの左右パディング
    constexpr int kBoxPaddingY = 40;  // ボックスの上下パディング
    constexpr int kBoxAlpha    = 180; // ボックスのアルファ値
    constexpr unsigned int kBoxColor = 0x000000; // ボックスの色

    // タイトル関連
    constexpr int kTitleFontSize = 28;       // タイトルのフォントサイズ
    constexpr int kTitleOffsetY  = 10;       // タイトルのYオフセット
    constexpr int kTitleColor    = 0xFFFFFF; // タイトルの色
    constexpr char kTitleText[]  = "[チュートリアル]"; // タイトルテキスト

    // 各メッセージの長さ（ピクセル）
    constexpr int kMsgWidthWASD  = 260; // "WASDで移動しよう!" の幅
    constexpr int kMsgWidthMouse = 320; // "マウスで視点を動かそう!" の幅
    constexpr int kMsgWidthJump  = 320; // "スペースキーでジャンプ!" の幅
    constexpr int kMsgWidthRun   = 320; // "Shift+Wで走ろう!" の幅

	// マウスの移動量しきい値
	constexpr float kMouseMovementThreshold = 5.0f; // マウスの移動量閾値
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
        if (m_completeWaitTime >= kCompleteWaitTime)
        {
            m_isCompletedDisplay = false;
            m_step = Step::Completed;
        }
    }

    // アニメタイマーを進める
    if (m_isMoveCheckAnim) m_isMoveCheckAnimTime += kFrameTime;
    if (m_isViewCheckAnim) m_isViewCheckAnimTime += kFrameTime;
    if (m_isJumpCheckAnim) m_isJumpCheckAnimTime += kFrameTime;
    if (m_isRunCheckAnim)  m_isRunCheckAnimTime  += kFrameTime;

    if (m_isCompletedDisplay) return;
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
        if (m_moveAccumTime >= kMoveAccumGoalTime) 
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
        bool isViewing = (std::abs(dx) > kMouseMovementThreshold || std::abs(dy) > kMouseMovementThreshold);
        if (isViewing) 
        {
            m_viewAccumTime += kFrameTime;
        }
        if (m_viewAccumTime >= kViewAccumGoalTime) 
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
        if (m_jumpAccumTime >= kJumpAccumGoalTime) 
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
        if (m_runAccumTime >= kRunAccumGoalTime) 
        {
            m_isRunDone          = true;
            m_isRunCheckAnim     = true;
            m_isRunCheckAnimTime = 0.0f;
            m_isCompletedDisplay = true;
            m_completeWaitTime   = 0.0f;
        }
    }
}

void TutorialManager::Draw(int screenW, int screenH)
{
    // 完了演出中も含めて表示
    if (m_step == Step::None) return;
    if (m_step == Step::Completed && !m_isCompletedDisplay) return;

    // UIボックスの描画範囲を計算
    int boxX1 = screenW - kMessageOffsetX - kBoxPaddingX;
    int boxY1 = kInitialYPos - kBoxPaddingY;
    int boxX2 = screenW - kBoxPaddingX;
    int boxY2 = kInitialYPos + kLineSpacing * 4 + kBoxPaddingY + kTitleFontSize + kTitleOffsetY; // タイトル分も考慮

    // 半透明の背景ボックスを描画
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, kBoxAlpha);
    DrawBox(boxX1, boxY1, boxX2, boxY2, kBoxColor, TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    // タイトル表示
    SetFontSize(kTitleFontSize);
    DrawFormatString(screenW - kMessageOffsetX, kInitialYPos - kTitleFontSize - kTitleOffsetY, kTitleColor, kTitleText);

    int x = screenW - kMessageOffsetX;
    int y = kInitialYPos;
    SetFontSize(kFontSize);

    // 1. WASD
    DrawFormatString(x, y, 0xffffff, "WASDで移動しよう!");
    if (m_isMoveDone && m_checkMarkHandle >= 0)
    {
        float scale = 1.0f;
        if (m_isMoveCheckAnim && m_isMoveCheckAnimTime < kCheckAnimDuration)
        {
            float t = m_isMoveCheckAnimTime / kCheckAnimDuration;
            scale = kCheckMarkAnimScale - t;
            if (scale < 1.0f) scale = 1.0f;
        }
        else 
        {
            m_isMoveCheckAnim = false;
        }
        int size = static_cast<int>(kCheckMarkBaseSize * scale);
        int cx = x + kMsgWidthWASD + kCheckMarkOffsetY; // メッセージの幅 + オフセット
        int cy = y + kCheckMarkOffsetY;
        DrawExtendGraph(cx - size * 0.5f, cy - size * 0.5f, cx + size * 0.5f, cy + size * 0.5f, m_checkMarkHandle, true);
    }
    y += kLineSpacing;

    // 2. 視点
    DrawFormatString(x, y, 0xffffff, "マウスで視点を動かそう!");
    if (m_isViewDone && m_checkMarkHandle >= 0) 
    {
        float scale = 1.0f;
        if (m_isViewCheckAnim && m_isViewCheckAnimTime < kCheckAnimDuration) 
        {
            float t = m_isViewCheckAnimTime / kCheckAnimDuration;
            scale = kCheckMarkAnimScale - t;
            if (scale < 1.0f) scale = 1.0f;
        }
        else 
        {
            m_isViewCheckAnim = false;
        }
        int size = static_cast<int>(kCheckMarkBaseSize * scale);
        int cx = x + kMsgWidthMouse + kCheckMarkOffsetY; // メッセージの幅 + オフセット
        int cy = y + kCheckMarkOffsetY;
        DrawExtendGraph(cx - size * 0.5f, cy - size * 0.5f, cx + size * 0.5f, cy + size * 0.5f, m_checkMarkHandle, true);
    }
    y += kLineSpacing;

    // 3. ジャンプ
    DrawFormatString(x, y, 0xffffff, "スペースキーでジャンプ!");
    if (m_isJumpDone && m_checkMarkHandle >= 0) 
    {
        float scale = 1.0f;
        if (m_isJumpCheckAnim && m_isJumpCheckAnimTime < kCheckAnimDuration) 
        {
            float t = m_isJumpCheckAnimTime / kCheckAnimDuration;
            scale = kCheckMarkAnimScale - t;
            if (scale < 1.0f) scale = 1.0f;
        }
        else 
        {
            m_isJumpCheckAnim = false;
        }
        int size = static_cast<int>(kCheckMarkBaseSize * scale);
        int cx = x + kMsgWidthJump + kCheckMarkOffsetY; // メッセージの幅 + オフセット
        int cy = y + kCheckMarkOffsetY;
        DrawExtendGraph(cx - size * 0.5f, cy - size * 0.5f, cx + size * 0.5f, cy + size * 0.5f, m_checkMarkHandle, true);
    }
    y += kLineSpacing;

    // 4. 走る
    DrawFormatString(x, y, 0xffffff, "Shift+Wで走ろう!");
    if (m_isRunDone && m_checkMarkHandle >= 0)
    {
        float scale = 1.0f;
        if (m_isRunCheckAnim && m_isRunCheckAnimTime < kCheckAnimDuration) 
        {
            float t = m_isRunCheckAnimTime / kCheckAnimDuration;
            scale = kCheckMarkAnimScale - t;
            if (scale < 1.0f) scale = 1.0f;
        }
        else
        {
            m_isRunCheckAnim = false;
        }
        int size = static_cast<int>(kCheckMarkBaseSize * scale);
        int cx = x + kMsgWidthRun + kCheckMarkOffsetY; // メッセージの幅 + オフセット
        int cy = y + kCheckMarkOffsetY;
        DrawExtendGraph(cx - size * 0.5f, cy - size * 0.5f, cx + size * 0.5f, cy + size * 0.5f, m_checkMarkHandle, true);
    }

    SetFontSize(kDefaultFontSize);
}

// チュートリアルがアクティブかどうか
bool TutorialManager::IsActive() const
{
    return m_step != Step::None && m_step != Step::Completed;
}

// チュートリアルが完了したかどうか
bool TutorialManager::IsCompleted() const
{
    return m_step == Step::Completed;
}
