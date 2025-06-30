#pragma once
#include "Collider.h"

/// <summary>
/// �J�v�Z���^�R���C�_�[�N���X
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
	/// <param name="outHtPos">���������ʒu</param>
	/// <param name="outHtDistSq">���������ʒu�܂ł̋����̓��</param>
    /// <returns></returns>
    bool IntersectsRay(const VECTOR& rayStart, const VECTOR& rayEnd, VECTOR& outHitPos, float& outHitDistSq) const override;

    /// <summary>
	/// �J�v�Z���̃Z�O�����gA���擾����
    /// </summary>
	/// <returns>�Z�O�����gA���a</returns>
    VECTOR GetSegmentA() const { return m_segmentA; }

    /// <summary>
	/// �J�v�Z���̃Z�O�����gB���擾����
    /// </summary>
	/// <returns>�Z�O�����gB���a</returns>
    VECTOR GetSegmentB() const { return m_segmentB; }

    /// <summary>
	/// �J�v�Z���̔��a���擾����
    /// </summary>
	/// <returns>�J�v�Z���̔��a</returns>
    float GetRadius() const { return m_radius; }

    /// <summary>
	/// �J�v�Z���̃Z�O�����gA��B��ݒ肷��
    /// </summary>
	/// <param name="a">�Z�O�����gA</param>
	/// <param name="b">�Z�O�����gB</param>
    void SetSegment(const VECTOR& a, const VECTOR& b) { m_segmentA = a; m_segmentB = b; }

    /// <summary>
	/// �J�v�Z���̔��a��ݒ肷��
    /// </summary>
	/// <param name="radius">�J�v�Z���̔��a</param>
    void SetRadius(float radius) { m_radius = radius; }

private:
	VECTOR m_segmentA; // �J�v�Z���̃Z�O�����gA
	VECTOR m_segmentB; // �J�v�Z���̃Z�O�����gB

	float  m_radius; // �J�v�Z���̔��a
};