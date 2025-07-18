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
        Completed
    };

    void Init();
    void Update();
    void Draw(int screenW, int screenH);

    bool IsActive() const;
    bool IsCompleted() const;

    // チュートリアル完了後の待機中かどうか
    bool IsCompletedDisplay() const { return m_isCompletedDisplay; }

private:
    Step m_step;
    bool m_moveDone;
    bool m_viewDone;
    int m_checkMarkHandle;
    Vec2 m_prevMousePos;
    float m_moveAccumTime = 0.0f; // WASD操作累積時間
    float m_viewAccumTime = 0.0f; // 視点操作累積時間
    float m_completeWaitTime = 0.0f; // チュートリアル完了後の待機タイマー
    bool m_isCompletedDisplay = false; // 完了演出表示中フラグ
    // チェックマークアニメーション用
    bool m_moveCheckAnim = false;
    float m_moveCheckAnimTime = 0.0f;
    bool m_viewCheckAnim = false;
    float m_viewCheckAnimTime = 0.0f;
};

