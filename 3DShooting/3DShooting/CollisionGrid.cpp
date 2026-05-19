#include "CollisionGrid.h"
#include "Collision.h"
#include "EnemyBase.h"
#include "Game.h"
#include "Player.h"
#include <cmath>
#include <string>
#include <algorithm>

namespace
{
    // グリッド構造
    constexpr int   kGridBorderMargin          = 2;        // グリッド境界の余白セル数
    constexpr float kRaycastStartY             = 10000.0f; // 高さ計算用レイキャストの開始 Y 座標
    constexpr float kCachedHeightBaseOffset    = 1.0f;     // レイが当たらなかった場合の高さオフセット
    constexpr float kCachedHeightHitOffset     = 1.5f;     // レイヒット時の高さオフセット
    constexpr int   kAccessFlagPersistDuration = 30;       // persist モードのアクセスフラグ維持フレーム数
    constexpr int   kDisplayTimerInterval      = 30;       // パフォーマンス表示の更新間隔（フレーム）

    // グリッドデバッグ描画（3D）
    constexpr float kDrawRange                 = 2500.0f;  // グリッドデバッグ描画の最大半径
    constexpr float kEnemyCountTextOffset      = 5.0f;     // 敵数テキストのセル平均高さからのオフセット

    // デバッグ UI 共通
    constexpr int   kDefaultScreenWidth        = 1280;     // デフォルト画面幅
    constexpr int   kDefaultScreenHeight       = 720;      // デフォルト画面高さ
    constexpr int   kUIAlpha                   = 180;      // デバッグ UI 背景の不透明度
    constexpr int   kUIPanelMargin             = 20;       // パネル外側マージン
    constexpr int   kUIBottomOffset            = 100;      // 下部ゲーム HUD 分の追加マージン

    // 凡例パネル
    constexpr int   kLegendPanelWidth          = 240;      // 凡例パネルの幅
    constexpr int   kLegendPanelHeight         = 90;       // 凡例パネルの高さ

    // 統計パネル
    constexpr int   kStatPanelWidth            = 320;      // 統計パネルの幅
    constexpr int   kStatPanelHeight           = 150;      // 統計パネルの高さ

    // テキスト・アイコンレイアウト
    constexpr int   kUITextPaddingX            = 10;       // パネル内テキストの X パディング
    constexpr int   kUITextPaddingY            = 10;       // パネル内テキストの Y パディング
    constexpr int   kUIIconSize                = 12;       // 凡例アイコンの一辺サイズ（px）
    constexpr int   kUIIconOffsetY             = 2;        // 凡例アイコンの Y オフセット
    constexpr int   kUIIconLabelGap            = 20;       // アイコンとラベルの水平間隔
    constexpr int   kUILineSpacingLarge        = 25;       // 大きい行間隔（px）
    constexpr int   kUILineSpacingSmall        = 20;       // 小さい行間隔（px）

    // デバッグ描画・UI 色
    constexpr unsigned int kColorGridLine     = 0x646464; // グリッド線
    constexpr unsigned int kColorEnemyCell    = 0x00FF00; // 敵ありセルの枠・UI アイコン・統計良好色
    constexpr unsigned int kColorSearchedCell = 0xFFFF00; // アクセスセルの枠・UI アイコン・時間テキスト
    constexpr unsigned int kColorDebugText    = 0xFFFFFF; // デバッグテキスト・UI 枠線・一般テキスト
    constexpr unsigned int kColorUIBackground = 0x000000; // UI パネル背景
    constexpr unsigned int kColorUIHeader     = 0x00FFFF; // UI ヘッダーテキスト
    constexpr unsigned int kColorStatWarn     = 0xFFAA00; // 統計警告色（総当たりモード）
}

bool CollisionGrid::s_drawGrid = false;
bool CollisionGrid::s_useSpatialPartitioning = true;

CollisionGrid::CollisionGrid()
    : m_minArea(VGet(0, 0, 0))
    , m_maxArea(VGet(0, 0, 0))
    , m_cellSize(0)
    , m_width(0)
    , m_height(0)
    , m_totalQueries(0)
    , m_totalEntitiesChecked(0)
    , m_totalSearchTime(0)
    , m_displayedSearchTime(0)
    , m_displayTimer(0)
    , m_totalEnemies(0)
{
}

