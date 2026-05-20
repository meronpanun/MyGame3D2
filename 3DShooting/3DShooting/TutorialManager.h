#pragma once
#include "Vec2.h"
#include <memory>
#include <string>
#include <vector>
#include "ManagedFont.h"
#include "ManagedGraph.h"


/// <summary>
/// チュートリアル管理クラス
/// </summary>
class TutorialManager
{
public:
    TutorialManager();
    ~TutorialManager();

    /// <summary>
    /// チュートリアルのステップ
    /// </summary>
    enum class Step
    {
        None,
        Move,            // WASD操作
        View,            // 視点操作
        Jump,            // ジャンプ操作
        Run,             // 走る操作
        Completed,       // チュートリアル完了
        CompletedDisplay // チュートリアル完了後の待機中
    };

    /// <summary>
    /// UIアニメーションの状態
    /// </summary>
    enum class UIState
    {
        Hidden,   // 隠れている
        Entering, // 画面に入ってくる
        OnScreen, // 画面に表示されている
        Exiting   // 画面から出ていく
    };

    /// <summary>
    /// 状況に応じて表示するチュートリアルメッセージ
    /// </summary>
    struct TutorialMessage
    {
        std::string title;
        std::string detail;
        float xOffset;
        float displayTimer;
        UIState state;
    };

    void Init();
    void Update();
    void Draw(int screenW, int screenH);

    /// <summary>
    /// チュートリアルをスキップして完了済みにする
    /// </summary>
    void Skip();

    /// <summary>
    /// チュートリアルがアクティブかどうかの判断
    /// </summary>
    /// <returns>アクティブならtrue</returns>
    bool IsActive() const;

    /// <summary>
    /// チュートリアルが完了したかどうか
    /// </summary>
    /// <returns>完了していればtrue</returns>
    bool IsCompleted() const;

    /// <summary>
    /// チュートリアル完了後の待機中かどうか 
    /// </summary>
    /// <returns>完了演出表示中ならtrue</returns>
    bool IsDisplayingCompletion() const { return m_isDisplayingCompletion; }

    /// <summary>
    /// 新しいチュートリアルメッセージを追加する
    /// </summary>
    /// <param name="title">メッセージのタイトル</param>
    /// <param name="detail">メッセージの詳細</param>
    void AddMessage(const std::string& title, const std::string& detail);

    // Getters for TutorialUI
    Step GetStep() const { return m_step; }
    const std::vector<TutorialMessage>& GetMessages() const { return m_messages; }
    
    bool HasCompletedMove() const { return m_hasCompletedMove; }
    bool HasCompletedView() const { return m_hasCompletedView; }
    bool HasCompletedJump() const { return m_hasCompletedJump; }
    bool HasCompletedRun() const { return m_hasCompletedRun; }
    
    bool IsStepCompleted() const { return m_isStepCompleted; }

private:
    /// <summary>
    /// メッセージの更新
    /// </summary>
    void UpdateMessages();

private:
    Vec2 m_prevMousePos;
    Step m_step;

    // チュートリアル進行関連
    float m_completeWaitTime;   // チュートリアル完了後の待機タイマー
    float m_runAccumTime;       // 走る操作累積時間
    float m_jumpAccumTime;      // ジャンプ操作累積時間
    float m_viewAccumTime;      // 視点操作累積時間
    float m_moveAccumTime;      // WASD操作累積時間
    bool  m_isDisplayingCompletion; // 完了演出表示中フラグ
    bool  m_hasCompletedRun;          // 走る操作が完了したか
    bool  m_hasCompletedJump;         // ジャンプ操作が完了したか
    bool  m_hasCompletedView;         // 視点操作が完了したか
    bool  m_hasCompletedMove;         // WASD操作が完了したか

    // ステップ完了後の待機
    float m_stepCompleteWaitTime; // ステップ完了後の待機タイマー
    bool  m_isStepCompleted;      // ステップ完了後の待機中フラグ

    // メッセージ関連
    std::vector<TutorialMessage> m_messages;
};
