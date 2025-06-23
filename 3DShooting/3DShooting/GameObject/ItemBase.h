#pragma once

class Player;

/// <summary>
/// �A�C�e�����N���X
/// </summary>
class ItemBase abstract
{
public:
	ItemBase() = default;
	virtual ~ItemBase() = default;

	virtual void Init()   abstract;
	virtual void Update(Player* player) abstract;
	virtual void Draw()   abstract;

	virtual bool IsUsed() const abstract;
};