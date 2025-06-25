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

protected:
};

