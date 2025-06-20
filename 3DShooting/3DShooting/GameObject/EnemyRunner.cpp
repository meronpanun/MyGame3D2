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
//    // モデルの読み込み
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
//    // モデルの位置を更新
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
//    else if (distance <= 30.0f)  // 追跡範囲が通常ゾンビより広い
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
//        // ランナーゾンビはより速く移動
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
//        // プレイヤーにダメージを与える
//        m_targetPlayer->TakeDamage(m_stats.attackPower);
//        m_attackTimer = m_stats.attackCooldown;
//    }
//    else 
//    {
//        m_attackTimer -= 1.0f / 60.0f; // 60FPSを想定
//    }
//}