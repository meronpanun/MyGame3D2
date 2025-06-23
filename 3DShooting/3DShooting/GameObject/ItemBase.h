#pragma once

class Player;

/// <summary>
/// アイテム基底クラス
/// </summary>
class ItemBase abstract
{
public:
	ItemBase() = default;
	virtual ~ItemBase() = default;

	virtual void Init()   abstract;
	virtual void Update(Player* player) abstract;
	virtual void Draw()   abstract;
};