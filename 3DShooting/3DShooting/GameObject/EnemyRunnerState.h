#pragma once
#include "EnemyState.h"

// 前方宣言
class EnemyRunner;

/// <summary>
/// 走行状態（プレイヤーを追跡・回避）
/// </summary>
class EnemyRunnerStateRun : public EnemyState<EnemyRunner>
{
public:
    void Enter(EnemyRunner* enemy) override;
    void Update(EnemyRunner* enemy, const EnemyUpdateContext& context) override;
};

/// <summary>
/// 攻撃状態
/// </summary>
class EnemyRunnerStateAttack : public EnemyState<EnemyRunner>
{
public:
    void Enter(EnemyRunner* enemy) override;
    void Update(EnemyRunner* enemy, const EnemyUpdateContext& context) override;
};

/// <summary>
/// 死亡状態
/// </summary>
class EnemyRunnerStateDead : public EnemyState<EnemyRunner>
{
public:
    void Enter(EnemyRunner* enemy) override;
    void Update(EnemyRunner* enemy, const EnemyUpdateContext& context) override;
};
