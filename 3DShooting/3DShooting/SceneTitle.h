#pragma once
#include "SceneBase.h"

/// <summary>
/// �^�C�g���V�[���N���X
/// </summary>
class SceneTitle : public SceneBase
{
public:
	SceneTitle(bool skipLogo = false);
	virtual ~SceneTitle();

	void Init() override;
	SceneBase* Update() override;
	void Draw() override;
};

