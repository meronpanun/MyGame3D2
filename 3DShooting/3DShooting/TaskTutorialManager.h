#pragma once
#include "AttackType.h"

class WaveManager;
class Player;     

/// <summary>
/// タスク型チュートリアルマネージャークラス
/// </summary>
class TaskTutorialManager
{
public:
    TaskTutorialManager();
    ~TaskTutorialManager();

    void Init(WaveManager* pWaveManager, Player* pPlayer);
    void Update();
    void Draw();

    // シングルトンインスタンスを取得
    static TaskTutorialManager* GetInstance();

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
    TaskTutorialManager(const TaskTutorialManager&) = delete;
    TaskTutorialManager& operator=(const TaskTutorialManager&) = delete;

    static TaskTutorialManager* m_instance;

    WaveManager* m_pWaveManager; // WaveManagerへのポインタ
    Player* m_pPlayer;           // Playerへのポインタ

    TaskStep m_step;
    int m_shootKills;
    int m_tackleKills;

    int m_fontHandle; // 文字描画用のフォントハンドル
};