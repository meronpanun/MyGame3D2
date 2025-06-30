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

	/// <summary>
	/// Rayとの当たり判定を行う
	/// </summary>
	/// <param name="rayStart">Rayの始点</param>
	/// <param name="rayEnd">Rayの終点</param>
	/// <param name="out_hitPos">当たった位置</param>
	/// <param name="out_hitDistSq">当たった位置までの距離の二乗</param>
	/// <returns></returns>
	virtual bool IntersectsRay(const VECTOR& rayStart, const VECTOR& rayEnd, VECTOR& out_hitPos, float& out_hitDistSq) const abstract;
};

