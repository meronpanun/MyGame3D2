#pragma once
#include <string>

/// <summary>
/// 1 ウェーブ分の敵出現設定を保持する構造体。
/// CSV から読み込まれ、WaveManager が EnemySpawnInfo を生成する際の「設計図」として使用する。
/// </summary>
struct WaveData
{
    int         wave              = 0;       // ウェーブ番号
    std::string enemyType;                   // 敵の種類識別子
    int         count             = 0;       // 出現数
    float       spawnInterval     = 0.0f;   // 敵を 1 体ずつ出現させる間隔（秒）
    float       startTime         = 0.0f;   // ウェーブ開始からスポーン開始までの遅延（秒）
    float       waveInterval      = 0.0f;   // 次ウェーブまでのインターバル（秒）
    int         spawnLocationType = 0;       // スポーン位置タイプ（0: ランダム, 1: 下段, 2: 中段, 3: 上段）
    bool        hasShield         = false;  // シールドを持って出現するかどうか
};
