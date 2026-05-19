#pragma once
#include "UIBase.h"
#include "TutorialManager.h"
#include "ManagedFont.h"
#include "ManagedGraph.h"
#include <vector>
#include <memory>

/// <summary>
/// チュートリアルの UI 表示を担当するクラス。
/// 現在のステップに対応した操作説明ボックスと
/// サイドメッセージポップアップをスライドアニメーション付きで描画する。
/// </summary>
class TutorialUI : public UIBase
{
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    /// <param name="pManager">チュートリアル状態を管理する TutorialManager のポインタ</param>
    TutorialUI(TutorialManager* pManager);

    virtual ~TutorialUI() = default;

    /// <summary>
    /// 初期化処理（フォントのロードを行う）
    /// </summary>
    void Init() override;

    /// <summary>
    /// 毎フレームの更新処理（UIスライドアニメーション・チェックマークアニメーション更新）
    /// </summary>
    /// <param name="deltaTime">前フレームからの経過時間（秒）</param>
    void Update(float deltaTime) override;

    /// <summary>
    /// 描画処理（タスクUIとサイドメッセージを描画する）
    /// </summary>
    void Draw() override;

private:
    /// <summary>
    /// 現在のステップに対応したタスク説明ボックスを描画する
    /// </summary>
    /// <param name="screenW">スクリーン幅</param>
    /// <param name="screenH">スクリーン高さ</param>
    /// <param name="scale">UIスケール値</param>
    void DrawTaskUI(int screenW, int screenH, float scale);

    /// <summary>
    /// サイドメッセージポップアップを描画する
    /// </summary>
    /// <param name="screenW">スクリーン幅</param>
    /// <param name="screenH">スクリーン高さ</param>
    /// <param name="scale">UIスケール値</param>
    void DrawMessagesUI(int screenW, int screenH, float scale);

    /// <summary>
    /// チュートリアル項目（アイコン・テキスト・チェックマーク）を1件描画する
    /// </summary>
    /// <param name="screenW">スクリーン幅</param>
    /// <param name="screenH">スクリーン高さ</param>
    /// <param name="scale">UIスケール値</param>
    /// <param name="text">説明テキスト</param>
    /// <param name="icons">表示するキーアイコン画像のリスト</param>
    /// <param name="isDone">項目が完了済みかどうか</param>
    /// <param name="isCheckAnim">チェックマークアニメーション再生中かどうか</param>
    /// <param name="checkAnimTime">チェックマークアニメーションの経過時間（秒）</param>
    void DrawTutorialItem(int screenW, int screenH, float scale, const char* text,
                          const std::vector<ManagedGraph*>& icons, bool isDone,
                          bool isCheckAnim, float checkAnimTime);

    /// <summary>
    /// UIスケールに合わせてフォントを再生成する
    /// </summary>
    /// <param name="scale">UIスケール値</param>
    void ReloadFonts(float scale);

private:
    TutorialManager* m_pManager; // チュートリアル状態を取得する TutorialManager のポインタ

    // UI スライドアニメーション状態
    float                      m_uiXOffset;   // タスクUIの現在X方向オフセット（0=画面内、正=画面外右）
    TutorialManager::UIState   m_uiState;     // タスクUIの表示状態（Entering/OnScreen/Exiting/Hidden）
    TutorialManager::Step      m_currentStep; // 現在描画中のチュートリアルステップ

    // 各チュートリアル項目のチェックマークアニメーションタイマー
    float m_moveCheckAnimTime; // 移動チュートリアルのチェックアニメーション経過時間
    float m_viewCheckAnimTime; // 視点チュートリアルのチェックアニメーション経過時間
    float m_jumpCheckAnimTime; // ジャンプチュートリアルのチェックアニメーション経過時間
    float m_runCheckAnimTime;  // 走りチュートリアルのチェックアニメーション経過時間

    // チェックマークアニメーション再生フラグ
    bool m_isPlayingMoveCheckAnim; // 移動チュートリアルのアニメーション再生中フラグ
    bool m_isPlayingViewCheckAnim; // 視点チュートリアルのアニメーション再生中フラグ
    bool m_isPlayingJumpCheckAnim; // ジャンプチュートリアルのアニメーション再生中フラグ
    bool m_isPlayingRunCheckAnim;  // 走りチュートリアルのアニメーション再生中フラグ

    // UI 画像リソース
    ManagedGraph m_checkMarkHandle;   // チェックマーク画像
    ManagedGraph m_wKeyHandle;        // W キー画像
    ManagedGraph m_aKeyHandle;        // A キー画像
    ManagedGraph m_sKeyHandle;        // S キー画像
    ManagedGraph m_dKeyHandle;        // D キー画像
    ManagedGraph m_mouseMoveHorHandle;// マウス横移動画像
    ManagedGraph m_spaceKeyHandle;    // スペースキー画像
    ManagedGraph m_leftShiftKeyHandle;// 左シフトキー画像
    ManagedGraph m_crossHandle;       // クロスアイコン画像

    // フォントリソース
    ManagedFont m_japaneseFontHandle;       // 通常サイズ日本語フォント
    ManagedFont m_japaneseLargeFontHandle;  // 大サイズ日本語フォント（メッセージタイトル用）
    ManagedFont m_messageDetailFontHandle;  // メッセージ詳細テキスト用フォント

    float m_prevScale; // スケール変更検知用の前フレームUIスケール値
};
