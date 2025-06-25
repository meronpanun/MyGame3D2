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

// Ray�Ƌ��̂̌�������̃w���p�[�֐� (SphereCollider::IntersectsRay���痬�p�E����)
// SphereCollider��Ray���肪�������@�\���Ă���O��
bool IntersectsRaySphere(const VECTOR& rayStart, const VECTOR& rayEnd, const VECTOR& sphereCenter, float sphereRadius, VECTOR& out_hitPos, float& out_hitDistSq)
{
    VECTOR rayDir = VSub(rayEnd, rayStart);
    float rayLengthSq = VDot(rayDir, rayDir);

    if (rayLengthSq < 0.0001f) // ���C�̒�����0�ɋ߂��ꍇ
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

    VECTOR m = VSub(rayStart, sphereCenter);
    float b = VDot(m, rayDir);
    float c = VDot(m, m) - sphereRadius * sphereRadius;

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

    if (t < 0.0f) // �n�_�������ɂ���ꍇ
    {
        t = 0.0f;
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
    // �J�v�Z���̎�AB��Ray�̕����x�N�g��D�ŁARay���J�v�Z���̒��S���ɑ΂��Đ����ȕ��ʂɓ��e���A�~�Ɛ��̌�������ɋA��������
    // �܂��́A�ȉ��̂悤�ȃA���S���Y����p����:
    // ref: Real-Time Collision Detection by Christer Ericson, Chapter 5.3.3 Ray vs. Capsule
    VECTOR A = m_segmentA;
    VECTOR B = m_segmentB;
    float r = m_radius;

    VECTOR OA = VSub(rayStart, A); // Ray�n�_����J�v�Z����A�ւ̃x�N�g��
    VECTOR AB = VSub(B, A);        // �J�v�Z�����̃x�N�g��

    float dot_dir_AB = VDot(rayDir, AB);
    float dot_OA_AB = VDot(OA, AB);
    float dot_AB_AB = VDot(AB, AB); // �J�v�Z�����̒�����2��

    VECTOR Q = VSub(VSub(VScale(rayDir, dot_AB_AB), VScale(AB, dot_dir_AB)), VSub(VScale(OA, dot_AB_AB), VScale(AB, dot_OA_AB)));
    float a = VDot(Q, Q);
    float b = 2.0f * VDot(Q, VSub(VScale(OA, dot_AB_AB), VScale(AB, dot_OA_AB)));
    float c = VDot(VSub(VScale(OA, dot_AB_AB), VScale(AB, dot_OA_AB)), VSub(VScale(OA, dot_AB_AB), VScale(AB, dot_OA_AB))) - r * r * dot_AB_AB * dot_AB_AB;

    float discr = b * b - 4.0f * a * c;

    if (discr >= 0.0f)
    {
        float sqrt_discr = std::sqrt(discr);
        float t0 = (-b - sqrt_discr) / (2.0f * a);
        float t1 = (-b + sqrt_discr) / (2.0f * a);

        if (t0 <= 1.0f && t1 >= 0.0f) // Ray���J�v�Z���ƌ�������͈͂ɂ���
        {
            float t_intersect =(std::max)(0.0f, t0); // ���C�̎n�_����O�̌����͖���

            if (t_intersect <= 1.0f)
            {
                // �����������C��̓_P
                VECTOR P = VAdd(rayStart, VScale(rayDir, t_intersect));

                // P���J�v�Z����AB�̊ԂɈʒu���邩�`�F�b�N
                // P���玲AB�ւ̍ŒZ�_S�����߂�
                VECTOR S = ClosestPtPointSegment(P, A, B);

                // P��S�̋��������ar�ȉ��ł���ΗL���ȏՓ�
                if (VDot(VSub(P, S), VSub(P, S)) <= r * r + 0.0001f) // �덷���e
                {
                    currentHitPos = P;
                    currentHitDistSq = VDot(VSub(P, rayStart), VSub(P, rayStart));
                    if (currentHitDistSq < minT) // ���߂��q�b�g��D��
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
    if (IntersectsRaySphere(rayStart, rayEnd, A, r, currentHitPos, currentHitDistSq))
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
    if (IntersectsRaySphere(rayStart, rayEnd, B, r, currentHitPos, currentHitDistSq))
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