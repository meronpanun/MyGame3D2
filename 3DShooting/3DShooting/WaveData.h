#pragma once
#include <string>

// Waveデータの構造体
struct WaveData
{
  int wave = 0;            // ウェーブ番号
  std::string enemyType;   // 敵の種類
  int count = 0;           // 出現数
  float spawnInterval = 0; // 出現間隔
  float startTime = 0;     // 出現開始時間
  float waveInterval = 0;  // ウェーブ間インターバル
  int spawnLocationType = 0; // スポーン位置タイプ (0:ランダム, 1:下段, 2:中段, 3:上段)
};
