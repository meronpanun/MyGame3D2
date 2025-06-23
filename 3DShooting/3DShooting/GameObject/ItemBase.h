#pragma once

/// <summary>
/// アイテム基底クラス
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