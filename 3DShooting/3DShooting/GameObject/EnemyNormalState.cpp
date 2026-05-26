#include "EnemyNormalState.h"
#include "EnemyNormal.h"

// 注意: 各ステートは EnemyNormal が公開している State 向けインターフェースのみを
//       使用する。private メンバへの直接アクセスはしない（friend なし）。

void EnemyNormalStateWalk::Enter(EnemyNormal* enemy)
{
    enemy->ChangeAnimation(EnemyBase::AnimState::Walk, true);
}

void EnemyNormalStateWalk::Update(EnemyNormal* enemy, const EnemyUpdateContext& context)
{
    if (!enemy->ShouldUpdateAI()) return;

    enemy->UpdateChaseBehavior(context.player);

    // 攻撃が届いたら Attack に遷移
    if (enemy->CanAttackPlayer(context.player))
    {
        enemy->ResetAttackHitFlag();
        enemy->ChangeState(std::make_shared<EnemyNormalStateAttack>());
    }
}

void EnemyNormalStateAttack::Enter(EnemyNormal* enemy)
{
    enemy->ResetAttackHitFlag();
    enemy->ChangeAnimation(EnemyBase::AnimState::Attack, false);
}

void EnemyNormalStateAttack::Update(EnemyNormal* enemy, const EnemyUpdateContext& context)
{
    if (!enemy->ShouldUpdateAI()) return;

    // 攻撃ヒット判定（アニメーション内の特定区間で行う）
    enemy->UpdateAttackHitDetection(context.player);

    // 攻撃アニメ終了でディレイ開始
    if (enemy->IsAttackAnimationFinished())
    {
        enemy->StartAttackEndDelay();
    }

    // ディレイ終了フレームで次状態へ遷移
    if (enemy->TickAttackEndDelay())
    {
        enemy->ResetAttackHitFlag();
        if (enemy->IsPlayerInAttackRange(context.player))
        {
            enemy->ChangeState(std::make_shared<EnemyNormalStateAttack>());
        }
        else
        {
            enemy->ChangeState(std::make_shared<EnemyNormalStateWalk>());
        }
    }
}

void EnemyNormalStateDamage::Enter(EnemyNormal* enemy)
{
    enemy->ChangeAnimation(EnemyBase::AnimState::Walk, true); // ダメージ中は歩行モーションを流用
}

void EnemyNormalStateDamage::Update(EnemyNormal* enemy, const EnemyUpdateContext& context)
{
    // ダメージタイマーは AI 間引きとは独立に毎フレーム更新
    if (enemy->TickDamageTimer())
    {
        enemy->ChangeState(std::make_shared<EnemyNormalStateWalk>());
    }

    enemy->ApplyDamageKnockback(context.player);
}

void EnemyNormalStateDead::Enter(EnemyNormal* enemy)
{
    enemy->ChangeAnimation(EnemyBase::AnimState::Dead, false);
}
