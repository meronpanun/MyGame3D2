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
    constexpr float kFrameTime            = 1.0f / 60.0f; // 1フレームの時間
    constexpr float kCompleteWaitTime     = 2.0f;  // チュートリアル完了後の待機時間
    constexpr float kStepCompleteWaitTime = 1.5f;  // ステップ完了後の待機時間
    constexpr float kMoveAccumGoalTime    = 2.0f;  // 移動チュートリアルの目標累積時間
    constexpr float kViewAccumGoalTime    = 1.0f;  // 視点チュートリアルの目標累積時間
    constexpr float kJumpAccumGoalTime    = 0.2f;  // ジャンプチュートリアルの目標累積時間
    constexpr float kRunAccumGoalTime     = 1.0f;  // 走行チュートリアルの目標累積時間
    constexpr float kCheckAnimDuration    = 0.3f;  // チェックマークのアニメーション時間

    // UI関連
    constexpr int   kFontSize               = 22;   // チュートリアルメッセージのフォントサイズ
    constexpr int   kDefaultFontSize        = 16;   // デフォルトのフォントサイズ
    constexpr int   kMessageOffsetX         = 420;  // メッセージのXオフセット
    constexpr int   kInitialYPos            = 40;   // メッセージの初期Y座標
    constexpr int   kLineSpacing            = 40;   // メッセージの行間
    constexpr int   kCheckMarkBaseSize      = 40;   // チェックマークの基本サイズ
    constexpr int   kCheckMarkOffsetXMove   = 280;  // 移動チェックマークのXオフセット
    constexpr int   kCheckMarkOffsetXOthers = 340;  // それ以外のチェックマークのXオフセット
    constexpr int   kCheckMarkOffsetY       = 20;   // チェックマークのYオフセット
    constexpr float kCheckMarkAnimScale     = 2.0f; // チェックマークアニメーションの最大スケール
	constexpr int   kKeyImageSize           = 40;   // キー画像のサイズ
	constexpr int   kKeyImageSpacing        = 5;	// キー画像の間隔
	constexpr int   kKeyImageWidth          = 80;   // キー画像の幅
	constexpr int   kKeyImageHeight         = 40;   // キー画像の高さ
	constexpr int   kShiftImageWidth        = 80;   // Shiftキー画像の幅
	constexpr float kCheckMarkDrawSize      = 1.0f; // チャックマーク画像の大きさ

    // UIボックス関連
    constexpr int   kBoxPaddingX = 20;  // ボックスの左右パディング
    constexpr int   kBoxPaddingY = 10;  // ボックスの上下パディング
    constexpr int   kBoxAlpha    = 180; // ボックスのアルファ値
    constexpr unsigned int kBoxColor = 0x000000; // ボックスの色

    // アニメーション関連
    constexpr float kUIAnimationSpeed = 15.0f; // UIがスライドする速度
    constexpr float kUIOffscreenOffsetX = 500.0f; // UIの画面外オフセット

    // タイトル関連
    constexpr int   kTitleFontSize = 28;       // タイトルのフォントサイズ
    constexpr int   kTitleOffsetY  = 10;       // タイトルのYオフセット
    constexpr int   kTitleColor    = 0xFFFFFF; // タイトルの色

	// マウスの移動量閾値
	constexpr float kMouseMovementThreshold = 2.0f;
}