void CollisionGrid::Init(const VECTOR& minArea, const VECTOR& maxArea, float cellSize)
{
    m_minArea  = minArea;
    m_maxArea  = maxArea;
    m_cellSize = cellSize;

    m_width  = static_cast<int>(ceilf((maxArea.x - minArea.x) / cellSize));
    m_height = static_cast<int>(ceilf((maxArea.z - minArea.z) / cellSize));

    // 境界チェックに余裕を持たせる
    m_width  += kGridBorderMargin;
    m_height += kGridBorderMargin;

    m_cells.assign(m_width * m_height, std::vector<EnemyBase*>());
    m_stageCells.assign(m_width * m_height, std::vector<const Stage::StageCollisionData*>());
    m_accessedCells.assign(m_width * m_height, 0);
    m_cachedHeights.assign((m_width + 1) * (m_height + 1), minArea.y + kCachedHeightBaseOffset);
}

void CollisionGrid::Clear()
{
    for (auto& cell : m_cells)      cell.clear();
    for (auto& cell : m_stageCells) cell.clear();
}

void CollisionGrid::ClearEnemies()
{
    for (auto& cell : m_cells) cell.clear();
}

void CollisionGrid::ResetAccessFlags()
{
    for (auto& val : m_accessedCells)
    {
        if (val > 0) val--;
    }
}

void CollisionGrid::ResetStats()
{
    m_totalQueries         = 0;
    m_totalEntitiesChecked = 0;
    m_totalSearchTime      = 0;
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

            VECTOR start   = VGet(posX, kRaycastStartY, posZ);
            VECTOR dir     = VGet(0.0f, -1.0f, 0.0f);
            float  minDist = FLT_MAX;
            bool   hit     = false;

            for (const auto& col : collisionData)
            {
                float t;
                if (Collision::IntersectRayTriangle(start, dir, col.v1, col.v2, col.v3, t))
                {
                    if (t < minDist)
                    {
                        minDist = t;
                        hit     = true;
                    }
                }
            }
            m_cachedHeights[z * (m_width + 1) + x] =
                hit ? (start.y - minDist + kCachedHeightHitOffset) : (m_minArea.y + kCachedHeightBaseOffset);
        }
    }
}

void CollisionGrid::RegisterStageTriangle(const Stage::StageCollisionData& triangle)
{
    // 三角形のバウンディングボックスを計算
    float minX = (std::min)({ triangle.v1.x, triangle.v2.x, triangle.v3.x });
    float maxX = (std::max)({ triangle.v1.x, triangle.v2.x, triangle.v3.x });
    float minZ = (std::min)({ triangle.v1.z, triangle.v2.z, triangle.v3.z });
    float maxZ = (std::max)({ triangle.v1.z, triangle.v2.z, triangle.v3.z });

    // バウンディングボックスが重なる全セルを特定
    int startX, startZ, endX, endZ;
    GetCellIndices(VGet(minX, 0, minZ), startX, startZ);
    GetCellIndices(VGet(maxX, 0, maxZ), endX, endZ);

    // 範囲をグリッド内に収める
    startX = (std::max)(0, startX);
    startZ = (std::max)(0, startZ);
    endX   = (std::min)(m_width  - 1, endX);
    endZ   = (std::min)(m_height - 1, endZ);

    for (int z = startZ; z <= endZ; ++z)
    {
        for (int x = startX; x <= endX; ++x)
        {
            m_stageCells[z * m_width + x].push_back(&triangle);
        }
    }
}

void CollisionGrid::RegisterEnemy(EnemyBase* enemy)
{
    if (!enemy) return;

    int cellIndex = GetCellIndex(enemy->GetPos());
    if (cellIndex != -1)
    {
        m_cells[cellIndex].push_back(enemy);
        m_totalEnemies++;
    }
}

