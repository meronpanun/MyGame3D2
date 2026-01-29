#include "TaskTutorialManager.h"
#include "DxLib.h"
#include "Game.h"
#include "Player.h"
#include "WaveManager.h"
#include <string>

namespace {
// タスクの目標キル数
constexpr int kShootKillGoal = 5;
constexpr int kTackleKillGoal = 5;
constexpr int kShieldThrowKillGoal = 2;
constexpr int kParryGoal = 3;

// UI関連
constexpr int kTaskTextX = 20;
constexpr int kTaskTextY = 20;
constexpr int kTaskFontSize = 24;
constexpr int kTaskFontThickness = 2;
constexpr unsigned int kTaskTextColor = 0xFFFFFF;

constexpr int kTitleFontSize = 32;
constexpr int kDiamondSize = 20;
constexpr int kMouseImgSize = 24;
constexpr int kSpacing = 5;

constexpr int kBarMaxWidth = 150;
constexpr int kBarHeight = 15;
} // namespace

// 静的メンバ変数の実体を定義
TaskTutorialManager *TaskTutorialManager::m_instance = nullptr;

TaskTutorialManager *TaskTutorialManager::GetInstance() {
  if (m_instance == nullptr) {
    m_instance = new TaskTutorialManager();
  }
  return m_instance;
}

TaskTutorialManager::TaskTutorialManager()
    : m_pWaveManager(nullptr), m_pPlayer(nullptr), m_step(TaskStep::None),
      m_shootKills(0), m_tackleKills(0), m_shieldThrowKills(0), m_parryCount(0),
      m_titleFontHandle(-1), m_taskFontHandle(-1), m_diamondImg(-1),
      m_mouseLeftImg(-1), m_mouseRightImg(-1), m_alpha1Img(-1), m_alpha2Img(-1),
      m_mouseWheelImg(-1), m_rKeyImg(-1), m_mouseRightGuardImg(-1),
      m_titlePosX(0.0f), m_titleAnimSpeed(5.0f), m_isTitleAnimFinished(false),
      m_taskAlpha(0), m_taskFadeSpeed(5.0f), m_animationWaitTimer(0),
      m_displayedShootProgress(0.0f), m_displayedTackleProgress(0.0f),
      m_displayedShieldThrowProgress(0.0f), m_displayedParryProgress(0.0f),
      m_progressAnimSpeed(0.02f), m_transitionDelayTimer(0),
      m_hasShownParryTutorial(false), m_isParryTutorialPaused(false) {
  // フォントの作成
  m_titleFontHandle = CreateFontToHandle("HGPｺﾞｼｯｸE", 32, kTaskFontThickness,
                                         DX_FONTTYPE_ANTIALIASING_EDGE_8X8);
  m_taskFontHandle = CreateFontToHandle("HGPｺﾞｼｯｸE", 20, kTaskFontThickness,
                                        DX_FONTTYPE_ANTIALIASING_EDGE_8X8);
  m_diamondImg = LoadGraph("data/image/Diamond.png");
  m_mouseLeftImg = LoadGraph("data/image/MouseLeft.png");
  m_mouseRightImg = LoadGraph("data/image/MouseRight.png");
  m_alpha1Img = LoadGraph("data/image/Alpha1.png");
  m_alpha2Img = LoadGraph("data/image/Alpha2.png");
  m_mouseWheelImg = LoadGraph("data/image/MouseWheel.png");
  m_rKeyImg = LoadGraph("data/image/R.png");
  m_mouseRightGuardImg = LoadGraph("data/image/MouseRight.png");
}

TaskTutorialManager::~TaskTutorialManager() {
  DeleteFontToHandle(m_titleFontHandle);
  DeleteFontToHandle(m_taskFontHandle);
  DeleteGraph(m_diamondImg);
  DeleteGraph(m_mouseLeftImg);
  DeleteGraph(m_mouseRightImg);
  DeleteGraph(m_alpha1Img);
  DeleteGraph(m_alpha2Img);
  DeleteGraph(m_mouseWheelImg);
  DeleteGraph(m_rKeyImg);
  DeleteGraph(m_mouseRightGuardImg);
}

