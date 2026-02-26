#pragma once
#include "CollisionGrid.h"
#include "DxLib.h"
#include "EnemyBase.h"
#include "SpawnAreaInfo.h"
#include "TransformDataLoader.h"
#include "WaveData.h"
#include "WaveUI.h"
#include <functional>
#include <memory>
#include <string>
#include <vector>

class EnemyNormal;
class EnemyRunner;
class EnemyAcid;
class EnemyBoss;
class Bullet;
class Player;
class Effect;

// 敵の出現情報
// ゲーム実行時に生成される「個別の敵の出現予定」
// WaveData の count の数だけ作成される
struct EnemySpawnInfo
{
    std::string enemyType;     // 敵の種類
    VECTOR spawnPos;           // 出現位置
    float spawnTime = 0;       // 出現時間
    bool isSpawned = false;    // 出現済みフラグ
	int spawnLocationType = 0; // スポーン位置タイプ (0:ランダム, 1:下段, 2:中段, 3:上段)
};

/// <summary>
/// ウェーブ管理クラス
/// </summary>
class WaveManager
{
public:
    WaveManager();
    ~WaveManager();

    void Init();
    void Reset();
    void Update();

    /// <summary>
    /// 現在のWave番号を取得
    /// </summary>
    int GetCurrentWave() const { return m_currentWave; }

    /// <summary>
    /// 敵のリストを取得
    /// </summary>
    std::vector<std::shared_ptr<EnemyBase>>& GetEnemyList() { return m_enemyList; }

    /// <summary>
    /// 敵の死亡時に呼ばれるコールバックを設定
    /// </summary>
    void SetOnEnemyDeathCallback(std::function<void(const VECTOR&)> callback);

    /// <summary>
    /// 敵ヒット時のコールバックを設定
    /// </summary>
    void SetOnEnemyHitCallback(std::function<void(EnemyBase::HitPart, float)> cb);

    /// <summary>
    /// RoadFloorオブジェクトの範囲を設定
    /// </summary>
    void SetRoadFloorBounds(const VECTOR& minPos, const VECTOR& maxPos);

    /// <summary>
    /// 敵の一括更新
    /// </summary>
    void UpdateEnemies(std::vector<Bullet>& bullets,
                  const Player::TackleInfo& tackleInfo, const Player& player,
                  const std::vector<Stage::StageCollisionData>& collisionData,
                  Effect* pEffect);

    /// <summary>
    /// 敵の一括描画
    /// </summary>
    void DrawEnemies(bool isTutorial = false);

    /// <summary>
    /// ウェーブUIの描画
    /// </summary>
    void DrawWaveUI();

    /// <summary>
    /// 現在のウェーブがアクティブかどうかを取得
    /// </summary>
    bool IsWaveActive() const { return m_isWaveActive; }

    /// <summary>
    /// すべてのウェーブが完了したかどうかを取得
    /// </summary>
    bool IsAllWavesCompleted() const { return m_isAllWavesCompleted; }

    /// <summary>
    /// ウェーブ1の敵がロードされたかどうかを取得
    /// </summary>
    bool IsWave1Loaded() const { return m_isWave1Loaded; }

    /// <summary>
    /// ウェーブ1の敵が実際に出現したかどうかを取得
    /// </summary>
    bool IsWave1EnemySpawned() const { return m_isWave1EnemySpawned; }

    /// <summary>
    /// 現在のスポーンタイマーを取得
    /// </summary>
    float GetSpawnTimer() const { return m_spawnTimer; }

    /// <summary>
    /// ショットチュートリアルがクリアされたかどうかを取得
    /// </summary>
    bool IsShotTutorialCleared() const { return m_isShotTutorialCleared; }

    /// <summary>
    /// タックルチュートリアルがクリアされたかどうかを取得
    /// </summary>
    bool IsTackleTutorialCleared() const { return m_isTackleTutorialCleared; }

    /// <summary>
    /// 生存している敵の数を取得
    /// </summary>
    int GetAliveEnemyCount() const;

    /// <summary>
    /// チュートリアル用の敵をスポーンさせる
    /// </summary>
    void SpawnTutorialWave(int tutorialWaveId);

    // デバッグ表示切り替え用 (後方互換性のため維持)
    static void SetDrawSpawnAreas(bool isDraw) { s_isDrawSpawnAreas = isDraw; }
    static bool IsDrawSpawnAreas() { return s_isDrawSpawnAreas; }

