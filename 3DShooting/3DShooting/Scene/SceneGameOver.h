#pragma once
#include "SceneBase.h"

/// <summary>
/// ゲームオーバーリザルトシーンクラス
/// </summary>
class SceneGameOver : public SceneBase
{
public:
    SceneGameOver(int wave, int killCount, int score);
    virtual ~SceneGameOver();

    void Init() override;
    SceneBase* Update() override;
    void Draw() override;

private:
    // リソース管理
    int m_backgroundHandle;          // 背景画像のハンドル
    int m_gameOverImageHandle;       // ゲームオーバー画像のハンドル
    int m_gameOverImageHandle2;      // ゲームオーバー画像2（乱れた画像）
    int m_gameOverImageHandle3;      // ゲームオーバー画像3（乱れた画像）
    int m_bgmHandle;                 // ゲームオーバーBGMのハンドル
    int m_returnSEHandle;            // 戻るボタンSEのハンドル

    int m_japaneseFontHandle;        // 日本語フォントハンドル
    int m_arialBlackFontHandle;      // Arial Blackフォントハンドル
    int m_arialBlackLargeFontHandle; // Arial Blackラージフォントハンドル
    int m_japaneseLargeFontHandle;   // 日本語ラージフォントハンドル
    int m_japaneseButtonFontHandle;  // 日本語ボタンフォントハンドル

    // ゲーム結果情報
    int m_wave;                      // ウェーブ
    int m_killCount;                 // キル数
    int m_score;                     // スコア

    // 背景スクロール管理
    float m_scrollX;                 // 背景のスクロールX座標
    float m_scrollY;                 // 背景のスクロールY座標

	// BGM管理
    bool m_isBGMStarted;             // BGM再生済みフラグ

    // 演出管理
    int m_currentImageIndex;         // 現在表示中の画像インデックス（0:通常, 1:乱れ2, 2:乱れ3）
    int m_imageChangeTimer;          // 画像切り替えタイマー
    int m_imageChangeInterval;       // 画像切り替え間隔（フレーム数）

    struct GameOverLayout {
        // 画像
        int imageDrawX, imageDrawY, imageDrawWidth, imageDrawHeight;

        // リザルト背景
        int resBgX, resBgY, resBgW, resBgH;

        // テキスト領域
        int textLabelX, textValueX, textBaseY;

        // ボタン
        int titleBtnX1, titleBtnY1, titleBtnX2, titleBtnY2;
        int retryBtnX1, retryBtnY1, retryBtnX2, retryBtnY2;
        int btnW, btnH;
    };

    GameOverLayout m_layout;

    /// <summary>
    /// 画面サイズに合わせてレイアウトを計算する
    /// </summary>
    void UpdateLayout();
};

