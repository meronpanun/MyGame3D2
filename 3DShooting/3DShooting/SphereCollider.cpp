#include "DxLib.h"
#include "SphereCollider.h"
#include "CapsuleCollider.h" 
#include <cmath> 
#include <algorithm>

SphereCollider::SphereCollider(const VECTOR& center, float radius)
    : m_center(center), m_radius(radius)
{
}

void SphereCollider::Init()
{
}

void SphereCollider::Update()
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

bool SphereCollider::IntersectsRay(const VECTOR& rayStart, const VECTOR& rayEnd, VECTOR& out_hitPos, float& out_hitDistSq) const
{
    VECTOR rayDir = VSub(rayEnd, rayStart);
    float rayLengthSq = VDot(rayDir, rayDir);

    if (rayLengthSq < 0.0001f) // ���C�̒�����0�ɋ߂��ꍇ
    {
        // �_�Ƌ��̔���Ƀt�H�[���o�b�N
        float distSq = VDot(VSub(m_center, rayStart), VSub(m_center, rayStart));
        if (distSq <= m_radius * m_radius)
        {
            out_hitPos = rayStart; // �n�_���̂��������Ȃ�n�_���q�b�g�ʒu�Ƃ���
            out_hitDistSq = 0.0f;
            return true;
        }
        return false;
    }

    VECTOR m = VSub(rayStart, m_center);
    float b = VDot(m, rayDir);
    float c = VDot(m, m) - m_radius * m_radius;

    // ���C�̎n�_�����̊O�������C�������痣��Ă����ꍇ
    if (c > 0.0f && b > 0.0f)
    {
        return false;
    }

    float discr = b * b - c;

    // ���ʎ������̏ꍇ�A�������Ȃ�
    if (discr < 0.0f)
    {
        return false;
    }

    // ��_�܂ł̋��� (�n�_����)
    float t = -b - std::sqrt(discr);

    // ��_�����C�͈̔͊O�̏ꍇ (0����1�͈̔�)
    if (t < 0.0f)
    {
        t = 0.0f; // �n�_�������ɂ���ꍇ�At�͕��ɂȂ肤��
    }

    // ���C�̒����ɐ��K��
    t /= std::sqrt(rayLengthSq);

    // ���C�̏I�_�𒴂��Ă���ꍇ
    if (t > 1.0f)
    {
        return false;
    }

    out_hitPos = VAdd(rayStart, VScale(rayDir, t));
    out_hitDistSq = VDot(VSub(out_hitPos, rayStart), VSub(out_hitPos, rayStart));
    return true;
}