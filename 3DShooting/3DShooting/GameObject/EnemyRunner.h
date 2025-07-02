#pragma once
#include "EnemyBase.h"
#include <vector>
#include <memory>

class Bullet;
class Player;
class Collider;
class CapsuleCollider;
class SphereCollider;

/// <summary>
/// 走る敵クラス
/// </summary>
class EnemyRunner : public EnemyBase
{
public:
	EnemyRunner();
	virtual ~EnemyRunner();

	void Init() override;
	void Update(std::vector<Bullet>& bullets, const Player::TackleInfo& tackleInfo, const Player& player) override;
	void Draw() override;
};

