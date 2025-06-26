#include "DxLib.h"
#include "CapsuleCollider.h"
#include "SphereCollider.h"
#include <algorithm>
#include <cmath>
#include <float.h>

// �w���p�[�֐�: �ł��߂��_�i����-�_�j
// ����pq�Ɠ_c�̊Ԃ̍ŒZ�������v�Z���A����pq��̍ŒZ�_���Ԃ�
VECTOR ClosestPtPointSegment(const VECTOR& c, const VECTOR& p, const VECTOR& q)
{
    VECTOR ab = VSub(q, p);
    float t = VDot(VSub(c, p), ab) / VDot(ab, ab);
    return VAdd(p, VScale(ab, std::clamp(t, 0.0f, 1.0f)));
}

// Ray�Ƌ��̂̌�������̃w���p�[�֐� (�C����)
bool IntersectsRaySphere(const VECTOR& rayStart, const VECTOR& rayEnd, const VECTOR& sphereCenter, float sphereRadius, VECTOR& out_hitPos, float& out_hitDistSq)
{
    VECTOR rayDir = VSub(rayEnd, rayStart);
    float rayLengthSq = VDot(rayDir, rayDir);

    if (rayLengthSq < 0.0001f)
    {
        float distSq = VDot(VSub(sphereCenter, rayStart), VSub(sphereCenter, rayStart));
        if (distSq <= sphereRadius * sphereRadius)
        {
            out_hitPos = rayStart;
            out_hitDistSq = 0.0f;
            return true;
        }
        return false;
    }

    VECTOR oc = VSub(rayStart, sphereCenter);
    float a = rayLengthSq;
    float b = 2.0f * VDot(oc, rayDir);
    float c = VDot(oc, oc) - sphereRadius * sphereRadius;
    float discriminant = b * b - 4 * a * c;

    if (discriminant < 0)
    {
        return false;
    }
    else
    {
        float t = (-b - std::sqrt(discriminant)) / (2.0f * a);
        float t1 = (-b + std::sqrt(discriminant)) / (2.0f * a);

        // Ray�͈̔� (0����1) ���ŁA�ł��߂���_��T��
        if (t < 0.0f || t > 1.0f)
        {
            t = t1;
            if (t < 0.0f || t > 1.0f)
            {
                return false;
            }
        }

        out_hitPos = VAdd(rayStart, VScale(rayDir, t));
        out_hitDistSq = VDot(VSub(out_hitPos, rayStart), VSub(out_hitPos, rayStart));
        return true;
    }
}


CapsuleCollider::CapsuleCollider(const VECTOR& segmentA, const VECTOR& segmentB, float radius)
    : m_segmentA(segmentA), m_segmentB(segmentB), m_radius(radius)
{
}

CapsuleCollider::~CapsuleCollider()
{
}

void CapsuleCollider::Init()
{
}

void CapsuleCollider::Update()
{
}

bool CapsuleCollider::Intersects(const Collider* other) const
{
    // ... (SphereCollider�Ƃ̔��胍�W�b�N�͕ύX�Ȃ�) ...
    if (!other) return false;

    // CapsuleCollider���m�̔���
    const CapsuleCollider* capsule = dynamic_cast<const CapsuleCollider*>(other);
    if (capsule)
    {
        // 2�̐����Ԃ̍ŒZ���������߂郍�W�b�N
        // ref: Real-Time Collision Detection by Christer Ericson, Chapter 5.1.8 Closest Point on Line Segments
        VECTOR d1 = VSub(m_segmentB, m_segmentA);
        VECTOR d2 = VSub(capsule->m_segmentB, capsule->m_segmentA);
        VECTOR r = VSub(m_segmentA, capsule->m_segmentA);

        float a = VDot(d1, d1); // squared length of segment 1
        float e = VDot(d2, d2); // squared length of segment 2
        float f = VDot(d2, r);  // dot product of segment 2 with vector from segment 2 start to segment 1 start

        float s = 0.0f, t = 0.0f;
        float c = VDot(d1, r);
        float b = VDot(d1, d2);
        float denom = a * e - b * b; // denominator of the equations

        if (denom != 0.0f)
        {
            s = std::clamp((b * f - c * e) / denom, 0.0f, 1.0f);
        }
        else // line segments are parallel
        {
            // ���ꂪ0�̏ꍇ�i���C�ƃJ�v�Z���̎������s�j�A�ŒZ�����𐳂����v�Z���邽�߂ɓ��ʂȏ������K�v
            // �����ł͊ȈՓI�ɁA�Е��̐����̒[�_�����������̐����ւ̍ŒZ�_�����߂�
            s = 0.0f;
        }

        t = (b * s + f) / e;

        if (t < 0.0f)
        {
            t = 0.0f;
            s = std::clamp((c) / a, 0.0f, 1.0f);
        }
        else if (t > 1.0f)
        {
            t = 1.0f;
            s = std::clamp((c + b) / a, 0.0f, 1.0f);
        }

        VECTOR p1 = VAdd(m_segmentA, VScale(d1, s));
        VECTOR p2 = VAdd(capsule->m_segmentA, VScale(d2, t));

        float distSq = VDot(VSub(p1, p2), VSub(p1, p2));
        float radiusSum = m_radius + capsule->m_radius;
        return distSq <= radiusSum * radiusSum;
    }

    // SphereCollider�Ƃ̔���iCapsuleCollider������SphereCollider�𔻒肷��j
    const SphereCollider* sphere = dynamic_cast<const SphereCollider*>(other);
    if (sphere)
    {
        // SphereCollider���Ŏ������Ă���̂ŁA��������Ăяo��
        return sphere->Intersects(this);
    }

    return false; // ���m�̃R���C�_�[�^�C�v
}

