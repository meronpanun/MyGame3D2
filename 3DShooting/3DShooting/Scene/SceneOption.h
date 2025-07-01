#pragma once
#include "SceneBase.h"

/// <summary>
/// オプションシーンクラス
/// </summary>
class SceneOption : public SceneBase
{
public:
	SceneOption(SceneBase* pScene);
	virtual ~SceneOption();

	void Init() override;
	SceneBase* Update() override;
	void Draw() override;
};

