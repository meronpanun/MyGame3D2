#pragma once

/// <summary>
/// シーン基底クラス
/// </summary>
class SceneBase abstract
{
public:
	SceneBase() = default;
	virtual ~SceneBase() = default;

	virtual void Init() abstract;
	virtual SceneBase* Update() abstract;
	virtual void Draw() abstract;

protected:
};

