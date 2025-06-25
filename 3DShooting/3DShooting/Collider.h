#pragma once

/// <summary>
/// 当たり判定の基底クラス
/// </summary>
class Collider abstract
{
public:
	Collider() = default;
	virtual ~Collider() = default;

	virtual void Init()   abstract;
	virtual void Update() abstract;

	// 純粋仮想関数として当たり判定メソッドを追加
	virtual bool Intersects(const Collider* other) const abstract;

	// Rayとの交差判定メソッドを追加
	// out_hitPos: 交差した場合のヒット座標
	// out_hitDistSq: 交差した場合の、Ray始点からの距離の2乗
	virtual bool IntersectsRay(const VECTOR& rayStart, const VECTOR& rayEnd, VECTOR& out_hitPos, float& out_hitDistSq) const abstract;


protected:
};

