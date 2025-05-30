#include "EnemyNormal.h"
#include "Player.h"
#include "DxLib.h"
#include <cassert>

EnemyNormal::EnemyNormal(const VECTOR& initPos)
{
    m_stats.maxHealth = NORMAL_ZOMBIE_HEALTH;
    m_stats.health = NORMAL_ZOMBIE_HEALTH;
    m_stats.moveSpeed = NORMAL_ZOMBIE_MOVE_SPEED;
    m_stats.attackPower = NORMAL_ZOMBIE_ATTACK_POWER;
    m_stats.attackRange = NORMAL_ZOMBIE_ATTACK_RANGE;
    m_stats.attackCooldown = NORMAL_ZOMBIE_ATTACK_COOLDOWN;
    m_attackTimer = 0.0f;
    m_currentState = EnemyState::IDLE;
    m_pos = initPos;
}

void EnemyNormal::Init()
{
    // ���f���̓ǂݍ���
    m_modelHandle = MV1LoadModel("data/image/NormalZombie.mv1");
    assert(m_modelHandle != -1);

    // �C�ӂ̈ʒu�ɃX�|�[��
    VECTOR spawnPos = { 10.0f, 0.0f, 5.0f };
    EnemyNormal* enemy = new EnemyNormal(spawnPos);
}

void EnemyNormal::Update()
{
    if (IsDead()) 
    {
        m_currentState = EnemyState::DEAD;
        return;
    }

    UpdateState();
    UpdateMovement();
    UpdateAttack();

    // ���f���̈ʒu���X�V
    MV1SetPosition(m_modelHandle, m_pos);
}

void EnemyNormal::Draw() 
{
    if (!IsDead())
    {
        MV1DrawModel(m_modelHandle);

        // �f�o�b�O�p�F�ʒu����ʂɕ\��
        // ��ʍ��W�ւ̕ϊ��i���[���h���W���X�N���[�����W�j
        VECTOR screenPos = ConvWorldPosToScreenPos(m_pos);
        if (screenPos.x >= 0 && screenPos.y >= 0)
        {
            DrawFormatString(
                static_cast<int>(screenPos.x),
                static_cast<int>(screenPos.y),
                GetColor(255, 0, 0),
                "EnemyPos: (%.1f, %.1f, %.1f)",
                m_pos.x, m_pos.y, m_pos.z
            );
        }
        else
        {
            // �ϊ��ł��Ȃ������ꍇ�͍���ɕ\��
            DrawFormatString(
                10, 10,
                GetColor(255, 0, 0),
                "EnemyPos: (%.1f, %.1f, %.1f)",
                m_pos.x, m_pos.y, m_pos.z
            );
        }
    }
}

void EnemyNormal::UpdateState() 
{
    if (!m_targetPlayer) return;

    VECTOR playerPos = m_targetPlayer->GetPos();
    float distance = VSize(VSub(playerPos, m_pos));

    if (distance <= m_stats.attackRange) 
    {
        m_currentState = EnemyState::ATTACK;
    }
    else if (distance <= 20.0f) // �ǐՔ͈�
    {
        m_currentState = EnemyState::CHASE;
    }
    else 
    {
        m_currentState = EnemyState::IDLE;
    }
}

void EnemyNormal::UpdateMovement()
{
    if (!m_targetPlayer || m_currentState == EnemyState::ATTACK) return;

    VECTOR playerPos = m_targetPlayer->GetPos();
    VECTOR direction = VSub(playerPos, m_pos);
    direction = VNorm(direction);

    if (m_currentState == EnemyState::CHASE)
    {
        m_pos = VAdd(m_pos, VScale(direction, m_stats.moveSpeed));
    }
}

void EnemyNormal::UpdateAttack() 
{
    if (m_currentState != EnemyState::ATTACK || !m_targetPlayer) return;

    if (m_attackTimer <= 0.0f) 
    {
        // �v���C���[�Ƀ_���[�W��^����
        m_targetPlayer->TakeDamage(m_stats.attackPower);
        m_attackTimer = m_stats.attackCooldown;
    }
    else 
    {
        m_attackTimer -= 1.0f / 60.0f; // 60FPS��z��
    }
}