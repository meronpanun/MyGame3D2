#pragma once
#include "EnemyState.h"

class EnemyNormal;

/// <summary>
/// 通常状態（プレイヤーを追跡・徘徊）
/// </summary>
class EnemyNormalStateWalk : public EnemyState<EnemyNormal>
{
public:
    void Enter(EnemyNormal* enemy) override;
    void Update(EnemyNormal* enemy, const EnemyUpdateContext& context) override;
};

/// <summary>
/// 攻撃状態
/// </summary>
class EnemyNormalStateAttack : public EnemyState<EnemyNormal>
{
public:
    void Enter(EnemyNormal* enemy) override;
    void Update(EnemyNormal* enemy, const EnemyUpdateContext& context) override;
};

/// <summary>
/// ダメージ（怯み）状態
/// </summary>
class EnemyNormalStateDamage : public EnemyState<EnemyNormal>
{
public:
    void Enter(EnemyNormal* enemy) override;
    void Update(EnemyNormal* enemy, const EnemyUpdateContext& context) override;
};

/// <summary>
/// 死亡状態
/// </summary>
class EnemyNormalStateDead : public EnemyState<EnemyNormal>
{
public:
    void Enter(EnemyNormal* enemy) override;
    void Update(EnemyNormal* enemy, const EnemyUpdateContext& context) override {} // 死亡ステートでは更新処理なし
};
