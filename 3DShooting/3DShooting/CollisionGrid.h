#pragma once
#include "DxLib.h"
#include "Stage.h"
#include <vector>

class EnemyBase;

/// <summary>
/// ステージポリゴンと敵を空間分割グリッドで管理するクラス。
/// セル単位でポリゴン・敵を登録し、指定座標周辺のみを高速に検索できる。
/// デバッグ描画・パフォーマンス計測機能も備える。
/// </summary>
class CollisionGrid
{
public:
    CollisionGrid();
    ~CollisionGrid() = default;

    /// <summary>
    /// グリッドを初期化する
    /// </summary>
    /// <param name="minArea">グリッド領域の最小座標</param>
    /// <param name="maxArea">グリッド領域の最大座標</param>
    /// <param name="cellSize">1 セルのサイズ</param>
    void Init(const VECTOR& minArea, const VECTOR& maxArea, float cellSize);

    /// <summary>
    /// 全セルの敵・ポリゴンをクリアする
    /// </summary>
    void Clear();

    /// <summary>
    /// 敵セルのみをクリアする
    /// </summary>
    void ClearEnemies();

    /// <summary>
    /// デバッグ用アクセスフラグを 1 フレーム分減衰させる
    /// </summary>
    void ResetAccessFlags();

    /// <summary>
    /// パフォーマンス計測カウンタをリセットする
    /// </summary>
    void ResetStats();

    /// <summary>
    /// グリッド頂点ごとの地形高さをレイキャストで事前計算してキャッシュする
    /// </summary>
    /// <param name="collisionData">高さ計算に使用するステージポリゴンデータ一覧</param>
    void CalculateHeights(const std::vector<Stage::StageCollisionData>& collisionData);

    /// <summary>
    /// ステージの三角形ポリゴンをバウンディングボックスが重なる全セルに登録する
    /// </summary>
    /// <param name="triangle">登録するポリゴンデータ</param>
    void RegisterStageTriangle(const Stage::StageCollisionData& triangle);

    /// <summary>
    /// 敵をその現在位置に対応するセルに登録する
    /// </summary>
    /// <param name="enemy">登録する敵（nullptr の場合は無視）</param>
    void RegisterEnemy(EnemyBase* enemy);

    /// <summary>
    /// 指定位置の周囲 3×3 セル内にいる敵を取得する
    /// </summary>
    /// <param name="pos">検索の中心座標</param>
    /// <param name="outNeighbors">結果を格納するベクタ</param>
    /// <param name="persist">true の場合アクセスフラグを複数フレーム維持する（デバッグ用）</param>
    void GetNeighbors(const VECTOR& pos, std::vector<EnemyBase*>& outNeighbors, bool persist = true) const;

    /// <summary>
    /// 指定位置の周囲 3×3 セル内にあるステージポリゴンを取得する
    /// </summary>
    /// <param name="pos">検索の中心座標</param>
    /// <param name="outTriangles">結果を格納するベクタ（重複は除去される）</param>
    void GetNearbyTriangles(const VECTOR& pos, std::vector<const Stage::StageCollisionData*>& outTriangles) const;

    /// <summary>
    /// グリッドを 3D ワールド上にデバッグ描画する（グリッド線・アクセスセルのハイライト）
    /// </summary>
    /// <param name="collisionData">（現在未使用）ステージポリゴンデータ</param>
    void Draw(const std::vector<Stage::StageCollisionData>& collisionData = {}) const;

    /// <summary>
    /// 凡例とパフォーマンス統計を 2D UI として描画する
    /// </summary>
    void DrawUI() const;

    /// <summary>
    /// グリッドデバッグ描画のオン/オフを設定する
    /// </summary>
    /// <param name="draw">true で描画有効</param>
    static void SetDrawGrid(bool draw) { s_drawGrid = draw; }

    /// <summary>
    /// グリッドデバッグ描画が有効かどうかを返す
    /// </summary>
    /// <returns>有効なら true</returns>
    static bool IsDrawGrid() { return s_drawGrid; }

