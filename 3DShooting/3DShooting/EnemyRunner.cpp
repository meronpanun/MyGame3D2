//#include "EnemyRunner.h"
//#include "Player.h"
//#include "DxLib.h"
//#include <cassert>
//
//EnemyRunner::EnemyRunner()
//{
//    m_stats.maxHealth = RUNNER_ZOMBIE_HEALTH;
//    m_stats.health = RUNNER_ZOMBIE_HEALTH;
//    m_stats.moveSpeed = RUNNER_ZOMBIE_MOVE_SPEED;
//    m_stats.attackPower = RUNNER_ZOMBIE_ATTACK_POWER;
//    m_stats.attackRange = RUNNER_ZOMBIE_ATTACK_RANGE;
//    m_stats.attackCooldown = RUNNER_ZOMBIE_ATTACK_COOLDOWN;
//    m_attackTimer = 0.0f;
//    m_currentState = EnemyState::IDLE;
//}
//
//void EnemyRunner::Init() 
//{
//    // ���f���̓ǂݍ���
//    m_modelHandle = MV1LoadModel("data/image/RunnerZombie.mv1");
//    assert(m_modelHandle != -1);
//}
//
//void EnemyRunner::Update() 
//{
//    if (IsDead()) 
//    {
//        m_currentState = EnemyState::DEAD;
//        return;
//    }
//
//    UpdateState();
//    UpdateMovement();
//    UpdateAttack();
//
//    // ���f���̈ʒu���X�V
//    MV1SetPosition(m_modelHandle, m_position);
//}
//
//void EnemyRunner::Draw() 
//{
//    if (!IsDead()) 
//    {
//        MV1DrawModel(m_modelHandle);
//    }
//}
//
//void EnemyRunner::UpdateState() 
//{
//    if (!m_targetPlayer) return;
//
//    VECTOR playerPos = m_targetPlayer->GetPosition();
//    float distance = VSize(VSub(playerPos, m_position));
//
//    if (distance <= m_stats.attackRange) 
//    {
//        m_currentState = EnemyState::ATTACK;
//    }
//    else if (distance <= 30.0f)  // �ǐՔ͈͂��ʏ�]���r���L��
//    {
//        m_currentState = EnemyState::CHASE;
//    }
//    else 
//    {
//        m_currentState = EnemyState::IDLE;
//    }
//}
//
//void EnemyRunner::UpdateMovement() 
//{
//    if (!m_targetPlayer || m_currentState == EnemyState::ATTACK) return;
//
//    VECTOR playerPos = m_targetPlayer->GetPosition();
//    VECTOR direction = VSub(playerPos, m_position);
//    direction = VNorm(direction);
//
//    if (m_currentState == EnemyState::CHASE) 
//    {
//        // �����i�[�]���r�͂�葬���ړ�
//        m_position = VAdd(m_position, VScale(direction, m_stats.moveSpeed));
//    }
//}
//
//void EnemyRunner::UpdateAttack() 
//{
//    if (m_currentState != EnemyState::ATTACK || !m_targetPlayer) return;
//
//    if (m_attackTimer <= 0.0f) 
//    {
//        // �v���C���[�Ƀ_���[�W��^����
//        m_targetPlayer->TakeDamage(m_stats.attackPower);
//        m_attackTimer = m_stats.attackCooldown;
//    }
//    else 
//    {
//        m_attackTimer -= 1.0f / 60.0f; // 60FPS��z��
//    }
//}