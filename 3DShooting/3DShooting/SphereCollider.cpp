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

bool SphereCollider::Intersects(const Collider* other) const
{
    if (!other) return false;

    // SphereCollider���m�̔���
    const SphereCollider* sphere = dynamic_cast<const SphereCollider*>(other);
    if (sphere)
    {
        float dx = m_center.x - sphere->m_center.x;
        float dy = m_center.y - sphere->m_center.y;
        float dz = m_center.z - sphere->m_center.z;
        float distSq = dx * dx + dy * dy + dz * dz;
        float radiusSum = m_radius + sphere->m_radius;
        return distSq <= radiusSum * radiusSum;
    }

    // CapsuleCollider�Ƃ̔���
    const CapsuleCollider* capsule = dynamic_cast<const CapsuleCollider*>(other);
    if (capsule)
    {
        // ���ƃJ�v�Z���̓����蔻�胍�W�b�N
        VECTOR capA = capsule->GetSegmentA();
        VECTOR capB = capsule->GetSegmentB();
        float capRadius = capsule->GetRadius();

        VECTOR ab = VSub(capB, capA);
        VECTOR ac = VSub(m_center, capA);

        float abLenSq = VDot(ab, ab);
        float t = 0.0f;

        if (abLenSq > 0.0f)
        {
            t = VDot(ac, ab) / abLenSq;
            t = (std::max)(0.0f, (std::min)(1.0f, t));
        }

        VECTOR closest = VAdd(capA, VScale(ab, t));

        float distSq = VDot(VSub(m_center, closest), VSub(m_center, closest));
        float radiusSum = m_radius + capRadius;

        return distSq <= radiusSum * radiusSum;
    }

    return false; // ���m�̃R���C�_�[�^�C�v
}

bool SphereCollider::IntersectsRay(const VECTOR& rayStart, const VECTOR& rayEnd, VECTOR& outHtPos, float& outHtDistSq) const
{
    VECTOR rayDir = VSub(rayEnd, rayStart);
    float rayLengthSq = VDot(rayDir, rayDir);

    // ���C�̒��������ɒZ���A�܂��̓[���̏ꍇ
    if (rayLengthSq < 0.0001f)
    {
        // ���C�̎n�_�Ƌ��̒��S�̋������v�Z
        float distSq = VDot(VSub(m_center, rayStart), VSub(m_center, rayStart));
        // �n�_���������ɂ���΃q�b�g
        if (distSq <= m_radius * m_radius)
        {
            outHtPos = rayStart;
            outHtDistSq = 0.0f;
            return true;
        }
        return false;
    }

    VECTOR oc = VSub(rayStart, m_center);
    float a = rayLengthSq; // VDot(rayDir, rayDir)
    float b = 2.0f * VDot(oc, rayDir);
    float c = VDot(oc, oc) - m_radius * m_radius;
    float discriminant = b * b - 4 * a * c;

    if (discriminant < 0)
    {
        return false; // �������Ȃ�
    }
    else
    {
        float t = (-b - std::sqrt(discriminant)) / (2.0f * a);
        float t1 = (-b + std::sqrt(discriminant)) / (2.0f * a); // ��ڂ̌�_

        // Ray�͈̔� (0����1) ���ŁA�ł��߂���_��T��
        if (t < 0.0f || t > 1.0f) // �ŏ��̌�_���͈͊O
        {
            t = t1; // ��ڂ̌�_������
            if (t < 0.0f || t > 1.0f) // ��ڂ̌�_���͈͊O
            {
                return false; // �ǂ���̌�_��Ray�͈̔͊O
            }
        }

        outHtPos = VAdd(rayStart, VScale(rayDir, t));
        outHtDistSq = VDot(VSub(outHtPos, rayStart), VSub(outHtPos, rayStart));
        return true;
    }
}