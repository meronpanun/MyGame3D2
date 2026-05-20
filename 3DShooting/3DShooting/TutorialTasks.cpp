#include "TutorialTasks.h"
#include "WaveManager.h"
#include "Player.h"
#include "TaskTutorialManager.h"

namespace
{
    // DrawTaskUI 共通レイアウト定数（基準解像度 720p 時のピクセル値）
    constexpr int kIconSpacing        =  8;  // アイコン間の水平間隔（px）
    constexpr int kDiamondIconSize    = 30;  // ダイヤモンドアイコンのサイズ（px）
    constexpr int kActionIconSize     = 36;  // マウス・キーアイコンのサイズ（px）
    constexpr int kHintOffsetY        = 80;  // ヒントテキストの本文からの縦オフセット（px）
    constexpr int kHintAlphaThreshold = 200; // ヒントを表示するアルファ値の閾値
}

// ============================================================
// ShootTutorialTask
// ============================================================

void ShootTutorialTask::Start(WaveManager* pWaveManager, Player* pPlayer)
{
    if (pWaveManager) pWaveManager->SpawnTutorialWave(1);
    if (pPlayer)      pPlayer->SetAttackRestrictions(AttackType::Shoot);
}

void ShootTutorialTask::Update() {}

void ShootTutorialTask::DrawTaskUI(int x, int y, float scale, int alpha,
                                   const TaskTutorialManager* pManager)
{
    int currentX    = x;
    int spacing     = static_cast<int>(kIconSpacing     * scale);
    int diamondSize = static_cast<int>(kDiamondIconSize * scale);
    int iconSize    = static_cast<int>(kActionIconSize  * scale);

    DrawExtendGraph(currentX, y, currentX + diamondSize, y + diamondSize,
                    pManager->GetDiamondImg(), true);
    currentX += diamondSize + spacing;

    DrawExtendGraph(currentX, y, currentX + iconSize, y + iconSize,
                    pManager->GetMouseLeftImg(), true);
    currentX += iconSize + spacing;

    DrawStringToHandle(currentX, y, "でゾンビを倒す", 0xFFFFFF, pManager->GetTaskFont());

    // フェードイン完了後に武器切り替えヒントを表示
    if (alpha >= kHintAlphaThreshold)
    {
        int hintY    = y + static_cast<int>(kHintOffsetY * scale);
        int hintX    = x;
        DrawStringToHandle(hintX, hintY, "武器切り替え: ", 0xFFFFFF, pManager->GetTaskFont());
        hintX += GetDrawStringWidthToHandle("武器切り替え: ", -1, pManager->GetTaskFont());

        DrawExtendGraph(hintX, hintY, hintX + iconSize, hintY + iconSize,
                        pManager->GetAlpha1Img(), true);
        hintX += iconSize + spacing;
        DrawExtendGraph(hintX, hintY, hintX + iconSize, hintY + iconSize,
                        pManager->GetAlpha2Img(), true);
        hintX += iconSize + spacing;
        DrawStringToHandle(hintX, hintY, " / ", 0xFFFFFF, pManager->GetTaskFont());
        hintX += GetDrawStringWidthToHandle(" / ", -1, pManager->GetTaskFont());
        DrawExtendGraph(hintX, hintY, hintX + iconSize, hintY + iconSize,
                        pManager->GetMouseWheelImg(), true);
    }
}

bool        ShootTutorialTask::IsCompleted()    const { return m_kills >= kGoal; }
float       ShootTutorialTask::GetProgress()    const { return static_cast<float>(m_kills) / kGoal; }
std::string ShootTutorialTask::GetProgressText() const { return std::to_string(m_kills) + "/" + std::to_string(kGoal); }

void ShootTutorialTask::NotifyEnemyKilled(int attackType)
{
    if (attackType == static_cast<int>(AttackType::Shoot) ||
        attackType == static_cast<int>(AttackType::Shotgun))
    {
        m_kills++;
    }
}

// ============================================================
// TackleTutorialTask
// ============================================================

void TackleTutorialTask::Start(WaveManager* pWaveManager, Player* pPlayer)
{
    if (pWaveManager) pWaveManager->SpawnTutorialWave(2);
    if (pPlayer)      pPlayer->SetAttackRestrictions(AttackType::Tackle);
}

void TackleTutorialTask::Update() {}

void TackleTutorialTask::DrawTaskUI(int x, int y, float scale, int alpha,
                                    const TaskTutorialManager* pManager)
{
    int currentX    = x;
    int spacing     = static_cast<int>(kIconSpacing     * scale);
    int diamondSize = static_cast<int>(kDiamondIconSize * scale);
    int iconSize    = static_cast<int>(kActionIconSize  * scale);

    DrawExtendGraph(currentX, y, currentX + diamondSize, y + diamondSize,
                    pManager->GetDiamondImg(), true);
    currentX += diamondSize + spacing;

    DrawExtendGraph(currentX, y, currentX + iconSize, y + iconSize,
                    pManager->GetMouseRightImg(), true);
    currentX += iconSize + spacing;

    DrawStringToHandle(currentX, y, "長押し", 0xFFFFFF, pManager->GetTaskFont());
    currentX += GetDrawStringWidthToHandle("長押し", -1, pManager->GetTaskFont()) + spacing;

    DrawExtendGraph(currentX, y, currentX + iconSize, y + iconSize,
                    pManager->GetLockOnUIImg(), true);
    currentX += iconSize + spacing;

    DrawExtendGraph(currentX, y, currentX + iconSize, y + iconSize,
                    pManager->GetMouseLeftImg(), true);
    currentX += iconSize + spacing;

    DrawStringToHandle(currentX, y, "でタックル", 0xFFFFFF, pManager->GetTaskFont());
}

