#pragma once

#include "DxLib.h"

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

	// 位置設定用の純粋仮想関数を追加
	virtual void SetPos(const VECTOR& pos) abstract;
};