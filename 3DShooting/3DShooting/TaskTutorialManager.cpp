#include "TaskTutorialManager.h"
#include "DxLib.h"
#include "Game.h"
#include "Player.h"
#include "WaveManager.h"
#include <string>

namespace
{
    // タスクの目標キル数
    constexpr int kShootKillGoal = 5;
    constexpr int kTackleKillGoal = 5;
    constexpr int kShieldThrowKillGoal = 2;
    constexpr int kParryGoal = 3;

    // UI関連
    constexpr int kTaskTextX = 60;
    constexpr int kTaskTextY = 60;
    constexpr int kTaskFontSize = 36;
    constexpr int kTaskFontThickness = 3;
    constexpr unsigned int kTaskTextColor = 0xFFFFFF;

    constexpr int kTitleFontSize = 48;
    constexpr int kDiamondSize = 30;
    constexpr int kMouseImgSize = 36;
    constexpr int kSpacing = 8;

    constexpr int kBarMaxWidth = 225;
    constexpr int kBarHeight = 22;

    // 背景ボックス関連
    constexpr int kBgBoxPaddingX = 20;
    constexpr int kBgBoxPaddingY = 15;
    constexpr int kBgBoxWidth = 500;
    constexpr int kBgBoxHeight = 250;
    constexpr unsigned int kBgBoxColor = 0x000000;
    constexpr int kBgBoxAlpha = 128;
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

TaskTutorialManager::TaskTutorialManager()
    : m_pWaveManager(nullptr)
    , m_pPlayer(nullptr)
    , m_step(TaskStep::None)
    , m_shootKills(0)
    , m_tackleKills(0)
    , m_shieldThrowKills(0)
    , m_parryCount(0)
    , m_titleFont("HGPｺﾞｼｯｸE", 48, 3, DX_FONTTYPE_ANTIALIASING_EDGE_8X8)
    , m_taskFont("HGPｺﾞｼｯｸE", 30, 3, DX_FONTTYPE_ANTIALIASING_EDGE_8X8)
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
    , m_titleAnimSpeed(5.0f)
    , m_isTitleAnimFinished(false)
    , m_taskAlpha(0)
    , m_taskFadeSpeed(5.0f)
    , m_animationWaitTimer(0)
    , m_displayedShootProgress(0.0f)
    , m_displayedTackleProgress(0.0f)
    , m_displayedShieldThrowProgress(0.0f)
    , m_displayedParryProgress(0.0f)
    , m_progressAnimSpeed(0.02f)
    , m_transitionDelayTimer(0)
    , m_hasShownParryTutorial(false)
    , m_isParryTutorialPaused(false)
    , m_prevScale(1.0f)
{
    // フォントの作成 (初期化リストで行われるが、念のため)
    ReloadFonts(1.0f);
}

TaskTutorialManager::~TaskTutorialManager()
{
    // 自動解放されるため処理不要
}

void TaskTutorialManager::ReloadFonts(float scale)
{
    m_titleFont.Reload(scale);
    m_taskFont.Reload(scale);
}

void TaskTutorialManager::Init(WaveManager* pWaveManager, Player* pPlayer)
{
    m_pWaveManager = pWaveManager;
    m_pPlayer = pPlayer;
    m_step = TaskStep::Shoot; // 最初のタスクは射撃
    m_shootKills = 0;
    m_tackleKills = 0;
    m_shieldThrowKills = 0;
    m_parryCount = 0;

    // アニメション初期化
    m_titlePosX = -450.0f; // 画面外からスタート
    m_isTitleAnimFinished = false;
    m_taskAlpha = 0;
    m_animationWaitTimer = 0;
    m_displayedShootProgress = 0.0f;
    m_displayedTackleProgress = 0.0f;
    m_displayedShieldThrowProgress = 0.0f;
    m_displayedParryProgress = 0.0f;
    m_transitionDelayTimer = 0;
    m_hasShownParryTutorial = false;
    m_isParryTutorialPaused = false;
    
    m_restrictedActionTimer = 0;
    m_restrictedActionType = AttackType::None;
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
    switch (m_step)
    {
    case TaskStep::Shoot:
        if (attackType == AttackType::Shoot || attackType == AttackType::Shotgun)
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
    case TaskStep::ShieldThrow:
        if (attackType == AttackType::ShieldThrow)
        {
            m_shieldThrowKills++;
        }
        break;
    default:
        break;
    }
}

void TaskTutorialManager::NotifyShieldThrowKill()
{
    if (m_step == TaskStep::ShieldThrow)
    {
        m_shieldThrowKills++;
    }
}

void TaskTutorialManager::NotifyRestrictedAction(AttackType attemptedType)
{
    // すでに表示中ならタイプだけ更新してタイマーリセット（または無視）
    // 違うタイプならリセット
    if (m_restrictedActionTimer > 0 && m_restrictedActionType == attemptedType)
    {
        // タイマー延長
        m_restrictedActionTimer = 120; // 2秒表示
    }
    else
    {
        m_restrictedActionType = attemptedType;
        m_restrictedActionTimer = 120;
        m_restrictedActionAlpha = 0; // フェードインから開始
    }
}

void TaskTutorialManager::NotifyParrySuccess()
{
    if (m_step == TaskStep::Parry)
    {
        m_parryCount++;
    }
}

void TaskTutorialManager::NotifyParryableAttack()
{
    // パリィタスク中かつ、まだ説明を表示していない場合
    if (m_step == TaskStep::Parry && !m_hasShownParryTutorial && !m_isParryTutorialPaused)
    {
        m_isParryTutorialPaused = true;
        m_hasShownParryTutorial = true;
        Game::SetPaused(true); // ゲームを一時停止
    }
}

void TaskTutorialManager::Reset()
{
    m_step = TaskStep::None;
    m_shootKills = 0;
    m_tackleKills = 0;
    m_pWaveManager = nullptr;
    m_pPlayer = nullptr;
    m_hasShownParryTutorial = false;
    m_isParryTutorialPaused = false;
    Game::SetPaused(false);
}

void TaskTutorialManager::Skip(WaveManager* pWaveManager)
{
    m_step = TaskStep::Completed;
    m_pWaveManager = pWaveManager;
    m_hasShownParryTutorial = false;
    m_isParryTutorialPaused = false;
    Game::SetPaused(false);
}

void TaskTutorialManager::Update()
{
    // パリィ説明表示中の更新処理
    if (m_isParryTutorialPaused)
    {
        // 右クリックで再開
        if (GetMouseInput() & MOUSE_INPUT_RIGHT)
        {
            m_isParryTutorialPaused = false;
            Game::SetPaused(false); // ゲーム再開
        }
        return; // 停止中は他の更新を行わない
    }

    // 制限アクションフィードバックの更新
    if (m_restrictedActionTimer > 0)
    {
        m_restrictedActionTimer--;
        
        // フェードイン・アウト
        if (m_restrictedActionTimer > 100) // 最初の20fでフェードイン
        {
            m_restrictedActionAlpha += 25;
            if (m_restrictedActionAlpha > 255) m_restrictedActionAlpha = 255;
        }
        else if (m_restrictedActionTimer < 30) // 最後の30fでフェードアウト
        {
            m_restrictedActionAlpha -= 10;
            if (m_restrictedActionAlpha < 0) m_restrictedActionAlpha = 0;
        }
        else
        {
            m_restrictedActionAlpha = 255;
        }
    }
    else
    {
        m_restrictedActionAlpha = 0;
    }

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

    // 盾投げタスクの進捗アニメーション
    if (m_step == TaskStep::ShieldThrow || m_step == TaskStep::ShieldThrowCompleteDelay)
    {
        float targetProgress = static_cast<float>(m_shieldThrowKills) / kShieldThrowKillGoal;
        if (m_displayedShieldThrowProgress < targetProgress)
        {
            m_displayedShieldThrowProgress += m_progressAnimSpeed;
            if (m_displayedShieldThrowProgress > targetProgress)
            {
                m_displayedShieldThrowProgress = targetProgress;
            }
        }
    }

    // パリィタスクの進捗アニメーション
    if (m_step == TaskStep::Parry || m_step == TaskStep::ParryCompleteDelay)
    {
        float targetProgress = static_cast<float>(m_parryCount) / kParryGoal;
        if (m_displayedParryProgress < targetProgress)
        {
            m_displayedParryProgress += m_progressAnimSpeed;
            if (m_displayedParryProgress > targetProgress)
            {
                m_displayedParryProgress = targetProgress;
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

            m_titlePosX = -450.0f;
            m_isTitleAnimFinished = false;
            m_taskAlpha = 0;
            m_animationWaitTimer = 0;
            m_displayedShootProgress = 0.0f;
            m_displayedTackleProgress = 0.0f;

            if (m_pWaveManager) m_pWaveManager->SpawnTutorialWave(2);
            if (m_pPlayer) m_pPlayer->SetAttackRestrictions(AttackType::Tackle);
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
            m_step = TaskStep::ShieldThrow; // 盾投げタスクへ

            // UI初期化
            m_titlePosX = -300.0f;
            m_isTitleAnimFinished = false;
            m_taskAlpha = 0;
            m_animationWaitTimer = 0;

            if (m_pWaveManager) m_pWaveManager->SpawnTutorialWave(3); // Wave 3
            if (m_pPlayer) m_pPlayer->SetAttackRestrictions(AttackType::ShieldThrow); // 盾投げのみ許可
        }
        break;
    case TaskStep::ShieldThrow:
        if (m_shieldThrowKills >= kShieldThrowKillGoal)
        {
            m_step = TaskStep::ShieldThrowCompleteDelay;
            m_transitionDelayTimer = 120;
        }
        break;
    case TaskStep::ShieldThrowCompleteDelay:
        m_transitionDelayTimer--;
        if (m_transitionDelayTimer <= 0)
        {
            m_step = TaskStep::Parry; // パリィタスクへ

            // UI初期化
            m_titlePosX = -300.0f;
            m_isTitleAnimFinished = false;
            m_taskAlpha = 0;
            m_animationWaitTimer = 0;

            if (m_pWaveManager) m_pWaveManager->SpawnTutorialWave(4); // Wave 4
            if (m_pPlayer) m_pPlayer->SetAttackRestrictions(AttackType::Parry); // パリィのみ許可
        }
        break;
    case TaskStep::Parry:
        if (m_parryCount >= kParryGoal)
        {
            m_step = TaskStep::ParryCompleteDelay;
            m_transitionDelayTimer = 120;
        }
        break;
    case TaskStep::ParryCompleteDelay:
        m_transitionDelayTimer--;
        if (m_transitionDelayTimer <= 0)
        {
            m_step = TaskStep::Completed; // チュートリアル完了
            if (m_pPlayer) m_pPlayer->SetAttackRestrictions(AttackType::None);
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
    float scale = Game::GetUIScale();
    if (fabs(scale - m_prevScale) > 0.001f)
    {
        ReloadFonts(scale);
        m_prevScale = scale;
    }

    int scaledTaskTextX = static_cast<int>(kTaskTextX * scale);
    int scaledTaskTextY = static_cast<int>(kTaskTextY * scale);
    int scaledBgBoxWidth = static_cast<int>(kBgBoxWidth * scale);
    int scaledBgBoxHeight = static_cast<int>(kBgBoxHeight * scale);
    int scaledBgBoxPaddingX = static_cast<int>(kBgBoxPaddingX * scale);
    int scaledBgBoxPaddingY = static_cast<int>(kBgBoxPaddingY * scale);
    int scaledTitleFontSize = static_cast<int>(kTitleFontSize * scale);
    int scaledTaskFontSize = static_cast<int>(kTaskFontSize * scale); 

    int scaledDiamondSize = static_cast<int>(kDiamondSize * scale);
    int scaledMouseImgSize = static_cast<int>(kMouseImgSize * scale);
    int scaledSpacing = static_cast<int>(kSpacing * scale);
    int scaledBarMaxWidth = static_cast<int>(kBarMaxWidth * scale);
    int scaledBarHeight = static_cast<int>(kBarHeight * scale);

    std::string taskText = "";

    switch (m_step)
    {
    case TaskStep::ShootCompleteDelay:
    case TaskStep::Shoot:
    {
        int displayedKills = static_cast<int>(m_displayedShootProgress * kShootKillGoal);
        if (displayedKills > kShootKillGoal)
        {
            displayedKills = kShootKillGoal;
        }
        taskText = std::to_string(displayedKills) + "/" + std::to_string(kShootKillGoal);

        // 背景ボックスの描画
        if (m_isTitleAnimFinished || m_titlePosX > -450.0f)
        {
            int bgX = static_cast<int>(m_titlePosX * scale) - scaledBgBoxPaddingX;
            int bgY = scaledTaskTextY - scaledBgBoxPaddingY;

            SetDrawBlendMode(DX_BLENDMODE_ALPHA, kBgBoxAlpha);
            DrawBox(bgX, bgY, bgX + scaledBgBoxWidth, bgY + scaledBgBoxHeight, kBgBoxColor, true);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        }

        // タイトル
        DrawStringToHandle(static_cast<int>(m_titlePosX * scale), scaledTaskTextY, "射撃訓練", kTaskTextColor, m_titleFont);

        // タスク内容
        if (m_isTitleAnimFinished)
        {
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, m_taskAlpha);

            int taskY = scaledTaskTextY + scaledTitleFontSize + static_cast<int>(10 * scale);
            int currentX = scaledTaskTextX;

            DrawExtendGraph(currentX, taskY, currentX + scaledDiamondSize, taskY + scaledDiamondSize, m_diamondImg, true);
            currentX += scaledDiamondSize + scaledSpacing;

            DrawExtendGraph(currentX, taskY, currentX + scaledMouseImgSize, taskY + scaledMouseImgSize, m_mouseLeftImg, true);
            currentX += scaledMouseImgSize + scaledSpacing;

            DrawStringToHandle(currentX, taskY, "でゾンビを倒す", kTaskTextColor, m_taskFont);

            // 進捗
            int barY = taskY + scaledTaskFontSize + static_cast<int>(10 * scale);
            float progress = m_displayedShootProgress;
            int currentBarWidth = static_cast<int>(scaledBarMaxWidth * progress);

            unsigned int barColor = (progress >= 1.0f) ? 0x00ff80 : 0x6496ff;

            DrawBox(scaledTaskTextX, barY, scaledTaskTextX + scaledBarMaxWidth, barY + scaledBarHeight, 0x646464, true);
            DrawBox(scaledTaskTextX, barY, scaledTaskTextX + currentBarWidth, barY + scaledBarHeight, barColor, true);

            DrawStringToHandle(scaledTaskTextX + scaledBarMaxWidth + static_cast<int>(5 * scale), barY, taskText.c_str(), kTaskTextColor, m_taskFont);

            // 武器切り替えヒントの表示 (射撃タスク中のみ、進捗バーの下に表示)
            if (m_taskAlpha >= 200)
            {
                int hintY = barY + scaledBarHeight + static_cast<int>(20 * scale);
                int hintX = scaledTaskTextX;

                DrawStringToHandle(hintX, hintY, "武器切り替え: ", kTaskTextColor, m_taskFont);
                hintX += GetDrawStringWidthToHandle("武器切り替え: ", -1, m_taskFont);

                // Alpha1画像
                DrawExtendGraph(hintX, hintY, hintX + scaledMouseImgSize, hintY + scaledMouseImgSize, m_alpha1Img, true);
                hintX += scaledMouseImgSize + scaledSpacing;

                // Alpha2画像
                DrawExtendGraph(hintX, hintY, hintX + scaledMouseImgSize, hintY + scaledMouseImgSize, m_alpha2Img, true);
                hintX += scaledMouseImgSize + scaledSpacing;

                // " / " テキスト
                DrawStringToHandle(hintX, hintY, " / ", kTaskTextColor, m_taskFont);
                hintX += GetDrawStringWidthToHandle(" / ", -1, m_taskFont);

                // マウスホイール画像
                DrawExtendGraph(hintX, hintY, hintX + scaledMouseImgSize, hintY + scaledMouseImgSize, m_mouseWheelImg, true);
            }

            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        }
    }
    break;
    case TaskStep::TackleCompleteDelay:
    case TaskStep::Tackle:
    {
        int displayedKills = static_cast<int>(m_displayedTackleProgress * kTackleKillGoal);
        if (displayedKills > kTackleKillGoal)
        {
            displayedKills = kTackleKillGoal;
        }
        taskText = std::to_string(displayedKills) + "/" + std::to_string(kTackleKillGoal);

        // 背景ボックスの描画
        if (m_isTitleAnimFinished || m_titlePosX > -450.0f)
        {
            int bgX = static_cast<int>(m_titlePosX * scale) - scaledBgBoxPaddingX;
            int bgY = scaledTaskTextY - scaledBgBoxPaddingY;

            SetDrawBlendMode(DX_BLENDMODE_ALPHA, kBgBoxAlpha);
            DrawBox(bgX, bgY, bgX + scaledBgBoxWidth, bgY + scaledBgBoxHeight, kBgBoxColor, true);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        }

        // タイトル
        DrawStringToHandle(static_cast<int>(m_titlePosX * scale), scaledTaskTextY, "タックル訓練", kTaskTextColor, m_titleFont);

        // タスク内容
        if (m_isTitleAnimFinished)
        {
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, m_taskAlpha);

            int taskY = scaledTaskTextY + scaledTitleFontSize + static_cast<int>(10 * scale);
            int currentX = scaledTaskTextX;

            DrawExtendGraph(currentX, taskY, currentX + scaledDiamondSize, taskY + scaledDiamondSize, m_diamondImg, true);
            currentX += scaledDiamondSize + scaledSpacing;

            // 右クリック(長押し)
            DrawExtendGraph(currentX, taskY, currentX + scaledMouseImgSize, taskY + scaledMouseImgSize, m_mouseRightImg, true);
            currentX += scaledMouseImgSize + scaledSpacing;

            DrawStringToHandle(currentX, taskY, "長押し", kTaskTextColor, m_taskFont);
            currentX += GetDrawStringWidthToHandle("長押し", -1, m_taskFont) + scaledSpacing;

            // ロックオンUI
            DrawExtendGraph(currentX, taskY, currentX + scaledMouseImgSize, taskY + scaledMouseImgSize, m_lockOnUIImg, true);
            currentX += scaledMouseImgSize + scaledSpacing;

            // 左クリック
            DrawExtendGraph(currentX, taskY, currentX + scaledMouseImgSize, taskY + scaledMouseImgSize, m_mouseLeftImg, true);
            currentX += scaledMouseImgSize + scaledSpacing;

            DrawStringToHandle(currentX, taskY, "でタックル", kTaskTextColor, m_taskFont);

            // 進捗
            int barY = taskY + scaledTaskFontSize + static_cast<int>(10 * scale);
            float progress = m_displayedTackleProgress;
            int currentBarWidth = static_cast<int>(scaledBarMaxWidth * progress);

            unsigned int barColor = (progress >= 1.0f) ? GetColor(0, 255, 128) : GetColor(100, 150, 255);

            DrawBox(scaledTaskTextX, barY, scaledTaskTextX + scaledBarMaxWidth, barY + scaledBarHeight, GetColor(100, 100, 100), true);
            DrawBox(scaledTaskTextX, barY, scaledTaskTextX + currentBarWidth, barY + scaledBarHeight, barColor, true);

            DrawStringToHandle(scaledTaskTextX + scaledBarMaxWidth + static_cast<int>(5 * scale), barY, taskText.c_str(), kTaskTextColor, m_taskFont);

            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        }
    }
    break;
    case TaskStep::ShieldThrowCompleteDelay:
    case TaskStep::ShieldThrow:
    {
        int displayedKills = static_cast<int>(m_displayedShieldThrowProgress * kShieldThrowKillGoal);
        if (displayedKills > kShieldThrowKillGoal)
        {
            displayedKills = kShieldThrowKillGoal;
        }
        taskText = std::to_string(displayedKills) + "/" + std::to_string(kShieldThrowKillGoal);

        // 背景ボックスの描画
        if (m_isTitleAnimFinished || m_titlePosX > -450.0f)
        {
            int bgX = static_cast<int>(m_titlePosX * scale) - scaledBgBoxPaddingX;
            int bgY = scaledTaskTextY - scaledBgBoxPaddingY;

            SetDrawBlendMode(DX_BLENDMODE_ALPHA, kBgBoxAlpha);
            DrawBox(bgX, bgY, bgX + scaledBgBoxWidth, bgY + scaledBgBoxHeight, kBgBoxColor, true);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        }

        // タイトル
        DrawStringToHandle(static_cast<int>(m_titlePosX * scale), scaledTaskTextY, "盾投げ訓練", kTaskTextColor, m_titleFont);

        if (m_isTitleAnimFinished)
        {
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, m_taskAlpha);

            int taskY = scaledTaskTextY + scaledTitleFontSize + static_cast<int>(10 * scale);
            int currentX = scaledTaskTextX;

            DrawExtendGraph(currentX, taskY, currentX + scaledDiamondSize, taskY + scaledDiamondSize, m_diamondImg, true);
            currentX += scaledDiamondSize + scaledSpacing;

            DrawExtendGraph(currentX, taskY, currentX + scaledMouseImgSize, taskY + scaledMouseImgSize, m_rKeyImg, true);
            currentX += scaledMouseImgSize + scaledSpacing;

            DrawStringToHandle(currentX, taskY, "でゾンビを倒す", kTaskTextColor, m_taskFont);

            // 進捗バー
            int barY = taskY + scaledTaskFontSize + static_cast<int>(10 * scale);
            float progress = m_displayedShieldThrowProgress;
            int currentBarWidth = static_cast<int>(scaledBarMaxWidth * progress);

            unsigned int barColor = (progress >= 1.0f) ? GetColor(0, 255, 128) : GetColor(100, 150, 255);

            DrawBox(scaledTaskTextX, barY, scaledTaskTextX + scaledBarMaxWidth, barY + scaledBarHeight, GetColor(100, 100, 100), true);
            DrawBox(scaledTaskTextX, barY, scaledTaskTextX + currentBarWidth, barY + scaledBarHeight, barColor, true);

            DrawStringToHandle(scaledTaskTextX + scaledBarMaxWidth + static_cast<int>(5 * scale), barY, taskText.c_str(), kTaskTextColor, m_taskFont);

            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        }
    }
    break;
    case TaskStep::ParryCompleteDelay:
    case TaskStep::Parry:
    {
        int displayedParries = static_cast<int>(m_displayedParryProgress * kParryGoal);
        if (displayedParries > kParryGoal)
        {
            displayedParries = kParryGoal;
        }
        taskText = std::to_string(displayedParries) + "/" + std::to_string(kParryGoal);

        // 背景ボックスの描画
        if (m_isTitleAnimFinished || m_titlePosX > -450.0f)
        {
            int bgX = static_cast<int>(m_titlePosX * scale) - scaledBgBoxPaddingX;
            int bgY = scaledTaskTextY - scaledBgBoxPaddingY;

            SetDrawBlendMode(DX_BLENDMODE_ALPHA, kBgBoxAlpha);
            DrawBox(bgX, bgY, bgX + scaledBgBoxWidth, bgY + scaledBgBoxHeight, kBgBoxColor, true);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        }

        // タイトル
        DrawStringToHandle(static_cast<int>(m_titlePosX * scale), scaledTaskTextY, "パリィ訓練", kTaskTextColor, m_titleFont);

        if (m_isTitleAnimFinished)
        {
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, m_taskAlpha);

            int taskY = scaledTaskTextY + scaledTitleFontSize + static_cast<int>(10 * scale);
            int currentX = scaledTaskTextX;

            DrawExtendGraph(currentX, taskY, currentX + scaledDiamondSize, taskY + scaledDiamondSize, m_diamondImg, true);
            currentX += scaledDiamondSize + scaledSpacing;

            DrawStringToHandle(currentX, taskY, "遠距離攻撃をパリィする", kTaskTextColor, m_taskFont);

            // 進捗バー
            int barY = taskY + scaledTaskFontSize + static_cast<int>(10 * scale);
            float progress = m_displayedParryProgress;
            int currentBarWidth = static_cast<int>(scaledBarMaxWidth * progress);

            unsigned int barColor = (progress >= 1.0f) ? GetColor(0, 255, 128) : GetColor(100, 150, 255);

            DrawBox(scaledTaskTextX, barY, scaledTaskTextX + scaledBarMaxWidth, barY + scaledBarHeight, GetColor(100, 100, 100), true);
            DrawBox(scaledTaskTextX, barY, scaledTaskTextX + currentBarWidth, barY + scaledBarHeight, barColor, true);

            DrawStringToHandle(scaledTaskTextX + scaledBarMaxWidth + static_cast<int>(5 * scale), barY, taskText.c_str(), kTaskTextColor, m_taskFont);

            // ガードヒント表示
            if (m_taskAlpha >= 200)
            {
                int hintY = barY + scaledBarHeight + static_cast<int>(20 * scale);
                int hintX = scaledTaskTextX;

                DrawStringToHandle(hintX, hintY, "ガード: ", kTaskTextColor, m_taskFont);
                hintX += GetDrawStringWidthToHandle("ガード: ", -1, m_taskFont);

                // マウス右クリック画像
                DrawExtendGraph(hintX, hintY, hintX + scaledMouseImgSize, hintY + scaledMouseImgSize, m_mouseRightGuardImg, true);
            }

            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        }

        // パリィ説明表示 (一時停止中)
        if (m_isParryTutorialPaused)
        {
            // 画面全体を少し暗くする
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);
            DrawBox(0, 0, Game::GetScreenWidth(), Game::GetScreenHeight(), 0x000000, true);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

            int centerX = Game::GetScreenWidth() / 2;
            int centerY = Game::GetScreenHeight() / 2;

            // テキスト表示
            // 「緑色の攻撃はタイミングよくシールドブロック（MouseRight.png）を行うことでパリィできる」
            std::string text1 = "緑色の攻撃はタイミングよくシールドブロック";
            std::string text2 = "を行うことでパリィできる";

            // 中央揃えのために幅計算
            int text1Width = GetDrawStringWidthToHandle(text1.c_str(), -1, m_titleFont);
            int iconWidth = static_cast<int>(32 * scale); // アイコンサイズ scaled
            int text2Width = GetDrawStringWidthToHandle(text2.c_str(), -1, m_titleFont);

            int totalWidth = text1Width + iconWidth + text2Width;
            int startX = centerX - totalWidth / 2;

            int currentX = startX;
            int currentY = centerY - static_cast<int>(50 * scale);

            // テキスト描画
            DrawStringToHandle(currentX, currentY, text1.c_str(), 0xFFFFFF, m_titleFont);
            currentX += text1Width;

            // アイコン描画
            DrawExtendGraph(currentX, currentY, currentX + iconWidth, currentY + iconWidth, m_mouseRightImg, true);
            currentX += iconWidth;

            // 残りのテキスト描画
            DrawStringToHandle(currentX, currentY, text2.c_str(), 0xFFFFFF, m_titleFont);

            // 続行案内
            std::string resumeText = "右クリックを押して再開";
            int resumeTextWidth = GetDrawStringWidthToHandle(resumeText.c_str(), -1, m_taskFont);
            DrawStringToHandle(centerX - resumeTextWidth / 2, centerY + static_cast<int>(50 * scale), resumeText.c_str(), 0xAAAAAA, m_taskFont);
        }

    }
    break;
    case TaskStep::Completed:
        break;
    default:
        break;
    }

    // 制限アクションフィードバックの描画
    if (m_restrictedActionTimer > 0 && m_restrictedActionAlpha > 0)
    {
        // 画面中央下
        int screenW = Game::GetScreenWidth();
        int screenH = Game::GetScreenHeight();
        
        // 画像サイズ取得
        int designerW = 0, designerH = 0;
        GetGraphSize(m_designerImg, &designerW, &designerH);
        
        // 少し縮小して表示 (1024x1024なのでかなり小さくする)
        float feedbackScale = scale * 0.12f; 
        int targetW = static_cast<int>(designerW * feedbackScale);
        int targetH = static_cast<int>(designerH * feedbackScale); 
        
        int centerX = screenW / 2;
        // 画面中央やや下（クロスヘアと警告UIの間）に表示
        int centerY = static_cast<int>(screenH * 0.6f);
        
        int drawX = centerX - targetW / 2;
        int drawY = centerY - targetH / 2;
        
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, m_restrictedActionAlpha);
        DrawExtendGraph(drawX, drawY, drawX + targetW, drawY + targetH, m_designerImg, true);
        
        // アクションアイコンの描画（Designer.pngの上に重ねる）
        int iconImg = -1;
        switch (m_restrictedActionType)
        {
        case AttackType::Shoot:
        case AttackType::Shotgun:
            iconImg = static_cast<int>(m_mouseLeftImg);
            break;
        case AttackType::Tackle:
            // タックルは右クリック+ロックオン+左などの組み合わせだが、とりあえず右クリックか左クリックを表示
            iconImg = static_cast<int>(m_mouseRightImg); 
            break;
        case AttackType::ShieldThrow:
            iconImg = static_cast<int>(m_rKeyImg);
            break;
        case AttackType::Parry:
             iconImg = static_cast<int>(m_mouseRightGuardImg);
             break;
        default:
            break;
        }
        
        if (iconImg != -1)
        {
            int iconW = static_cast<int>(kMouseImgSize * scale * 1.5f); // 少し大きめに
            int iconH = static_cast<int>(kMouseImgSize * scale * 1.5f);
            
            // Designer.pngの中央あたりに表示（微調整必要）
             DrawExtendGraph(centerX - iconW / 2, drawY + targetH / 2 - iconH / 2, centerX + iconW / 2, drawY + targetH / 2 + iconH / 2, iconImg, true);
        }
        
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }
}