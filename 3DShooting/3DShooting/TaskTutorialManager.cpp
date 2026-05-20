#include "TaskTutorialManager.h"
#include "DxLib.h"
#include "Game.h"
#include "Player.h"
#include "WaveManager.h"
#include "Scene/SceneMain.h"
#include "TutorialManager.h"
#include "TutorialTasks.h"
#include <string>
#include <algorithm>

namespace
{
    // UI レイアウト（基準解像度 720p 時のピクセル値）
    constexpr int kTaskTextX     =  60; // タスクテキストの X 座標
    constexpr int kTaskTextY     = 200; // タスクテキストの Y 座標
    constexpr int kTitleFontSize =  48; // タイトルフォントサイズ
    constexpr int kBarMaxWidth   = 300; // 進捗バーの最大幅
    constexpr int kBarHeight     =  22; // 進捗バーの高さ

    // 背景ボックス
    constexpr int          kBgBoxPaddingX =  20;      // 背景ボックス左右パディング
    constexpr int          kBgBoxPaddingY =  15;      // 背景ボックス上下パディング
    constexpr int          kBgBoxWidth    = 580;      // 背景ボックス幅
    constexpr int          kBgBoxHeight   = 250;      // 背景ボックス高さ
    constexpr unsigned int kBgBoxColor    = 0x000000; // 背景ボックスの色
    constexpr int          kBgBoxAlpha    = 128;      // 背景ボックスのアルファ値

    // アニメーション・ステップ遷移タイミング
    constexpr int   kTitleAnimWaitFrames  =  30;     // タイトルアニメーション完了後の待機フレーム数
    constexpr int   kStepTransitionDelay  =  60;     // ステップ遷移までの待機フレーム数
    constexpr float kTitleStartX          = -450.0f; // タイトルスライドインの開始 X 座標
    constexpr float kTitleStartXShort     = -300.0f; // 短いタイトル用の開始 X 座標（盾投げ・パリィ）
    constexpr float kScaleChangeTolerance =   0.001f; // UI スケール変化を検出する閾値

    // 制限アクションフィードバック
    constexpr int kRestrictedActionTimerMax         = 120; // 制限アクション表示の最大タイマー値
    constexpr int kRestrictedActionFadeInThreshold  = 100; // フェードイン継続の閾値（タイマー残量）
    constexpr int kRestrictedActionFadeOutThreshold =  30; // フェードアウト開始の閾値（タイマー残量）
    constexpr int kRestrictedActionAlphaIncrement   =  25; // フェードイン時のアルファ加算量
    constexpr int kRestrictedActionAlphaDecrement   =  10; // フェードアウト時のアルファ減算量

    // 制限アクション・フィードバック画像描画
    constexpr float kFeedbackImageScale        = 0.12f; // フィードバック背景画像のスケール
    constexpr float kFeedbackImageCenterYRatio = 0.6f;  // 画面高さに対するフィードバック画像の縦位置比率
    constexpr int   kIconBaseSize              =  36;   // アイコンの基準サイズ（px）
    constexpr float kIconScaleFactor           = 1.5f;  // アイコンのスケール倍率

    // パリィ説明オーバーレイ
    constexpr int kParryOverlayAlpha = 128; // 暗転オーバーレイのアルファ値
    constexpr int kParryIconSize     =  32; // パリィ説明中のアイコンサイズ（px）
    constexpr int kParryTextOffsetY  =  50; // パリィ説明テキストの縦オフセット（px）

    // Draw 内のレイアウト補助
    constexpr int kTitleToTaskGapY   = 10; // タイトルとタスクテキストの縦間隔（px）
    constexpr int kProgressBarOffsetY = 40; // タスクテキストから進捗バーまでの縦間隔（px）
    constexpr int kBarToTextGap      =  5; // 進捗バーとテキストの横間隔（px）
}

TaskTutorialManager* TaskTutorialManager::GetInstance()
{
    static TaskTutorialManager instance;
    return &instance;
}

