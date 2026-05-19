#include "TutorialUI.h"
#include "Game.h"
#include "DxLib.h"
#include <sstream>
#include <algorithm>

namespace
{
    // UI関連定数
    constexpr int kFontSize = 34;
    constexpr int kCheckMarkBaseSize = 60;
    constexpr int kKeyImageSize = 60;
    constexpr int kKeyImageSpacing = 8;
    constexpr int kKeyImageWidth = 120;
    constexpr int kKeyImageHeight = 60;
    constexpr int kShiftImageWidth = 120;
    
    constexpr int kUIOffsetY = 200;
    constexpr int kBoxPaddingX = 30;
    constexpr int kBoxPaddingY = 15;
    constexpr int kBoxAlpha = 180;
    constexpr unsigned int kBoxColor = 0x000000;

    constexpr float kUIAnimationSpeed = 15.0f;
    constexpr float kUIOffscreenOffsetX = 750.0f;
    constexpr float kCheckAnimDuration = 0.3f;
    constexpr float kCheckMarkAnimScale = 2.0f;

    // メッセージ関連定数
    constexpr float kMessageDisplayTime    =  5.0f;
    constexpr int   kMessageTitleFontSize  =  36;
    constexpr int   kMessageDetailFontSize =  28;
    constexpr int   kMessageOffsetY        = 220;
    constexpr int   kMessageLineSpacing    =   8;
    constexpr int   kMessageTimeBarHeight  =   6;
    constexpr int   kMessageTimeBarPaddingY=  24;

    // レイアウト・余白関連定数
    constexpr int   kBoxRightMargin        =  60;   // ボックスの右マージン（画面端からの距離）
    constexpr int   kBoxInnerSpacing       =  10;   // ボックス内要素間の追加スペース
    constexpr int   kMessageBoxSpacing     =  10;   // メッセージボックス間の縦スペース

    // 色関連定数
    constexpr unsigned int kColorWhite     = 0xffffff; // テキスト色（白）
    constexpr unsigned int kColorTimeBar   = 0xFF0000; // メッセージ時間バー色（赤）

    // フォント関連定数
    constexpr char kJapaneseFontName[]     = "HGPゴシックE";
    constexpr int  kJapaneseFontSize       = 30; // 通常サイズ日本語フォントのサイズ
    constexpr int  kJapaneseLargeFontSize  = 54; // 大サイズ日本語フォントのサイズ
    constexpr int  kFontThickness          =  3; // フォントの太さ

    // スケール変更検知
    constexpr float kScaleChangeTolerance  = 0.001f; // スケール変更検知のしきい値
}

TutorialUI::TutorialUI(TutorialManager* pManager)
    : m_pManager(pManager)
    , m_uiXOffset(kUIOffscreenOffsetX)
    , m_uiState(TutorialManager::UIState::Hidden)
    , m_currentStep(TutorialManager::Step::None)
    , m_moveCheckAnimTime(0.0f)
    , m_viewCheckAnimTime(0.0f)
    , m_jumpCheckAnimTime(0.0f)
    , m_runCheckAnimTime(0.0f)
    , m_isPlayingMoveCheckAnim(false)
    , m_isPlayingViewCheckAnim(false)
    , m_isPlayingJumpCheckAnim(false)
    , m_isPlayingRunCheckAnim(false)
    , m_checkMarkHandle("data/image/CheckMark.png")
    , m_wKeyHandle("data/image/W.png")
    , m_aKeyHandle("data/image/A.png")
    , m_sKeyHandle("data/image/S.png")
    , m_dKeyHandle("data/image/D.png")
    , m_mouseMoveHorHandle("data/image/MouseMoveHor.png")
    , m_spaceKeyHandle("data/image/Space.png")
    , m_leftShiftKeyHandle("data/image/LeftShift.png")
    , m_crossHandle("data/image/Cross.png")
    , m_japaneseFontHandle(kJapaneseFontName, kJapaneseFontSize,      kFontThickness, DX_FONTTYPE_ANTIALIASING_EDGE_8X8)
    , m_japaneseLargeFontHandle(kJapaneseFontName, kJapaneseLargeFontSize, kFontThickness, DX_FONTTYPE_ANTIALIASING_EDGE_8X8)
    , m_messageDetailFontHandle(kJapaneseFontName, kMessageDetailFontSize,  kFontThickness, DX_FONTTYPE_ANTIALIASING_EDGE_8X8)
    , m_prevScale(1.0f)
{
}

void TutorialUI::Init()
{
    ReloadFonts(Game::GetUIScale());
}

