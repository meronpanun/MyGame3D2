#include "Player.h"
#include "EnemyNormal.h"
#include "Bullet.h"
#include "DxLib.h"
#include <cassert>
#include <algorithm>
#include <cmath>

namespace
{
    // �U���A�j���[�V����
	constexpr char kAttackAnimName[] = "ATK";

    // �ʒu
    constexpr VECTOR kInitialPosition = { 0.0f, -30.0f, 0.0f };

	// AABB�̍ŏ����W�ƍő���W
	constexpr VECTOR kAABBMin = { -20.0f, 0.0f, -15.0f };
	constexpr VECTOR kAABBMax = { 20.0f, 128.0f, 15.0f };

	// �w�b�h�V���b�g����p���S���W
	constexpr VECTOR kHeadShotPosition = { 0.0f, 160.0f, -6.0f };

    // �w�b�h�V���b�g�̔��蔼�a
    constexpr float kHeadRadius = 13.5f;

	// �����̗�
	constexpr float kInitialHP = 200.0f;

    // VECTOR�̒����̓����v�Z����֐�
    float VLenSq(const VECTOR& vec)
    {
        return vec.x * vec.x + vec.y * vec.y + vec.z * vec.z;
    }

    // AABB�Ƌ��̓����蔻��(�w���p�[�֐��Ƃ���namespace���Ɏc��)
    static bool CheckCapsuleSphereHit(
        const VECTOR& capA, const VECTOR& capB, float capRadius,
        const VECTOR& sphereCenter, float sphereRadius)
    {
        // ����capA-capB��̍ŋߓ_�����߂�
        VECTOR ab = VSub(capB, capA);
        VECTOR ac = VSub(sphereCenter, capA);

        float abLenSq = VDot(ab, ab); // �����̒����̓��
        float t       = 0.0f; // �ŋߓ_�̐�����̈ʒu

        // �����̒�����0�łȂ��ꍇ
        if (abLenSq > 0.0f)
        {
            t = VDot(ac, ab) / abLenSq; // ������̈ʒu���v�Z
            t = (std::max)(0.0f, (std::min)(1.0f, t)); // t��0����1�͈̔͂ɐ���
        }

        // �ŋߓ_�̍��W���v�Z
        VECTOR closest = VAdd(capA, VScale(ab, t));

        // �ŋߓ_�Ƌ��̒��S�̋������v�Z
        float distSq = VLenSq(VSub(sphereCenter, closest));
        float radiusSum = capRadius + sphereRadius;

        // �������Ă��邩�ǂ�����Ԃ�
        return distSq <= radiusSum * radiusSum;
    }

    // 2�̃J�v�Z���̓����蔻��
    static bool CheckCapsuleCapsuleHit(
        const VECTOR& a1, const VECTOR& a2, float r1,
        const VECTOR& b1, const VECTOR& b2, float r2)
    {
        VECTOR d1 = VSub(a2, a1);
        VECTOR d2 = VSub(b2, b1);
        VECTOR r = VSub(a1, b1);
        float a = VDot(d1, d1);
        float e = VDot(d2, d2);
        float f = VDot(d2, r);

        float s = 0.0f, t = 0.0f;
        float c = VDot(d1, r);
        float b = VDot(d1, d2);
        float denom = a * e - b * b;

        if (denom != 0.0f)
            s = std::clamp((b * f - c * e) / denom, 0.0f, 1.0f);
        else
            s = 0.0f;

        t = (b * s + f) / e;
        t = std::clamp(t, 0.0f, 1.0f);

        s = (b * t - c) / a;
        s = std::clamp(s, 0.0f, 1.0f);

        VECTOR p1 = VAdd(a1, VScale(d1, s));
        VECTOR p2 = VAdd(b1, VScale(d2, t));
        float distSq = VDot(VSub(p1, p2), VSub(p1, p2));
        float radiusSum = r1 + r2;
        return distSq <= radiusSum * radiusSum;
    }

}

EnemyNormal::EnemyNormal() :
    m_aabbMin{ kAABBMin },
    m_aabbMax{ kAABBMax },
    m_headPos{ kHeadShotPosition },
    m_headRadius(kHeadRadius),
	m_isTackleHit(false),
    m_lastTackleId(-1)
{
    // ���f���̓ǂݍ���
    m_modelHandle = MV1LoadModel("data/model/NormalZombie.mv1");
    assert(m_modelHandle != -1);
}

