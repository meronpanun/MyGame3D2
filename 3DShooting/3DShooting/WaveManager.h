#pragma once
#include <vector>
#include <memory>
#include <string>
#include <functional>
#include "DxLib.h"
#include "Player.h"
#include "Bullet.h"
#include "EnemyBase.h"

//class EnemyBase;
class EnemyNormal;
class EnemyRunner;
class EnemyAcid;

// Waveデータの構造体
struct WaveData
{
    int wave;
    std::string enemyType;
    int count;
    float spawnInterval;
    float delay;
};

// 敵の出現情報
struct EnemySpawnInfo
{
    std::string enemyType;
    VECTOR spawnPosition;
    float spawnTime;
    bool isSpawned;
};

class WaveManager
{
public:
    WaveManager();
    ~WaveManager();

    void Init();
    void Update();
    void Draw();

    // 現在のwaveを取得
    int GetCurrentWave() const { return m_currentWave; }

    // 敵のリストを取得
    std::vector<std::shared_ptr<EnemyBase>>& GetEnemyList() { return m_enemyList; }

    // 敵の死亡コールバックを設定
    void SetOnEnemyDeathCallback(std::function<void(const VECTOR&)> callback);

    // 敵ヒット時コールバックを設定
    void SetOnEnemyHitCallback(std::function<void(EnemyBase::HitPart)> cb) { m_onEnemyHitCallback = cb; }

    // Road_floorオブジェクトの範囲を設定
    void SetRoadFloorBounds(const VECTOR& minPos, const VECTOR& maxPos);

    // デバッグ情報を表示
    void DrawDebugInfo();

    // 敵の一括更新
    void UpdateEnemies(std::vector<Bullet>& bullets, const Player::TackleInfo& tackleInfo, const Player& player);

    // 敵の一括描画
    void DrawEnemies();

private:
    // CSVファイルからWaveDataを読み込み
    void LoadWaveData();

    // ランダムな出現位置を生成
    VECTOR GenerateRandomSpawnPosition(const VECTOR& playerPos);

    // 敵を生成
    std::shared_ptr<EnemyBase> CreateEnemy(const std::string& enemyType);

    // 次のwaveに進む
    void NextWave();

    // 現在のwaveを開始
    void StartCurrentWave(const VECTOR& playerPos = VGet(0.0f, 0.0f, 0.0f));

    // 現在のwaveの敵がすべて倒されたかチェック
    bool IsCurrentWaveCleared();

    // 敵の死亡処理
    void OnEnemyDeath(const VECTOR& position);

private:
    std::vector<WaveData> m_waveDataList;
    std::vector<EnemySpawnInfo> m_spawnInfoList;
    std::vector<std::shared_ptr<EnemyBase>> m_enemyList;

    int m_currentWave;
    float m_waveTimer;
    float m_spawnTimer;
    int m_currentSpawnIndex;

    bool m_isWaveActive;
    bool m_isAllWavesCompleted;

    // Road_floorオブジェクトの範囲
    VECTOR m_roadFloorMin;
    VECTOR m_roadFloorMax;
    bool m_isRoadFloorBoundsSet;

    // コールバック
    std::function<void(const VECTOR&)> m_onEnemyDeathCallback;
    std::function<void(EnemyBase::HitPart)> m_onEnemyHitCallback; // 部位情報付き

    // 敵のテンプレート
    std::shared_ptr<EnemyNormal> m_enemyNormalTemplate;
    std::shared_ptr<EnemyRunner> m_enemyRunnerTemplate;
    std::shared_ptr<EnemyAcid> m_enemyAcidTemplate;

    std::vector<std::shared_ptr<EnemyNormal>> m_enemyNormalPool;
    std::vector<std::shared_ptr<EnemyRunner>> m_enemyRunnerPool;
    std::vector<std::shared_ptr<EnemyAcid>> m_enemyAcidPool;

    std::shared_ptr<EnemyNormal> GetPooledNormalEnemy();
    std::shared_ptr<EnemyRunner> GetPooledRunnerEnemy();
    std::shared_ptr<EnemyAcid> GetPooledAcidEnemy();
};