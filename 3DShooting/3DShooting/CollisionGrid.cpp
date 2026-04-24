#include "CollisionGrid.h"
#include "Collision.h"
#include "EnemyBase.h"
#include "Game.h"
#include "Player.h"
#include <cmath>
#include <string>
#include <algorithm>

bool CollisionGrid::s_drawGrid = false;

CollisionGrid::CollisionGrid()
    : m_minArea(VGet(0, 0, 0))
    , m_maxArea(VGet(0, 0, 0))
    , m_cellSize(0)
    , m_width(0)
    , m_height(0)
{
}

CollisionGrid::~CollisionGrid()
{
}

void CollisionGrid::Init(const VECTOR& minArea, const VECTOR& maxArea, float cellSize)
{
    m_minArea = minArea;
    m_maxArea = maxArea;
    m_cellSize = cellSize;

    m_width = (int)ceilf((maxArea.x - minArea.x) / cellSize);
    m_height = (int)ceilf((maxArea.z - minArea.z) / cellSize);

    // 境界チェックに余裕を持たせる
    m_width += 2;
    m_height += 2;

    m_cells.assign(m_width * m_height, std::vector<EnemyBase*>());
    m_accessedCells.assign(m_width * m_height, false);
    m_cachedHeights.assign((m_width + 1) * (m_height + 1), minArea.y + 1.0f);
}

void CollisionGrid::Clear()
{
    for (auto& cell : m_cells)
    {
        cell.clear();
    }
}

void CollisionGrid::ResetAccessFlags()
{
    std::fill(m_accessedCells.begin(), m_accessedCells.end(), false);
}

void CollisionGrid::CalculateHeights(const std::vector<Stage::StageCollisionData>& collisionData)
{
    if (collisionData.empty()) return;

    for (int z = 0; z <= m_height; ++z)
    {
        for (int x = 0; x <= m_width; ++x)
        {
            float posX = m_minArea.x + x * m_cellSize;
            float posZ = m_minArea.z + z * m_cellSize;

            VECTOR start = VGet(posX, 10000.0f, posZ);
            VECTOR dir = VGet(0, -1.0f, 0);
            float minDist = FLT_MAX;
            bool hit = false;

            for (const auto& col : collisionData)
            {
                float t;
                if (Collision::IntersectRayTriangle(start, dir, col.v1, col.v2, col.v3, t))
                {
                    if (t < minDist)
                    {
                        minDist = t;
                        hit = true;
                    }
                }
            }
            m_cachedHeights[z * (m_width + 1) + x] = hit ? (start.y - minDist + 1.5f) : (m_minArea.y + 1.0f);
        }
    }
}

void CollisionGrid::RegisterEnemy(EnemyBase* enemy)
{
    if (!enemy)
        return;

    int cellIndex = GetCellIndex(enemy->GetPos());
    if (cellIndex != -1)
    {
        m_cells[cellIndex].push_back(enemy);
    }
}

