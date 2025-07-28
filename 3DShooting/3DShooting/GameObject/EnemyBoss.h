#pragma once
#include "EnemyBase.h"

/// <summary>
/// 敵ボスクラス
/// </summary>
class EnemyBoss : public EnemyBase
{
public:
	EnemyBoss();
	virtual ~EnemyBoss();
	
	void Init() override;
	void Update(std::vector<Bullet>& bullets, const Player::TackleInfo& tackleInfo, const Player& player, const std::vector<EnemyBase*>& enemyList) override;
	void Draw() override;

private:
};

