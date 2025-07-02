#include "EnemyRunner.h"
#include "Bullet.h"
#include "Player.h"
#include "DxLib.h"
#include "DebugUtil.h"
#include "CapsuleCollider.h"
#include "SphereCollider.h"
#include <cassert>

namespace
{
	// アニメーション関連
	constexpr char kAttackAnimName[] = "Armature|Attack"; // 攻撃アニメーション
	constexpr char kRunAnimName[]    = "Armature|Run";    // 走るアニメーション
	constexpr char kDeadAnimName[]   = "Armature|Death";  // 死亡アニメーション
}

EnemyRunner::EnemyRunner()
{
	m_modelHandle = MV1LoadModel("data/model/RunnerZombie.mv1");
	assert(m_modelHandle != -1);
}

EnemyRunner::~EnemyRunner()
{
	MV1DeleteModel(m_modelHandle);
}

void EnemyRunner::Init()
{
}

void EnemyRunner::Update(std::vector<Bullet>& bullets, const Player::TackleInfo& tackleInfo, const Player& player)
{
}

void EnemyRunner::Draw()
{
}
