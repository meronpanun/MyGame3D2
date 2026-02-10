#pragma once
#include "SceneBase.h"

/// <summary>
/// クリアリザルトシーンクラス
/// </summary>
class SceneResult : public SceneBase
{
public:
    SceneResult();
    virtual ~SceneResult();

    void Init() override;
    SceneBase* Update() override;
    void Draw() override;

private:
    /// <summary>
    /// 画面サイズに合わせてレイアウトを計算する
    /// </summary>
    void UpdateLayout();

    /// <summary>
    /// グラデーション矩形描画
    /// </summary>
    void DrawGradientBox(int x1, int y1, int x2, int y2, unsigned int topColor, unsigned int bottomColor);

private:
    // リソース管理
    int m_backgroundHandle;          // 背景画像のハンドル
    int m_gameClearImageHandle;      // ゲームクリア画像のハンドル
    int m_returnSEHandle;            // 戻るボタンSEのハンドル
    int m_bgmHandle;                 // ゲームクリアBGMのハンドル

    int m_japaneseFontHandle;        // 日本語フォントハンドル
    int m_arialBlackFontHandle;      // Arial Blackフォントハンドル
    int m_arialBlackLargeFontHandle; // Arial Blackラージフォントハンドル
    int m_japaneseLargeFontHandle;   // 日本語ラージフォントハンドル
    int m_japaneseButtonFontHandle;  // 日本語ボタンフォントハンドル

    // 背景スクロール管理
    float m_scrollX;                 // 背景のスクロールX座標
    float m_scrollY;                 // 背景のスクロールY座標

    // BGM管理
    bool m_isBGMStarted;             // BGM再生済みフラグ

    // スケール管理
    float m_prevScale = 1.0f;        // 前回のスケール

    /// <summary>
    /// フォントを再読み込みする
    /// </summary>
    /// <param name="scale">UIスケール</param>
    void ReloadFonts(float scale);

    struct ResultLayout
    {
        // 画像
        int imageDrawX, imageDrawY, imageDrawWidth, imageDrawHeight;

        // リザルト背景
        int resBgX, resBgY, resBgW, resBgH;

        // テキスト
        int textLabelX, textValueX, textBaseY, textIntervalHigh;

        // ハイスコア位置
        int highScoreY;

        // ボタン
        int titleBtnX1, titleBtnY1, titleBtnX2, titleBtnY2;
        int retryBtnX1, retryBtnY1, retryBtnX2, retryBtnY2;
        int btnW, btnH;
    };

    ResultLayout m_layout;
};

