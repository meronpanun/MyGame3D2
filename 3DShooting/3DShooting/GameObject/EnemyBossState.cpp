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

void EnemyBossStateAttack::Enter(EnemyBoss* enemy)
{
    enemy->m_hasAttackHit = false;
    enemy->m_hasPlayedCloseRangeEffect = false;
    enemy->ChangeAnimation(EnemyBase::AnimState::Attack, false);
}

void EnemyBossStateLongRange::Enter(EnemyBoss* enemy)
{
    enemy->m_hasShotLongRange = false;
    enemy->ChangeAnimation(EnemyBase::AnimState::LongRangeAttack, false);
}

void EnemyBossStateStunned::Enter(EnemyBoss* enemy)
{
    enemy->m_isStunned = true;
    enemy->m_stunTimer = EnemyBossConstants::kStunDuration;
    // 死亡アニメーションを怯みとして流用
    enemy->ChangeAnimation(EnemyBase::AnimState::Dead, false);
}

void EnemyBossStateDead::Enter(EnemyBoss* enemy)
{
    enemy->ChangeAnimation(EnemyBase::AnimState::Dead, false);
    enemy->m_isDeadAnimPlaying = true;
}

