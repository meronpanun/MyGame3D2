#include "DxLib.h"
#include "CapsuleCollider.h"
#include "SphereCollider.h"
#include <algorithm>
#include <cmath>
#include <float.h>

// ����pq�Ɠ_c�̊Ԃ̍ŒZ�������v�Z���A����pq��̍ŒZ�_���Ԃ�
VECTOR ClosestPtPointSegment(const VECTOR& c, const VECTOR& p, const VECTOR& q)
{
	VECTOR ab = VSub(q, p); // ����pq�̃x�N�g��
	float   t = VDot(VSub(c, p), ab) / VDot(ab, ab); // ����pq��̓_c����̓��e�p�����[�^

	return VAdd(p, VScale(ab, std::clamp(t, 0.0f, 1.0f))); // ����pq��̍ŒZ�_��Ԃ�
}

// Ray�Ƌ��̂̌�������
bool IntersectsRaySphere(const VECTOR& rayStart, const VECTOR& rayEnd, const VECTOR& sphereCenter, float sphereRadius, VECTOR& outHtPos, float& outHtDistSq)
{
	VECTOR rayDir      = VSub(rayEnd, rayStart); // Ray�̕����x�N�g��
	float  rayLengthSq = VDot(rayDir, rayDir);   // Ray�̒����̓��

	// Ray�̒�����0�ɋ߂��ꍇ�A�_�Ƌ��̂̌���������s��
	if (rayLengthSq < 0.0001f)
    {
		float distSq = VDot(VSub(sphereCenter, rayStart), VSub(sphereCenter, rayStart)); // ���S��Ray�n�_�̋����̓��

		// ���S��Ray�n�_�̋��������̔��a�ȉ��Ȃ�ARay�͋��ɐڐG���Ă���
        if (distSq <= sphereRadius * sphereRadius)
        {
			outHtPos    = rayStart; // ��_��Ray�̎n�_
			outHtDistSq = 0.0f;     // ��_�܂ł̋����̓���0
            return true;
        }
        return false;
    }
	VECTOR oc = VSub(rayStart, sphereCenter); // Ray�n�_���狅�S�ւ̃x�N�g��

	float a = rayLengthSq;                                // Ray�̒����̓��
	float b = 2.0f * VDot(oc, rayDir);                    // Ray�n�_���狅�S�ւ̃x�N�g����Ray�̕����x�N�g���̓��ς�2�{
	float c = VDot(oc, oc) - sphereRadius * sphereRadius; // ���S��Ray�n�_�̋����̓�悩�狅�̔��a�̓����������l
	float discriminant = b * b - 4 * a * c;               // ���ʎ�

	// ���ʎ������̏ꍇ�ARay�Ƌ��͌������Ȃ�
    if (discriminant < 0)
    {
        return false;
    }
    else
    {
		float t  = (-b - std::sqrt(discriminant)) / (2.0f * a); // Ray�Ƌ��̌�_�܂ł̃p�����[�^t
		float t1 = (-b + std::sqrt(discriminant)) / (2.0f * a); // ������̌�_�܂ł̃p�����[�^t

        // Ray�͈̔�(0����1)���ŁA�ł��߂���_��T��
        if (t < 0.0f || t > 1.0f)
        {
            t = t1;

			// ����t1���͈͊O�Ȃ�A�������Ȃ�
            if (t < 0.0f || t > 1.0f)
            {
                return false;
            }
        }

		outHtPos    = VAdd(rayStart, VScale(rayDir, t)); // ��_�̈ʒu���v�Z
		outHtDistSq = VDot(VSub(outHtPos, rayStart), VSub(outHtPos, rayStart)); // ��_�܂ł̋����̓����v�Z

        return true;
    }
}


CapsuleCollider::CapsuleCollider(const VECTOR& segmentA, const VECTOR& segmentB, float radius) : 
    m_segmentA(segmentA), 
    m_segmentB(segmentB), 
    m_radius(radius)
{
}

CapsuleCollider::~CapsuleCollider()
{
}

