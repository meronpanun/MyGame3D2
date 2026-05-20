#pragma once
#include "Vec2.h"
#include "ManagedFont.h"
#include "ManagedGraph.h"
#include <memory>
#include <string>
#include <vector>

/// <summary>
/// 基本操作チュートリアル管理クラス。
/// 移動・視点・ジャンプ・走りの 4 ステップをプレイヤーの入力累積時間で判定し、
/// 全ステップ完了後に完了演出を経て Completed 状態へ移行する。
/// </summary>
class TutorialManager
{
public:
    /// <summary>
    /// チュートリアルの進行ステップ
    /// </summary>
    enum class Step
    {
        None,
        Move,            // WASD 操作
        View,            // 視点操作
        Jump,            // ジャンプ操作
        Run,             // 走る操作
        Completed,       // チュートリアル完了
        CompletedDisplay // 完了演出表示中
    };

    /// <summary>
    /// チュートリアルメッセージ UI のアニメーション状態
    /// </summary>
    enum class UIState
    {
        Hidden,   // 非表示
        Entering, // スライドイン中
        OnScreen, // 表示中
        Exiting   // スライドアウト中
    };

    /// <summary>
    /// 画面上に表示するチュートリアルメッセージ 1 件分
    /// </summary>
    struct TutorialMessage
    {
        std::string title;        // メッセージのタイトル
        std::string detail;       // メッセージの詳細テキスト
        float       xOffset;      // 現在の X オフセット（スライドアニメーション用）
        float       displayTimer; // 表示開始からの経過時間（秒）
        UIState     state;        // 現在の UI アニメーション状態
    };

    TutorialManager();
    ~TutorialManager() = default;

    /// <summary>
    /// チュートリアルを初期化し、移動ステップから開始する
    /// </summary>
    void Init();

    /// <summary>
    /// 毎フレームの更新処理（入力判定・ステップ遷移・完了演出を処理する）
    /// </summary>
    void Update();

    /// <summary>
    /// チュートリアル UI を描画する
    /// </summary>
    /// <param name="screenW">画面幅（ピクセル）</param>
    /// <param name="screenH">画面高さ（ピクセル）</param>
    void Draw(int screenW, int screenH);

    /// <summary>
    /// 全ステップを完了済みにしてチュートリアルをスキップする
    /// </summary>
    void Skip();

    /// <summary>
    /// チュートリアルが進行中かどうかを返す（None・Completed 以外なら true）
    /// </summary>
    /// <returns>進行中なら true</returns>
    bool IsActive() const;

    /// <summary>
    /// チュートリアルが完了したかどうかを返す
    /// </summary>
    /// <returns>Completed 状態なら true</returns>
    bool IsCompleted() const;

    /// <summary>
    /// 完了演出の表示中かどうかを返す
    /// </summary>
    /// <returns>完了演出表示中なら true</returns>
    bool IsDisplayingCompletion() const { return m_isDisplayingCompletion; }

    /// <summary>
    /// 新しいチュートリアルメッセージをキューに追加する
    /// </summary>
    /// <param name="title">メッセージのタイトル</param>
    /// <param name="detail">メッセージの詳細テキスト</param>
    void AddMessage(const std::string& title, const std::string& detail);

    /// <summary>現在の進行ステップを返す</summary>
    Step GetStep() const { return m_step; }

    /// <summary>表示中のメッセージリストを返す</summary>
    const std::vector<TutorialMessage>& GetMessages() const { return m_messages; }

    /// <summary>移動ステップが完了しているかを返す</summary>
    bool HasCompletedMove() const { return m_hasCompletedMove; }
    /// <summary>視点操作ステップが完了しているかを返す</summary>
    bool HasCompletedView() const { return m_hasCompletedView; }
    /// <summary>ジャンプステップが完了しているかを返す</summary>
    bool HasCompletedJump() const { return m_hasCompletedJump; }
    /// <summary>走りステップが完了しているかを返す</summary>
    bool HasCompletedRun()  const { return m_hasCompletedRun;  }

    /// <summary>現在のステップが完了して次ステップへの待機中かを返す</summary>
    bool IsStepCompleted() const { return m_isStepCompleted; }

private:
    /// <summary>
    /// 表示中のメッセージの経過タイマーを更新する
    /// </summary>
    void UpdateMessages();

private:
    Vec2 m_prevMousePos; // 前フレームのマウス座標（視点移動量の算出用）
    Step m_step;         // 現在の進行ステップ

    // チュートリアル進行関連
    float m_completeWaitTime;     // 完了演出のタイマー（秒）
    float m_moveAccumTime;        // 移動操作の累積時間（秒）
    float m_viewAccumTime;        // 視点操作の累積時間（秒）
    float m_jumpAccumTime;        // ジャンプ操作の累積時間（秒）
    float m_runAccumTime;         // 走る操作の累積時間（秒）
    bool  m_isDisplayingCompletion; // 完了演出表示中フラグ
    bool  m_hasCompletedMove;     // 移動ステップ完了フラグ
    bool  m_hasCompletedView;     // 視点ステップ完了フラグ
    bool  m_hasCompletedJump;     // ジャンプステップ完了フラグ
    bool  m_hasCompletedRun;      // 走りステップ完了フラグ

    // ステップ完了後の待機
    float m_stepCompleteWaitTime; // ステップ完了後の待機タイマー（秒）
    bool  m_isStepCompleted;      // ステップ完了して次ステップ待機中フラグ

    // メッセージ表示
    std::vector<TutorialMessage> m_messages; // 表示中のメッセージリスト
};
