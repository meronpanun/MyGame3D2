#include "DxLib.h"
#include "SphereCollider.h"
#include "CapsuleCollider.h" 
#include <cmath> 
#include <algorithm>

SphereCollider::SphereCollider(const VECTOR& center, float radius) :
    m_center(center), 
    m_radius(radius)
{
}

// �����蔻����s��
bool SphereCollider::Intersects(const Collider* other) const
{
	// ���̃R���C�_�[��nullptr�̏ꍇ�͌������Ȃ�
    if (!other) return false;

    // SphereCollider���m�̔���
    const SphereCollider* sphere = dynamic_cast<const SphereCollider*>(other);
    if (sphere)
    {
		// �����m�̓����蔻��
		float dx = m_center.x - sphere->m_center.x; // X���W�̍�
		float dy = m_center.y - sphere->m_center.y; // Y���W�̍�
		float dz = m_center.z - sphere->m_center.z; // Z���W�̍�
		float distSq = dx * dx + dy * dy + dz * dz; // ���S�Ԃ̋����̓����v�Z
		float radiusSum = m_radius + sphere->m_radius; // ���a�̘a���v�Z

        // ���̒��S�Ԃ̋����̓�悪���a�̘a�̓��ȉ��Ȃ瓖�����Ă���
		return distSq <= radiusSum * radiusSum; 
    }

    // CapsuleCollider�Ƃ̔���
    const CapsuleCollider* capsule = dynamic_cast<const CapsuleCollider*>(other);
    if (capsule)
    {
        // ���ƃJ�v�Z���̓����蔻��
        VECTOR capA = capsule->GetSegmentA();
        VECTOR capB = capsule->GetSegmentB();
        float capRadius = capsule->GetRadius();

		// �J�v�Z���̒��S�����̃x�N�g�����v�Z
        VECTOR ab = VSub(capB, capA);
        VECTOR ac = VSub(m_center, capA);

		float abLenSq = VDot(ab, ab); // ����AB�̒����̓��
		float t = 0.0f; // ����AB��̓_�ւ̃p�����[�^t

		// ����AB�̒������[���ɋ߂��ꍇ�A���̒��S���J�v�Z���̒[�_�ɋ߂����ǂ����𔻒�
        if (abLenSq > 0.0f)
        {
			t = VDot(ac, ab) / abLenSq; // ���̒��S�������AB�ւ̓��e�p�����[�^
			t = (std::max)(0.0f, (std::min)(1.0f, t)); // t��0����1�͈̔͂ɐ���
        }

		VECTOR closest = VAdd(capA, VScale(ab, t)); // ����AB��̍ł��߂��_���v�Z

		// ���̒��S�Ɛ���AB��̍ł��߂��_�Ƃ̋������v�Z
        float distSq = VDot(VSub(m_center, closest), VSub(m_center, closest));
        float radiusSum = m_radius + capRadius;

        // ���a�̘a�̓��ȉ��Ȃ瓖�����Ă���
		return distSq <= radiusSum * radiusSum; 
    }

    return false; // ���m�̃R���C�_�[�^�C�v
}

// Ray�Ƃ̓����蔻����s��
bool SphereCollider::IntersectsRay(const VECTOR& rayStart, const VECTOR& rayEnd, VECTOR& outHtPos, float& outHtDistSq) const
{
	// Ray�̎n�_�ƏI�_���x�N�g���Ƃ��Ď擾
    VECTOR rayDir = VSub(rayEnd, rayStart);
    float rayLengthSq = VDot(rayDir, rayDir);

    // Ray�̒��������ɒZ���A�܂��̓[���̏ꍇ
    if (rayLengthSq < 0.0001f)
    {
        // Ray�̎n�_�Ƌ��̒��S�̋������v�Z
        float distSq = VDot(VSub(m_center, rayStart), VSub(m_center, rayStart));
        // �n�_���������ɂ���΃q�b�g
        if (distSq <= m_radius * m_radius)
        {
            outHtPos    = rayStart;
            outHtDistSq = 0.0f;
            return true;
        }
        return false;
    }

	// Ray�̕����𐳋K��
    VECTOR oc = VSub(rayStart, m_center);
    float a = rayLengthSq;
    float b = 2.0f * VDot(oc, rayDir);
    float c = VDot(oc, oc) - m_radius * m_radius;
    float discriminant = b * b - 4 * a * c;

	// ���ʎ������̏ꍇ�ARay�Ƌ��͌������Ȃ�
    if (discriminant < 0)
    {
        return false; // �������Ȃ�
    }
    else
    {
		float t  = (-b - std::sqrt(discriminant)) / (2.0f * a); // �ŏ��̌�_�܂ł̃p�����[�^t
		float t1 = (-b + std::sqrt(discriminant)) / (2.0f * a); // ������̌�_�܂ł̃p�����[�^t

        // Ray�͈̔�(0����1)���ŁA�ł��߂���_��T��
        if (t < 0.0f || t > 1.0f) // �ŏ��̌�_���͈͊O
        {
            t = t1; // ��ڌ�_������

            // ��ڂ̌�_���͈͊O
            if (t < 0.0f || t > 1.0f) 
            {
                return false; // �ǂ���̌�_��Ray�͈̔͊O
            }
        }

        outHtPos    = VAdd(rayStart, VScale(rayDir, t));
        outHtDistSq = VDot(VSub(outHtPos, rayStart), VSub(outHtPos, rayStart));
        return true; 
    } 
}