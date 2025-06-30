#pragma once
#include "Collider.h"

/// <summary>
/// �J�v�Z���^�̃R���C�_�[
/// </summary>
class CapsuleCollider : public Collider
{
public:
    CapsuleCollider(const VECTOR& segmentA = VGet(0, 0, 0), const VECTOR& segmentB = VGet(0, 1, 0), float radius = 1.0f);
    virtual ~CapsuleCollider();

    /// <summary>
	/// ���̃R���C�_�[�Ɠ������Ă��邩�ǂ����𔻒肷��
    /// </summary>
	/// <param name="other">���̃R���C�_�[</param>s
	/// <returns>true�Ȃ瓖�����Ă���</returns>
    bool Intersects(const Collider* other) const override; 

    /// <summary>
	/// Ray�Ƃ̓����蔻����s��
    /// </summary>
	/// <param name="rayStart">Ray�̎n�_</param>
	/// <param name="rayEnd">Ray�̏I�_</param>
    /// <param name="out_hitPos"></param>
    /// <param name="out_hitDistSq"></param>
    /// <returns></returns>
    bool IntersectsRay(const VECTOR& rayStart, const VECTOR& rayEnd, VECTOR& out_hitPos, float& out_hitDistSq) const override;

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