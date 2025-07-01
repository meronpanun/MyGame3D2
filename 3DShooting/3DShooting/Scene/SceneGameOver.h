#pragma once
#include "SceneBase.h"

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

