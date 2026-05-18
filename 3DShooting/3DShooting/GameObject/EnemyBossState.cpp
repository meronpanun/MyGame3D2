#include "EnemyBossState.h"
#include "EnemyBoss.h"
#include "Player.h"
#include "Game.h"
#include "DxLib.h"
#include "SphereCollider.h"
#include "CapsuleCollider.h"

void EnemyBossStateWalk::Enter(EnemyBoss* enemy)
{
    enemy->ChangeAnimation(EnemyBase::AnimState::Walk, true);
}

void EnemyBossStateWalk::Update(EnemyBoss* enemy, const EnemyUpdateContext& context)
{
    // 移動と遷移判断はEnemyBoss::Update内の共通ロジックに任せる
}

void EnemyBossStateAttack::Enter(EnemyBoss* enemy)
{
    enemy->m_hasAttackHit = false;
    enemy->m_hasPlayedCloseRangeEffect = false;
    enemy->ChangeAnimation(EnemyBase::AnimState::Attack, false);
}

void EnemyBossStateAttack::Update(EnemyBoss* enemy, const EnemyUpdateContext& context)
{
    // ヒット判定やエフェクト再生はEnemyBoss::Update内の共通ロジックに任せる
}

void EnemyBossStateLongRange::Enter(EnemyBoss* enemy)
{
    enemy->m_hasShotLongRange = false;
    enemy->ChangeAnimation(EnemyBase::AnimState::LongRangeAttack, false);
}

void EnemyBossStateLongRange::Update(EnemyBoss* enemy, const EnemyUpdateContext& context)
{
    // 弾の発射ロジックはEnemyBoss::Update内の共通ロジックに任せる
}

void EnemyBossStateStunned::Enter(EnemyBoss* enemy)
{
    enemy->m_isStunned = true;
    enemy->m_stunTimer = EnemyBossConstants::kStunDuration;
    // 死亡アニメーションを怯みとして流用
    enemy->ChangeAnimation(EnemyBase::AnimState::Dead, false);
}

void EnemyBossStateStunned::Update(EnemyBoss* enemy, const EnemyUpdateContext& context)
{
    // 怯みタイマー更新はEnemyBoss::Update内の共通ロジックに任せる
}

void EnemyBossStateDead::Enter(EnemyBoss* enemy)
{
    enemy->ChangeAnimation(EnemyBase::AnimState::Dead, false);
    enemy->m_isDeadAnimPlaying = true;
}

void EnemyBossStateDead::Update(EnemyBoss* enemy, const EnemyUpdateContext& context)
{
    // 死亡時は何もしない
}
