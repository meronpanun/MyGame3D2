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
    int m_backgroundHandle; // 背景画像のハンドル
    float m_scrollX;        // 背景のスクロールX座標
    float m_scrollY;        // 背景のスクロールY座標
    int m_bgmHandle;       // ゲームクリアBGMのハンドル
    int m_returnSEHandle;  // 戻るボタンSEのハンドル
    bool m_isBGMStarted;     // BGM再生済みフラグ
};

