#pragma once
#include "Collider.h"

/// <summary>
/// ���̃R���C�_�[�N���X
/// </summary>
class SphereCollider : public Collider
{
public:
    SphereCollider(const VECTOR& center = VGet(0, 0, 0), float radius = 1.0f);
    virtual ~SphereCollider() = default;

    /// <summary>
	/// �����蔻����s��
    /// </summary>
	/// <param name="other">���̃R���C�_�[</param>
	/// <returns>true: �������Ă���, false: �������Ă��Ȃ�</returns>
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
	/// ���̒��S�Ɣ��a���擾����
    /// </summary>
	/// <returns>���̒��S�Ɣ��a</returns>
    VECTOR GetCenter() const { return m_center; }

    /// <summary>
	/// ���̔��a���擾����
    /// </summary>
	/// <returns>���̔��a</returns>
    float GetRadius() const { return m_radius; }

    /// <summary>
	/// ���̒��S�Ɣ��a��ݒ肷��
    /// </summary>
	/// <param name="center">���̒��S</param>
    void SetCenter(const VECTOR& center) { m_center = center; }

    /// <summary>
	/// ���̔��a��ݒ肷��  
    /// </summary>
	/// <param name="radius">���̔��a</param>
    void SetRadius(float radius) { m_radius = radius; }

private:
	VECTOR m_center; // ���̒��S

	float  m_radius; // ���̔��a
};