#pragma once
#include "UIBase.h"
#include "TutorialManager.h"
#include "ManagedFont.h"
#include "ManagedGraph.h"
#include <vector>
#include <memory>

/// <summary>
/// チュートリアルの表示を担当するUIクラス
/// </summary>
class TutorialUI : public UIBase
{
public:
    TutorialUI(TutorialManager* pManager);
    virtual ~TutorialUI();

    void Init() override;
    void Update(float deltaTime) override;
    void Draw() override;

private:
    /// <summary>
    /// 現在のタスク（移動、視点操作など）の描画
    /// </summary>
    void DrawTaskUI(int screenW, int screenH, float scale);

    /// <summary>
    /// サイドメッセージ（ポップアップ）の描画
    /// </summary>
    void DrawMessagesUI(int screenW, int screenH, float scale);

    /// <summary>
    /// 各チュートリアル項目の共通描画処理
    /// </summary>
    void DrawTutorialItem(int screenW, int screenH, float scale, const char* text, 
                          const std::vector<ManagedGraph*>& icons, bool isDone, 
                          bool isCheckAnim, float checkAnimTime);

    void ReloadFonts(float scale);

private:
    TutorialManager* m_pManager;

    // UIアニメーション状態
    float m_uiXOffset;
    TutorialManager::UIState m_uiState;
    TutorialManager::Step m_currentStep;

    // 各項目のアニメーションタイマー
    float m_moveCheckAnimTime;
    float m_viewCheckAnimTime;
    float m_jumpCheckAnimTime;
    float m_runCheckAnimTime;

    bool m_isPlayingMoveCheckAnim;
    bool m_isPlayingViewCheckAnim;
    bool m_isPlayingJumpCheckAnim;
    bool m_isPlayingRunCheckAnim;

    // リソース関連
    ManagedGraph m_checkMarkHandle;
    ManagedGraph m_wKeyHandle;
    ManagedGraph m_aKeyHandle;
    ManagedGraph m_sKeyHandle;
    ManagedGraph m_dKeyHandle;
    ManagedGraph m_mouseMoveHorHandle;
    ManagedGraph m_spaceKeyHandle;
    ManagedGraph m_leftShiftKeyHandle;
    ManagedGraph m_crossHandle;

    ManagedFont m_japaneseFontHandle;
    ManagedFont m_japaneseLargeFontHandle;
    ManagedFont m_messageDetailFontHandle;

    float m_prevScale;
};
