#pragma once
#include "Vec3.h"
#include <vector>

class Player;
class EnemyBase;

/// <summary>
/// 方向インジケータークラス
/// </summary>
class DirectionIndicator
{
public:
    DirectionIndicator();
    ~DirectionIndicator();

    void Init(Player* player);
    void Update(const std::vector<std::shared_ptr<EnemyBase>>& enemies);
    void Draw();

    /// <summary>
	/// 共有リソースの読み込み
    /// </summary>
    static void LoadResources(); 

    /// <summary>
	/// 共有リソースの解放
    /// </summary>
    static void DeleteResources();

private:
    Player* m_pPlayer;
	std::vector<Vec3> m_allEnemyPositions; // 全ての敵の位置
	static int s_indicatorImage; // インジケーターの画像ハンドル
};