// CapsuleCollider���m�̌�������
bool CapsuleCollider::Intersects(const Collider* other) const
{
	// ���̃R���C�_�[��nullptr�̏ꍇ�͌������Ȃ�
	if (!other) return false; 

    // CapsuleCollider���m�̔���
    const CapsuleCollider* capsule = dynamic_cast<const CapsuleCollider*>(other);
    if (capsule)
    {
        // 2�̐����Ԃ̍ŒZ���������߂郍�W�b�N
		VECTOR d1 = VSub(m_segmentB, m_segmentA);
		VECTOR d2 = VSub(capsule->m_segmentB, capsule->m_segmentA);
		VECTOR r  = VSub(m_segmentA, capsule->m_segmentA); 

		// d1��d2�̒������v�Z
        float a = VDot(d1, d1);
        float e = VDot(d2, d2);
        float f = VDot(d2, r); 

		// d1��d2�̓��ς��v�Z
        float s = 0.0f, t = 0.0f;
        float c = VDot(d1, r);
        float b = VDot(d1, d2);
        float denom = a * e - b * b;

		// ���ꂪ0�łȂ��ꍇ�As���v�Z
        if (denom != 0.0f)
        {
            s = std::clamp((b * f - c * e) / denom, 0.0f, 1.0f);
        }
        else 
        {
            s = 0.0f;
        }
		// ���ꂪ0�łȂ��ꍇ�At���v�Z
        t = (b * s + f) / e;

		// t���͈͊O�̏ꍇ�As�𒲐�
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

		// �ŒZ�_���v�Z
        VECTOR p1 = VAdd(m_segmentA, VScale(d1, s));
        VECTOR p2 = VAdd(capsule->m_segmentA, VScale(d2, t));

		// �ŒZ�_�Ԃ̋������v�Z
        float distSq    = VDot(VSub(p1, p2), VSub(p1, p2));
        float radiusSum = m_radius + capsule->m_radius;

		// ���a�̘a�̓��ƍŒZ�_�Ԃ̋����̓����r
        return distSq <= radiusSum * radiusSum;
    }

    // SphereCollider�Ƃ̔���(CapsuleCollider������SphereCollider�𔻒肷��)
    const SphereCollider* sphere = dynamic_cast<const SphereCollider*>(other);
    if (sphere)
    {
        // SphereCollider���Ŏ������Ă���̂ŁA��������Ăяo��
        return sphere->Intersects(this);
    }

    return false; // ���m�̃R���C�_�[�^�C�v
}

