#include "EnemyNormal.h"
#include "Player.h"
#include "Bullet.h"
#include "DxLib.h"
#include <cassert>
#include <algorithm>

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
}

EnemyNormal::EnemyNormal() :
    m_aabbMin{ kAABBMin },
    m_aabbMax{ kAABBMax },
    m_headPos{ kHeadShotPosition },
    m_headRadius(kHeadRadius)
{
    // ���f���̓ǂݍ���
    m_modelHandle = MV1LoadModel("data/image/NormalZombie.mv1");
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
    m_hp              = kInitialHP;
    m_hitDisplayTimer = 0;
    m_lastHitPart     = HitPart::None;
    m_pos             = kInitialPosition;
}

void EnemyNormal::Update(const std::vector<Bullet>& bullets)
{
    // ���f���̈ʒu���X�V
    MV1SetPosition(m_modelHandle, m_pos);

	// �e�̓����蔻����`�F�b�N
	CheckHitAndDamage(const_cast<std::vector<Bullet>&>(bullets));

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
	unsigned int color = 0xff0000;

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
    unsigned int headColor =0x00ff00;
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

// �G���e�ɓ����������ǂ������`�F�b�N���A�_���[�W���󂯂鏈��
void EnemyNormal::CheckHitAndDamage(std::vector<Bullet>& bullets) 
{
    for (auto& bullet : bullets) 
    {
		if (!bullet.IsActive()) continue; // �e����A�N�e�B�u�Ȃ�X�L�b�v

        // �ǂ��ɓ������������`�F�b�N
		HitPart part = CheckHitPart(bullet); 

        // �w�b�h�V���b�g����
		if (part == HitPart::Head) 
        {
            // �w�b�h�V���b�g�̓_���[�W2�{
            TakeDamage(bullet.GetDamage() * 2.0f); 

			m_lastHitPart     = HitPart::Head;       // �Ō�ɓ����������ʂ��w�b�h�V���b�g�ɐݒ�
			m_hitDisplayTimer = kHitDisplayDuration; // �q�b�g�\���^�C�}�[�����Z�b�g

			bullet.Deactivate(); // �e���A�N�e�B�u��
            break;
        }
        // �{�f�B�q�b�g����
		else if (part == HitPart::Body)
        {
            // �{�f�B�q�b�g�͒ʏ�_���[�W
			TakeDamage(bullet.GetDamage()); 

			m_lastHitPart     = HitPart::Body;        // �Ō�ɓ����������ʂ��{�f�B�q�b�g�ɐݒ�
			m_hitDisplayTimer = kHitDisplayDuration;  // �q�b�g�\���^�C�}�[�����Z�b�g

			bullet.Deactivate(); // �e���A�N�e�B�u��
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

