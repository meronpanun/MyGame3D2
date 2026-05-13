#pragma once
#include "EffekseerWarningSuppress.h"
#include <vector>
#include "Stage.h"

class Player;

/// <summary>
/// アイテム基底クラス
/// </summary>
class ItemBase
{
public:
	ItemBase() = default;
	virtual ~ItemBase() = default;

	virtual void Init()   = 0;
	virtual void Update(Player* player, const std::vector<Stage::StageCollisionData>& collisionData) = 0;
	virtual void Draw()   = 0;

	virtual bool IsUsed() const = 0;
	virtual bool IsExpired() const { return false; }

	// 位置設定用
	// 位置設定用
	virtual void SetPos(const VECTOR& pos) = 0;

	// デバッグ表示用
	static void SetDrawCollision(bool isDraw) { s_shouldDrawCollision = isDraw; }
	static bool ShouldDrawCollision() { return s_shouldDrawCollision; }

protected:
	virtual void DrawCollisionDebug() {}
	static bool s_shouldDrawCollision;
};
