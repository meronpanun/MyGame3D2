//#include "EnemyBoss.h"
//#include "Player.h"
//#include "EnemyNormal.h"
//#include "DxLib.h"
//#include <cassert>
//
//EnemyBoss::EnemyBoss() {
//    m_stats.maxHealth = BOSS_ZOMBIE_HEALTH;
//    m_stats.health = BOSS_ZOMBIE_HEALTH;
//    m_stats.moveSpeed = BOSS_ZOMBIE_MOVE_SPEED;
//    m_stats.attackPower = BOSS_ZOMBIE_ATTACK_POWER;
//    m_stats.attackRange = BOSS_ZOMBIE_ATTACK_RANGE;
//    m_stats.attackCooldown = BOSS_ZOMBIE_ATTACK_COOLDOWN;
//    m_attackTimer = 0.0f;
//    m_summonTimer = 0.0f;
//    m_currentState = EnemyState::IDLE;
//}
//
//void EnemyBoss::Init() 
//{
//    // ���f���̓ǂݍ���
//    m_modelHandle = MV1LoadModel("data/image/BossZombie.mv1");
//    assert(m_modelHandle != -1);
//}
//
//void EnemyBoss::Update()
//{
//    if (IsDead()) 
//    {
//        m_currentState = EnemyState::DEAD;
//        return;
//    }
//
//    UpdateState();
//    UpdateAttack();
//    UpdateMinions();
//    UpdateMovement();
//
//    // ���f���̈ʒu���X�V
//    MV1SetPosition(m_modelHandle, m_position);
//}
//
//void EnemyBoss::Draw()
//{
//    if (!IsDead()) 
//    {
//        MV1DrawModel(m_modelHandle);
//    }
//
//    // ���������~�j�I�����`��
//    for (auto& minion : m_minions) 
//    {
//        minion->Draw();
//    }
//}
//
//void EnemyBoss::UpdateState()
//{
//    if (!
// 
// 
// ) return;
//
//    VECTOR playerPos = m_targetPlayer->GetPosition();
//    float distance = VSize(VSub(playerPos, m_position));
//
//    if (distance <= m_stats.attackRange) 
//    {
//        m_currentState = EnemyState::ATTACK;
//    }
//    else if (distance <= 25.0f) 
//    {
//        m_currentState = EnemyState::CHASE;
//    }
//    else
//    {
//        m_currentState = EnemyState::IDLE;
//    }
//}
//
//void EnemyBoss::UpdateMovement() 
//{
//    if (!m_targetPlayer || m_currentState == EnemyState::ATTACK) return;
//
//    VECTOR playerPos = m_targetPlayer->GetPosition();
//    VECTOR direction = VSub(playerPos, m_position);
//    direction = VNorm(direction);
//
//    if (m_currentState == EnemyState::CHASE) 
//    {
//        m_position = VAdd(m_position, VScale(direction, m_stats.moveSpeed));
//    }
//}
//
//void EnemyBoss::UpdateAttack() 
//{
//    if (m_currentState != EnemyState::ATTACK || !m_targetPlayer) return;
//
//    if (m_attackTimer <= 0.0f) 
//    {
//        m_targetPlayer->TakeDamage(m_stats.attackPower);
//        m_attackTimer = m_stats.attackCooldown;
//    }
//    else
//    {
//        m_attackTimer -= 1.0f / 60.0f;
//    }
//
//    // �~�j�I�������̃N�[���_�E��
//    if (m_summonTimer <= 0.0f && m_minions.size() < MAX_MINIONS) 
//    {
//        SummonMinions();
//        m_summonTimer = BOSS_SUMMON_COOLDOWN;
//    }
//    else 
//    {
//        m_summonTimer -= 1.0f / 60.0f;
//    }
//}
//
//void EnemyBoss::SummonMinions() 
//{
//    // �{�X�̎��͂ɒʏ�]���r������
//    for (int i = 0; i < 2 && m_minions.size() < MAX_MINIONS; ++i) 
//    {
//        auto minion = std::make_shared<NormalZombie>();
//        minion->Init();
//
//        // �{�X�̎��͂Ƀ����_���Ȉʒu�ŏ���
//        float angle = (float)(rand() % 360) * DX_PI_F / 180.0f;
//        float distance = 3.0f + (float)(rand() % 3);
//        VECTOR offset = VGet(
//            cosf(angle) * distance,
//            0.0f,
//            sinf(angle) * distance
//        );
//
//        minion->SetPosition(VAdd(m_position, offset));
//        m_minions.push_back(minion);
//    }
//}
//
//void EnemyBoss::UpdateMinions() 
//{
//    // ���񂾃~�j�I�����폜
//    m_minions.erase(
//        std::remove_if(m_minions.begin(), m_minions.end(),
//            [](const auto& minion) { return minion->IsDead(); }),
//        m_minions.end()
//    );
//
//    // �����Ă���~�j�I���̍X�V
//    for (auto& minion : m_minions) 
//    {
//        minion->Update();
//    }
//}