void TaskTutorialManager::Init(WaveManager *pWaveManager, Player *pPlayer) {
  m_pWaveManager = pWaveManager;
  m_pPlayer = pPlayer;
  m_step = TaskStep::Shoot; // 最初のタスクは射撃
  m_shootKills = 0;
  m_tackleKills = 0;
  m_shieldThrowKills = 0;
  m_parryCount = 0;

  // アニメション初期化
  m_titlePosX = -300.0f; // 画面外からスタート
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

  if (m_pWaveManager) {
    m_pWaveManager->SpawnTutorialWave(1);
  }
  if (m_pPlayer) {
    m_pPlayer->SetAttackRestrictions(AttackType::Shoot);
  }
}

void TaskTutorialManager::NotifyEnemyKilled(AttackType attackType) {
  switch (m_step) {
  case TaskStep::Shoot:
    if (attackType == AttackType::Shoot) {
      m_shootKills++;
    }
    break;
  case TaskStep::Tackle:
    if (attackType == AttackType::Tackle) {
      m_tackleKills++;
    }
    break;
  case TaskStep::ShieldThrow:
    if (attackType == AttackType::ShieldThrow) {
      m_shieldThrowKills++;
    }
    break;
  default:
    break;
  }
}

void TaskTutorialManager::NotifyShieldThrowKill() {
  if (m_step == TaskStep::ShieldThrow) {
    m_shieldThrowKills++;
  }
}

void TaskTutorialManager::NotifyParrySuccess() {
  if (m_step == TaskStep::Parry) {
    m_parryCount++;
  }
}

void TaskTutorialManager::NotifyParryableAttack() {
  // パリィタスク中かつ、まだ説明を表示していない場合
  if (m_step == TaskStep::Parry && !m_hasShownParryTutorial &&
      !m_isParryTutorialPaused) {
    m_isParryTutorialPaused = true;
    m_hasShownParryTutorial = true;
    Game::SetPaused(true); // ゲームを一時停止
  }
}

void TaskTutorialManager::Reset() {
  m_step = TaskStep::None;
  m_shootKills = 0;
  m_tackleKills = 0;
  m_pWaveManager = nullptr;
  m_pPlayer = nullptr;
  m_hasShownParryTutorial = false;
  m_isParryTutorialPaused = false;
  Game::SetPaused(false);
}

void TaskTutorialManager::Skip(WaveManager *pWaveManager) {
  m_step = TaskStep::Completed;
  m_pWaveManager = pWaveManager;
  m_hasShownParryTutorial = false;
  m_isParryTutorialPaused = false;
  Game::SetPaused(false);
}