// Ray�ƃJ�v�Z���̌�������
bool CapsuleCollider::IntersectsRay(const VECTOR& rayStart, const VECTOR& rayEnd, VECTOR& outHitPos, float& outHitDistSq) const
{
	// Ray�̎n�_�ƏI�_���x�N�g���Ƃ��Ď擾
    VECTOR rayDir      = VSub(rayEnd, rayStart); 
    float  rayLengthSq = VDot(rayDir, rayDir);

    // Ray�̒�����0�ɋ߂��ꍇ�A�_�ƃJ�v�Z���̌���������s��
    if (rayLengthSq < 0.0001f)
    {
        VECTOR closestPointOnCapsuleSegment = ClosestPtPointSegment(rayStart, m_segmentA, m_segmentB);
        float distSq = VDot(VSub(rayStart, closestPointOnCapsuleSegment), VSub(rayStart, closestPointOnCapsuleSegment));

		// Ray�n�_�ƃJ�v�Z���̐�����̍ŒZ�_�̋������J�v�Z���̔��a�ȉ��Ȃ�ARay�̓J�v�Z���ɐڐG���Ă���
        if (distSq <= m_radius * m_radius)
        {
			outHitPos    = rayStart; // ��_��Ray�̎n�_
			outHitDistSq = 0.0f;     // ��_�܂ł̋����̓���0
            return true;
        }
        return false;
    }

    float minT = FLT_MAX; // Ray�̃p�����[�^t�̍ŏ��l
    bool hit = false;
    VECTOR currentHitPos;
    float currentHitDistSq;

    // �J�v�Z���̉~��������Ray�̌�������
    // �J�v�Z�������AB�Ɣ��ar�̖������~���Ƃ��Ĉ���
    VECTOR OA = VSub(rayStart, m_segmentA);   // Ray�n�_����J�v�Z����A�ւ̃x�N�g��
    VECTOR AB = VSub(m_segmentB, m_segmentA); // �J�v�Z�����̃x�N�g��

    float abLenSq = VDot(AB, AB);

    // �J�v�Z�������ɂȂ��Ă���ꍇ(����AB�̒������[���ɋ߂�)
    if (abLenSq < 0.0001f) 
    {
        return IntersectsRaySphere(rayStart, rayEnd, m_segmentA, m_radius, outHitPos, outHitDistSq);
    }

    VECTOR u = VNorm(AB); // �J�v�Z�����̒P�ʃx�N�g��

    // Ray���J�v�Z�����ɒ������镽�ʂɓ��e�����x�N�g�����l����
    VECTOR v = VSub(rayDir, VScale(u, VDot(rayDir, u)));
    VECTOR w = VSub(OA, VScale(u, VDot(OA, u)));

	// �~���̕������Ɋ�Â��āARay�Ɖ~���̌���������s��
    float a = VDot(v, v);
    float b = 2.0f * VDot(v, w);
    float c = VDot(w, w) - m_radius * m_radius;
    float discriminant = b * b - 4 * a * c;

	// ���ʎ���0�ȏ�̏ꍇ�ARay�Ɖ~���͌�������
    if (discriminant >= 0)
    {
		// �����_��t���v�Z
        float sqrtDiscr = std::sqrt(discriminant);
        float tCyl0 = (-b - sqrtDiscr) / (2.0f * a);
        float tCyl1 = (-b + sqrtDiscr) / (2.0f * a);

		// �����_��t��Ray�͈̔�(0����1)���ɂ��邩�m�F
        for (float tCyl : {tCyl0, tCyl1})
        {
            if (tCyl >= 0.0f && tCyl <= 1.0f) 
            {
				// �����_�̈ʒu���v�Z
                VECTOR pIntersect  = VAdd(rayStart, VScale(rayDir, tCyl)); 
                VECTOR projOnAxis  = VAdd(m_segmentA, VScale(u, VDot(VSub(pIntersect, m_segmentA), u)));

				float t_axis = VDot(VSub(projOnAxis, m_segmentA), AB) / abLenSq; // ����AB��̌����_�̃p�����[�^t

				// �����_������AB��ɂ��邩�m�F
                if (t_axis >= 0.0f && t_axis <= 1.0f)
                {
                    currentHitPos    = pIntersect;
                    currentHitDistSq = VDot(VSub(currentHitPos, rayStart), VSub(currentHitPos, rayStart));

                    // �����_�̋������ŏ��l��菬�����ꍇ
					if (currentHitDistSq < minT) 
                    {
						// �ŏ��̌����������X�V
                        minT = currentHitDistSq;

						// �����_�̈ʒu�Ƌ������o��
                        outHitPos    = currentHitPos; 
                        outHitDistSq = currentHitDistSq;
                        hit = true;
                    }
                }
            }
        }
    }

    // �J�v�Z���̗��[�̔�����Ray�̌�������
    // Ray��A�[�̋��̌���
    if (IntersectsRaySphere(rayStart, rayEnd, m_segmentA, m_radius, currentHitPos, currentHitDistSq))
    {
		// �����_�̋������ŏ��l��菬�����ꍇ�A�X�V
        if (currentHitDistSq < minT)
        {
			// �ŏ��̌����������X�V
            minT = currentHitDistSq;

			// �����_�̈ʒu�Ƌ������o��
            outHitPos    = currentHitPos;
            outHitDistSq = currentHitDistSq;
            hit = true;
        }
    }

    // Ray��B�[�̋��̌���
    if (IntersectsRaySphere(rayStart, rayEnd, m_segmentB, m_radius, currentHitPos, currentHitDistSq))
    {
		// �����_�̋������ŏ��l��菬�����ꍇ�A�X�V
        if (currentHitDistSq < minT)
        {
			// �ŏ��̌����������X�V
            minT = currentHitDistSq;

			// �����_�̈ʒu�Ƌ������o��
            outHitPos    = currentHitPos;
            outHitDistSq = currentHitDistSq;
            hit = true;
        }
    }

    return hit; // �ŏI�I�ȏՓˌ��ʂ�Ԃ�
}