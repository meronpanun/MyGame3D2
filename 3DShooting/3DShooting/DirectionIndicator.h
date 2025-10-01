#pragma once
#include "Vec3.h"
#include <vector>

class Player;
class EnemyBase;

class DirectionIndicator
{
public:
    DirectionIndicator();
    ~DirectionIndicator();

    void Init(Player* player);
    void Update(const std::vector<std::shared_ptr<EnemyBase>>& enemies);
    void Draw();

private:
    Player* m_pPlayer;
    std::vector<Vec3> m_allEnemyPositions;
    int m_indicatorImage;
};