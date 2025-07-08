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

	/// <summary>
	/// タイトルロゴをスキップする
	/// </summary>
	void SkipLogo();

private:
	int m_logoHandle;	   // タイトルロゴのハンドル
	int m_fadeAlpha;	   // フェードのアルファ値
	int m_fadeFrame;	   // フェードのフレームカウント
	int m_sceneFadeAlpha;  // シーンフェードのアルファ値
	int m_waitFrame;	   // 待機フレーム

	bool m_isFadeComplete; // フェード完了フラグ
	bool m_isFadeOut;	   // フェードアウトフラグ
	bool m_skipLogo;       // ロゴスキップ用のフラグ
	bool m_isSceneFadeIn;  // シーンフェードインフラグ
};

