#pragma once
#include "SceneBase.h"
#include "ManagedFont.h"
#include "ManagedGraph.h"
#include "ManagedSound.h"
#include <vector>

/// <summary>
/// ゲームクリアリザルトシーンクラス
/// </summary>
class SceneResult : public SceneBase
{
public:
    /// <summary>
    /// コンストラクタ（ScoreManagerからスコア情報を取得してランクを計算する）
    /// </summary>
    SceneResult();

    virtual ~SceneResult() = default;

    /// <summary>
    /// 初期化処理（マウス表示・スコア保存・BGM再生など）
    /// </summary>
    void Init() override;

    /// <summary>
    /// 毎フレームの更新処理
    /// </summary>
    /// <returns>遷移先シーンのポインタ（遷移しない場合はnullptrを返す）</returns>
    SceneBase* Update() override;

    /// <summary>
    /// 描画処理
    /// </summary>
    void Draw() override;

    /// <summary>
    /// UIスケールに合わせてフォントを再生成する
    /// </summary>
    /// <param name="scale">UIスケール値</param>
    void ReloadFonts(float scale);

private:
    /// <summary>
    /// 画面サイズに合わせてレイアウトを計算する
    /// </summary>
    void UpdateLayout();

    /// <summary>
    /// グラデーションボックスを描画する
    /// </summary>
    void DrawGradientBox(int x1, int y1, int x2, int y2, unsigned int topColor, unsigned int bottomColor);

private:
    // リソース管理
    ManagedGraph m_background;          // 背景画像のハンドル
    ManagedGraph m_gameClearImage;      // ゲームクリア画像のハンドル

    ManagedFont m_japaneseFont;         // 日本語フォントハンドル
    ManagedFont m_arialBlackFont;       // Arial Blackフォントハンドル
    ManagedFont m_arialBlackLargeFont;  // Arial Blackラージフォントハンドル
    ManagedFont m_japaneseLargeFont;    // 日本語ラージフォントハンドル
    ManagedFont m_japaneseButtonFont;   // 日本語ボタンフォントハンドル

    // ゲーム結果情報
    int  m_score;        // 合計スコア
    int  m_killCount;    // ボディキル数
    int  m_headShotCount;// ヘッドショット数
    int  m_maxCombo;     // 最大コンボ数
    char m_rank;         // 評価ランク（S/A/B/C）

    // 演出アニメーション
    int   m_frame;          // 経過フレーム数
    int   m_scoreAnim;      // スコアカウントアップアニメーション値
    int   m_killAnim;       // キル数カウントアップアニメーション値
    int   m_headAnim;       // ヘッドショット数カウントアップアニメーション値
    int   m_rankAnimAlpha;  // ランク表示のアルファ値
    int   m_buttonAnimAlpha;// ボタン表示のアルファ値
    float m_rankScale;      // ランク表示のスケール値

    // スケール変更検知
    float m_prevScale; // 前フレームのUIスケール値

    /// <summary>
    /// 画面解像度に応じたレイアウト座標を保持する構造体
    /// </summary>
    struct ResultLayout
    {
        // ゲームクリア画像
        int imageDrawWidth, imageDrawHeight;
        int imageDrawX, imageDrawY;

        // リザルト背景
        int resBgW, resBgH;
        int resBgX, resBgY;

        // テキスト領域
        int textLabelX, textValueX;
        int textBaseY, textIntervalHigh;
        int highScoreY;

        // ボタン
        int btnW, btnH;
        int titleBtnX1, titleBtnY1, titleBtnX2, titleBtnY2;
        int retryBtnX1, retryBtnY1, retryBtnX2, retryBtnY2;
    };

    ResultLayout m_layout; // 現在の画面解像度に対応したレイアウト情報

    // 背景スクロール管理
    float m_scrollX; // 背景のスクロールX座標
    float m_scrollY; // 背景のスクロールY座標
};
