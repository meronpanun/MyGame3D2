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
    int m_backgroundHandle; // 背景画像のハンドル
    float m_scrollX;        // 背景のスクロールX座標
    float m_scrollY;        // 背景のスクロールY座標
    int m_bgmHandle;       // ゲームオーバーBGMのハンドル
    int m_returnSEHandle;  // 戻るボタンSEのハンドル
    bool m_bgmStarted;     // BGM再生済みフラグ
    int m_wave;
    int m_killCount;
    int m_score;
};