EnemyNormal::~EnemyNormal()
{
    // ���f���̉��
	DeleteGraph(m_modelHandle); 
}


void EnemyNormal::Init()
{
    // ������
    m_hp                = kInitialHP;
    m_pos               = kInitialPosition;
	m_attackRange       = 200.0f; // �U���͈�
	m_attackPower       = 20.0f;  // �U����
	m_attackCooldownMax = 45;     // �U���N�[���_�E���̍ő�l
}

void EnemyNormal::Update(std::vector<Bullet>& bullets, const Player::TackleInfo& tackleInfo)
{
    // ���f���̈ʒu���X�V
    MV1SetPosition(m_modelHandle, m_pos);

	// �e�̓����蔻����`�F�b�N
	CheckHitAndDamage(const_cast<std::vector<Bullet>&>(bullets));

    // �^�b�N�������蔻��̒ǉ�
    if (tackleInfo.isTackling && m_hp > 0.0f && tackleInfo.tackleId != m_lastTackleId)
    {
        // �G�̃J�v�Z�����
        VECTOR boxMin = {
            m_pos.x + m_aabbMin.x,
            m_pos.y + m_aabbMin.y,
            m_pos.z + m_aabbMin.z
        };
        VECTOR boxMax = {
            m_pos.x + m_aabbMax.x,
            m_pos.y + m_aabbMax.y,
            m_pos.z + m_aabbMax.z
        };
        VECTOR capA = { (boxMin.x + boxMax.x) * 0.5f, boxMin.y, (boxMin.z + boxMax.z) * 0.5f };
        VECTOR capB = { (boxMin.x + boxMax.x) * 0.5f, boxMax.y, (boxMin.z + boxMax.z) * 0.5f };
        float capRadius = (std::max)(std::abs(boxMax.x - boxMin.x), std::abs(boxMax.z - boxMin.z)) * 0.5f;

        // �J�v�Z�����m�̓����蔻��
        if (CheckCapsuleCapsuleHit(
            tackleInfo.capA, tackleInfo.capB, tackleInfo.radius,
            capA, capB, capRadius))
        {
            TakeTackleDamage(tackleInfo.damage);
            m_lastTackleId = tackleInfo.tackleId; // ���̃^�b�N��ID�Ń_���[�W���󂯂����Ƃ��L�^
        }
    }

    // �f�o�b�O�\���^�C�}�[����
    if (m_hitDisplayTimer > 0) 
    {
        --m_hitDisplayTimer; 
        if (m_hitDisplayTimer == 0) 
        {
            m_lastHitPart = HitPart::None;
        }
    }
}

void EnemyNormal::Draw()
{
    // HP��0���傫���ꍇ�̂ݕ`��
    if (m_hp > 0.0f)
    {
        // ���f���̕`��
        MV1DrawModel(m_modelHandle);

#ifdef _DEBUG
        // �f�o�b�O�p�̓����蔻��`��
        DrawCollisionDebug();

        // �f�o�b�O�\��
        const char* hitMsg = "";

		// �q�b�g���ʂɉ����ă��b�Z�[�W��ݒ�
        switch (m_lastHitPart)
        {
        case HitPart::Head: hitMsg = "HeadShot!"; break;
        case HitPart::Body: hitMsg = "BodyHit!"; break;
        default: break;
        }
        if (*hitMsg)
        {
            DrawFormatString(20, 100, 0xff0000, "%s", hitMsg);
        }

        // �̗͂̃f�o�b�O�\��
        DrawFormatString(20, 80, 0x000000, "Enemy HP: %.1f", m_hp);
#endif
    }
}

