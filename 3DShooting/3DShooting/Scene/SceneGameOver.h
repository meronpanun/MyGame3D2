#pragma once
#include "SceneBase.h"

/// <summary>
/// ゲームオーバーリザルトシーンクラス
/// </summary>
class SceneGameOver : public SceneBase
{
public:
	SceneGameOver();
	virtual ~SceneGameOver();

	void Init() override;
	SceneBase* Update() override;
	void Draw() override;

private:
};