TutorialManager::TutorialManager() : 
    m_step(Step::None),
    m_uiState(UIState::Hidden),
    m_uiXOffset(kUIOffscreenOffsetX),
    m_isMoveDone(false),
    m_isViewDone(false),
    m_checkMarkHandle(-1),
    m_prevMousePos{0,0},
    m_moveAccumTime(0.0f),
    m_viewAccumTime(0.0f),
    m_completeWaitTime(0.0f),
    m_isCompletedDisplay(false),
    m_isMoveCheckAnim(false),
    m_moveCheckAnimTime(0.0f),
    m_isViewCheckAnim(false),
    m_viewCheckAnimTime(0.0f),
    m_isJumpDone(false),
    m_isRunDone(false),
    m_jumpAccumTime(0.0f),
    m_runAccumTime(0.0f),
    m_isJumpCheckAnim(false),
    m_jumpCheckAnimTime(0.0f),
    m_isRunCheckAnim(false),
    m_runCheckAnimTime(0.0f),
    m_stepCompleteWaitTime(0.0f),
    m_isStepCompleted(false)
{
	// チェックマーク画像の読み込み
    m_checkMarkHandle = LoadGraph("data/image/CheckMark.png");
    assert(m_checkMarkHandle != -1);

    // キー画像の読み込み
    m_wKeyHandle = LoadGraph("data/image/W.png");
    m_aKeyHandle = LoadGraph("data/image/A.png");
    m_sKeyHandle = LoadGraph("data/image/S.png");
    m_dKeyHandle = LoadGraph("data/image/D.png");
    assert(m_wKeyHandle != -1);
    assert(m_aKeyHandle != -1);
    assert(m_sKeyHandle != -1);
    assert(m_dKeyHandle != -1);

    m_mouseMoveHorHandle = LoadGraph("data/image/MouseMoveHor.png");
    assert(m_mouseMoveHorHandle != -1);

    m_spaceKeyHandle = LoadGraph("data/image/Space.png");
    assert(m_spaceKeyHandle != -1);

    m_leftShiftKeyHandle = LoadGraph("data/image/LeftShift.png");
    assert(m_leftShiftKeyHandle != -1);

    m_crossHandle = LoadGraph("data/image/Cross.png");
    assert(m_crossHandle != -1);

    // フォントの作成
    m_japaneseFontHandle = CreateFontToHandle("HGPｺﾞｼｯｸE", 20, 3, DX_FONTTYPE_ANTIALIASING_EDGE_8X8);
    assert(m_japaneseFontHandle != -1);
    m_japaneseLargeFontHandle = CreateFontToHandle("HGPｺﾞｼｯｸE", 36, 3, DX_FONTTYPE_ANTIALIASING_EDGE_8X8);
    assert(m_japaneseLargeFontHandle != -1);
}

TutorialManager::~TutorialManager()
{
	// チェックマーク画像の解放
    DeleteGraph(m_checkMarkHandle);

    // キー画像の解放
    DeleteGraph(m_wKeyHandle);
    DeleteGraph(m_aKeyHandle);
    DeleteGraph(m_sKeyHandle);
    DeleteGraph(m_dKeyHandle);
    DeleteGraph(m_mouseMoveHorHandle);
    DeleteGraph(m_spaceKeyHandle);
    DeleteGraph(m_leftShiftKeyHandle);
    DeleteGraph(m_crossHandle);

    // フォントの解放
    DeleteFontToHandle(m_japaneseFontHandle);
    DeleteFontToHandle(m_japaneseLargeFontHandle);
}

void TutorialManager::Init()
{
    m_step = Step::Move;
    m_uiState = UIState::Hidden; //最初は隠しておく
}

void TutorialManager::UpdateUI()
{
    switch (m_uiState)
    {
    case UIState::Hidden:
        // 新しいステップが始まったらEntering状態へ
        if (m_step != Step::None && m_step != Step::Completed)
        {
            m_uiState = UIState::Entering;
        }
        break;
    case UIState::Entering:
        m_uiXOffset -= kUIAnimationSpeed;
        if (m_uiXOffset <= 0.0f)
        {
            m_uiXOffset = 0.0f;
            m_uiState = UIState::OnScreen;
        }
        break;
    case UIState::OnScreen:
        // ステップ完了を待つ
        break;
    case UIState::Exiting:
        m_uiXOffset += kUIAnimationSpeed;
        if (m_uiXOffset >= kUIOffscreenOffsetX)
        {
            m_uiXOffset = kUIOffscreenOffsetX;
            m_uiState = UIState::Hidden;

            // 最後のステップだったら完了演出へ
            if (m_step == Step::Run)
            {
                m_isCompletedDisplay = true;
                m_completeWaitTime = 0.0f;
            }

            // 次のステップへ
            m_step = static_cast<Step>(static_cast<int>(m_step) + 1);
        }
        break;
    }
}

