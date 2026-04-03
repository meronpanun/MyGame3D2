#include "CollisionGrid.h"
#include "EnemyBase.h"
#include <cmath>

CollisionGrid::CollisionGrid()
    : m_minArea(VGet(0, 0, 0))
    , m_maxArea(VGet(0, 0, 0))
    , m_cellSize(100.0f)
    , m_width(0)
    , m_height(0)
{
}

CollisionGrid::~CollisionGrid()
{
}

void CollisionGrid::Init(const VECTOR& minArea, const VECTOR& maxArea,
    float cellSize)
{
    m_minArea = minArea;
    m_maxArea = maxArea;
    m_cellSize = cellSize;

    float worldWidth = m_maxArea.x - m_minArea.x;
    float worldHeight = m_maxArea.z - m_minArea.z;

    m_width = static_cast<int>(std::ceil(worldWidth / m_cellSize));
    m_height = static_cast<int>(std::ceil(worldHeight / m_cellSize));

    // 安全マージン
    m_width += 2;
    m_height += 2;

    m_cells.clear();
    m_cells.resize(m_width * m_height);
}

void CollisionGrid::Clear()
{
    for (auto& cell : m_cells)
    {
        cell.clear();
    }
}

void CollisionGrid::RegisterEnemy(EnemyBase* enemy)
{
    if (!enemy)
        return;
    int index = GetCellIndex(enemy->GetPos());
    if (index >= 0 && index < static_cast<int>(m_cells.size()))
    {
        m_cells[index].push_back(enemy);
    }
}

void CollisionGrid::GetNeighbors(const VECTOR& pos, std::vector<EnemyBase*>& outNeighbors) const
{
    outNeighbors.clear();

    /*空間分割による近傍オブジェクトの高速探索*/ 
    // 指定されたワールド座標が属するセルのグリッド座標 (cx, cz) を算出
    int cx, cz;
    GetCellIndices(pos, cx, cz);

    // 自身が属するセルを中心に、周囲3×3 = 9セル分のエリアを走査
    for (int z = cz - 1; z <= cz + 1; ++z)
    {
        // 縦（Z軸）方向のフィールド範囲外アクセスを防ぐ（外側ループで早期スキップし最適化）
        if (z < 0 || z >= m_height) continue;

        for (int x = cx - 1; x <= cx + 1; ++x)
        {
            // 横（X軸）方向のフィールド範囲外アクセスを防ぐ
            if (x < 0 || x >= m_width) continue;

            //該当セルに登録済みの敵リストを合成 
            // 2次元グリッド座標を1次元配列用インデックスに変換
            int cellIndex = z * m_width + x;
            const auto& targetCell = m_cells[cellIndex];
            
            // ベクターの末尾に該当セルの敵リスト（ポインタ）を一括挿入
            // これにより、O(N^2)となる全敵との総当たり判定を防ぎ、処理負荷を圧縮
            outNeighbors.insert(outNeighbors.end(), targetCell.begin(), targetCell.end());
        }
    }
}

int CollisionGrid::GetCellIndex(const VECTOR& pos) const
{
    int x, z;
    GetCellIndices(pos, x, z);
    if (x < 0 || x >= m_width || z < 0 || z >= m_height)
    {
        return -1;
    }
    return z * m_width + x;
}

void CollisionGrid::GetCellIndices(const VECTOR& pos, int& x, int& z) const
{
    float localX = pos.x - m_minArea.x;
    float localZ = pos.z - m_minArea.z;

    x = static_cast<int>(std::floor(localX / m_cellSize));
    z = static_cast<int>(std::floor(localZ / m_cellSize));
}