void CollisionGrid::GetNeighbors(const VECTOR& pos, std::vector<EnemyBase*>& outNeighbors, bool persist) const
{
    LONGLONG startTime = GetNowHiPerformanceCount();
    m_totalQueries++;

    // 空間分割を使用しない場合は総当たり
    if (!s_useSpatialPartitioning)
    {
        for (int z = 0; z < m_height; ++z)
        {
            for (int x = 0; x < m_width; ++x)
            {
                const auto& targetCell = m_cells[z * m_width + x];
                m_totalEntitiesChecked += static_cast<int>(targetCell.size());
                outNeighbors.insert(outNeighbors.end(), targetCell.begin(), targetCell.end());
            }
        }
        m_totalSearchTime += GetNowHiPerformanceCount() - startTime;
        return;
    }

    int cx, cz;
    GetCellIndices(pos, cx, cz);

    // 指定位置の周囲 3×3 セルをチェック
    for (int z = cz - 1; z <= cz + 1; ++z)
    {
        for (int x = cx - 1; x <= cx + 1; ++x)
        {
            if (x < 0 || x >= m_width || z < 0 || z >= m_height) continue;

            int index = z * m_width + x;
            // persist=true なら複数フレーム維持、false なら即時クリア対象（デバッグ用）
            const_cast<CollisionGrid*>(this)->m_accessedCells[index] = persist ? kAccessFlagPersistDuration : 1;

            const auto& targetCell = m_cells[index];
            m_totalEntitiesChecked += static_cast<int>(targetCell.size());
            outNeighbors.insert(outNeighbors.end(), targetCell.begin(), targetCell.end());
        }
    }
    m_totalSearchTime += GetNowHiPerformanceCount() - startTime;
}

void CollisionGrid::GetNearbyTriangles(const VECTOR& pos, std::vector<const Stage::StageCollisionData*>& outTriangles) const
{
    if (!s_useSpatialPartitioning)
    {
        for (const auto& cell : m_stageCells)
        {
            outTriangles.insert(outTriangles.end(), cell.begin(), cell.end());
        }
        return;
    }

    int cx, cz;
    GetCellIndices(pos, cx, cz);

    for (int z = cz - 1; z <= cz + 1; ++z)
    {
        for (int x = cx - 1; x <= cx + 1; ++x)
        {
            if (x < 0 || x >= m_width || z < 0 || z >= m_height) continue;

            const auto& targetCell = m_stageCells[z * m_width + x];
            outTriangles.insert(outTriangles.end(), targetCell.begin(), targetCell.end());
        }
    }

    // 同一三角形が複数セルにまたがる場合の重複を削除
    std::sort(outTriangles.begin(), outTriangles.end());
    outTriangles.erase(std::unique(outTriangles.begin(), outTriangles.end()), outTriangles.end());
}