void TaskTutorialManager::Update() {
  // パリィ説明表示中の更新処理
  if (m_isParryTutorialPaused) {
    // 右クリックで再開
    if (GetMouseInput() & MOUSE_INPUT_RIGHT) {
      m_isParryTutorialPaused = false;
      Game::SetPaused(false); // ゲーム再開
    }
    return; // 停止中は他の更新を行わない
  }

  // プログレスバーのアニメーション
  if (m_step == TaskStep::Shoot || m_step == TaskStep::ShootCompleteDelay) {
    float targetProgress = static_cast<float>(m_shootKills) / kShootKillGoal;
    if (m_displayedShootProgress < targetProgress) {
      m_displayedShootProgress += m_progressAnimSpeed;
      if (m_displayedShootProgress > targetProgress) {
        m_displayedShootProgress = targetProgress;
      }
    }
  } else if (m_step == TaskStep::Tackle ||
             m_step == TaskStep::TackleCompleteDelay) {
    float targetProgress = static_cast<float>(m_tackleKills) / kTackleKillGoal;
    if (m_displayedTackleProgress < targetProgress) {
      m_displayedTackleProgress += m_progressAnimSpeed;
      if (m_displayedTackleProgress > targetProgress) {
        m_displayedTackleProgress = targetProgress;
      }
    }
  }

  // 盾投げタスクの進捗アニメーション
  if (m_step == TaskStep::ShieldThrow ||
      m_step == TaskStep::ShieldThrowCompleteDelay) {
    float targetProgress =
        static_cast<float>(m_shieldThrowKills) / kShieldThrowKillGoal;
    if (m_displayedShieldThrowProgress < targetProgress) {
      m_displayedShieldThrowProgress += m_progressAnimSpeed;
      if (m_displayedShieldThrowProgress > targetProgress) {
        m_displayedShieldThrowProgress = targetProgress;
      }
    }
  }

  // パリィタスクの進捗アニメーション
  if (m_step == TaskStep::Parry || m_step == TaskStep::ParryCompleteDelay) {
    float targetProgress = static_cast<float>(m_parryCount) / kParryGoal;
    if (m_displayedParryProgress < targetProgress) {
      m_displayedParryProgress += m_progressAnimSpeed;
      if (m_displayedParryProgress > targetProgress) {
        m_displayedParryProgress = targetProgress;
      }
    }
  }

  if (!m_isTitleAnimFinished) {
    m_titlePosX += m_titleAnimSpeed;
    if (m_titlePosX >= kTaskTextX) {
      m_titlePosX = kTaskTextX;
      m_isTitleAnimFinished = true;
      m_animationWaitTimer = 60; // 60フレーム待機
    }
  } else {
    if (m_animationWaitTimer > 0) {
      m_animationWaitTimer--;
    } else {
      if (m_taskAlpha < 255) {
        m_taskAlpha += m_taskFadeSpeed;
        if (m_taskAlpha > 255) {
          m_taskAlpha = 255;
        }
      }
    }
  }

  switch (m_step) {
  case TaskStep::Shoot:
    if (m_shootKills >= kShootKillGoal) {
      m_step = TaskStep::ShootCompleteDelay;
      m_transitionDelayTimer = 120; // 120フレーム待機
    }
    break;
  case TaskStep::ShootCompleteDelay:
    m_transitionDelayTimer--;
    if (m_transitionDelayTimer <= 0) {
      m_step = TaskStep::Tackle; // 次のステップへ

      m_titlePosX = -300.0f;
      m_isTitleAnimFinished = false;
      m_taskAlpha = 0;
      m_animationWaitTimer = 0;
      m_displayedShootProgress = 0.0f;
      m_displayedTackleProgress = 0.0f;

      if (m_pWaveManager) {
        m_pWaveManager->SpawnTutorialWave(2);
      }
      if (m_pPlayer) {
        m_pPlayer->SetAttackRestrictions(AttackType::Tackle);
      }
    }
    break;
  case TaskStep::Tackle:
    if (m_tackleKills >= kTackleKillGoal) {
      m_step = TaskStep::TackleCompleteDelay;
      m_transitionDelayTimer = 120; // 120フレーム待機
    }
    break;
  case TaskStep::TackleCompleteDelay:
    m_transitionDelayTimer--;
    if (m_transitionDelayTimer <= 0) {
      m_step = TaskStep::ShieldThrow; // 盾投げタスクへ

      // UI初期化
      m_titlePosX = -300.0f;
      m_isTitleAnimFinished = false;
      m_taskAlpha = 0;
      m_animationWaitTimer = 0;

      if (m_pWaveManager) {
        m_pWaveManager->SpawnTutorialWave(3); // Wave 3
      }
      if (m_pPlayer) {
        m_pPlayer->SetAttackRestrictions(
            AttackType::ShieldThrow); // 盾投げのみ許可
      }
    }
    break;
  case TaskStep::ShieldThrow:
    if (m_shieldThrowKills >= kShieldThrowKillGoal) {
      m_step = TaskStep::ShieldThrowCompleteDelay;
      m_transitionDelayTimer = 120;
    }
    break;
  case TaskStep::ShieldThrowCompleteDelay:
    m_transitionDelayTimer--;
    if (m_transitionDelayTimer <= 0) {
      m_step = TaskStep::Parry; // パリィタスクへ

      // UI初期化
      m_titlePosX = -300.0f;
      m_isTitleAnimFinished = false;
      m_taskAlpha = 0;
      m_animationWaitTimer = 0;

      if (m_pWaveManager) {
        m_pWaveManager->SpawnTutorialWave(4); // Wave 4
      }
      if (m_pPlayer) {
        m_pPlayer->SetAttackRestrictions(AttackType::Parry); // パリィのみ許可
      }
    }
    break;
  case TaskStep::Parry:
    if (m_parryCount >= kParryGoal) {
      m_step = TaskStep::ParryCompleteDelay;
      m_transitionDelayTimer = 120;
    }
    break;
  case TaskStep::ParryCompleteDelay:
    m_transitionDelayTimer--;
    if (m_transitionDelayTimer <= 0) {
      m_step = TaskStep::Completed; // チュートリアル完了
      if (m_pPlayer) {
        m_pPlayer->SetAttackRestrictions(AttackType::None);
      }
    }
    break;
  case TaskStep::Completed:
    break;
  default:
    break;
  }
}

