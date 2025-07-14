#pragma once
#include "DxLib.h"
#include <vector>
#include <memory>
#include <string>
#include <functional>

class EnemyBase;
class EnemyNormal;
class EnemyRunner;
class EnemyAcid;
class Bullet;
class Player;

// Waveデータの構造体
struct WaveData
{
	std::string enemyType; // 敵の種類
	int wave;              // Wave番号
	int count;             // 出現数
	float spawnInterval;   // 出現間隔
	float delay;           // 出現までの遅延時間
};

// 敵の出現情報
struct EnemySpawnInfo
{
	std::string enemyType; // 敵の種類
	VECTOR spawnPos;       // 出現位置
	float  spawnTime;      // 出現時間
	bool   isSpawned;      // すでに出現したかどうか
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
    void Update();

    /// <summary>
	/// 現在のWave番号を取得
    /// </summary>
	/// <returns>現在のWave番号</returns>
    int GetCurrentWave() const { return m_currentWave; }

    /// <summary>
    /// 敵のリストを取得
    /// </summary>
	/// <returns>敵のリスト</returns>
    std::vector<std::shared_ptr<EnemyBase>>& GetEnemyList() { return m_enemyList; }

    /// <summary>
	/// 敵の死亡時に呼ばれるコールバックを設定
    /// </summary>
	/// <param name="callback">コールバック関数</param>
    void SetOnEnemyDeathCallback(std::function<void(const VECTOR&)> callback);

    /// <summary>
	/// 敵ヒット時のコールバックを設定
    /// </summary>
	/// <param name="cb">コールバック関数</param>
    void SetOnEnemyHitCallback(std::function<void(EnemyBase::HitPart)> cb) { m_onEnemyHitCallback = cb; }

    /// <summary>
	/// Road_floorオブジェクトの範囲を設定
    /// </summary>
	/// <param name="minPos">最小位置</param>
	/// <param name="maxPos">最大位置</param>
    void SetRoadFloorBounds(const VECTOR& minPos, const VECTOR& maxPos);

    /// <summary>
	/// デバッグ情報を表示
    /// </summary>
    void DrawDebugInfo();

    /// <summary>
    /// 敵の一括更新
    /// </summary>
	/// <param name="bullets">弾のリスト</param>
	/// <param name="tackleInfo">タックル情報</param>
	/// <param name="player">プレイヤーオブジェクト</param>
    void UpdateEnemies(std::vector<Bullet>& bullets, const Player::TackleInfo& tackleInfo, const Player& player);

    /// <summary>
    /// 敵の一括描画
    /// </summary>
    void DrawEnemies();

private:
    /// <summary>
	/// ウェーブデータを読み込む
    /// </summary>
    void LoadWaveData();

    /// <summary>
    /// ランダムな出現位置を生成 
    /// </summary>
	/// <param name="playerPos">プレイヤーの位置</param>
	/// <returns>ランダムな出現位置</returns>
    VECTOR GenerateRandomSpawnPos(const VECTOR& playerPos);

    /// <summary>
    /// 敵を生成
    /// </summary>
	/// <param name="enemyType">敵の種類</param>
	/// <returns>生成された敵のポインタ</returns>
    std::shared_ptr<EnemyBase> CreateEnemy(const std::string& enemyType);

    /// <summary>
	/// 次のウェーブに進む
    /// </summary>
    void NextWave();

    /// <summary>
    /// 現在のウェーブを開始
    /// </summary>
	/// <param name="playerPos">プレイヤーの位置</param>
    void StartCurrentWave(const VECTOR& playerPos = VGet(0.0f, 0.0f, 0.0f));
    
    /// <summary>
    /// 現在のウェーブの敵がすべて倒されたかチェック
    /// </summary>
	/// <returns>すべて倒された場合はtrue</returns>
    bool IsCurrentWaveCleared();

    /// <summary>
    /// 敵の死亡処理
    /// </summary>
	/// <param name="position">敵の位置</param>
    void OnEnemyDeath(const VECTOR& pos);

private:
    std::vector<WaveData> m_waveDataList;
    std::vector<EnemySpawnInfo> m_spawnInfoList;
    std::vector<std::shared_ptr<EnemyBase>> m_enemyList;

    // 敵のテンプレート
    std::shared_ptr<EnemyNormal> m_pEnemyNormalTemplate;
    std::shared_ptr<EnemyRunner> m_pEnemyRunnerTemplate;
    std::shared_ptr<EnemyAcid> m_pEnemyAcidTemplate;

    std::vector<std::shared_ptr<EnemyNormal>> m_enemyNormalPool;
    std::vector<std::shared_ptr<EnemyRunner>> m_enemyRunnerPool;
    std::vector<std::shared_ptr<EnemyAcid>>   m_enemyAcidPool;

    std::shared_ptr<EnemyNormal> GetPooledNormalEnemy();
    std::shared_ptr<EnemyRunner> GetPooledRunnerEnemy();
    std::shared_ptr<EnemyAcid>   GetPooledAcidEnemy();

    // コールバック
    std::function<void(const VECTOR&)> m_onEnemyDeathCallback;    // 敵の死亡時コールバック
    std::function<void(EnemyBase::HitPart)> m_onEnemyHitCallback; // 部位情報付き

	int   m_currentWave;       // 現在のWave番号
	int   m_currentSpawnIndex; // 現在の出現インデックス
	float m_waveTimer;         // ウェーブのタイマー
	float m_spawnTimer;        // 敵の出現タイマー

	bool m_isWaveActive;        // 現在のウェーブがアクティブかどうか
	bool m_isAllWavesCompleted; // すべてのウェーブが完了したかどうか

    // Road_floorオブジェクトの範囲
	VECTOR m_roadFloorMin;       // 最小位置
	VECTOR m_roadFloorMax;       // 最大位置
	bool m_isRoadFloorBoundsSet; // 範囲が設定されているかどうか
};