int CollisionGrid::GetCellIndex(const VECTOR& pos) const
{
    int x, z;
    GetCellIndices(pos, x, z);
    if (x < 0 || x >= m_width || z < 0 || z >= m_height) return -1;
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
    VECTOR     playerPos   = Game::m_pPlayer ? Game::m_pPlayer->GetPos() : VGet(0, 0, 0);
    const float drawRangeSq = kDrawRange * kDrawRange;

    // 深度テストを有効にし、モデルに隠れるようにする
    SetUseZBuffer3D(true);
    SetWriteZBuffer3D(false);


    auto GetCachedHeight = [&](int x, int z) -> float
    {
        if (x < 0 || x > m_width || z < 0 || z > m_height) return m_minArea.y + kCachedHeightBaseOffset;
        return m_cachedHeights[z * (m_width + 1) + x];
    };

    // 縦のグリッド線
    for (int x = 0; x <= m_width; ++x)
    {
        float posX = m_minArea.x + x * m_cellSize;
        for (int z = 0; z < m_height; ++z)
        {
            float posZ1 = m_minArea.z + z * m_cellSize;
            float posZ2 = m_minArea.z + (z + 1) * m_cellSize;

            float dx = posX - playerPos.x;
            float dz = ((posZ1 + posZ2) * 0.5f) - playerPos.z;
            if (dx * dx + dz * dz > drawRangeSq) continue;

            DrawLine3D(VGet(posX, GetCachedHeight(x, z), posZ1), VGet(posX, GetCachedHeight(x, z + 1), posZ2), kColorGridLine);
        }
    }

    // 横のグリッド線
    for (int z = 0; z <= m_height; ++z)
    {
        float posZ = m_minArea.z + z * m_cellSize;
        for (int x = 0; x < m_width; ++x)
        {
            float posX1 = m_minArea.x + x * m_cellSize;
            float posX2 = m_minArea.x + (x + 1) * m_cellSize;

            float dx = ((posX1 + posX2) * 0.5f) - playerPos.x;
            float dz = posZ - playerPos.z;
            if (dx * dx + dz * dz > drawRangeSq) continue;

            DrawLine3D(VGet(posX1, GetCachedHeight(x, z), posZ), VGet(posX2, GetCachedHeight(x + 1, z), posZ), kColorGridLine);
        }
    }

    // 敵が存在するセル・アクセスされたセルをハイライト
    for (int z = 0; z < m_height; ++z)
    {
        for (int x = 0; x < m_width; ++x)
        {
            float minX = m_minArea.x + x * m_cellSize;
            float minZ = m_minArea.z + z * m_cellSize;

            float dx = (minX + m_cellSize * 0.5f) - playerPos.x;
            float dz = (minZ + m_cellSize * 0.5f) - playerPos.z;
            if (dx * dx + dz * dz > drawRangeSq) continue;

            int  index      = z * m_width + x;
            bool hasEnemies = !m_cells[index].empty();
            bool isAccessed = m_accessedCells[index] > 0;

            if (!hasEnemies && !isAccessed) continue;

            float maxX = minX + m_cellSize;
            float maxZ = minZ + m_cellSize;
            float h00  = GetCachedHeight(x,     z);
            float h10  = GetCachedHeight(x + 1, z);
            float h11  = GetCachedHeight(x + 1, z + 1);
            float h01  = GetCachedHeight(x,     z + 1);

            // 敵が存在するセルは明るい緑の二重枠で表示
            if (hasEnemies)
            {
                const float offset = 1.0f;
                const float inner  = 0.5f;
                DrawLine3D(VGet(minX, h00 + offset, minZ), VGet(maxX, h10 + offset, minZ), kColorEnemyCell);
                DrawLine3D(VGet(maxX, h10 + offset, minZ), VGet(maxX, h11 + offset, maxZ), kColorEnemyCell);
                DrawLine3D(VGet(maxX, h11 + offset, maxZ), VGet(minX, h01 + offset, maxZ), kColorEnemyCell);
                DrawLine3D(VGet(minX, h01 + offset, maxZ), VGet(minX, h00 + offset, minZ), kColorEnemyCell);

                DrawLine3D(VGet(minX + inner, h00 + offset, minZ + inner), VGet(maxX - inner, h10 + offset, minZ + inner), kColorEnemyCell);
                DrawLine3D(VGet(maxX - inner, h10 + offset, minZ + inner), VGet(maxX - inner, h11 + offset, maxZ - inner), kColorEnemyCell);
                DrawLine3D(VGet(maxX - inner, h11 + offset, maxZ - inner), VGet(minX + inner, h01 + offset, maxZ - inner), kColorEnemyCell);
                DrawLine3D(VGet(minX + inner, h01 + offset, maxZ - inner), VGet(minX + inner, h00 + offset, minZ + inner), kColorEnemyCell);
            }

            // アクセスされたセルは黄色の二重枠で表示
            if (isAccessed)
            {
                const float offset = 10.0f;
                const float shift  = 1.0f;
                DrawLine3D(VGet(minX,        h00 + offset, minZ),        VGet(maxX,        h10 + offset, minZ),        kColorSearchedCell);
                DrawLine3D(VGet(maxX,        h10 + offset, minZ),        VGet(maxX,        h11 + offset, maxZ),        kColorSearchedCell);
                DrawLine3D(VGet(maxX,        h11 + offset, maxZ),        VGet(minX,        h01 + offset, maxZ),        kColorSearchedCell);
                DrawLine3D(VGet(minX,        h01 + offset, maxZ),        VGet(minX,        h00 + offset, minZ),        kColorSearchedCell);

                DrawLine3D(VGet(minX + shift, h00 + offset, minZ + shift), VGet(maxX - shift, h10 + offset, minZ + shift), kColorSearchedCell);
                DrawLine3D(VGet(maxX - shift, h10 + offset, minZ + shift), VGet(maxX - shift, h11 + offset, maxZ - shift), kColorSearchedCell);
                DrawLine3D(VGet(maxX - shift, h11 + offset, maxZ - shift), VGet(minX + shift, h01 + offset, maxZ - shift), kColorSearchedCell);
                DrawLine3D(VGet(minX + shift, h01 + offset, maxZ - shift), VGet(minX + shift, h00 + offset, minZ + shift), kColorSearchedCell);
            }

            // 敵数をセル中央に表示（カメラ前方のみ）
            if (hasEnemies)
            {
                float  avgH    = (h00 + h10 + h11 + h01) * 0.25f;
                VECTOR center  = VGet((minX + maxX) * 0.5f, avgH + kEnemyCountTextOffset, (minZ + maxZ) * 0.5f);
                char   buf[32];
                snprintf(buf, sizeof(buf), "%d", static_cast<int>(m_cells[index].size()));

                VECTOR camPos    = GetCameraPosition();
                VECTOR camTarget = GetCameraTarget();
                VECTOR camDir    = VSub(camTarget, camPos);
                VECTOR toCenter  = VSub(center, camPos);

                if (VDot(camDir, toCenter) > 0.0f)
                {
                    VECTOR screenPos = ConvWorldPosToScreenPos(center);
                    DrawString(static_cast<int>(screenPos.x), static_cast<int>(screenPos.y), buf, kColorDebugText);
                }
            }
        }
    }
}

