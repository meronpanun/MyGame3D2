#pragma once
#include "EnemyBase.h"

class RunnerZombie : public EnemyBase {
public:
    RunnerZombie();
    virtual ~RunnerZombie() = default;

    void Init() override;
    void Update() override;
    void Draw() override;

protected:
    void UpdateState() override;
    void UpdateMovement() override;
    void UpdateAttack() override;

private:
    static constexpr float RUNNER_ZOMBIE_HEALTH = 50.0f;
    static constexpr float RUNNER_ZOMBIE_MOVE_SPEED = 3.0f;
    static constexpr float RUNNER_ZOMBIE_ATTACK_POWER = 15.0f;
    static constexpr float RUNNER_ZOMBIE_ATTACK_RANGE = 1.5f;
    static constexpr float RUNNER_ZOMBIE_ATTACK_COOLDOWN = 1.5f;
}; 