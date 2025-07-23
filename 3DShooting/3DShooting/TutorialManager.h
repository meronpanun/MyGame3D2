#pragma once
#include "Mouse.h"
#include <memory>

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
		Move,			 // WASD操作
		View,		     // 視点操作
		Completed,		 // チュートリアル完了
		Jump,			 // ジャンプ操作
		Run,             // 走る操作
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

	int m_checkMarkHandle; // チェックマーク画像のハンドル

	float m_moveAccumTime;     // WASD操作累積時間
	float m_viewAccumTime;     // 視点操作累積時間
	float m_completeWaitTime;  // チュートリアル完了後の待機タイマー
	float m_isMoveCheckAnimTime; // WASD操作のアニメーションタイマー
	float m_isViewCheckAnimTime; // 視点操作のアニメーションタイマー
	float m_jumpAccumTime;     // ジャンプ操作累積時間
	float m_runAccumTime;      // 走る操作累積時間
	float m_isRunCheckAnimTime;  // 走る操作のアニメーションタイマー
	float m_isJumpCheckAnimTime; // ジャンプ操作のアニメーションタイマー

    bool m_isCompletedDisplay; // 完了演出表示中フラグ
	bool m_isMoveDone;		   // WASD操作が完了したか
	bool m_isViewDone;		   // 視点操作が完了したか
	bool m_isMoveCheckAnim;	   // WASD操作のアニメーションが進行中か
	bool m_isViewCheckAnim;    // 視点操作のアニメーションが進行中か
	bool m_isJumpDone;         // ジャンプ操作が完了したか
	bool m_isRunDone;          // 走る操作が完了したか
	bool m_isJumpCheckAnim;    // ジャンプ操作のアニメーションが進行中か
	bool m_isRunCheckAnim;     // 走る操作のアニメーションが進行中か
};