    static void SetShowActiveEnemyCount(bool isShow) { s_isShowActiveEnemyCount = isShow; }
    static bool IsShowActiveEnemyCount() { return s_isShowActiveEnemyCount; }

    static void SetShowDrawnEnemyCount(bool isShow) { s_isShowDrawnEnemyCount = isShow; }
    static bool IsShowDrawnEnemyCount() { return s_isShowDrawnEnemyCount; }

    std::shared_ptr<EnemyNormal> GetPooledNormalEnemy();
    std::shared_ptr<EnemyRunner> GetPooledRunnerEnemy();
    std::shared_ptr<EnemyAcid> GetPooledAcidEnemy();
    std::shared_ptr<EnemyBoss> GetPooledBossEnemy();

    // リファクタリング用ヘルパー
    std::shared_ptr<EnemyBase> GetPooledEnemy(const std::string& type);
    void InitEnemyPools();

    // スポーン位置計算ヘルパー
    const SpawnAreaInfo* SelectSpawnArea(int type, const std::string& enemyType, const VECTOR& playerPos, int spawnLocationType);
    VECTOR CalculateRandomSpawnPos(const SpawnAreaInfo& area);

private:
    /// <summary>
    /// ランダムな出現位置を生成
    /// </summary>
    VECTOR GenerateRandomSpawnPos(const VECTOR& playerPos);

    /// <summary>
    /// 出現位置を生成（エリア定義があればそれを使用、なければランダム）
    /// </summary>
    VECTOR GenerateSpawnPos(int type, const std::string& enemyType, const VECTOR& playerPos, int spawnLocationType = 0);

    /// <summary>
    /// 敵を生成
    /// </summary>
    std::shared_ptr<EnemyBase> CreateEnemy(const std::string& enemyType, const VECTOR& spawnPos);

    /// <summary>
    /// 次のウェーブに進む
    /// </summary>
    void NextWave();

    /// <summary>
    /// 現在のウェーブを開始
    /// </summary>
    void StartCurrentWave(const VECTOR& playerPos = VGet(0.0f, 0.0f, 0.0f));

    /// <summary>
    /// 現在のウェーブの敵がすべて倒されたかチェック
    /// </summary>
    bool IsCurrentWaveCleared();

    /// <summary>
    /// 敵が死亡したときの処理
    /// </summary>
    void OnEnemyDeath(const VECTOR& pos);

private:
    std::unique_ptr<WaveUI> m_pWaveUI; // UI管理クラス

    // データリスト
    std::vector<WaveData> m_waveDataList;
    std::vector<EnemySpawnInfo> m_spawnInfoList;
    std::vector<SpawnAreaInfo> m_spawnAreaList;
    std::vector<std::shared_ptr<EnemyBase>> m_enemyList;

    CollisionGrid m_collisionGrid;

    // 敵パラメータ
    std::vector<ObjectTransformData> m_enemyData;

    // 敵のプール
    std::vector<std::shared_ptr<EnemyNormal>> m_enemyNormalPool;
    std::vector<std::shared_ptr<EnemyRunner>> m_enemyRunnerPool;
    std::vector<std::shared_ptr<EnemyAcid>> m_enemyAcidPool;
    std::vector<std::shared_ptr<EnemyBoss>> m_enemyBossPool;

    // コールバック
    std::function<void(const VECTOR&)> m_onEnemyDeathCallback;
    std::function<void(EnemyBase::HitPart, float)> m_onEnemyHitCallback;

    // RoadFloor範囲
    VECTOR m_roadFloorMin;
    VECTOR m_roadFloorMax;

    int m_totalSpawnedCount; // 累計出現数

    // 状態フラグ
    bool m_isWave1Loaded;
    bool m_isWave1EnemySpawned;
    bool m_isShotTutorialCleared;
    bool m_isTackleTutorialCleared;
    bool m_isRoadFloorBoundsSet;
    bool m_isTutorialMode;

    // ウェーブ進行管理
    int m_currentWave;
    int m_currentSpawnIndex;
    float m_waveTimer;
    float m_spawnTimer;
    float m_waveIntervalTimer;
    bool m_isWaveActive;
    bool m_isAllWavesCompleted;

    // スタティックメンバ (デバッグフラグ)
    static bool s_isDrawSpawnAreas;
    static bool s_isShowActiveEnemyCount;
    static bool s_isShowDrawnEnemyCount;
};