bool CapsuleCollider::IntersectsRay(const VECTOR& rayStart, const VECTOR& rayEnd, VECTOR& out_hitPos, float& out_hitDistSq) const
{
    VECTOR rayDir = VSub(rayEnd, rayStart);
    float rayLengthSq = VDot(rayDir, rayDir);

    if (rayLengthSq < 0.0001f) // ���C�̒�����0�ɋ߂��ꍇ�A�_�ƃJ�v�Z���̔���Ƀt�H�[���o�b�N
    {
        VECTOR closestPointOnCapsuleSegment = ClosestPtPointSegment(rayStart, m_segmentA, m_segmentB);
        float distSq = VDot(VSub(rayStart, closestPointOnCapsuleSegment), VSub(rayStart, closestPointOnCapsuleSegment));
        if (distSq <= m_radius * m_radius)
        {
            out_hitPos = rayStart;
            out_hitDistSq = 0.0f;
            return true;
        }
        return false;
    }

    float minT = FLT_MAX; // Ray�̃p�����[�^t�̍ŏ��l
    bool hit = false;
    VECTOR currentHitPos;
    float currentHitDistSq;

    // --- 1. �J�v�Z���̉~��������Ray�̌������� ---
    // �J�v�Z�������AB�Ɣ��ar�̖������~���Ƃ��Ĉ����B
    // ���̌�A����AB�͈̔͂ɃN���b�s���O����B

    VECTOR OA = VSub(rayStart, m_segmentA); // Ray�n�_����J�v�Z����A�ւ̃x�N�g��
    VECTOR AB = VSub(m_segmentB, m_segmentA); // �J�v�Z�����̃x�N�g��

    float abLenSq = VDot(AB, AB);
    if (abLenSq < 0.0001f) // �J�v�Z�������ɂȂ��Ă���ꍇ (����AB�̒������[���ɋ߂�)
    {
        return IntersectsRaySphere(rayStart, rayEnd, m_segmentA, m_radius, out_hitPos, out_hitDistSq);
    }

    VECTOR u = VNorm(AB); // �J�v�Z�����̒P�ʃx�N�g��

    // Ray���J�v�Z�����ɒ������镽�ʂɓ��e�����x�N�g�����l����
    VECTOR v = VSub(rayDir, VScale(u, VDot(rayDir, u)));
    VECTOR w = VSub(OA, VScale(u, VDot(OA, u)));

    float a = VDot(v, v);
    float b = 2.0f * VDot(v, w);
    float c = VDot(w, w) - m_radius * m_radius;

    float discriminant = b * b - 4 * a * c;

    if (discriminant >= 0)
    {
        float sqrt_discr = std::sqrt(discriminant);
        float t_cyl0 = (-b - sqrt_discr) / (2.0f * a);
        float t_cyl1 = (-b + sqrt_discr) / (2.0f * a);

        // Cylindrical part intersection
        for (float t_cyl : {t_cyl0, t_cyl1})
        {
            if (t_cyl >= 0.0f && t_cyl <= 1.0f) // Ray segment intersects the infinite cylinder
            {
                VECTOR p_intersect = VAdd(rayStart, VScale(rayDir, t_cyl));

                // Check if the intersection point lies within the finite capsule segment
                VECTOR proj_on_axis = VAdd(m_segmentA, VScale(u, VDot(VSub(p_intersect, m_segmentA), u)));

                // t_axis represents the parameter along the capsule's segment
                float t_axis = VDot(VSub(proj_on_axis, m_segmentA), AB) / abLenSq;

                if (t_axis >= 0.0f && t_axis <= 1.0f)
                {
                    currentHitPos = p_intersect;
                    currentHitDistSq = VDot(VSub(currentHitPos, rayStart), VSub(currentHitPos, rayStart));
                    if (currentHitDistSq < minT)
                    {
                        minT = currentHitDistSq;
                        out_hitPos = currentHitPos;
                        out_hitDistSq = currentHitDistSq;
                        hit = true;
                    }
                }
            }
        }
    }

    // --- 2. �J�v�Z���̗��[�̔�����Ray�̌������� ---
    // Ray��A�[�̋��̌���
    if (IntersectsRaySphere(rayStart, rayEnd, m_segmentA, m_radius, currentHitPos, currentHitDistSq))
    {
        if (currentHitDistSq < minT)
        {
            minT = currentHitDistSq;
            out_hitPos = currentHitPos;
            out_hitDistSq = currentHitDistSq;
            hit = true;
        }
    }

    // Ray��B�[�̋��̌���
    if (IntersectsRaySphere(rayStart, rayEnd, m_segmentB, m_radius, currentHitPos, currentHitDistSq))
    {
        if (currentHitDistSq < minT)
        {
            minT = currentHitDistSq;
            out_hitPos = currentHitPos;
            out_hitDistSq = currentHitDistSq;
            hit = true;
        }
    }

    return hit;
}