void TutorialManager::Update()
{
    UpdateUI();

    // チュートリアル完了後の待機演出
    if (m_isCompletedDisplay)
    {
        m_completeWaitTime += kFrameTime;
        if (m_completeWaitTime >= kCompleteWaitTime)
        {
            m_isCompletedDisplay = false;
            m_step = Step::Completed;
        }
        return; // 待機中は他の処理をしない
    }

    // アニメタイマーを進める
    if (m_isMoveCheckAnim) m_moveCheckAnimTime += kFrameTime;
    if (m_isViewCheckAnim) m_viewCheckAnimTime += kFrameTime;
    if (m_isJumpCheckAnim) m_jumpCheckAnimTime += kFrameTime;
    if (m_isRunCheckAnim)  m_runCheckAnimTime  += kFrameTime;

    // UIが表示されているときだけ入力チェック
    if (m_uiState != UIState::OnScreen) return;
    
    // ステップ完了後の待機処理
    if (m_isStepCompleted)
    {
        m_stepCompleteWaitTime += kFrameTime;
        if (m_stepCompleteWaitTime >= kStepCompleteWaitTime)
        {
            m_isStepCompleted = false;
            m_uiState = UIState::Exiting; // 退場開始
        }
        return; // 待機中は他の入力を受け付けない
    }
    
    switch (m_step)
    {
    case Step::Move:
        if (!m_isMoveDone)
        {
            bool isMoving = CheckHitKey(KEY_INPUT_W) || CheckHitKey(KEY_INPUT_A) ||
                                CheckHitKey(KEY_INPUT_S) || CheckHitKey(KEY_INPUT_D);
            if (isMoving) m_moveAccumTime += kFrameTime;
            if (m_moveAccumTime >= kMoveAccumGoalTime)
            {
                m_isMoveDone = true;
                m_isMoveCheckAnim = true;
                m_moveCheckAnimTime = 0.0f;
            }
        }
        else if (m_moveCheckAnimTime >= kCheckAnimDuration) // チェックアニメ完了後
        {
            m_isStepCompleted = true; // 待機開始
            m_stepCompleteWaitTime = 0.0f;
        }
        break;
    
    case Step::View:
        if (!m_isViewDone)
        {
            Vec2 now = Mouse::GetPos();
            float dx = now.x - m_prevMousePos.x;
            float dy = now.y - m_prevMousePos.y;
            if (std::abs(dx) > kMouseMovementThreshold || std::abs(dy) > kMouseMovementThreshold)
            {
                m_viewAccumTime += kFrameTime;
            }
            if (m_viewAccumTime >= kViewAccumGoalTime)
            {
                m_isViewDone = true;
                m_isViewCheckAnim = true;
                m_viewCheckAnimTime = 0.0f;
            }
            m_prevMousePos = now;
        }
        else if (m_viewCheckAnimTime >= kCheckAnimDuration)
        {
            m_isStepCompleted = true; // 待機開始
            m_stepCompleteWaitTime = 0.0f;
        }
        break;
    
    case Step::Jump:
        if (!m_isJumpDone)
        {
            if (CheckHitKey(KEY_INPUT_SPACE)) m_jumpAccumTime += kFrameTime;
            if (m_jumpAccumTime >= kJumpAccumGoalTime)
            {
                m_isJumpDone = true;
                m_isJumpCheckAnim = true;
                m_jumpCheckAnimTime = 0.0f;
            }
        }
        else if (m_jumpCheckAnimTime >= kCheckAnimDuration)
        {
            m_isStepCompleted = true; // 待機開始
            m_stepCompleteWaitTime = 0.0f;
        }
        break;
    
    case Step::Run:
        if (!m_isRunDone)
        {
            if (CheckHitKey(KEY_INPUT_W) && CheckHitKey(KEY_INPUT_LSHIFT))
            {
                m_runAccumTime += kFrameTime;
            }
            if (m_runAccumTime >= kRunAccumGoalTime)
            {
                m_isRunDone = true;
                m_isRunCheckAnim = true;
                m_runCheckAnimTime = 0.0f;
            }
        }
        else if (m_runCheckAnimTime >= kCheckAnimDuration)
        {
            m_isStepCompleted = true; // 待機開始
            m_stepCompleteWaitTime = 0.0f;
        }
        break;
    }
}