bool        TackleTutorialTask::IsCompleted()    const { return m_kills >= kGoal; }
float       TackleTutorialTask::GetProgress()    const { return static_cast<float>(m_kills) / kGoal; }
std::string TackleTutorialTask::GetProgressText() const { return std::to_string(m_kills) + "/" + std::to_string(kGoal); }

void TackleTutorialTask::NotifyEnemyKilled(int attackType)
{
    if (attackType == static_cast<int>(AttackType::Tackle)) m_kills++;
}

// ============================================================
// ShieldThrowTutorialTask
// ============================================================

void ShieldThrowTutorialTask::Start(WaveManager* pWaveManager, Player* pPlayer)
{
    if (pWaveManager) pWaveManager->SpawnTutorialWave(3);
    if (pPlayer)      pPlayer->SetAttackRestrictions(AttackType::ShieldThrow);
}

void ShieldThrowTutorialTask::Update() {}

void ShieldThrowTutorialTask::DrawTaskUI(int x, int y, float scale, int alpha,
                                         const TaskTutorialManager* pManager)
{
    int currentX    = x;
    int spacing     = static_cast<int>(kIconSpacing     * scale);
    int diamondSize = static_cast<int>(kDiamondIconSize * scale);
    int iconSize    = static_cast<int>(kActionIconSize  * scale);

    DrawExtendGraph(currentX, y, currentX + diamondSize, y + diamondSize,
                    pManager->GetDiamondImg(), true);
    currentX += diamondSize + spacing;

    DrawExtendGraph(currentX, y, currentX + iconSize, y + iconSize,
                    pManager->GetRKeyImg(), true);
    currentX += iconSize + spacing;

    DrawStringToHandle(currentX, y, "でシールドを破壊し敵を倒す", 0xFFFFFF, pManager->GetTaskFont());
}

bool        ShieldThrowTutorialTask::IsCompleted()    const { return m_kills >= kGoal; }
float       ShieldThrowTutorialTask::GetProgress()    const { return static_cast<float>(m_kills) / kGoal; }
std::string ShieldThrowTutorialTask::GetProgressText() const { return std::to_string(m_kills) + "/" + std::to_string(kGoal); }

void ShieldThrowTutorialTask::NotifyEnemyKilled(int attackType)
{
    // 盾投げタスク中は攻撃種別に関わらずキルをカウントする
    // （盾の連鎖破壊による巻き込みキルなど、種別が混在するため）
    (void)attackType;
    m_kills++;
}

void ShieldThrowTutorialTask::NotifyShieldThrowKill() { m_kills++; }

// ============================================================
// ParryTutorialTask
// ============================================================

void ParryTutorialTask::Start(WaveManager* pWaveManager, Player* pPlayer)
{
    if (pWaveManager) pWaveManager->SpawnTutorialWave(4);
    if (pPlayer)      pPlayer->SetAttackRestrictions(AttackType::Parry);
}

void ParryTutorialTask::Update() {}

void ParryTutorialTask::DrawTaskUI(int x, int y, float scale, int alpha,
                                   const TaskTutorialManager* pManager)
{
    int currentX    = x;
    int spacing     = static_cast<int>(kIconSpacing     * scale);
    int diamondSize = static_cast<int>(kDiamondIconSize * scale);
    int iconSize    = static_cast<int>(kActionIconSize  * scale);

    DrawExtendGraph(currentX, y, currentX + diamondSize, y + diamondSize,
                    pManager->GetDiamondImg(), true);
    currentX += diamondSize + spacing;

    DrawStringToHandle(currentX, y, "遠距離攻撃をパリィする", 0xFFFFFF, pManager->GetTaskFont());

    // フェードイン完了後にガード操作ヒントを表示
    if (alpha >= kHintAlphaThreshold)
    {
        int hintY = y + static_cast<int>(kHintOffsetY * scale);
        int hintX = x;
        DrawStringToHandle(hintX, hintY, "ガード: ", 0xFFFFFF, pManager->GetTaskFont());
        hintX += GetDrawStringWidthToHandle("ガード: ", -1, pManager->GetTaskFont());
        DrawExtendGraph(hintX, hintY, hintX + iconSize, hintY + iconSize,
                        pManager->GetMouseRightImg(), true);
    }
}

bool        ParryTutorialTask::IsCompleted()    const { return m_parryCount >= kGoal; }
float       ParryTutorialTask::GetProgress()    const { return static_cast<float>(m_parryCount) / kGoal; }
std::string ParryTutorialTask::GetProgressText() const { return std::to_string(m_parryCount) + "/" + std::to_string(kGoal); }

void ParryTutorialTask::NotifyParrySuccess() { m_parryCount++; }
