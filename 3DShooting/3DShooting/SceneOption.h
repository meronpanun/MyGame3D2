#pragma once
#include "SceneBase.h"

/// <summary>
/// �I�v�V�����V�[���N���X
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