void CollisionGrid::DrawUI() const
{
    if (!s_drawGrid) return;

    // 2D UI 描画時は深度テストを無効化
    SetUseZBuffer3D(false);
    SetWriteZBuffer3D(false);

    int screenW = kDefaultScreenWidth;
    int screenH = kDefaultScreenHeight;
    GetWindowSize(&screenW, &screenH);

    // 凡例パネル
    const int x = screenW - kLegendPanelWidth - kUIPanelMargin;
    const int y = screenH - kLegendPanelHeight - kUIPanelMargin - kUIBottomOffset;

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, kUIAlpha);
    DrawBox(x, y, x + kLegendPanelWidth, y + kLegendPanelHeight, kColorUIBackground, true);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    DrawBox(x, y, x + kLegendPanelWidth, y + kLegendPanelHeight, kColorDebugText, false);

    int textX = x + kUITextPaddingX;
    int textY = y + kUITextPaddingY;
    DrawBox(textX, textY + kUIIconOffsetY, textX + kUIIconSize, textY + kUIIconOffsetY + kUIIconSize, kColorEnemyCell, true);
    DrawString(textX + kUIIconLabelGap, textY, "：敵が存在するセル", kColorDebugText);

    textY += kUILineSpacingLarge;
    DrawBox(textX, textY + kUIIconOffsetY, textX + kUIIconSize, textY + kUIIconOffsetY + kUIIconSize, kColorSearchedCell, true);
    DrawString(textX + kUIIconLabelGap, textY, "：検索・アクセス範囲", kColorDebugText);

    textY += kUILineSpacingLarge;
    DrawString(textX, textY, "数字：セル内の敵の数", kColorDebugText);

    // パフォーマンス統計パネル
    const int sx = kUIPanelMargin;
    const int sy = screenH - kStatPanelHeight - kUIPanelMargin - kUIBottomOffset;

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, kUIAlpha);
    DrawBox(sx, sy, sx + kStatPanelWidth, sy + kStatPanelHeight, kColorUIBackground, true);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    DrawBox(sx, sy, sx + kStatPanelWidth, sy + kStatPanelHeight, kColorDebugText, false);

    int sTextX = sx + kUITextPaddingX;
    int sTextY = sy + kUITextPaddingY;
    DrawString(sTextX, sTextY, "【空間分割パフォーマンス評価】", kColorUIHeader);

    sTextY += kUILineSpacingLarge;
    DrawFormatString(sTextX, sTextY, kColorDebugText, "現在のモード: %s", s_useSpatialPartitioning ? "空間分割 (ON)" : "総当たり (OFF)");

    sTextY += kUILineSpacingSmall;
    DrawFormatString(sTextX, sTextY, kColorDebugText, "総敵数: %d / クエリ数: %d", m_totalEnemies, m_totalQueries);

    sTextY += kUILineSpacingSmall;
    int   fullScanChecks = m_totalEnemies * m_totalQueries;
    float reduction      = 0.0f;
    if (fullScanChecks > 0)
    {
        reduction = (1.0f - static_cast<float>(m_totalEntitiesChecked) / static_cast<float>(fullScanChecks)) * 100.0f;
    }

    unsigned int statColor = s_useSpatialPartitioning ? kColorEnemyCell : kColorStatWarn;
    DrawFormatString(sTextX, sTextY, statColor, "判定対象削減率: %.1f%%", s_useSpatialPartitioning ? reduction : 0.0f);

    sTextY += kUILineSpacingSmall;
    DrawFormatString(sTextX, sTextY, kColorDebugText, "実判定数: %d (総当りなら: %d)", m_totalEntitiesChecked, fullScanChecks);

    sTextY += kUILineSpacingLarge;

    // 表示更新頻度を落とす
    m_displayTimer--;
    if (m_displayTimer <= 0)
    {
        m_displayedSearchTime = m_totalSearchTime;
        m_displayTimer        = kDisplayTimerInterval;
    }
    DrawFormatString(sTextX, sTextY, kColorSearchedCell, "検索処理時間: %lld us (マイクロ秒)", m_displayedSearchTime);
}
