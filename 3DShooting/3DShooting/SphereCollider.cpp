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

    // ���C�̒��������ɒZ���A�܂��̓[���̏ꍇ
    if (rayLengthSq < 0.0001f)
    {
        // ���C�̎n�_�Ƌ��̒��S�̋������v�Z
        float distSq = VDot(VSub(m_center, rayStart), VSub(m_center, rayStart));
        // �n�_���������ɂ���΃q�b�g
        if (distSq <= m_radius * m_radius)
        {
            out_hitPos = rayStart;
            out_hitDistSq = 0.0f;
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

        // ���C�����̓�������n�܂�A�O���֌������ꍇ���l��
        // t�����ł����Ă��A�n�_�����̓����ɂ���At*t*rayLengthSq�����a�̓���菬������΁A
        // ���̂܂܌v�Z��i�߂Ă����Ȃ��B
        // �������ARay�̐����Ƃ��Ă̏Փ˂𐳊m�Ɉ����ɂ�t��[0,1]�ɃN�����v����K�v������B
        // �����ł́At��[0,1]�͈̔͂Ɍ��肵�A���͈͓̔��̍ŏ��̃q�b�g��������

        out_hitPos = VAdd(rayStart, VScale(rayDir, t));
        out_hitDistSq = VDot(VSub(out_hitPos, rayStart), VSub(out_hitPos, rayStart));
        return true;
    }
}