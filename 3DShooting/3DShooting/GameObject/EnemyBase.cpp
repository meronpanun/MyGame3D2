#include "Player.h"
#include "EnemyBase.h"
#include "Bullet.h"
#include "Collider.h"

namespace
{
	constexpr int   kDefaultHitDisplayDuration = 60;     // 1�b�ԕ\��
	constexpr float kDefaultInitialHP = 100.0f; // �f�t�H���g�̏����̗� 

	constexpr float kDefaultCooldownMax = 60;     // �U���N�[���_�E���̍ő�l
	constexpr float kDefaultAttackPower = 10.0f;  // �U����
}

EnemyBase::EnemyBase() :
	m_pos{ 0, 0, 0 },
	m_modelHandle(-1),
	// m_colRadius(1.0f), // �폜
	m_targetPlayer(nullptr),
	m_hp(kDefaultInitialHP),
	m_lastHitPart(HitPart::None),
	m_hitDisplayTimer(0),
	m_isAlive(true),
	m_isTackleHit(false),
	m_attackCooldown(0),
	m_attackCooldownMax(kDefaultCooldownMax),
	m_attackPower(kDefaultAttackPower),
	m_attackHitFrame(0),
	m_isAttacking(false)
{
}

void EnemyBase::CheckHitAndDamage(std::vector<Bullet>& bullets)
{
    // �ł��߂��q�b�g����ێ�
    int hitBulletIndex = -1;
    float minHitDistSq = FLT_MAX; // �ł��߂��Փ˂܂ł̋�����2��
    HitPart determinedHitPart = HitPart::None; // �ŏI�I�Ɍ��肳�ꂽ�q�b�g����

    for (int i = 0; i < bullets.size(); ++i)
    {
        auto& bullet = bullets[i];
        if (!bullet.IsActive()) continue;

        // �e��Ray�����擾 (�O�t���[���ʒu -> ���݈ʒu)
        VECTOR rayStart = bullet.GetPrevPos();
        VECTOR rayEnd = bullet.GetPos();

        // �ǂ��ɓ��������̂����`�F�b�N
        // CheckHitPart�͍ł��߂��Փ˓_���l������HitPart��Ԃ��悤�ɂ���
        // �����ł͋����̏��������ŗ��p����悤�ɂ���
        VECTOR currentHitPos;
        float currentHitDistSq;
        HitPart part = CheckHitPart(rayStart, rayEnd, currentHitPos, currentHitDistSq); // ���������󂯎��悤�ɕύX

        if (part != HitPart::None)
        {
            if (currentHitDistSq < minHitDistSq)
            {
                minHitDistSq = currentHitDistSq;
                hitBulletIndex = i;
                determinedHitPart = part; // �ł��߂��q�b�g�̕��ʂ�ێ�
            }
        }
    }

    // �ł��߂��e�Ń_���[�W�������s��
    if (hitBulletIndex != -1)
    {
        auto& bullet = bullets[hitBulletIndex];
        float damage = CalcDamage(bullet.GetDamage(), determinedHitPart);
        TakeDamage(damage);

        m_lastHitPart = determinedHitPart;
        m_hitDisplayTimer = kDefaultHitDisplayDuration;

        bullet.Deactivate(); // �G�ɓ��������e�͔�A�N�e�B�u�ɂ���
    }
}

// �G���_���[�W���󂯂鏈��
void EnemyBase::TakeDamage(float damage)
{
	m_hp -= damage;
	if (m_hp <= 0.0f)
	{
		m_hp = 0.0f;
		m_isAlive = false;
	}
}

// �G���^�b�N���_���[�W���󂯂鏈��
void EnemyBase::TakeTackleDamage(float damage)
{
	TakeDamage(damage); // �f�t�H���g�͒ʏ�_���[�W�Ɠ���

	// �q�b�g�\����̃q�b�g�Ƃ��čX�V
	m_lastHitPart = HitPart::Body;
	m_hitDisplayTimer = kDefaultHitDisplayDuration; // 1�b�ԕ\��
}