void TutorialUI::ReloadFonts(float scale)
{
    m_japaneseFontHandle.Reload(scale);
    m_japaneseLargeFontHandle.Reload(scale);
    m_messageDetailFontHandle.Reload(scale);
}

void TutorialUI::Update(float deltaTime)
{
    if (!m_pManager) return;

    float scale = Game::GetUIScale();
    if (fabsf(scale - m_prevScale) > kScaleChangeTolerance)
    {
        ReloadFonts(scale);
        m_prevScale = scale;
    }

    // ステップが変更されたかチェック
    TutorialManager::Step nextStep = m_pManager->GetStep();
    if (m_currentStep != nextStep)
    {
        if (m_uiState == TutorialManager::UIState::OnScreen || m_uiState == TutorialManager::UIState::Entering)
        {
            m_uiState = TutorialManager::UIState::Exiting;
        }
        else if (m_uiState == TutorialManager::UIState::Hidden)
        {
            if (nextStep != TutorialManager::Step::None && nextStep != TutorialManager::Step::Completed)
            {
                m_currentStep = nextStep;
                m_uiState = TutorialManager::UIState::Entering;
            }
        }
    }

    // UIアニメーション更新
    switch (m_uiState)
    {
    case TutorialManager::UIState::Entering:
        m_uiXOffset -= kUIAnimationSpeed;
        if (m_uiXOffset <= 0.0f)
        {
            m_uiXOffset = 0.0f;
            m_uiState = TutorialManager::UIState::OnScreen;
        }
        break;
    case TutorialManager::UIState::OnScreen:
        if (m_pManager->IsStepCompleted())
        {
            m_uiState = TutorialManager::UIState::Exiting;
        }
        break;
    case TutorialManager::UIState::Exiting:
        m_uiXOffset += kUIAnimationSpeed;
        if (m_uiXOffset >= kUIOffscreenOffsetX)
        {
            m_uiXOffset = kUIOffscreenOffsetX;
            m_uiState = TutorialManager::UIState::Hidden;
            m_currentStep = m_pManager->GetStep(); // 次のステップへ同期
        }
        break;
    default:
        break;
    }

    // チェックマークアニメーションの開始トリガー
    if (m_pManager->HasCompletedMove() && !m_isPlayingMoveCheckAnim) { m_isPlayingMoveCheckAnim = true; m_moveCheckAnimTime = 0.0f; }
    if (m_pManager->HasCompletedView() && !m_isPlayingViewCheckAnim) { m_isPlayingViewCheckAnim = true; m_viewCheckAnimTime = 0.0f; }
    if (m_pManager->HasCompletedJump() && !m_isPlayingJumpCheckAnim) { m_isPlayingJumpCheckAnim = true; m_jumpCheckAnimTime = 0.0f; }
    if (m_pManager->HasCompletedRun()  && !m_isPlayingRunCheckAnim)  { m_isPlayingRunCheckAnim  = true; m_runCheckAnimTime  = 0.0f; }

    // アニメタイマー更新
    if (m_isPlayingMoveCheckAnim) m_moveCheckAnimTime += deltaTime;
    if (m_isPlayingViewCheckAnim) m_viewCheckAnimTime += deltaTime;
    if (m_isPlayingJumpCheckAnim) m_jumpCheckAnimTime += deltaTime;
    if (m_isPlayingRunCheckAnim)  m_runCheckAnimTime  += deltaTime;

    // メッセージのアニメーション更新 (Manager側のメッセージリストを直接更新)
    // プロフェッショナルな設計としては、Manager側のconstなメッセージ情報を元に、
    // UI側で独自のUIStateリストを管理するのが理想だが、今回は簡略化のため
    // Manager側の構造体を直接（無理やり）更新するか、Manager側に更新メソッドを残す。
    // ここでは、Manager側のメッセージのUIプロパティを更新する。
    // ※本来はTutorialManager::UpdateMessagesで行っていた内容
    auto& messages = const_cast<std::vector<TutorialManager::TutorialMessage>&>(m_pManager->GetMessages());
    for (auto& msg : messages)
    {
        switch (msg.state)
        {
        case TutorialManager::UIState::Entering:
            msg.xOffset -= kUIAnimationSpeed;
            if (msg.xOffset <= 0.0f) { msg.xOffset = 0.0f; msg.state = TutorialManager::UIState::OnScreen; }
            break;
        case TutorialManager::UIState::OnScreen:
            if (msg.displayTimer >= kMessageDisplayTime) msg.state = TutorialManager::UIState::Exiting;
            break;
        case TutorialManager::UIState::Exiting:
            msg.xOffset += kUIAnimationSpeed;
            if (msg.xOffset >= kUIOffscreenOffsetX) msg.state = TutorialManager::UIState::Hidden;
            break;
        default:
            break;
        }
    }
}

