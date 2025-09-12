#pragma once
#include "EffekseerForDXLib.h"

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

	virtual bool IsUsed() const abstract;

	// 位置設定用
	virtual void SetPos(const VECTOR& pos) abstract;
};