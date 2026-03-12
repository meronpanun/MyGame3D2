#include "CollisionGrid.h"
#include "GameObject/EnemyBase.h"
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

    int cx, cz;
    GetCellIndices(pos, cx, cz);

    // 周囲9セル（自分含む）を検索
    for (int z = cz - 1; z <= cz + 1; ++z)
    {
        for (int x = cx - 1; x <= cx + 1; ++x)
        {
            if (x >= 0 && x < m_width && z >= 0 && z < m_height)
            {
                int index = z * m_width + x;
                const auto& cell = m_cells[index];
                outNeighbors.insert(outNeighbors.end(), cell.begin(), cell.end());
            }
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
