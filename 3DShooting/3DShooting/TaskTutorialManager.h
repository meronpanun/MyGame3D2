#pragma once
#include "AttackType.h"
#include "ManagedFont.h"
#include "ManagedGraph.h"
#include <memory>

class WaveManager;
class Player;
class ITutorialTask;

/// <summary>
/// タスク型チュートリアルマネージャークラス。Meyer's シングルトン。
/// 射撃・タックル・盾投げ・パリィの 4 タスクを順番にプレイヤーへ提示し、
/// 達成状況をフォント・画像・プログレスバーで UI 表示する。
/// </summary>
class TaskTutorialManager
{
public:
    ~TaskTutorialManager() = default;

    // コピー・ムーブ禁止（シングルトン）
    TaskTutorialManager(const TaskTutorialManager&)            = delete;
    TaskTutorialManager& operator=(const TaskTutorialManager&) = delete;

    /// <summary>
    /// シングルトンインスタンスを返す（Meyer's static local）
    /// </summary>
    /// <returns>唯一のインスタンスへのポインタ</returns>
    static TaskTutorialManager* GetInstance();

    /// <summary>
    /// チュートリアルを初期化し、射撃タスクから開始する
    /// </summary>
    /// <param name="pWaveManager">ウェーブ制御オブジェクト</param>
    /// <param name="pPlayer">プレイヤーオブジェクト</param>
    void Init(WaveManager* pWaveManager, Player* pPlayer);

    /// <summary>
    /// 毎フレームの更新処理（タスク進行・アニメーション・ステップ遷移）
    /// </summary>
    void Update();

    /// <summary>
    /// UI を描画する（タスク説明・プログレスバー・フィードバック画像）
    /// </summary>
    void Draw();

    /// <summary>
    /// 敵が倒されたことをアクティブタスクに通知する
    /// </summary>
    /// <param name="attackType">倒した際の攻撃種別</param>
    void NotifyEnemyKilled(AttackType attackType);

    /// <summary>
    /// 盾投げで敵が倒されたことをアクティブタスクに通知する
    /// </summary>
    void NotifyShieldThrowKill();

    /// <summary>
    /// パリィ成功をアクティブタスクに通知する
    /// </summary>
    void NotifyParrySuccess();

    /// <summary>
    /// パリィ可能な攻撃が来たことを通知する（パリィ説明の一時停止トリガー）
    /// </summary>
    void NotifyParryableAttack();

    /// <summary>
    /// チュートリアルが全タスク完了しているかを返す
    /// </summary>
    /// <returns>全タスク完了なら true</returns>
    bool IsCompleted() const;

    /// <summary>
    /// チュートリアルをリセットし、待機状態に戻す
    /// </summary>
    void Reset();

    /// <summary>
    /// チュートリアルを強制完了させる
    /// </summary>
    /// <param name="pWaveManager">完了後のウェーブ制御に使用するポインタ</param>
    void Skip(WaveManager* pWaveManager);

    /// <summary>
    /// パリィタスクまでスキップし、パリィ説明から再開する
    /// </summary>
    void SkipToParry();

    /// <summary>
    /// 現在のタスクで制限されたアクションが試みられたことを通知する
    /// </summary>
    /// <param name="attemptedType">試みられた攻撃種別</param>
    void NotifyRestrictedAction(AttackType attemptedType);

    /// <summary>
    /// 画面スケール変更時にフォントを再生成する
    /// </summary>
    /// <param name="scale">新しい UI スケール係数</param>
    void ReloadFonts(float scale);

    /// <summary>タスク UI 用ダイヤモンド画像ハンドルを返す</summary>
    int GetDiamondImg()    const { return m_diamondImg;    }
    /// <summary>マウス左ボタン画像ハンドルを返す</summary>
    int GetMouseLeftImg()  const { return m_mouseLeftImg;  }
    /// <summary>マウス右ボタン画像ハンドルを返す</summary>
    int GetMouseRightImg() const { return m_mouseRightImg; }
    /// <summary>キーボード 1 キー画像ハンドルを返す</summary>
    int GetAlpha1Img()     const { return m_alpha1Img;     }
    /// <summary>キーボード 2 キー画像ハンドルを返す</summary>
    int GetAlpha2Img()     const { return m_alpha2Img;     }
    /// <summary>マウスホイール画像ハンドルを返す</summary>
    int GetMouseWheelImg() const { return m_mouseWheelImg; }
    /// <summary>R キー画像ハンドルを返す</summary>
    int GetRKeyImg()       const { return m_rKeyImg;       }
    /// <summary>ロックオン UI 画像ハンドルを返す</summary>
    int GetLockOnUIImg()   const { return m_lockOnUIImg;   }
    /// <summary>タスク説明用フォントハンドルを返す</summary>
    int GetTaskFont()      const { return m_taskFont;      }

private:
    TaskTutorialManager();

    /// <summary>
    /// チュートリアルの進行ステップ
    /// </summary>
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

    WaveManager* m_pWaveManager; // WaveManager への非所有ポインタ
    Player*      m_pPlayer;      // Player への非所有ポインタ

    TaskStep                       m_step;        // 現在のチュートリアルステップ
    std::unique_ptr<ITutorialTask> m_currentTask; // 現在実行中のタスク

    // 進捗バーアニメーション
    float m_displayedProgress; // 表示中の進捗値（アニメーション補間）
    float m_progressAnimSpeed; // 進捗バーのアニメーション速度

    ManagedFont m_titleFont; // タイトル用フォント
    ManagedFont m_taskFont;  // タスク説明用フォント

    // UI 画像リソース
    ManagedGraph m_diamondImg;        // ダイヤモンドアイコン
    ManagedGraph m_mouseLeftImg;      // マウス左ボタン
    ManagedGraph m_mouseRightImg;     // マウス右ボタン
    ManagedGraph m_alpha1Img;         // キーボード 1 キー
    ManagedGraph m_alpha2Img;         // キーボード 2 キー
    ManagedGraph m_mouseWheelImg;     // マウスホイール
    ManagedGraph m_rKeyImg;           // R キー
    ManagedGraph m_lockOnUIImg;       // ロックオン UI
    ManagedGraph m_mouseRightGuardImg; // マウス右ボタン（ガード用）
    ManagedGraph m_designerImg;        // フィードバック用背景画像

    // タイトルスライドインアニメーション
    float m_titlePosX;          // タイトルテキストの現在 X 座標
    float m_titleAnimSpeed;     // タイトルスライドインの速度
    bool  m_isTitleAnimFinished; // タイトルアニメーション完了フラグ

    // タスク内容フェードイン
    int   m_taskAlpha;     // タスクテキストのアルファ値（0〜255）
    float m_taskFadeSpeed; // タスクテキストのフェードイン速度

    int m_animationWaitTimer;   // タイトルアニメーション後の待機タイマー
    int m_transitionDelayTimer; // ステップ移行の遅延タイマー

    // パリィチュートリアル一時停止
    bool m_hasShownParryTutorial; // パリィ説明を表示済みかどうか
    bool m_isParryTutorialPaused; // パリィ説明表示中で一時停止しているか

    // 制限アクションフィードバック
    int        m_restrictedActionTimer; // 表示タイマー（0 で非表示）
    AttackType m_restrictedActionType;  // 制限されたアクションの種類
    int        m_restrictedActionAlpha; // フェード用アルファ値

    float m_prevScale; // 前フレームの UI スケール値（フォントリロード検出用）
};
