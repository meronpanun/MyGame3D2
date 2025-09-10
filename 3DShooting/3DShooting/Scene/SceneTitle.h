#pragma once
#include "SceneBase.h"

/// <summary>
/// タイトルシーンクラス
/// </summary>
class SceneTitle : public SceneBase
{
public:
	SceneTitle(bool skipLogo = false);
	virtual ~SceneTitle();

	void Init() override;
	SceneBase* Update() override;
	void Draw() override;

private:
    // リソース管理
    int m_fontHandle;      // フォントハンドル
    int m_titleLogo;       // タイトルロゴのハンドル
    int m_bannerHandle;    // バナー画像のハンドル
    int m_bgmHandle;       // タイトルBGMのハンドル
    int m_confirmSEHandle; // 決定ボタンSEのハンドル

    // フェード・シーン遷移管理
    int  m_fadeAlpha;       // フェードのアルファ値
    int  m_fadeFrame;       // フェードのフレームカウント
    int  m_sceneFadeAlpha;  // シーンフェードのアルファ値
    bool m_isFadeComplete;  // フェード完了フラグ
    bool m_isFadeOut;       // フェードアウトフラグ
    bool m_isSceneFadeIn;   // シーンフェードインフラグ

	// 演出管理
    int  m_waitFrame;       // 待機フレーム
    bool m_isBGMStarted;    // BGM再生済みフラグ

    // ゲームスタートテキスト演出関連
    int m_gameStartTextAlpha;     // ゲームスタートテキストのアルファ値
    int m_gameStartTextAlphaDir;  // ゲームスタートテキストのアルファ値の増減方向
};