    /// <summary>
    /// 空間分割の使用/不使用を設定する
    /// </summary>
    /// <param name="use">true で空間分割を使用（false で総当たり）</param>
    static void SetUseSpatialPartitioning(bool use) { s_useSpatialPartitioning = use; }

    /// <summary>
    /// 空間分割が有効かどうかを返す
    /// </summary>
    /// <returns>有効なら true</returns>
    static bool IsUseSpatialPartitioning() { return s_useSpatialPartitioning; }

    /// <summary>
    /// 累計クエリ数を取得する
    /// </summary>
    /// <returns>累計クエリ数</returns>
    int GetTotalQueries() const { return m_totalQueries; }

    /// <summary>
    /// 累計判定対象エンティティ数を取得する
    /// </summary>
    /// <returns>累計判定対象エンティティ数</returns>
    int GetTotalEntitiesChecked() const { return m_totalEntitiesChecked; }

    /// <summary>
    /// 登録済み敵の総数を取得する
    /// </summary>
    /// <returns>登録済み敵の総数</returns>
    int GetTotalEnemies() const { return m_totalEnemies; }

    /// <summary>
    /// 登録済み敵の総数を設定する
    /// </summary>
    /// <param name="count">設定する敵数</param>
    void SetTotalEnemies(int count) { m_totalEnemies = count; }

    /// <summary>
    /// 累計検索処理時間を取得する（マイクロ秒単位）
    /// </summary>
    /// <returns>累計検索処理時間</returns>
    LONGLONG GetTotalSearchTime() const { return m_totalSearchTime; }

private:
    /// <summary>
    /// 座標に対応するフラット配列のセルインデックスを返す
    /// </summary>
    /// <param name="pos">検索座標</param>
    /// <returns>セルインデックス。範囲外の場合は -1</returns>
    int GetCellIndex(const VECTOR& pos) const;

    /// <summary>
    /// 座標をグリッドの X・Z セル番号に変換する
    /// </summary>
    /// <param name="pos">変換する座標</param>
    /// <param name="x">X 方向のセル番号（出力）</param>
    /// <param name="z">Z 方向のセル番号（出力）</param>
    void GetCellIndices(const VECTOR& pos, int& x, int& z) const;

private:
    std::vector<std::vector<EnemyBase*>>                       m_cells;         // 敵管理セル配列
    std::vector<std::vector<const Stage::StageCollisionData*>> m_stageCells;    // ステージポリゴン管理セル配列
    mutable std::vector<int>                                   m_accessedCells; // アクセスフラグ（デバッグ用）
    std::vector<float>                                         m_cachedHeights; // グリッド頂点の地形高さキャッシュ

    VECTOR m_minArea;  // グリッド領域の最小座標
    VECTOR m_maxArea;  // グリッド領域の最大座標
    float  m_cellSize; // 1 セルのサイズ
    int    m_width;    // X 方向のセル数
    int    m_height;   // Z 方向のセル数

    // パフォーマンス計測
    mutable int      m_totalQueries;         // 累計クエリ数
    mutable int      m_totalEntitiesChecked; // 累計判定対象エンティティ数
    mutable LONGLONG m_totalSearchTime;      // 累計検索処理時間（マイクロ秒）
    mutable LONGLONG m_displayedSearchTime;  // 表示用にホールドする検索処理時間
    mutable int      m_displayTimer;         // パフォーマンス表示の更新タイマー
    int              m_totalEnemies;         // 登録済み敵の総数

    // 三角形検索パフォーマンス計測
    mutable int      m_totalTriangleQueries;         // 累計三角形クエリ数
    mutable int      m_totalTrianglesReturned;       // 累計返却三角形数
    mutable LONGLONG m_totalTriangleSearchTime;      // 累計三角形検索処理時間（マイクロ秒）
    mutable LONGLONG m_displayedTriangleSearchTime;  // 表示用にホールドする三角形検索処理時間
    int              m_registeredTriangleCount;      // 登録済みユニーク三角形総数
    mutable unsigned int m_triQueryStamp;            // GetNearbyTriangles 重複除去用のクエリ世代カウンタ

    static bool s_drawGrid;               // グリッドデバッグ描画フラグ
    static bool s_useSpatialPartitioning; // 空間分割使用フラグ
};