TaskTutorialManager::TaskTutorialManager()
    : m_pWaveManager(nullptr)
    , m_pPlayer(nullptr)
    , m_step(TaskStep::None)
    , m_currentTask(nullptr)
    , m_titleFont("HGPゴシックE", 48, 3, DX_FONTTYPE_ANTIALIASING_EDGE_8X8)
    , m_taskFont("HGPゴシックE", 30, 3, DX_FONTTYPE_ANTIALIASING_EDGE_8X8)
    , m_diamondImg("data/image/Diamond.png")
    , m_mouseLeftImg("data/image/MouseLeft.png")
    , m_mouseRightImg("data/image/MouseRight.png")
    , m_alpha1Img("data/image/Alpha1.png")
    , m_alpha2Img("data/image/Alpha2.png")
    , m_mouseWheelImg("data/image/MouseWheel.png")
    , m_rKeyImg("data/image/R.png")
    , m_lockOnUIImg("data/image/LockOnUI.png")
    , m_mouseRightGuardImg("data/image/MouseRight.png")
    , m_designerImg("data/image/Designer.png")
    , m_titlePosX(0.0f)
    , m_titleAnimSpeed(15.0f)
    , m_isTitleAnimFinished(false)
    , m_taskAlpha(0)
    , m_taskFadeSpeed(15.0f)
    , m_animationWaitTimer(0)
    , m_displayedProgress(0.0f)
    , m_progressAnimSpeed(0.05f)
    , m_transitionDelayTimer(0)
    , m_hasShownParryTutorial(false)
    , m_isParryTutorialPaused(false)
    , m_restrictedActionTimer(0)
    , m_restrictedActionType(AttackType::None)
    , m_restrictedActionAlpha(0)
    , m_prevScale(1.0f)
{
    ReloadFonts(1.0f);
}

void TaskTutorialManager::ReloadFonts(float scale)
{
    m_titleFont.Reload(scale);
    m_taskFont.Reload(scale);
}

void TaskTutorialManager::Init(WaveManager* pWaveManager, Player* pPlayer)
{
    m_pWaveManager = pWaveManager;
    m_pPlayer      = pPlayer;
    m_step         = TaskStep::Shoot;
    m_currentTask  = std::make_unique<ShootTutorialTask>();
    m_currentTask->Start(m_pWaveManager, m_pPlayer);

    // アニメーション初期化
    m_titlePosX           = kTitleStartX;
    m_isTitleAnimFinished = false;
    m_taskAlpha           = 0;
    m_animationWaitTimer  = 0;
    m_displayedProgress   = 0.0f;
    m_transitionDelayTimer = 0;
    m_hasShownParryTutorial = false;
    m_isParryTutorialPaused = false;

    m_restrictedActionTimer = 0;
    m_restrictedActionType  = AttackType::None;
    m_restrictedActionAlpha = 0;

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
    if (m_currentTask) m_currentTask->NotifyEnemyKilled(static_cast<int>(attackType));
}

void TaskTutorialManager::NotifyShieldThrowKill()
{
    if (m_currentTask) m_currentTask->NotifyShieldThrowKill();
}

void TaskTutorialManager::NotifyRestrictedAction(AttackType attemptedType)
{
    if (m_restrictedActionTimer > 0 && m_restrictedActionType == attemptedType)
    {
        m_restrictedActionTimer = kRestrictedActionTimerMax; // タイマー延長
    }
    else
    {
        m_restrictedActionType  = attemptedType;
        m_restrictedActionTimer = kRestrictedActionTimerMax;
        m_restrictedActionAlpha = 0; // フェードインから開始
    }
}

void TaskTutorialManager::NotifyParrySuccess()
{
    if (m_currentTask) m_currentTask->NotifyParrySuccess();
}

void TaskTutorialManager::NotifyParryableAttack()
{
    // パリィタスク中かつ、まだ説明を表示していない場合に一時停止する
    if (m_step == TaskStep::Parry && !m_hasShownParryTutorial && !m_isParryTutorialPaused)
    {
        m_isParryTutorialPaused = true;
        m_hasShownParryTutorial = true;
        Game::SetPaused(true);
    }
}

void TaskTutorialManager::Reset()
{
    m_step        = TaskStep::None;
    m_currentTask = nullptr;
    m_pWaveManager = nullptr;
    if (m_pPlayer) m_pPlayer->SetAttackRestrictions(AttackType::None);
    m_pPlayer               = nullptr;
    m_hasShownParryTutorial = false;
    m_isParryTutorialPaused = false;
    Game::SetPaused(false);
}

