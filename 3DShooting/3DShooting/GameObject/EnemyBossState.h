#pragma once
#include "EnemyState.h"

class EnemyBoss;

/// <summary>
/// 移動・追跡状態
/// </summary>
class EnemyBossStateWalk : public EnemyState<EnemyBoss>
{
public:
    void Enter(EnemyBoss* enemy) override;
    void Update(EnemyBoss* enemy, const EnemyUpdateContext& context) override;
};

/// <summary>
/// 近接攻撃状態
/// </summary>
class EnemyBossStateAttack : public EnemyState<EnemyBoss>
{
public:
    void Enter(EnemyBoss* enemy) override;
    void Update(EnemyBoss* enemy, const EnemyUpdateContext& context) override;
};

/// <summary>
/// 遠距離攻撃状態
/// </summary>
class EnemyBossStateLongRange : public EnemyState<EnemyBoss>
{
public:
    void Enter(EnemyBoss* enemy) override;
    void Update(EnemyBoss* enemy, const EnemyUpdateContext& context) override;
};

/// <summary>
/// 怯み（スタン）状態
/// </summary>
class EnemyBossStateStunned : public EnemyState<EnemyBoss>
{
public:
    void Enter(EnemyBoss* enemy) override;
    void Update(EnemyBoss* enemy, const EnemyUpdateContext& context) override;
};

/// <summary>
/// 死亡状態
/// </summary>
class EnemyBossStateDead : public EnemyState<EnemyBoss>
{
public:
    void Enter(EnemyBoss* enemy) override;
    void Update(EnemyBoss* enemy, const EnemyUpdateContext& context) override;
};
