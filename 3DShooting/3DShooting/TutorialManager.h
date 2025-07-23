#pragma once
#include <memory>
#include "Mouse.h"

/// <summary>
/// チュートリアル管理クラス
/// </summary>
class TutorialManager
{
public:
    TutorialManager();
	~TutorialManager();

    enum class Step 
    {
        None,
        Move,
        View,
		Completed,
		Jump,
		Run,
		CompletedDisplay // チュートリアル完了後の待機中
    };

    void Init();
    void Update();
    void Draw(int screenW, int screenH);

    /// <summary>
    /// チュートリアルがアクティブかどうか
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
    bool IsCompletedDisplay() const { return m_isCompletedDisplay; }

private:
    Vec2 m_prevMousePos;
    Step m_step;

    bool m_moveDone;
    bool m_viewDone;
    int m_checkMarkHandle;
    float m_moveAccumTime; // WASD操作累積時間
    float m_viewAccumTime; // 視点操作累積時間
    float m_completeWaitTime; // チュートリアル完了後の待機タイマー
    bool m_isCompletedDisplay; // 完了演出表示中フラグ

    // チェックマークアニメーション用
    bool m_moveCheckAnim;
    float m_moveCheckAnimTime;
    bool m_viewCheckAnim;
    float m_viewCheckAnimTime;

    // ジャンプ・走るチュートリアル用
    bool m_jumpDone;
    bool m_runDone;
    float m_jumpAccumTime;
    float m_runAccumTime;
    bool m_jumpCheckAnim;
    float m_jumpCheckAnimTime;
    bool m_runCheckAnim;
    float m_runCheckAnimTime;
};