void TaskTutorialManager::Skip(WaveManager* pWaveManager)
{
    m_step         = TaskStep::Completed;
    m_pWaveManager = pWaveManager;
    if (m_pPlayer) m_pPlayer->SetAttackRestrictions(AttackType::None);
    m_hasShownParryTutorial = false;
    m_isParryTutorialPaused = false;
    Game::SetPaused(false);
}

void TaskTutorialManager::SkipToParry()
{
    // 必要なポインタがセットされていない場合は SceneMain から取得を試みる
    if (!m_pWaveManager || !m_pPlayer)
    {
        if (SceneMain::Instance())
        {
            m_pWaveManager = SceneMain::Instance()->GetWaveManager();
            m_pPlayer      = SceneMain::Instance()->GetPlayerPtr();
        }
    }
    if (!m_pWaveManager || !m_pPlayer) return;

    // 前半の基本チュートリアル（TutorialManager）が未完了の場合は強制終了させる
    if (SceneMain::Instance() && SceneMain::Instance()->GetTutorialManager())
    {
        SceneMain::Instance()->GetTutorialManager()->Skip();
        SceneMain::Instance()->SetTaskTutorialInit(true);
    }

    // パリィタスクから再開（アニメーション状態も一括リセット）
    m_animationWaitTimer   = 0;
    m_transitionDelayTimer = 0;
    BeginTask(TaskStep::Parry, std::make_unique<ParryTutorialTask>(), kTitleStartX);

    Game::SetPaused(false);
}

void TaskTutorialManager::BeginTask(TaskStep nextStep, std::unique_ptr<ITutorialTask> task, float startX)
{
    m_step        = nextStep;
    m_currentTask = std::move(task);
    m_currentTask->Start(m_pWaveManager, m_pPlayer);

    m_titlePosX           = startX;
    m_isTitleAnimFinished = false;
    m_taskAlpha           = 0;
    m_displayedProgress   = 0.0f;
}