// �G�̓����蔻����s���֐�
bool EnemyNormal::IsHit(const Bullet& bullet) const
{
    // �J�v�Z���̒��S��
    VECTOR boxMin = {
        m_pos.x + m_aabbMin.x,
        m_pos.y + m_aabbMin.y,
        m_pos.z + m_aabbMin.z
    };
    VECTOR boxMax = {
        m_pos.x + m_aabbMax.x,
        m_pos.y + m_aabbMax.y,
        m_pos.z + m_aabbMax.z
    };

	// �J�v�Z���̏㉺���S���v�Z
    VECTOR capA = { (boxMin.x + boxMax.x) * 0.5f, boxMin.y, (boxMin.z + boxMax.z) * 0.5f };
    VECTOR capB = { (boxMin.x + boxMax.x) * 0.5f, boxMax.y, (boxMin.z + boxMax.z) * 0.5f };

    float capRadius = (std::max)( 
        std::abs(boxMax.x - boxMin.x),
        std::abs(boxMax.z - boxMin.z)
        ) * 0.5f;

    // �J�v�Z���Ƌ��̓����蔻��
    return CheckCapsuleSphereHit(capA, capB, capRadius, bullet.GetPos(), bullet.GetRadius());
}

// �f�o�b�O�p�̓����蔻��`��
void EnemyNormal::DrawCollisionDebug() const
{
    VECTOR boxMin = {
        m_pos.x + m_aabbMin.x,
        m_pos.y + m_aabbMin.y,
        m_pos.z + m_aabbMin.z
    };
    VECTOR boxMax = {
        m_pos.x + m_aabbMax.x,
        m_pos.y + m_aabbMax.y,
        m_pos.z + m_aabbMax.z
    };

    // �J�v�Z���̒��S����AABB�̏㉺���S��
    VECTOR centerMin = { (boxMin.x + boxMax.x) * 0.5f, boxMin.y, (boxMin.z + boxMax.z) * 0.5f };
    VECTOR centerMax = { (boxMin.x + boxMax.x) * 0.5f, boxMax.y, (boxMin.z + boxMax.z) * 0.5f };

    // ���a��X,Z������AABB�T�C�Y�̂����傫�����̔���
    float radius = (std::max)(
        std::abs(boxMax.x - boxMin.x),
        std::abs(boxMax.z - boxMin.z)
        ) * 0.5f;

    DrawCapsule3D(centerMin, centerMax, radius, 16, 0xff0000, 0xff0000, false);

    // �w�b�h�V���b�g����f�o�b�O�`��
    VECTOR headCenter = {
        m_pos.x + m_headPos.x,
        m_pos.y + m_headPos.y,
        m_pos.z + m_headPos.z
    };
    DrawSphere3D(headCenter, m_headRadius, 16, 0x00ff00, 0x00ff00, false);

    // �U���͈͂̃f�o�b�O�\��
    DrawSphere3D(m_pos, m_attackRange, 16, 0xff8000, 0xff8000, false);
}

// �ǂ��ɓ������������肷��֐�
EnemyBase::HitPart EnemyNormal::CheckHitPart(const Bullet& bullet) const 
{
    // �w�b�h�V���b�g����
    VECTOR headCenter = {
        m_pos.x + m_headPos.x,
        m_pos.y + m_headPos.y,
        m_pos.z + m_headPos.z
    };
    VECTOR bulletPos = bullet.GetPos();

	// �w�b�h�V���b�g����̂��߂̋����v�Z
	float dx        = bulletPos.x - headCenter.x;        // X���W�̍�
	float dy        = bulletPos.y - headCenter.y;        // Y���W�̍�
	float dz        = bulletPos.z - headCenter.z;        // Z���W�̍�
	float distSq    = dx * dx + dy * dy + dz * dz;       // �����̓����v�Z
	float radiusSum = m_headRadius + bullet.GetRadius(); // �w�b�h�V���b�g���a�ƒe�̔��a�̘a���v�Z

	// �w�b�h�V���b�g����
    if (distSq <= radiusSum * radiusSum)
    {
        return HitPart::Head;
    }

    // �{�f�B�q�b�g����
    if (IsHit(bullet)) 
    {
        return HitPart::Body;
    }
    return HitPart::None;
}

// �_���[�W�v�Z
float EnemyNormal::CalcDamage(const Bullet& bullet, HitPart part) const
{
    if (part == HitPart::Head)
    {
        return bullet.GetDamage() * 2.0f; // �w�b�h�V���b�g�̓_���[�W2�{
    }
    else if (part == HitPart::Body)
    {
        return bullet.GetDamage();
    }
    return 0.0f;
}

void EnemyNormal::UpdateAttack()
{
}

void EnemyNormal::AttackPlayer(Player* player)
{
}


