#pragma once
#include "EnemyState.h"

class EnemyAcid;

/// <summary>
/// 徘徊・追跡状態
/// </summary>
class EnemyAcidStateWalk : public EnemyState<EnemyAcid>
{
public:
    void Enter(EnemyAcid* enemy) override;
    void Update(EnemyAcid* enemy, const EnemyUpdateContext& context) override;
};

/// <summary>
/// 後退状態
/// </summary>
class EnemyAcidStateBack : public EnemyState<EnemyAcid>
{
public:
    void Enter(EnemyAcid* enemy) override;
    void Update(EnemyAcid* enemy, const EnemyUpdateContext& context) override;
};

/// <summary>
/// 攻撃状態
/// </summary>
class EnemyAcidStateAttack : public EnemyState<EnemyAcid>
{
public:
    void Enter(EnemyAcid* enemy) override;
    void Update(EnemyAcid* enemy, const EnemyUpdateContext& context) override;
};

/// <summary>
/// 怯み（スタン）状態
/// </summary>
class EnemyAcidStateStunned : public EnemyState<EnemyAcid>
{
public:
    void Enter(EnemyAcid* enemy) override;
    void Update(EnemyAcid* enemy, const EnemyUpdateContext& context) override;
};

/// <summary>
/// 死亡状態
/// </summary>
class EnemyAcidStateDead : public EnemyState<EnemyAcid>
{
public:
    void Enter(EnemyAcid* enemy) override;
    void Update(EnemyAcid* enemy, const EnemyUpdateContext& context) override;
};
