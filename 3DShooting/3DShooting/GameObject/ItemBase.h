#pragma once

/// <summary>
/// �A�C�e�����N���X
/// </summary>
class ItemBase abstract
{
public:
	ItemBase() = default;
	virtual ~ItemBase() = default;

	virtual void Init()   abstract;
	virtual void Update() abstract;
	virtual void Draw()   abstract;
};