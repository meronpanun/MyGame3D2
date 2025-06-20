//#pragma once
//#include "EnemyBase.h"
//#include <vector>
//#include <memory>
//
//class EnemyBoss : public EnemyBase
//{
//public:
//    EnemyBoss();
//    virtual ~EnemyBoss() = default;
//
//    void Init() override;
//    void Update() override;
//    void Draw() override;
//
//protected:
//    void UpdateState() override;
//    void UpdateMovement() override;
//    void UpdateAttack() override;
//
//private:
//    void SummonMinions();
//    void UpdateMinions();
//
//    static constexpr float BOSS_ZOMBIE_HEALTH = 500.0f;
//    static constexpr float BOSS_ZOMBIE_MOVE_SPEED = 1.2f;
//    static constexpr float BOSS_ZOMBIE_ATTACK_POWER = 30.0f;
//    static constexpr float BOSS_ZOMBIE_ATTACK_RANGE = 3.0f;
//    static constexpr float BOSS_ZOMBIE_ATTACK_COOLDOWN = 2.0f;
//    static constexpr float BOSS_SUMMON_COOLDOWN = 10.0f;
//    static constexpr int MAX_MINIONS = 5;
//
//    float m_summonTimer;
//    std::vector<std::shared_ptr<EnemyBase>> m_minions;
//};
//
