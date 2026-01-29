#pragma once
#include "DxLib.h"
#include <vector>

class EnemyBase;

class CollisionGrid {
public:
  CollisionGrid();
  ~CollisionGrid();

  /// <summary>
  /// グリッドの初期化
  /// </summary>
  /// <param name="minArea">領域の最小座標</param>
  /// <param name="maxArea">領域の最大座標</param>
  /// <param name="cellSize">セルのサイズ</param>
  void Init(const VECTOR &minArea, const VECTOR &maxArea, float cellSize);

  /// <summary>
  /// グリッドのクリア
  /// </summary>
  void Clear();

  /// <summary>
  /// 敵をグリッドに登録
  /// </summary>
  /// <param name="enemy">登録する敵</param>
  void RegisterEnemy(EnemyBase *enemy);

  /// <summary>
  /// 指定位置周辺の敵を取得
  /// </summary>
  /// <param name="pos">検索中心座標</param>
  /// <param name="outNeighbors">結果を格納するベクタ</param>
  void GetNeighbors(const VECTOR &pos,
                    std::vector<EnemyBase *> &outNeighbors) const;

private:
  int GetCellIndex(const VECTOR &pos) const;
  void GetCellIndices(const VECTOR &pos, int &x, int &z) const;

private:
  std::vector<std::vector<EnemyBase *>> m_cells;
  VECTOR m_minArea;
  VECTOR m_maxArea;
  float m_cellSize;
  int m_width;
  int m_height;
};
