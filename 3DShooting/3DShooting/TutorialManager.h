#pragma once
#include "Vec2.h"
#include <memory>
#include <string>
#include <vector>

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
		Jump,			 // ジャンプ操作
		Run,             // 走る操作
		Completed,		 // チュートリアル完了
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
		float x_offset;
		float display_timer;
		UIState state;
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

	/// <summary>
	/// 新しいチュートリアルメッセージを追加する
	/// </summary>
	/// <param name="title">メッセージのタイトル</param>
	/// <param name="detail">メッセージの詳細</param>
	void AddMessage(const std::string& title, const std::string& detail);

	// スケール変更時のフォントリロード
	void ReloadFonts(float scale);

private:
	/// <summary>
	/// UIアニメーション用の更新
	/// </summary>
	void UpdateUI();

	/// <summary>
	/// メッセージの更新
	/// </summary>
	void UpdateMessages();

	/// <summary>
	/// メッセージの描画
	/// </summary>
	void DrawMessages(int screenW, int screenH);

private:
	Vec2 m_prevMousePos;
	Step m_step;
	UIState m_uiState; // UIの状態

	// リソース関連
	int m_checkMarkHandle;         // チェックマーク画像のハンドル
	int m_japaneseFontHandle;      // 日本語フォントハンドル
	int m_japaneseLargeFontHandle; // 日本語ラージフォントハンドル
	int m_messageDetailFontHandle; // メッセージ詳細用のフォントハンドル
	int m_wKeyHandle;
	int m_aKeyHandle;
	int m_sKeyHandle;
	int m_dKeyHandle;
	int m_mouseMoveHorHandle;
	int m_spaceKeyHandle;
	int m_leftShiftKeyHandle;
	int m_crossHandle;

	// チュートリアル進行関連
	float m_completeWaitTime;   // チュートリアル完了後の待機タイマー
	float m_runAccumTime;       // 走る操作累積時間
	float m_jumpAccumTime;      // ジャンプ操作累積時間
	float m_viewAccumTime;      // 視点操作累積時間
	float m_moveAccumTime;      // WASD操作累積時間
	bool  m_isCompletedDisplay; // 完了演出表示中フラグ
	bool  m_isRunDone;          // 走る操作が完了したか
	bool  m_isJumpDone;         // ジャンプ操作が完了したか
	bool  m_isViewDone;		    // 視点操作が完了したか
	bool  m_isMoveDone;		    // WASD操作が完了したか

	// ステップ完了後の待機
	float m_stepCompleteWaitTime; // ステップ完了後の待機タイマー
	bool  m_isStepCompleted;      // ステップ完了後の待機中フラグ

	// アニメーション関連
	float m_uiXOffset;         // UIのX座標オフセット
	float m_moveCheckAnimTime; // WASD操作のアニメーションタイマー
	float m_viewCheckAnimTime; // 視点操作のアニメーションタイマー
	float m_runCheckAnimTime;  // 走る操作のアニメーションタイマー
	float m_jumpCheckAnimTime; // ジャンプ操作のアニメーションタイマー
	bool  m_isMoveCheckAnim;   // WASD操作のアニメーションが進行中か
	bool  m_isViewCheckAnim;   // 視点操作のアニメーションが進行中か
	bool  m_isJumpCheckAnim;   // ジャンプ操作のアニメーションが進行中か
	bool  m_isRunCheckAnim;    // 走る操作のアニメーションが進行中か

	// メッセージ関連
	std::vector<TutorialMessage> m_messages;
    
    float m_prevScale;
};