bool TaskTutorialManager::IsCompleted() const {
  return m_step == TaskStep::Completed;
}

void TaskTutorialManager::Draw() {
  std::string taskText = "";

  switch (m_step) {
  case TaskStep::ShootCompleteDelay:
  case TaskStep::Shoot: {
    int displayedKills =
        static_cast<int>(m_displayedShootProgress * kShootKillGoal);
    if (displayedKills > kShootKillGoal) {
      displayedKills = kShootKillGoal;
    }
    taskText =
        std::to_string(displayedKills) + "/" + std::to_string(kShootKillGoal);

    // タイトル
    DrawStringToHandle(m_titlePosX, kTaskTextY, "射撃訓練", kTaskTextColor,
                       m_titleFontHandle);

    // タスク内容
    if (m_isTitleAnimFinished) {
      SetDrawBlendMode(DX_BLENDMODE_ALPHA, m_taskAlpha);

      int taskY = kTaskTextY + kTitleFontSize + 10;
      int currentX = kTaskTextX;

      DrawExtendGraph(currentX, taskY, currentX + kDiamondSize,
                      taskY + kDiamondSize, m_diamondImg, true);
      currentX += kDiamondSize + kSpacing;

      DrawExtendGraph(currentX, taskY, currentX + kMouseImgSize,
                      taskY + kMouseImgSize, m_mouseLeftImg, true);
      currentX += kMouseImgSize + kSpacing;

      DrawStringToHandle(currentX, taskY, "でゾンビを倒す", kTaskTextColor,
                         m_taskFontHandle);

      // 進捗
      int barY = taskY + kTaskFontSize + 10;
      float progress = m_displayedShootProgress;
      int currentBarWidth = static_cast<int>(kBarMaxWidth * progress);

      unsigned int barColor = (progress >= 1.0f) ? 0x00ff80 : 0x6496ff;

      DrawBox(kTaskTextX, barY, kTaskTextX + kBarMaxWidth, barY + kBarHeight,
              0x646464, true);
      DrawBox(kTaskTextX, barY, kTaskTextX + currentBarWidth, barY + kBarHeight,
              barColor, true);

      DrawStringToHandle(kTaskTextX + kBarMaxWidth + 5, barY, taskText.c_str(),
                         kTaskTextColor, m_taskFontHandle);

      // 武器切り替えヒントの表示 (射撃タスク中のみ、進捗バーの下に表示)
      if (m_taskAlpha >= 200) {
        int hintY = barY + kBarHeight + 20;
        int hintX = kTaskTextX;

        DrawStringToHandle(hintX, hintY, "武器切り替え: ", kTaskTextColor,
                           m_taskFontHandle);
        hintX +=
            GetDrawStringWidthToHandle("武器切り替え: ", -1, m_taskFontHandle);

        // Alpha1画像
        DrawExtendGraph(hintX, hintY, hintX + kMouseImgSize,
                        hintY + kMouseImgSize, m_alpha1Img, true);
        hintX += kMouseImgSize + kSpacing;

        // Alpha2画像
        DrawExtendGraph(hintX, hintY, hintX + kMouseImgSize,
                        hintY + kMouseImgSize, m_alpha2Img, true);
        hintX += kMouseImgSize + kSpacing;

        // " / " テキスト
        DrawStringToHandle(hintX, hintY, " / ", kTaskTextColor,
                           m_taskFontHandle);
        hintX += GetDrawStringWidthToHandle(" / ", -1, m_taskFontHandle);

        // マウスホイール画像
        DrawExtendGraph(hintX, hintY, hintX + kMouseImgSize,
                        hintY + kMouseImgSize, m_mouseWheelImg, true);
      }

      SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }
  } break;
  case TaskStep::TackleCompleteDelay:
  case TaskStep::Tackle: {
    int displayedKills =
        static_cast<int>(m_displayedTackleProgress * kTackleKillGoal);
    if (displayedKills > kTackleKillGoal) {
      displayedKills = kTackleKillGoal;
    }
    taskText =
        std::to_string(displayedKills) + "/" + std::to_string(kTackleKillGoal);

    // タイトル
    DrawStringToHandle(m_titlePosX, kTaskTextY, "タックル訓練", kTaskTextColor,
                       m_titleFontHandle);

    // タスク内容
    if (m_isTitleAnimFinished) {
      SetDrawBlendMode(DX_BLENDMODE_ALPHA, m_taskAlpha);

      int taskY = kTaskTextY + kTitleFontSize + 10;
      int currentX = kTaskTextX;

      DrawExtendGraph(currentX, taskY, currentX + kDiamondSize,
                      taskY + kDiamondSize, m_diamondImg, true);
      currentX += kDiamondSize + kSpacing;

      DrawExtendGraph(currentX, taskY, currentX + kMouseImgSize,
                      taskY + kMouseImgSize, m_mouseRightImg, true);
      currentX += kMouseImgSize + kSpacing;

      DrawStringToHandle(currentX, taskY, "でゾンビを倒す", kTaskTextColor,
                         m_taskFontHandle);

      // 進捗
      int barY = taskY + kTaskFontSize + 10;
      float progress = m_displayedTackleProgress;
      int currentBarWidth = static_cast<int>(kBarMaxWidth * progress);

      unsigned int barColor =
          (progress >= 1.0f) ? GetColor(0, 255, 128) : GetColor(100, 150, 255);

      DrawBox(kTaskTextX, barY, kTaskTextX + kBarMaxWidth, barY + kBarHeight,
              GetColor(100, 100, 100), true);
      DrawBox(kTaskTextX, barY, kTaskTextX + currentBarWidth, barY + kBarHeight,
              barColor, true);

      DrawStringToHandle(kTaskTextX + kBarMaxWidth + 5, barY, taskText.c_str(),
                         kTaskTextColor, m_taskFontHandle);

      SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }
  } break;
  case TaskStep::ShieldThrowCompleteDelay:
  case TaskStep::ShieldThrow: {
    int displayedKills =
        static_cast<int>(m_displayedShieldThrowProgress * kShieldThrowKillGoal);
    if (displayedKills > kShieldThrowKillGoal) {
      displayedKills = kShieldThrowKillGoal;
    }
    taskText = std::to_string(displayedKills) + "/" +
               std::to_string(kShieldThrowKillGoal);

    DrawStringToHandle(m_titlePosX, kTaskTextY, "盾投げ訓練", kTaskTextColor,
                       m_titleFontHandle);

    if (m_isTitleAnimFinished) {
      SetDrawBlendMode(DX_BLENDMODE_ALPHA, m_taskAlpha);

      int taskY = kTaskTextY + kTitleFontSize + 10;
      int currentX = kTaskTextX;

      DrawExtendGraph(currentX, taskY, currentX + kDiamondSize,
                      taskY + kDiamondSize, m_diamondImg, true);
      currentX += kDiamondSize + kSpacing;

      DrawExtendGraph(currentX, taskY, currentX + kMouseImgSize,
                      taskY + kMouseImgSize, m_rKeyImg, true);
      currentX += kMouseImgSize + kSpacing;

      DrawStringToHandle(currentX, taskY, "でゾンビを倒す", kTaskTextColor,
                         m_taskFontHandle);

      // 進捗バー
      int barY = taskY + kTaskFontSize + 10;
      float progress = m_displayedShieldThrowProgress;
      int currentBarWidth = static_cast<int>(kBarMaxWidth * progress);

      unsigned int barColor =
          (progress >= 1.0f) ? GetColor(0, 255, 128) : GetColor(100, 150, 255);

      DrawBox(kTaskTextX, barY, kTaskTextX + kBarMaxWidth, barY + kBarHeight,
              GetColor(100, 100, 100), true);
      DrawBox(kTaskTextX, barY, kTaskTextX + currentBarWidth, barY + kBarHeight,
              barColor, true);

      DrawStringToHandle(kTaskTextX + kBarMaxWidth + 5, barY, taskText.c_str(),
                         kTaskTextColor, m_taskFontHandle);

      SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }
  } break;
  case TaskStep::ParryCompleteDelay:
  case TaskStep::Parry: {
    int displayedParries =
        static_cast<int>(m_displayedParryProgress * kParryGoal);
    if (displayedParries > kParryGoal) {
      displayedParries = kParryGoal;
    }
    taskText =
        std::to_string(displayedParries) + "/" + std::to_string(kParryGoal);

    DrawStringToHandle(m_titlePosX, kTaskTextY, "パリィ訓練", kTaskTextColor,
                       m_titleFontHandle);

    if (m_isTitleAnimFinished) {
      SetDrawBlendMode(DX_BLENDMODE_ALPHA, m_taskAlpha);

      int taskY = kTaskTextY + kTitleFontSize + 10;
      int currentX = kTaskTextX;

      DrawExtendGraph(currentX, taskY, currentX + kDiamondSize,
                      taskY + kDiamondSize, m_diamondImg, true);
      currentX += kDiamondSize + kSpacing;

      DrawStringToHandle(currentX, taskY, "遠距離攻撃をパリィする",
                         kTaskTextColor, m_taskFontHandle);

      // 進捗バー
      int barY = taskY + kTaskFontSize + 10;
      float progress = m_displayedParryProgress;
      int currentBarWidth = static_cast<int>(kBarMaxWidth * progress);

      unsigned int barColor =
          (progress >= 1.0f) ? GetColor(0, 255, 128) : GetColor(100, 150, 255);

      DrawBox(kTaskTextX, barY, kTaskTextX + kBarMaxWidth, barY + kBarHeight,
              GetColor(100, 100, 100), true);
      DrawBox(kTaskTextX, barY, kTaskTextX + currentBarWidth, barY + kBarHeight,
              barColor, true);

      DrawStringToHandle(kTaskTextX + kBarMaxWidth + 5, barY, taskText.c_str(),
                         kTaskTextColor, m_taskFontHandle);

      // ガードヒント表示
      if (m_taskAlpha >= 200) {
        int hintY = barY + kBarHeight + 20;
        int hintX = kTaskTextX;

        DrawStringToHandle(hintX, hintY, "ガード: ", kTaskTextColor,
                           m_taskFontHandle);
        hintX += GetDrawStringWidthToHandle("ガード: ", -1, m_taskFontHandle);

        // マウス右クリック画像
        DrawExtendGraph(hintX, hintY, hintX + kMouseImgSize,
                        hintY + kMouseImgSize, m_mouseRightGuardImg, true);
      }

      SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }

    // パリィ説明表示 (一時停止中)
    if (m_isParryTutorialPaused) {
      // 画面全体を少し暗くする
      SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);
      DrawBox(0, 0, 1920, 1080, 0x000000, true);
      SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

      int centerX = 1920 / 2;
      int centerY = 1080 / 2;

      // テキスト表示
      // 「緑色の攻撃はタイミングよくシールドブロック（MouseRight.png）を行うことでパリィできる」
      std::string text1 = "緑色の攻撃はタイミングよくシールドブロック";
      std::string text2 = "を行うことでパリィできる";

      // 中央揃えのために幅計算
      int text1Width =
          GetDrawStringWidthToHandle(text1.c_str(), -1, m_titleFontHandle);
      int iconWidth = 32; // アイコンサイズ
      int text2Width =
          GetDrawStringWidthToHandle(text2.c_str(), -1, m_titleFontHandle);

      int totalWidth = text1Width + iconWidth + text2Width;
      int startX = centerX - totalWidth / 2;

      int currentX = startX;
      int currentY = centerY - 50;

      // テキスト描画
      DrawStringToHandle(currentX, currentY, text1.c_str(), 0xFFFFFF,
                         m_titleFontHandle);
      currentX += text1Width;

      // アイコン描画
      DrawExtendGraph(currentX, currentY, currentX + iconWidth,
                      currentY + iconWidth, m_mouseRightImg, true);
      currentX += iconWidth;

      // 残りのテキスト描画
      DrawStringToHandle(currentX, currentY, text2.c_str(), 0xFFFFFF,
                         m_titleFontHandle);

      // 続行案内
      std::string resumeText = "右クリックを押して再開";
      int resumeTextWidth =
          GetDrawStringWidthToHandle(resumeText.c_str(), -1, m_taskFontHandle);
      DrawStringToHandle(centerX - resumeTextWidth / 2, centerY + 50,
                         resumeText.c_str(), 0xAAAAAA, m_taskFontHandle);
    }

  } break;
  case TaskStep::Completed:
    break;
  default:
    break;
  }
}