#pragma once
#include "TutorialTask.h"
#include "AttackType.h"

/// <summary>
/// 射撃チュートリアルタスク。
/// アサルトライフルまたはショットガンで敵を kGoal 体倒すと完了する。
/// </summary>
class ShootTutorialTask : public ITutorialTask
{
public:
    /// <summary>ウェーブ 1 をスポーンし、射撃のみ使用可能に制限する</summary>
    void Start(WaveManager* pWaveManager, Player* pPlayer) override;
    void Update() override;
    /// <summary>操作説明アイコンと武器切り替えヒントを描画する</summary>
    void DrawTaskUI(int x, int y, float scale, int alpha,
                    const TaskTutorialManager* pManager) override;
    bool        IsCompleted()    const override;
    std::string GetTitle()       const override { return "射撃訓練"; }
    float       GetProgress()    const override;
    std::string GetProgressText() const override;
    /// <summary>射撃系の攻撃種別（Shoot / Shotgun）のキルのみカウントする</summary>
    void NotifyEnemyKilled(int attackType) override;

private:
    int m_kills = 0;                    // 現在のキル数
    static constexpr int kGoal = 5;    // クリアに必要なキル数
};

/// <summary>
/// タックルチュートリアルタスク。
/// タックルで敵を kGoal 体倒すと完了する。
/// </summary>
class TackleTutorialTask : public ITutorialTask
{
public:
    /// <summary>ウェーブ 2 をスポーンし、タックルのみ使用可能に制限する</summary>
    void Start(WaveManager* pWaveManager, Player* pPlayer) override;
    void Update() override;
    /// <summary>操作説明アイコンを描画する</summary>
    void DrawTaskUI(int x, int y, float scale, int alpha,
                    const TaskTutorialManager* pManager) override;
    bool        IsCompleted()    const override;
    std::string GetTitle()       const override { return "タックル訓練"; }
    float       GetProgress()    const override;
    std::string GetProgressText() const override;
    /// <summary>タックル攻撃種別のキルのみカウントする</summary>
    void NotifyEnemyKilled(int attackType) override;

private:
    int m_kills = 0;                    // 現在のキル数
    static constexpr int kGoal = 5;    // クリアに必要なキル数
};

/// <summary>
/// 盾投げチュートリアルタスク。
/// 盾投げで敵を kGoal 体倒すと完了する。
/// </summary>
class ShieldThrowTutorialTask : public ITutorialTask
{
public:
    /// <summary>ウェーブ 3 をスポーンし、盾投げのみ使用可能に制限する</summary>
    void Start(WaveManager* pWaveManager, Player* pPlayer) override;
    void Update() override;
    /// <summary>操作説明アイコンを描画する</summary>
    void DrawTaskUI(int x, int y, float scale, int alpha,
                    const TaskTutorialManager* pManager) override;
    bool        IsCompleted()    const override;
    std::string GetTitle()       const override { return "盾投げ訓練"; }
    float       GetProgress()    const override;
    std::string GetProgressText() const override;
    void NotifyEnemyKilled(int attackType) override;
    /// <summary>盾投げによるキルをカウントする</summary>
    void NotifyShieldThrowKill() override;

private:
    int m_kills = 0;                    // 現在のキル数
    static constexpr int kGoal = 2;    // クリアに必要なキル数
};

/// <summary>
/// パリィチュートリアルタスク。
/// 遠距離攻撃をパリィして kGoal 回成功すると完了する。
/// </summary>
class ParryTutorialTask : public ITutorialTask
{
public:
    /// <summary>ウェーブ 4 をスポーンし、パリィのみ使用可能に制限する</summary>
    void Start(WaveManager* pWaveManager, Player* pPlayer) override;
    void Update() override;
    /// <summary>操作説明アイコンとガードヒントを描画する</summary>
    void DrawTaskUI(int x, int y, float scale, int alpha,
                    const TaskTutorialManager* pManager) override;
    bool        IsCompleted()    const override;
    std::string GetTitle()       const override { return "パリィ訓練"; }
    float       GetProgress()    const override;
    std::string GetProgressText() const override;
    /// <summary>パリィ成功回数をカウントする</summary>
    void NotifyParrySuccess() override;

private:
    int m_parryCount = 0;               // 現在のパリィ成功回数
    static constexpr int kGoal = 3;    // クリアに必要なパリィ回数
};