void TaskTutorialManager::Update()
{
    // パリィ説明表示中：右クリックで再開
    if (m_isParryTutorialPaused)
    {
        if (GetMouseInput() & MOUSE_INPUT_RIGHT)
        {
            m_isParryTutorialPaused = false;
            Game::SetPaused(false);
        }
        return;
    }

    // 制限アクションフィードバックのアルファ更新
    if (m_restrictedActionTimer > 0)
    {
        m_restrictedActionTimer--;
        if      (m_restrictedActionTimer > kRestrictedActionFadeInThreshold)
            m_restrictedActionAlpha = (std::min)(m_restrictedActionAlpha + kRestrictedActionAlphaIncrement, 255);
        else if (m_restrictedActionTimer < kRestrictedActionFadeOutThreshold)
            m_restrictedActionAlpha = (std::max)(m_restrictedActionAlpha - kRestrictedActionAlphaDecrement, 0);
        else
            m_restrictedActionAlpha = 255;
    }

    // 進捗バーのアニメーション補間
    if (m_currentTask)
    {
        float target = m_currentTask->GetProgress();
        if (m_displayedProgress < target)
        {
            m_displayedProgress += m_progressAnimSpeed;
            if (m_displayedProgress > target) m_displayedProgress = target;
        }
    }

    // タイトルスライドインアニメーション
    if (!m_isTitleAnimFinished)
    {
        m_titlePosX += m_titleAnimSpeed;
        if (m_titlePosX >= kTaskTextX)
        {
            m_titlePosX           = static_cast<float>(kTaskTextX);
            m_isTitleAnimFinished = true;
            m_animationWaitTimer  = kTitleAnimWaitFrames;
        }
    }
    else if (m_animationWaitTimer > 0)
    {
        m_animationWaitTimer--;
    }
    else if (m_taskAlpha < 255)
    {
        m_taskAlpha = (std::min)(m_taskAlpha + static_cast<int>(m_taskFadeSpeed), 255);
    }

    // ステップ遷移ロジック
    switch (m_step)
    {
    case TaskStep::Shoot:
        if (m_currentTask && m_currentTask->IsCompleted())
        {
            m_step                 = TaskStep::ShootCompleteDelay;
            m_transitionDelayTimer = kStepTransitionDelay;
        }
        break;

    case TaskStep::ShootCompleteDelay:
        if (--m_transitionDelayTimer <= 0)
            BeginTask(TaskStep::Tackle, std::make_unique<TackleTutorialTask>(), kTitleStartX);
        break;

    case TaskStep::Tackle:
        if (m_currentTask && m_currentTask->IsCompleted())
        {
            m_step                 = TaskStep::TackleCompleteDelay;
            m_transitionDelayTimer = kStepTransitionDelay;
        }
        break;

    case TaskStep::TackleCompleteDelay:
        if (--m_transitionDelayTimer <= 0)
            BeginTask(TaskStep::ShieldThrow, std::make_unique<ShieldThrowTutorialTask>(), kTitleStartXShort);
        break;

    case TaskStep::ShieldThrow:
        if (m_currentTask && m_currentTask->IsCompleted())
        {
            m_step                 = TaskStep::ShieldThrowCompleteDelay;
            m_transitionDelayTimer = kStepTransitionDelay;
        }
        break;

    case TaskStep::ShieldThrowCompleteDelay:
        if (--m_transitionDelayTimer <= 0)
            BeginTask(TaskStep::Parry, std::make_unique<ParryTutorialTask>(), kTitleStartXShort);
        break;

    case TaskStep::Parry:
        if (m_currentTask && m_currentTask->IsCompleted())
        {
            m_step                 = TaskStep::ParryCompleteDelay;
            m_transitionDelayTimer = kStepTransitionDelay;
        }
        break;

    case TaskStep::ParryCompleteDelay:
        if (--m_transitionDelayTimer <= 0)
        {
            m_step = TaskStep::Completed;
            if (m_pPlayer) m_pPlayer->SetAttackRestrictions(AttackType::None);
        }
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
    float scale = Game::GetUIScale();
    if (fabs(scale - m_prevScale) > kScaleChangeTolerance)
    {
        ReloadFonts(scale);
        m_prevScale = scale;
    }

    if (m_step == TaskStep::Completed || m_step == TaskStep::None) return;
    if (!m_currentTask) return;

    int scaledTaskTextX    = static_cast<int>(kTaskTextX     * scale);
    int scaledTaskTextY    = static_cast<int>(kTaskTextY     * scale);
    int scaledBgBoxWidth   = static_cast<int>(kBgBoxWidth    * scale);
    int scaledBgBoxHeight  = static_cast<int>(kBgBoxHeight   * scale);
    int scaledBgBoxPadX    = static_cast<int>(kBgBoxPaddingX * scale);
    int scaledBgBoxPadY    = static_cast<int>(kBgBoxPaddingY * scale);
    int scaledTitleFontSize = static_cast<int>(kTitleFontSize * scale);
    int scaledBarMaxWidth  = static_cast<int>(kBarMaxWidth   * scale);
    int scaledBarHeight    = static_cast<int>(kBarHeight     * scale);

    // 背景ボックス（タイトルが少しでも動き始めたら表示）
    if (m_isTitleAnimFinished || m_titlePosX > kTitleStartX)
    {
        int bgX = static_cast<int>(m_titlePosX * scale) - scaledBgBoxPadX;
        int bgY = scaledTaskTextY - scaledBgBoxPadY;
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, kBgBoxAlpha);
        DrawBox(bgX, bgY, bgX + scaledBgBoxWidth, bgY + scaledBgBoxHeight, kBgBoxColor, true);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }

    // タイトルテキスト
    DrawStringToHandle(static_cast<int>(m_titlePosX * scale), scaledTaskTextY,
                       m_currentTask->GetTitle().c_str(), 0xFFFFFF, m_titleFont);

    // タスク内容・進捗バー（タイトルアニメーション完了後に表示）
    if (m_isTitleAnimFinished)
    {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, m_taskAlpha);

        int taskY = scaledTaskTextY + scaledTitleFontSize + static_cast<int>(kTitleToTaskGapY * scale);
        m_currentTask->DrawTaskUI(scaledTaskTextX, taskY, scale, m_taskAlpha, this);

        int barY           = taskY + static_cast<int>(kProgressBarOffsetY * scale);
        int currentBarWidth = static_cast<int>(scaledBarMaxWidth * m_displayedProgress);
        unsigned int barColor = (m_displayedProgress >= 1.0f) ? 0x00ff80 : 0x6496ff;

        DrawBox(scaledTaskTextX, barY, scaledTaskTextX + scaledBarMaxWidth, barY + scaledBarHeight, 0x646464, true);
        DrawBox(scaledTaskTextX, barY, scaledTaskTextX + currentBarWidth,   barY + scaledBarHeight, barColor, true);
        DrawStringToHandle(scaledTaskTextX + scaledBarMaxWidth + static_cast<int>(kBarToTextGap * scale),
                           barY, m_currentTask->GetProgressText().c_str(), 0xFFFFFF, m_taskFont);

        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }

    // 制限アクションフィードバック画像
    if (m_restrictedActionAlpha > 0)
    {
        int designerW = 0, designerH = 0;
        GetGraphSize(m_designerImg, &designerW, &designerH);

        float feedbackScale = scale * kFeedbackImageScale;
        int targetW = static_cast<int>(designerW * feedbackScale);
        int targetH = static_cast<int>(designerH * feedbackScale);

        int centerX = Game::GetScreenWidth()  / 2;
        int centerY = static_cast<int>(Game::GetScreenHeight() * kFeedbackImageCenterYRatio);
        int drawX   = centerX - targetW / 2;
        int drawY   = centerY - targetH / 2;

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, m_restrictedActionAlpha);
        DrawExtendGraph(drawX, drawY, drawX + targetW, drawY + targetH, m_designerImg, true);

        int iconImg = -1;
        switch (m_restrictedActionType)
        {
        case AttackType::Shoot:
        case AttackType::Shotgun:
        case AttackType::Tackle:    iconImg = m_mouseLeftImg;      break;
        case AttackType::ShieldThrow: iconImg = m_rKeyImg;         break;
        case AttackType::Parry:     iconImg = m_mouseRightGuardImg; break;
        default: break;
        }

        if (iconImg != -1)
        {
            int iconW = static_cast<int>(kIconBaseSize * scale * kIconScaleFactor);
            int iconH = iconW;
            int iconX = centerX - iconW / 2;
            int iconY = drawY + targetH / 2 - iconH / 2;
            DrawExtendGraph(iconX, iconY, iconX + iconW, iconY + iconH, iconImg, true);
        }

        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }

    // パリィ説明オーバーレイ（一時停止中）
    if (m_isParryTutorialPaused)
    {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, kParryOverlayAlpha);
        DrawBox(0, 0, Game::GetScreenWidth(), Game::GetScreenHeight(), 0x000000, true);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

        int centerX = Game::GetScreenWidth()  / 2;
        int centerY = Game::GetScreenHeight() / 2;

        std::string text1 = "緑色の攻撃はタイミングよくシールドブロック";
        std::string text2 = "を行うことでパリィできる";

        int text1Width = GetDrawStringWidthToHandle(text1.c_str(), -1, m_titleFont);
        int iconWidth  = static_cast<int>(kParryIconSize * scale);
        int text2Width = GetDrawStringWidthToHandle(text2.c_str(), -1, m_titleFont);
        int totalWidth = text1Width + iconWidth + text2Width;
        int startX     = centerX - totalWidth / 2;
        int currentY   = centerY - static_cast<int>(kParryTextOffsetY * scale);

        DrawStringToHandle(startX, currentY, text1.c_str(), 0xFFFFFF, m_titleFont);
        DrawExtendGraph(startX + text1Width, currentY,
                        startX + text1Width + iconWidth, currentY + iconWidth,
                        m_mouseRightImg, true);
        DrawStringToHandle(startX + text1Width + iconWidth, currentY, text2.c_str(), 0xFFFFFF, m_titleFont);

        std::string resumeText = "右クリックを押して再開";
        int resumeW = GetDrawStringWidthToHandle(resumeText.c_str(), -1, m_taskFont);
        DrawStringToHandle(centerX - resumeW / 2,
                           centerY + static_cast<int>(kParryTextOffsetY * scale),
                           resumeText.c_str(), 0xAAAAAA, m_taskFont);
    }
}
