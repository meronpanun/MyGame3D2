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
    int m_bgmHandle;       // ゲームオーバーBGMのハンドル
    bool m_bgmStarted;     // BGM再生済みフラグ
    int m_wave;
    int m_killCount;
    int m_score;
};

