#pragma once
#include "AttackType.h"

class WaveManager;

class TaskTutorialManager
{
public:
    // シングルトンインスタンスを取得
    static TaskTutorialManager* GetInstance();

    void Init(WaveManager* pWaveManager);
    void Update();
    void Draw();

    // 敵が倒されたことを通知する
    void NotifyEnemyKilled(AttackType attackType);

    // チュートリアルが完了したか
    bool IsCompleted() const;

private:
    // チュートリアルの進行ステップ
    enum class TaskStep
    {
        None,
        Shoot,  // 射撃タスク
        Tackle, // タックルタスク
        Completed // 全て完了
    };

    TaskTutorialManager();
    ~TaskTutorialManager();
    TaskTutorialManager(const TaskTutorialManager&) = delete;
    TaskTutorialManager& operator=(const TaskTutorialManager&) = delete;

    static TaskTutorialManager* m_instance;

    WaveManager* m_pWaveManager; // WaveManagerへのポインタ

    TaskStep m_step;
    int m_shootKills;
    int m_tackleKills;

    int m_fontHandle; // 文字描画用のフォントハンドル
};