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
};

