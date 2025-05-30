#pragma once
#include "EnemyBase.h"


class EnemyNormal : public EnemyBase
{
public:
    EnemyNormal(const VECTOR& initPos = { 0.0f, 0.0f, 0.0f }); 
    virtual ~EnemyNormal() = default;

    void Init() override;
    void Update() override;
    void Draw() override;

protected:
    void UpdateState() override;
    void UpdateMovement() override;
    void UpdateAttack() override;

private:
    static constexpr float NORMAL_ZOMBIE_HEALTH = 100.0f;
    static constexpr float NORMAL_ZOMBIE_MOVE_SPEED = 1.0f;
    static constexpr float NORMAL_ZOMBIE_ATTACK_POWER = 10.0f;
    static constexpr float NORMAL_ZOMBIE_ATTACK_RANGE = 2.0f;
    static constexpr float NORMAL_ZOMBIE_ATTACK_COOLDOWN = 2.0f;
};

