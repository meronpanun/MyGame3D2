#include "EnemyNormal.h"
#include "Player.h"
#include "Bullet.h"
#include "DxLib.h"
#include <cassert>

namespace
{
	// �q�b�g�\���̎�������
	constexpr int kHitDisplayDuration = 60; // 1�b�ԕ\��

    // �G�̈ʒu
    constexpr VECTOR kInitialPosition = { 0.0f, -30.0f, 0.0f };

	// AABB�̍ŏ����W�ƍő���W
	constexpr VECTOR kAABBMin = { -20.0f, 0.0f, -15.0f };
	constexpr VECTOR kAABBMax = { 20.0f, 128.0f, 15.0f };

	// �G�̃w�b�h�V���b�g����p���S���W
	constexpr VECTOR kHeadShotPosition = { 0.0f, 160.0f, -6.0f };

    // �w�b�h�V���b�g�̔��蔼�a
    constexpr float kHeadRadius = 12.5f;

	// �G�̏����̗�
	constexpr float kInitialHP = 200.0f;
}

EnemyNormal::EnemyNormal():
    m_colRadius(1.0f),
	m_pos{ kInitialPosition },
    m_aabbMin{ kAABBMin },
    m_aabbMax{ kAABBMax },
	m_headPos{ kHeadShotPosition },
	m_headRadius(kHeadRadius),
	m_hp(kInitialHP)
{
}


void EnemyNormal::Init()
{
    // ���f���̓ǂݍ���
    m_modelHandle = MV1LoadModel("data/image/NormalZombie.mv1");
    assert(m_modelHandle != -1);
}

void EnemyNormal::Update(const std::vector<Bullet>& bullets)
{
    // ���f���̈ʒu���X�V
    MV1SetPosition(m_modelHandle, m_pos);

    // �����蔻��
    CheckHitAndDamage(bullets);


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
    }
}

// AABB�Ƌ��̓����蔻��
static bool CheckCapsuleSphereHit(
    const VECTOR& capA, const VECTOR& capB, float capRadius,
    const VECTOR& sphereCenter, float sphereRadius)
{
    // ����capA-capB��̍ŋߓ_�����߂�
    VECTOR ab = { capB.x - capA.x, capB.y - capA.y, capB.z - capA.z };
    VECTOR ac = { sphereCenter.x - capA.x, sphereCenter.y - capA.y, sphereCenter.z - capA.z };

	float abLenSq = ab.x * ab.x + ab.y * ab.y + ab.z * ab.z; // �����̒����̓��
	float t = 0.0f; // �ŋߓ_�̐�����̈ʒu

    // �����̒�����0�łȂ��ꍇ
	if (abLenSq > 0.0f)  
    {
		t = (ac.x * ab.x + ac.y * ab.y + ac.z * ab.z) / abLenSq; // ������̈ʒu���v�Z
		t = (std::max)(0.0f, (std::min)(1.0f, t));               // t��0����1�͈̔͂ɐ���
    }

	// �ŋߓ_�̍��W���v�Z
    VECTOR closest = { 
        capA.x + ab.x * t,
        capA.y + ab.y * t,
        capA.z + ab.z * t
    };

	// �ŋߓ_�Ƌ��̒��S�̋������v�Z
	float dx = sphereCenter.x - closest.x;
	float dy = sphereCenter.y - closest.y;
	float dz = sphereCenter.z - closest.z;
    float distSq = dx * dx + dy * dy + dz * dz;
    float radiusSum = capRadius + sphereRadius;

    // �������Ă��邩�ǂ�����Ԃ�
	return distSq <= radiusSum * radiusSum; 
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
    unsigned int color = GetColor(255, 0, 0);

    // �J�v�Z���̒��S����AABB�̏㉺���S��
    VECTOR centerMin = { (boxMin.x + boxMax.x) * 0.5f, boxMin.y, (boxMin.z + boxMax.z) * 0.5f };
    VECTOR centerMax = { (boxMin.x + boxMax.x) * 0.5f, boxMax.y, (boxMin.z + boxMax.z) * 0.5f };

    // ���a��X,Z������AABB�T�C�Y�̂����傫�����̔���
    float radius = (std::max)(
        std::abs(boxMax.x - boxMin.x),
        std::abs(boxMax.z - boxMin.z)
        ) * 0.5f;

    DrawCapsule3D(centerMin, centerMax, radius, 16, color, color, false);

    // �w�b�h�V���b�g����f�o�b�O�`��
    VECTOR headCenter = {
        m_pos.x + m_headPos.x,
        m_pos.y + m_headPos.y,
        m_pos.z + m_headPos.z
    };
    unsigned int headColor = GetColor(0, 255, 0); // �ΐF�ŕ`��
    DrawSphere3D(headCenter, m_headRadius, 16, headColor, headColor, false);
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
    float dx = bulletPos.x - headCenter.x;
    float dy = bulletPos.y - headCenter.y;
    float dz = bulletPos.z - headCenter.z;
    float distSq = dx * dx + dy * dy + dz * dz;
    float radiusSum = m_headRadius + bullet.GetRadius();
    if (distSq <= radiusSum * radiusSum)
    {
        return HitPart::Head;
    }

    // �̔���
    if (IsHit(bullet)) 
    {
        return HitPart::Body;
    }
    return HitPart::None;
}

// �G���e�ɓ����������ǂ������`�F�b�N���A�_���[�W���󂯂鏈��
void EnemyNormal::CheckHitAndDamage(const std::vector<Bullet>& bullets)
{
    // �q�b�g���̓��Z�b�g���Ȃ�
    for (auto& bullet : bullets)
    {
        if (!bullet.IsActive()) continue;
        HitPart part = CheckHitPart(bullet);
        if (part == HitPart::Head)
        {
            TakeDamage(50.0f);
            m_lastHitPart = HitPart::Head;
            m_hitDisplayTimer = kHitDisplayDuration;
            const_cast<Bullet&>(bullet).Deactivate();
            break;
        }
        else if (part == HitPart::Body)
        {
            TakeDamage(10.0f);
            m_lastHitPart = HitPart::Body;
            m_hitDisplayTimer = kHitDisplayDuration;
            const_cast<Bullet&>(bullet).Deactivate();
            break;
        }
    }
}

// �G���_���[�W���󂯂鏈��
void EnemyNormal::TakeDamage(float damage)
{
    m_hp -= damage;
    if (m_hp < 0) m_hp = 0;
}