void TutorialUI::Draw()
{
    if (!m_pManager) return;
    if (m_uiState == TutorialManager::UIState::Hidden && m_pManager->GetMessages().empty()) return;

    int screenW = Game::GetScreenWidth();
    int screenH = Game::GetScreenHeight();
    float scale = Game::GetUIScale();

    DrawMessagesUI(screenW, screenH, scale);
    DrawTaskUI(screenW, screenH, scale);
}

void TutorialUI::DrawTaskUI(int screenW, int screenH, float scale)
{
    if (m_uiState == TutorialManager::UIState::Hidden) return;

    switch (m_currentStep)
    {
    case TutorialManager::Step::Move:
        DrawTutorialItem(screenW, screenH, scale, "で移動しよう!", 
            { &m_wKeyHandle, &m_aKeyHandle, &m_sKeyHandle, &m_dKeyHandle }, 
            m_pManager->HasCompletedMove(), m_isPlayingMoveCheckAnim, m_moveCheckAnimTime);
        break;
    case TutorialManager::Step::View:
        DrawTutorialItem(screenW, screenH, scale, "で視点を動かそう!", 
            { &m_mouseMoveHorHandle }, 
            m_pManager->HasCompletedView(), m_isPlayingViewCheckAnim, m_viewCheckAnimTime);
        break;
    case TutorialManager::Step::Jump:
        DrawTutorialItem(screenW, screenH, scale, "でジャンプ!", 
            { &m_spaceKeyHandle }, 
            m_pManager->HasCompletedJump(), m_isPlayingJumpCheckAnim, m_jumpCheckAnimTime);
        break;
    case TutorialManager::Step::Run:
        DrawTutorialItem(screenW, screenH, scale, "で走ろう!", 
            { &m_leftShiftKeyHandle, &m_crossHandle, &m_wKeyHandle }, 
            m_pManager->HasCompletedRun(), m_isPlayingRunCheckAnim, m_runCheckAnimTime);
        break;
    default:
        break;
    }
}

void TutorialUI::DrawTutorialItem(int screenW, int screenH, float scale, const char* text, 
                                  const std::vector<ManagedGraph*>& icons, bool isDone, 
                                  bool isCheckAnim, float checkAnimTime)
{
    int remainingTextWidth = GetDrawStringWidthToHandle(text, static_cast<int>(strlen(text)), m_japaneseFontHandle);

    int scaledKeyImageSize = static_cast<int>(kKeyImageSize * scale);
    int scaledKeyImageSpacing = static_cast<int>(kKeyImageSpacing * scale);
    int scaledCheckMarkBaseSize = static_cast<int>(kCheckMarkBaseSize * scale);
    int scaledBoxPaddingX = static_cast<int>(kBoxPaddingX * scale);
    int scaledBoxPaddingY = static_cast<int>(kBoxPaddingY * scale);

    // 特殊対応: Shiftキーなどは幅が広い
    int totalIconsWidth = 0;
    for (auto icon : icons) {
        if (icon == &m_spaceKeyHandle || icon == &m_leftShiftKeyHandle) totalIconsWidth += static_cast<int>(kKeyImageWidth * scale);
        else totalIconsWidth += scaledKeyImageSize;
        totalIconsWidth += scaledKeyImageSpacing;
    }
    if (!icons.empty()) totalIconsWidth -= scaledKeyImageSpacing;

    int boxWidth  = totalIconsWidth + remainingTextWidth + scaledCheckMarkBaseSize + scaledBoxPaddingX * 2 + kBoxInnerSpacing;
    int boxHeight = scaledKeyImageSize + scaledBoxPaddingY * 2;

    int boxX = screenW - boxWidth - static_cast<int>(kBoxRightMargin * scale) + static_cast<int>(m_uiXOffset * scale);
    int boxY = static_cast<int>(kUIOffsetY * scale);

    // 背景ボックス
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, kBoxAlpha);
    DrawBox(boxX, boxY, boxX + boxWidth, boxY + boxHeight, kBoxColor, true);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    // アイコン描画
    int currentX = boxX + scaledBoxPaddingX;
    for (auto icon : icons) {
        int iconW = scaledKeyImageSize;
        if (icon == &m_spaceKeyHandle || icon == &m_leftShiftKeyHandle) iconW = static_cast<int>(kKeyImageWidth * scale);
        
        DrawExtendGraph(currentX, boxY + scaledBoxPaddingY, currentX + iconW, boxY + scaledBoxPaddingY + scaledKeyImageSize, *icon, true);
        currentX += iconW + scaledKeyImageSpacing;
    }

    // テキスト
    int textY = boxY + static_cast<int>((boxHeight - static_cast<int>(kFontSize * scale)) * 0.5f);
    DrawStringToHandle(currentX, textY, text, kColorWhite, m_japaneseFontHandle);

    // チェックマーク
    if (isDone)
    {
        float animScale = 1.0f;
        if (isCheckAnim && checkAnimTime < kCheckAnimDuration)
        {
            float t = checkAnimTime / kCheckAnimDuration;
            animScale = kCheckMarkAnimScale - t;
            if (animScale < 1.0f) animScale = 1.0f;
        }

        int size = static_cast<int>(scaledCheckMarkBaseSize * animScale);
        int cx = currentX + remainingTextWidth + scaledBoxPaddingX + static_cast<int>(scaledCheckMarkBaseSize * 0.5f);
        int cy = boxY + static_cast<int>(boxHeight * 0.5f);
        DrawExtendGraph(cx - size / 2, cy - size / 2, cx + size / 2, cy + size / 2, m_checkMarkHandle, true);
    }
}

