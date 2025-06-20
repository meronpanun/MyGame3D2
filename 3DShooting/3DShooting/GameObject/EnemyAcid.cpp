//#include "EnemyAcid.h"
//#include "Player.h"
//#include "DxLib.h"
//#include <cassert>
//
//EnemyAcid::EnemyAcid()
//{
//    m_stats.maxHealth = ACID_ZOMBIE_HEALTH;
//    m_stats.health = ACID_ZOMBIE_HEALTH;
//    m_stats.moveSpeed = ACID_ZOMBIE_MOVE_SPEED;
//    m_stats.attackPower = ACID_ZOMBIE_ATTACK_POWER;
//    m_stats.attackRange = ACID_ZOMBIE_ATTACK_RANGE;
//    m_stats.attackCooldown = ACID_ZOMBIE_ATTACK_COOLDOWN;
//    m_attackTimer = 0.0f;
//    m_currentState = EnemyState::IDLE;
//}
//
//void EnemyAcid::Init() 
//{
//    // ���f���̓ǂݍ���
//    m_modelHandle = MV1LoadModel("data/image/AcidZombie.mv1");
//    assert(m_modelHandle != -1);
//}
//
//void EnemyAcid::Update() 
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
//void EnemyAcid::Draw() 
//{
//    if (!IsDead())
//    {
//        MV1DrawModel(m_modelHandle);
//    }
//}
//
//void EnemyAcid::UpdateState() 
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
//    else if (distance <= 15.0f) { // �ǐՔ͈͂�����
//        m_currentState = EnemyState::CHASE;
//    }
//    else
//    {
//        m_currentState = EnemyState::IDLE;
//    }
//}
//
//void EnemyAcid::UpdateMovement() 
//{
//    if (!m_targetPlayer || m_currentState == EnemyState::ATTACK) return;
//
//    VECTOR playerPos = m_targetPlayer->GetPosition();
//    VECTOR direction = VSub(playerPos, m_position);
//    direction = VNorm(direction);
//
//    if (m_currentState == EnemyState::CHASE) 
//    {
//        // �_��f���]���r�͈ړ����x��
//        m_position = VAdd(m_position, VScale(direction, m_stats.moveSpeed));
//    }
//}
//
//void EnemyAcid::UpdateAttack() 
//{
//    if (m_currentState != EnemyState::ATTACK || !m_targetPlayer) return;
//
//    if (m_attackTimer <= 0.0f) 
//    {
//        ShootAcid();
//        m_attackTimer = m_stats.attackCooldown;
//    }
//    else
//    {
//        m_attackTimer -= 1.0f / 60.0f; // 60FPS��z��
//    }
//}
//
//void EnemyAcid::ShootAcid()
//{
//    if (!m_targetPlayer) return;
//
//    VECTOR playerPos = m_targetPlayer->GetPosition();
//    VECTOR direction = VSub(playerPos, m_position);
//    direction = VNorm(direction);
//
//    // �_�̒e�𐶐����Ĕ���
//    // �����ł͊ȈՓI�Ȏ����Ƃ��āA�v���C���[�ɒ��ڃ_���[�W��^����
//    float distance = VSize(VSub(playerPos, m_position));
//    if (distance <= m_stats.attackRange)
//    {
//        m_targetPlayer->TakeDamage(m_stats.attackPower);
//    }
//}