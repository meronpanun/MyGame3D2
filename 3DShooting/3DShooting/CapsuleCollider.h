#pragma once
#include "Collider.h"

/// <summary>
/// カプセル型のコライダー
/// </summary>
class CapsuleCollider : public Collider
{
public:
	CapsuleCollider(const VECTOR& segmentA = VGet(0, 0, 0), const VECTOR& segmentB = VGet(0, 1, 0), float radius = 1.0f);
	virtual ~CapsuleCollider();

	void Init() override;
	void Update() override;

	bool Intersects(const Collider* other) const override;

	VECTOR GetSegmentA() const { return m_segmentA; }
	VECTOR GetSegmentB() const { return m_segmentB; }
	float GetRadius() const { return m_radius; }

	void SetSegment(const VECTOR& a, const VECTOR& b) { m_segmentA = a; m_segmentB = b; }
	void SetRadius(float radius) { m_radius = radius; }

private:
	VECTOR m_segmentA;
	VECTOR m_segmentB;
	float  m_radius;
};
