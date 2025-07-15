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
};