void CollisionGrid::GetNeighbors(const VECTOR& pos, std::vector<EnemyBase*>& outNeighbors) const
{
    int cx, cz;
    GetCellIndices(pos, cx, cz);

    // 自身が属するセルを中心に、周囲3×3 = 9セル分のエリアを走査
    for (int z = cz - 1; z <= cz + 1; ++z)
    {
        for (int x = cx - 1; x <= cx + 1; ++x)
        {
            if (x >= 0 && x < m_width && z >= 0 && z < m_height)
            {
                int index = z * m_width + x;
                const_cast<CollisionGrid*>(this)->m_accessedCells[index] = true; // アクセス記録
                const auto& targetCell = m_cells[index];
            
                // ベクターの末尾に該当セルの敵リスト（ポインタ）を一括挿入
                outNeighbors.insert(outNeighbors.end(), targetCell.begin(), targetCell.end());
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

void CollisionGrid::Draw(const std::vector<Stage::StageCollisionData>& collisionData) const
{
    if (!s_drawGrid) return;

    // プレイヤーの周囲のみ描画して負荷を抑える
    VECTOR playerPos = VGet(0, 0, 0);
    if (Game::m_pPlayer) playerPos = Game::m_pPlayer->GetPos();
    const float drawRange = 2500.0f; 
    const float drawRangeSq = drawRange * drawRange;

    // 深度テストを有効にし、モデルに隠れるようにする
    SetUseZBuffer3D(true);
    SetWriteZBuffer3D(false);

    unsigned int lineColor = 0x646464;
    unsigned int activeCellColor = 0x00FF00;
    unsigned int searchedCellColor = 0xFFFF00;
    unsigned int textColor = 0xFFFFFF;

    auto GetCachedHeight = [&](int x, int z) {
        if (x < 0 || x > m_width || z < 0 || z > m_height) return m_minArea.y + 1.0f;
        return m_cachedHeights[z * (m_width + 1) + x];
    };

    // グリッド線の描画
    // 縦線
    for (int x = 0; x <= m_width; ++x)
    {
        float posX = m_minArea.x + x * m_cellSize;
        for (int z = 0; z < m_height; ++z)
        {
            float posZ1 = m_minArea.z + z * m_cellSize;
            float posZ2 = m_minArea.z + (z + 1) * m_cellSize;
            
            // プレイヤーからの距離チェック
            float dx = posX - playerPos.x;
            float dz = ((posZ1 + posZ2) * 0.5f) - playerPos.z;
            if (dx * dx + dz * dz > drawRangeSq) continue;

            DrawLine3D(VGet(posX, GetCachedHeight(x, z), posZ1), VGet(posX, GetCachedHeight(x, z + 1), posZ2), lineColor);
        }
    }

    // 横線
    for (int z = 0; z <= m_height; ++z)
    {
        float posZ = m_minArea.z + z * m_cellSize;
        for (int x = 0; x < m_width; ++x)
        {
            float posX1 = m_minArea.x + x * m_cellSize;
            float posX2 = m_minArea.x + (x + 1) * m_cellSize;

            // プレイヤーからの距離チェック
            float dx = ((posX1 + posX2) * 0.5f) - playerPos.x;
            float dz = posZ - playerPos.z;
            if (dx * dx + dz * dz > drawRangeSq) continue;

            DrawLine3D(VGet(posX1, GetCachedHeight(x, z), posZ), VGet(posX2, GetCachedHeight(x + 1, z), posZ), lineColor);
        }
    }

    // 敵がいるセルをハイライト
    for (int z = 0; z < m_height; ++z)
    {
        for (int x = 0; x < m_width; ++x)
        {
            float minX = m_minArea.x + x * m_cellSize;
            float minZ = m_minArea.z + z * m_cellSize;
            
            // プレイヤーからの距離チェック
            float dx = (minX + m_cellSize * 0.5f) - playerPos.x;
            float dz = (minZ + m_cellSize * 0.5f) - playerPos.z;
            if (dx * dx + dz * dz > drawRangeSq) continue;

            int index = z * m_width + x;
            bool hasEnemies = !m_cells[index].empty();
            bool isAccessed = m_accessedCells[index];

            if (hasEnemies || isAccessed)
            {
                float maxX = minX + m_cellSize;
                float maxZ = minZ + m_cellSize;

                float h00 = GetCachedHeight(x, z);
                float h10 = GetCachedHeight(x + 1, z);
                float h11 = GetCachedHeight(x + 1, z + 1);
                float h01 = GetCachedHeight(x, z + 1);

                // 敵が存在するセルは常に明るい緑の枠を表示
                if (hasEnemies)
                {
                    unsigned int brightGreen = 0x00FF00;
                    float offset = 1.0f; // 地面から少し離す
                    DrawLine3D(VGet(minX, h00 + offset, minZ), VGet(maxX, h10 + offset, minZ), brightGreen);
                    DrawLine3D(VGet(maxX, h10 + offset, minZ), VGet(maxX, h11 + offset, maxZ), brightGreen);
                    DrawLine3D(VGet(maxX, h11 + offset, maxZ), VGet(minX, h01 + offset, maxZ), brightGreen);
                    DrawLine3D(VGet(minX, h01 + offset, maxZ), VGet(minX, h00 + offset, minZ), brightGreen);

                    // 太く見せるために少し内側にも描画
                    float inner = 0.5f;
                    DrawLine3D(VGet(minX + inner, h00 + offset, minZ + inner), VGet(maxX - inner, h10 + offset, minZ + inner), brightGreen);
                    DrawLine3D(VGet(maxX - inner, h10 + offset, minZ + inner), VGet(maxX - inner, h11 + offset, maxZ - inner), brightGreen);
                    DrawLine3D(VGet(maxX - inner, h11 + offset, maxZ - inner), VGet(minX + inner, h01 + offset, maxZ - inner), brightGreen);
                    DrawLine3D(VGet(minX + inner, h01 + offset, maxZ - inner), VGet(minX + inner, h00 + offset, minZ + inner), brightGreen);
                }

                // 検索（アクセス）されたセルは黄色い太枠（さらに上の高さ）で表示
                if (isAccessed)
                {
                    float offset = 10.0f; // 緑より高くする
                    DrawLine3D(VGet(minX, h00 + offset, minZ), VGet(maxX, h10 + offset, minZ), searchedCellColor);
                    DrawLine3D(VGet(maxX, h10 + offset, minZ), VGet(maxX, h11 + offset, maxZ), searchedCellColor);
                    DrawLine3D(VGet(maxX, h11 + offset, maxZ), VGet(minX, h01 + offset, maxZ), searchedCellColor);
                    DrawLine3D(VGet(minX, h01 + offset, maxZ), VGet(minX, h00 + offset, minZ), searchedCellColor);
                    
                    // 少しずらして描画して太線に見せる
                    float shift = 1.0f;
                    DrawLine3D(VGet(minX+shift, h00 + offset, minZ+shift), VGet(maxX-shift, h10 + offset, minZ+shift), searchedCellColor);
                    DrawLine3D(VGet(maxX-shift, h10 + offset, minZ+shift), VGet(maxX-shift, h11 + offset, maxZ-shift), searchedCellColor);
                    DrawLine3D(VGet(maxX-shift, h11 + offset, maxZ-shift), VGet(minX+shift, h01 + offset, maxZ-shift), searchedCellColor);
                    DrawLine3D(VGet(minX+shift, h01 + offset, maxZ-shift), VGet(minX+shift, h00 + offset, minZ+shift), searchedCellColor);
                }

                if (hasEnemies)
                {
                    float avgH = (h00 + h10 + h11 + h01) * 0.25f;
                    VECTOR center = VGet((minX + maxX) * 0.5f, avgH + 5.0f, (minZ + maxZ) * 0.5f);
                    char buf[32];
                    snprintf(buf, sizeof(buf), "%d", (int)m_cells[index].size());

                    // カメラの前方にある場合のみ描画
                    VECTOR camPos = GetCameraPosition();
                    VECTOR camTarget = GetCameraTarget();
                    VECTOR camDir = VSub(camTarget, camPos);
                    VECTOR toCenter = VSub(center, camPos);
                    
                    if (VDot(camDir, toCenter) > 0.0f)
                    {
                        VECTOR screenPos = ConvWorldPosToScreenPos(center);
                        DrawString(static_cast<int>(screenPos.x), static_cast<int>(screenPos.y), buf, textColor);
                    }
                }
            }
        }
    }

    // デバッグ用の凡例（説明）を描画
    int screenW = 1280;
    int screenH = 720;
    GetWindowSize(&screenW, &screenH);

    int margin = 20;
    int rectW = 240;
    int rectH = 90; // 高さを少し増やす
    int x = screenW - rectW - margin;
    int y = screenH - rectH - margin - 100;

    // 半透明の背景
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
    DrawBox(x, y, x + rectW, y + rectH, 0x000000, TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    DrawBox(x, y, x + rectW, y + rectH, 0xffffff, FALSE);

    // 凡例テキスト
    int textX = x + 10;
    int textY = y + 10;
    DrawBox(textX, textY + 2, textX + 12, textY + 14, 0x00FF00, TRUE);
    DrawString(textX + 20, textY, "：敵が存在するセル", 0xffffff);

    textY += 25;
    DrawBox(textX, textY + 2, textX + 12, textY + 14, 0xFFFF00, TRUE);
    DrawString(textX + 20, textY, "：検索・アクセス範囲", 0xffffff);

    textY += 25;
    DrawString(textX, textY, "数字：セル内の敵の数", 0xffffff);
}
