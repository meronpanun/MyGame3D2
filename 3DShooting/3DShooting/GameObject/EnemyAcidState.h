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
    void Update(EnemyAcid* enemy, const EnemyUpdateContext& context) override {} // 移動AIはUpdateMovementAIに委譲
};

/// <summary>
/// 後退状態
/// </summary>
class EnemyAcidStateBack : public EnemyState<EnemyAcid>
{
public:
    void Enter(EnemyAcid* enemy) override;
    void Update(EnemyAcid* enemy, const EnemyUpdateContext& context) override {} // 移動AIはUpdateMovementAIに委譲
};

/// <summary>
/// 攻撃状態
/// </summary>
class EnemyAcidStateAttack : public EnemyState<EnemyAcid>
{
public:
    void Enter(EnemyAcid* enemy) override;
    void Update(EnemyAcid* enemy, const EnemyUpdateContext& context) override {} // 攻撃ロジックはUpdateMovementAIに委譲
};

/// <summary>
/// 怯み（スタン）状態
/// </summary>
class EnemyAcidStateStunned : public EnemyState<EnemyAcid>
{
public:
    void Enter(EnemyAcid* enemy) override;
    void Update(EnemyAcid* enemy, const EnemyUpdateContext& context) override {} // 怯み処理はUpdateAIで優先的に処理される
};

/// <summary>
/// 死亡状態
/// </summary>
class EnemyAcidStateDead : public EnemyState<EnemyAcid>
{
public:
    void Enter(EnemyAcid* enemy) override;
    void Update(EnemyAcid* enemy, const EnemyUpdateContext& context) override {} // 死亡ステートでは更新処理なし
};