void TutorialUI::DrawMessagesUI(int screenW, int screenH, float scale)
{
    const auto& messages = m_pManager->GetMessages();
    int yPos = static_cast<int>(kMessageOffsetY * scale);
    
    int scaledBoxPaddingX = static_cast<int>(kBoxPaddingX * scale);
    int scaledBoxPaddingY = static_cast<int>(kBoxPaddingY * scale);

    for (const auto& msg : messages)
    {
        if (msg.state == TutorialManager::UIState::Hidden) continue;

        // 文字列分割
        std::vector<std::string> detailLines;
        std::string line;
        std::istringstream iss(msg.detail);
        while (std::getline(iss, line, '\n')) detailLines.push_back(line);

        // サイズ計算
        int titleW = GetDrawStringWidthToHandle(msg.title.c_str(), -1, m_japaneseLargeFontHandle);
        int maxDetailW = 0;
        for (const auto& l : detailLines) maxDetailW = (std::max)(maxDetailW, GetDrawStringWidthToHandle(l.c_str(), -1, m_messageDetailFontHandle));
        
        int boxW = (std::max)(titleW, maxDetailW) + scaledBoxPaddingX * 2;
        int boxH = scaledBoxPaddingY * 2 + static_cast<int>(kMessageTitleFontSize * scale) + static_cast<int>(kMessageTimeBarPaddingY * 2 * scale) + static_cast<int>(kMessageTimeBarHeight * scale) + static_cast<int>(kMessageDetailFontSize * detailLines.size() * scale);

        int boxX = screenW - boxW - static_cast<int>(kBoxRightMargin * scale) + static_cast<int>(msg.xOffset * scale);
        
        // 描画
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, kBoxAlpha);
        DrawBox(boxX, yPos, boxX + boxW, yPos + boxH, kBoxColor, true);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

        int curY = yPos + scaledBoxPaddingY;
        DrawStringToHandle(boxX + scaledBoxPaddingX, curY, msg.title.c_str(), kColorWhite, m_japaneseLargeFontHandle);
        curY += static_cast<int>(kMessageTitleFontSize * scale) + static_cast<int>(kMessageTimeBarPaddingY * scale);

        if (msg.state == TutorialManager::UIState::OnScreen)
        {
            float progress = 1.0f - (msg.displayTimer / kMessageDisplayTime);
            DrawBox(boxX + scaledBoxPaddingX, curY, boxX + scaledBoxPaddingX + static_cast<int>((boxW - scaledBoxPaddingX * 2) * progress), curY + static_cast<int>(kMessageTimeBarHeight * scale), kColorTimeBar, true);
        }
        curY += static_cast<int>(kMessageTimeBarHeight * scale) + static_cast<int>(kMessageTimeBarPaddingY * scale);

        for (const auto& l : detailLines)
        {
            DrawStringToHandle(boxX + scaledBoxPaddingX, curY, l.c_str(), kColorWhite, m_messageDetailFontHandle);
            curY += static_cast<int>(kMessageDetailFontSize * scale) + static_cast<int>(kMessageLineSpacing * scale);
        }

        yPos += boxH + static_cast<int>(kMessageBoxSpacing * scale);
    }
}