void TutorialManager::Draw(int screenW, int screenH)
{
    if (m_uiState == UIState::Hidden) return;

    const char* text = "";
    bool is_done = false;
    bool is_check_anim = false;
    float check_anim_time = 0.0f;

    switch (m_step)
    {
        case Step::Move:
        {
            is_done = m_isMoveDone;
            is_check_anim = m_isMoveCheckAnim;
            check_anim_time = m_moveCheckAnimTime;
            
            const char* remaining_text = "で移動しよう!";
            int remaining_text_width = GetDrawStringWidthToHandle(remaining_text, strlen(remaining_text), m_japaneseFontHandle);

            int images_width = kKeyImageSize * 4 + kKeyImageSpacing * 3;
            int box_width = images_width + remaining_text_width + kCheckMarkBaseSize + kBoxPaddingX * 2;
            int box_height = kKeyImageSize + kBoxPaddingY * 2;

            int box_x = screenW - box_width - 20 + m_uiXOffset;
            int box_y = 20;

            // 半透明の背景ボックスを描画
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, kBoxAlpha);
            DrawBox(box_x, box_y, box_x + box_width, box_y + box_height, kBoxColor, true);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

            // キー画像を描画
            int image_x = box_x + kBoxPaddingX;
            int image_y = box_y + kBoxPaddingY;
            DrawExtendGraph(image_x, image_y, image_x + kKeyImageSize, image_y + kKeyImageSize, m_wKeyHandle, true);
            image_x += kKeyImageSize + kKeyImageSpacing;
            DrawExtendGraph(image_x, image_y, image_x + kKeyImageSize, image_y + kKeyImageSize, m_aKeyHandle, true);
            image_x += kKeyImageSize + kKeyImageSpacing;
            DrawExtendGraph(image_x, image_y, image_x + kKeyImageSize, image_y + kKeyImageSize, m_sKeyHandle, true);
            image_x += kKeyImageSize + kKeyImageSpacing;
            DrawExtendGraph(image_x, image_y, image_x + kKeyImageSize, image_y + kKeyImageSize, m_dKeyHandle, true);

            // 残りのテキストを描画
            int text_x = image_x + kKeyImageSize + kKeyImageSpacing;
            int text_y = box_y + (box_height - kFontSize) * 0.5f;
            DrawStringToHandle(text_x, text_y, remaining_text, 0xffffff, m_japaneseFontHandle);

            // チェックマークを描画
            if (is_done && m_checkMarkHandle >= 0)
            {
                float scale = 1.0f;
                if (is_check_anim && check_anim_time < kCheckAnimDuration)
                {
                    float t = check_anim_time / kCheckAnimDuration;
                    scale = kCheckMarkAnimScale - t;
                    if (scale < 1.0f) scale = 1.0f;
                }

                int size = static_cast<int>(kCheckMarkBaseSize * scale);
                int cx = text_x + remaining_text_width + kBoxPaddingX + (kCheckMarkBaseSize * 0.5f);
                int cy = box_y + box_height * 0.5f;
                DrawExtendGraph(cx - size * 0.5f, cy - size * 0.5f, cx + size * 0.5f, cy + size * 0.5f, m_checkMarkHandle, true);
            }
        }
        break;
    case Step::View:
        {
            is_done = m_isViewDone;
            is_check_anim = m_isViewCheckAnim;
            check_anim_time = m_viewCheckAnimTime;

            const char* remaining_text = "で視点を動かそう!";
            int remaining_text_width = GetDrawStringWidthToHandle(remaining_text, strlen(remaining_text), m_japaneseFontHandle);

            int images_width = kKeyImageSize;
            int box_width = images_width + remaining_text_width + kCheckMarkBaseSize + kBoxPaddingX * 2;
            int box_height = kKeyImageSize + kBoxPaddingY * 2;

            int box_x = screenW - box_width - 20 + m_uiXOffset;
            int box_y = 20;

            // 半透明の背景ボックスを描画
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, kBoxAlpha);
            DrawBox(box_x, box_y, box_x + box_width, box_y + box_height, kBoxColor, true);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

            // キー画像を描画
            int image_x = box_x + kBoxPaddingX;
            int image_y = box_y + kBoxPaddingY;
            DrawExtendGraph(image_x, image_y, image_x + kKeyImageSize, image_y + kKeyImageSize, m_mouseMoveHorHandle, true);

            // 残りのテキストを描画
            int text_x = image_x + kKeyImageSize + 5;
            int text_y = box_y + (box_height - kFontSize) * 0.5f;
            DrawStringToHandle(text_x, text_y, remaining_text, 0xffffff, m_japaneseFontHandle);

            // チェックマークを描画
            if (is_done && m_checkMarkHandle >= 0)
            {
                float scale = 1.0f;
                if (is_check_anim && check_anim_time < kCheckAnimDuration)
                {
                    float t = check_anim_time / kCheckAnimDuration;
                    scale = kCheckMarkAnimScale - t;
                    if (scale < 1.0f) scale = 1.0f;
                }

                int size = static_cast<int>(kCheckMarkBaseSize * scale);
                int cx = text_x + remaining_text_width + kBoxPaddingX + (kCheckMarkBaseSize * 0.5f);
                int cy = box_y + box_height * 0.5f;
                DrawExtendGraph(cx - size * 0.5f, cy - size * 0.5f, cx + size * 0.5f, cy + size * 0.5f, m_checkMarkHandle, true);
            }
        }
        break;
    case Step::Jump:
        {
            is_done = m_isJumpDone;
            is_check_anim = m_isJumpCheckAnim;
            check_anim_time = m_jumpCheckAnimTime;

            const char* remaining_text = "でジャンプ!";
            int remaining_text_width = GetDrawStringWidthToHandle(remaining_text, strlen(remaining_text), m_japaneseFontHandle);

            int images_width = kKeyImageWidth;
            int box_width = images_width + remaining_text_width + kCheckMarkBaseSize + kBoxPaddingX * 2;
            int box_height = kKeyImageHeight + kBoxPaddingY * 2;

            int box_x = screenW - box_width - 20 + m_uiXOffset;
            int box_y = 20;

            // 半透明の背景ボックスを描画
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, kBoxAlpha);
            DrawBox(box_x, box_y, box_x + box_width, box_y + box_height, kBoxColor, true);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

            // キー画像を描画
            int image_x = box_x + kBoxPaddingX;
            int image_y = box_y + kBoxPaddingY;
            DrawExtendGraph(image_x, image_y, image_x + kKeyImageWidth, image_y + kKeyImageHeight, m_spaceKeyHandle, true);

            // 残りのテキストを描画
            int text_x = image_x + kKeyImageWidth + 5;
            int text_y = box_y + (box_height - kFontSize) * 0.5f;
            DrawStringToHandle(text_x, text_y, remaining_text, 0xffffff, m_japaneseFontHandle);

            // チェックマークを描画
            if (is_done && m_checkMarkHandle >= 0)
            {
                float scale = 1.0f;
                if (is_check_anim && check_anim_time < kCheckAnimDuration)
                {
                    float t = check_anim_time / kCheckAnimDuration;
                    scale = kCheckMarkAnimScale - t;
                    if (scale < 1.0f) scale = 1.0f;
                }

                int size = static_cast<int>(kCheckMarkBaseSize * scale);
                int cx = text_x + remaining_text_width + kBoxPaddingX + (kCheckMarkBaseSize * 0.5f);
                int cy = box_y + box_height * 0.5f;
                DrawExtendGraph(cx - size * 0.5f, cy - size * 0.5f, cx + size * 0.5f, cy + size * 0.5f, m_checkMarkHandle, true);
            }
        }
        break;
    case Step::Run:
        {
            is_done = m_isRunDone;
            is_check_anim = m_isRunCheckAnim;
            check_anim_time = m_runCheckAnimTime;
            
            const char* remaining_text = "で走ろう!";
            int remaining_text_width = GetDrawStringWidthToHandle(remaining_text, strlen(remaining_text), m_japaneseFontHandle);

            int images_width = kShiftImageWidth + kKeyImageSize + kKeyImageSize + kKeyImageSpacing * 2;
            int box_width = images_width + remaining_text_width + kCheckMarkBaseSize + kBoxPaddingX * 2;
            int box_height = kShiftImageWidth * 0.5f + kBoxPaddingY * 2;

            int box_x = screenW - box_width - 20 + m_uiXOffset;
            int box_y = 20;

            // 半透明の背景ボックスを描画
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, kBoxAlpha);
            DrawBox(box_x, box_y, box_x + box_width, box_y + box_height, kBoxColor, true);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

            // キー画像を描画
            int image_x = box_x + kBoxPaddingX;
            int image_y = box_y + kBoxPaddingY;
            DrawExtendGraph(image_x, image_y, image_x + kShiftImageWidth, image_y + kKeyImageSize, m_leftShiftKeyHandle, true);
            image_x += kShiftImageWidth + kKeyImageSpacing;
            DrawExtendGraph(image_x, image_y, image_x + kKeyImageSize, image_y + kKeyImageSize, m_crossHandle, true);
            image_x += kKeyImageSize + kKeyImageSpacing;
            DrawExtendGraph(image_x, image_y, image_x + kKeyImageSize, image_y + kKeyImageSize, m_wKeyHandle, true);

            // 残りのテキストを描画
            int text_x = image_x + kKeyImageSize + 5;
            int text_y = box_y + (box_height - kFontSize) * 0.5f;
            DrawStringToHandle(text_x, text_y, remaining_text, 0xffffff, m_japaneseFontHandle);

            // チェックマークを描画
            if (is_done && m_checkMarkHandle >= 0)
            {
                float scale = kCheckMarkDrawSize;
                if (is_check_anim && check_anim_time < kCheckAnimDuration)
                {
                    float t = check_anim_time / kCheckAnimDuration;
                    scale = kCheckMarkAnimScale - t;
                    if (scale < 1.0f) scale = 1.0f;
                }

                int size = static_cast<int>(kCheckMarkBaseSize * scale);
                int cx = text_x + remaining_text_width + kBoxPaddingX + (kCheckMarkBaseSize * 0.5f);
                int cy = box_y + box_height * 0.5f;
                DrawExtendGraph(cx - size * 0.5f, cy - size * 0.5f, cx + size * 0.5f, cy + size * 0.5f, m_checkMarkHandle, true);
            }
        }
        break;
    default:
        return;
    }

    if (m_step != Step::Move && m_step != Step::View && m_step != Step::Jump && m_step != Step::Run)
    {
        int text_width = GetDrawStringWidthToHandle(text, strlen(text), m_japaneseFontHandle);
        int box_width = text_width + kCheckMarkBaseSize + kBoxPaddingX * 2;
        int box_height = kCheckMarkBaseSize + kBoxPaddingY * 2;

        int box_x = screenW - box_width - 20 + m_uiXOffset;
        int box_y = 20; 

        // 半透明の背景ボックスを描画
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, kBoxAlpha);
        DrawBox(box_x, box_y, box_x + box_width, box_y + box_height, kBoxColor, true);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

        // テキストを描画
        int text_x = box_x + kBoxPaddingX;
        int text_y = box_y + (box_height - kFontSize) * 0.5f;
        DrawStringToHandle(text_x, text_y, text, 0xffffff, m_japaneseFontHandle);

        // チェックマークを描画
        if (is_done && m_checkMarkHandle >= 0)
        {
            float scale = 1.0f;
            if (is_check_anim && check_anim_time < kCheckAnimDuration)
            {
                float t = check_anim_time / kCheckAnimDuration;
                scale = kCheckMarkAnimScale - t;
                if (scale < 1.0f) scale = 1.0f;
            }

            int size = static_cast<int>(kCheckMarkBaseSize * scale);
            int cx = text_x + text_width + kBoxPaddingX + (kCheckMarkBaseSize * 0.5f);
            int cy = box_y + box_height * 0.5f;
            DrawExtendGraph(cx - size * 0.5f, cy - size * 0.5f, cx + size * 0.5f, cy + size * 0.5f, m_checkMarkHandle, true);
        }
    }
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
