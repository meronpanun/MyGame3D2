#include "DxLib.h"
#pragma once

/// <summary>
/// 当たり判定関数
/// </summary>
class Collider abstract
{
public:
	Collider() = default;
	virtual ~Collider() = default;

	// 当たり判定関数
	virtual bool Intersects(const Collider* other) const abstract;

	// Rayとの当たり判定関数
	// out_hitPos: 当たった場合のヒット位置
	// out_hitDistSq: 当たった場合の、Ray始点からの距離の2乗
	virtual bool IntersectsRay(const VECTOR& rayStart, const VECTOR& rayEnd, VECTOR& out_hitPos, float& out_hitDistSq) const abstract;

protected:
};

