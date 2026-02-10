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

    // 盾投げで敵が倒されたことを通知する
    void NotifyShieldThrowKill();

    // パリィ成功を通知する
    void NotifyParrySuccess();

    // パリィ可能な攻撃が来たことを通知する（チュートリアル停止用）
    void NotifyParryableAttack();

    // チュートリアルが完了したか
    bool IsCompleted() const;

    // 状態をリセットする
    void Reset();

    // チュートリアルをスキップする
    void Skip(WaveManager* pWaveManager);

    // 制限されたアクションが行われたことを通知する
    void NotifyRestrictedAction(AttackType attemptedType);

    // スケール変更時のフォントリロード
    void ReloadFonts(float scale);

private:
    // チュートリアルの進行ステップ
    enum class TaskStep
    {
        None,
        Shoot,                    // 射撃タスク
        ShootCompleteDelay,       // 射撃タスク完了後の待機
        Tackle,                   // タックルタスク
        TackleCompleteDelay,      // タックルタスク完了後の待機
        ShieldThrow,              // 盾投げタスク
        ShieldThrowCompleteDelay, // 盾投げタスク完了後の待機
        Parry,                    // パリィタスク
        ParryCompleteDelay,       // パリィタスク完了後の待機
        Completed                 // 全て完了
    };
    TaskTutorialManager(const TaskTutorialManager&) = delete;
    TaskTutorialManager& operator=(const TaskTutorialManager&) = delete;

    static TaskTutorialManager* m_instance;

    WaveManager* m_pWaveManager; // WaveManagerへのポインタ
    Player* m_pPlayer;           // Playerへのポインタ

    TaskStep m_step;
    int m_shootKills;
    int m_tackleKills;
    int m_shieldThrowKills; // 盾投げキル数
    int m_parryCount;       // パリィ成功回数

    int m_titleFontHandle; // タイトル用のフォントハンドル
    int m_taskFontHandle;  // タスク内容用のフォントハンドル
    int m_diamondImg;
    int m_mouseLeftImg;
    int m_mouseRightImg;

    // 武器切り替えヒント用画像
    int m_alpha1Img;     // キーボード1キーの画像
    int m_alpha2Img;     // キーボード2キーの画像
    int m_mouseWheelImg; // マウスホイールの画像

    // 盾投げ・パリィタスク用画像
    int m_rKeyImg;            // Rキーの画像
    int m_lockOnUIImg;        // ロックオンUIの画像
    int m_mouseRightGuardImg; // マウス右クリック(ガード用)の画像
    int m_designerImg;        // Designer.png画像

    // タイトルアニメーション用
    float m_titlePosX;
    float m_titleAnimSpeed;
    bool m_isTitleAnimFinished;

    // タスク内容フェードイン用
    int m_taskAlpha;
    float m_taskFadeSpeed;

    // アニメーション後の待機タイマー
    int m_animationWaitTimer;

    // プログレスバーアニメーション用
    float m_displayedShootProgress;       // 射撃タスクの表示用進捗
    float m_displayedTackleProgress;      // タックルタスクの表示用進捗
    float m_displayedShieldThrowProgress; // 盾投げタスクの表示用進捗
    float m_displayedParryProgress;       // パリィタスクの表示用進捗
    float m_progressAnimSpeed;            // 進捗バーのアニメーション速度

    // ステップ移行の遅延タイマー
    int m_transitionDelayTimer;

    // パリィチュートリアル用一時停止制御
    bool m_hasShownParryTutorial; // パリィ説明を表示したかどうか
    bool m_isParryTutorialPaused; // パリィ説明表示中で停止しているか

    // 制限アクションフィードバック用
    int m_restrictedActionTimer;      // 表示タイマー
    AttackType m_restrictedActionType; // 制限されたアクションの種類
    int m_restrictedActionAlpha;      // フェード用アルファ値

    float m_prevScale; // 前回のスケール値
};