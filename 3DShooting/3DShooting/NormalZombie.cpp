#include "NormalZombie.h"
#include "Player.h"
#include "DxLib.h"

NormalZombie::NormalZombie() {
    m_stats.maxHealth = NORMAL_ZOMBIE_HEALTH;
    m_stats.health = NORMAL_ZOMBIE_HEALTH;
    m_stats.moveSpeed = NORMAL_ZOMBIE_MOVE_SPEED;
    m_stats.attackPower = NORMAL_ZOMBIE_ATTACK_POWER;
    m_stats.attackRange = NORMAL_ZOMBIE_ATTACK_RANGE;
    m_stats.attackCooldown = NORMAL_ZOMBIE_ATTACK_COOLDOWN;
    m_attackTimer = 0.0f;
    m_currentState = EnemyState::IDLE;
}

void NormalZombie::Init() {
    // モデルの読み込み
    m_modelHandle = MV1LoadModel("data/image/NormalZombie.mv1");
    assert(m_modelHandle != -1);
}

void NormalZombie::Update() {
    if (IsDead()) {
        m_currentState = EnemyState::DEAD;
        return;
    }

    UpdateState();
    UpdateMovement();
    UpdateAttack();

    // モデルの位置を更新
    MV1SetPosition(m_modelHandle, m_position);
}

void NormalZombie::Draw() {
    if (!IsDead()) {
        MV1DrawModel(m_modelHandle);
    }
}

void NormalZombie::UpdateState() {
    if (!m_targetPlayer) return;

    VECTOR playerPos = m_targetPlayer->GetPosition();
    float distance = VSize(VSub(playerPos, m_position));

    if (distance <= m_stats.attackRange) {
        m_currentState = EnemyState::ATTACK;
    }
    else if (distance <= 20.0f) { // 追跡範囲
        m_currentState = EnemyState::CHASE;
    }
    else {
        m_currentState = EnemyState::IDLE;
    }
}

void NormalZombie::UpdateMovement() {
    if (!m_targetPlayer || m_currentState == EnemyState::ATTACK) return;

    VECTOR playerPos = m_targetPlayer->GetPosition();
    VECTOR direction = VSub(playerPos, m_position);
    direction = VNorm(direction);

    if (m_currentState == EnemyState::CHASE) {
        m_position = VAdd(m_position, VScale(direction, m_stats.moveSpeed));
    }
}

void NormalZombie::UpdateAttack() {
    if (m_currentState != EnemyState::ATTACK || !m_targetPlayer) return;

    if (m_attackTimer <= 0.0f) {
        // プレイヤーにダメージを与える
        m_targetPlayer->TakeDamage(m_stats.attackPower);
        m_attackTimer = m_stats.attackCooldown;
    }
    else {
        m_attackTimer -= 1.0f / 60.0f; // 60FPSを想定
    }
} 