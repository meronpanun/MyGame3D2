#include "Player.h"
#include "EnemyBase.h"
#include "Bullet.h"

namespace
{
	constexpr int   kDefaultHitDisplayDuration = 60;     // 1�b�ԕ\��
	constexpr float kDefaultInitialHP          = 100.0f; // �f�t�H���g�̏����̗� 

	constexpr float kDefaultCooldownMax = 60;     // �U���N�[���_�E���̍ő�l
	constexpr float kDefaultAttackPower = 10.0f;  // �U����
}

EnemyBase::EnemyBase() :
	m_pos{ 0, 0, 0 },
	m_modelHandle(-1),
	m_colRadius(1.0f), 
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
	for (auto& bullet : bullets)
	{
		// �e����A�N�e�B�u�Ȃ�X�L�b�v
		if (!bullet.IsActive()) continue;

		// �ǂ��ɓ��������̂����`�F�b�N
		HitPart part = CheckHitPart(bullet);

		if (part == HitPart::Head || part == HitPart::Body)
		{
			float damage = CalcDamage(bullet, part);
			TakeDamage(damage);

			m_lastHitPart = part;
			m_hitDisplayTimer = kDefaultHitDisplayDuration;

			bullet.Deactivate();
			break;
		}
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

// �G�̍U�����X�V���鏈��
void EnemyBase::UpdateAttack()
{
}

// �v���C���[�ɍU�����s������
void EnemyBase::AttackPlayer(Player* player)
{
	if (!player) return;
	player->TakeDamage(m_attackPower);
}

