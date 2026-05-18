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
    void Update(EnemyBoss* enemy, const EnemyUpdateContext& context) override {} // 移動・遷移判断はUpdateAIに委譲
};

/// <summary>
/// 近接攻撃状態
/// </summary>
class EnemyBossStateAttack : public EnemyState<EnemyBoss>
{
public:
    void Enter(EnemyBoss* enemy) override;
    void Update(EnemyBoss* enemy, const EnemyUpdateContext& context) override {} // ヒット判定・エフェクトはUpdateAIに委譲
};

/// <summary>
/// 遠距離攻撃状態
/// </summary>
class EnemyBossStateLongRange : public EnemyState<EnemyBoss>
{
public:
    void Enter(EnemyBoss* enemy) override;
    void Update(EnemyBoss* enemy, const EnemyUpdateContext& context) override {} // 弾発射ロジックはUpdateAIに委譲
};

/// <summary>
/// 怯み（スタン）状態
/// </summary>
class EnemyBossStateStunned : public EnemyState<EnemyBoss>
{
public:
    void Enter(EnemyBoss* enemy) override;
    void Update(EnemyBoss* enemy, const EnemyUpdateContext& context) override {} // 怯みタイマー更新はUpdateAIで優先的に処理される
};

/// <summary>
/// 死亡状態
/// </summary>
class EnemyBossStateDead : public EnemyState<EnemyBoss>
{
public:
    void Enter(EnemyBoss* enemy) override;
    void Update(EnemyBoss* enemy, const EnemyUpdateContext& context) override {} // 死亡ステートでは更新処理なし
};
