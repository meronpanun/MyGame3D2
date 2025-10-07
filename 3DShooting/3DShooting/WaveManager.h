#pragma once
#include "TransformDataLoader.h"
#include "EnemyBase.h"
#include <vector>
#include <memory>
#include <string>

class EnemyNormal;
class EnemyRunner;
class EnemyAcid;
class Bullet;
class Player;

// Waveデータの構造体
struct WaveData
{
	int wave = 0;            // ウェーブ番号
    std::string enemyType;   // 敵の種類
	int count = 0;           // 出現数
	float spawnInterval = 0; // 出現間隔
	float startTime     = 0; // 出現開始時間
	float waveInterval  = 0; // ウェーブ間インターバル
};

// 敵の出現情報
struct EnemySpawnInfo
{
	std::string enemyType;    // 敵の種類
	VECTOR spawnPos;          // 出現位置
	float  spawnTime = 0;     // 出現時間
	bool   isSpawned = false; // 出現済みフラグ
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
    void SetOnEnemyHitCallback(std::function<void(EnemyBase::HitPart)> cb);

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
    void UpdateEnemies(std::vector<Bullet>& bullets, const Player::TackleInfo& tackleInfo, const Player& player, Effect* pEffect);

    /// <summary>
    /// 敵の一括描画
    /// </summary>
    void DrawEnemies();

    /// <summary>
	/// 現在のウェーブがアクティブかどうかを取得
    /// </summary>
	/// <returns>現在のウェーブがアクティブならtrue</returns>
    bool IsWaveActive() const { return m_isWaveActive; }

    /// <summary>
	/// すべてのウェーブが完了したかどうかを取得
    /// </summary>
	/// <returns>すべてのウェーブが完了していればtrue</returns>
    bool IsAllWavesCompleted() const { return m_isAllWavesCompleted; }

    /// <summary>
	/// ウェーブ1の敵がロードされたかどうかを取得
    /// </summary>
	/// <returns>ウェーブ1の敵がロードされていればtrue</returns>
    bool IsWave1Loaded() const { return m_isWave1Loaded; }

    /// <summary>
	/// ウェーブ1の敵が実際に出現したかどうかを取得
    /// </summary>
	/// <returns>ウェーブ1の敵が出現していればtrue</returns>
    bool IsWave1EnemySpawned() const { return m_isWave1EnemySpawned; }

    /// <summary>
	/// 現在のウェーブの画像ハンドルを取得
    /// </summary>
	/// <returns>現在のウェーブの画像ハンドル</returns>
    float GetSpawnTimer() const { return m_spawnTimer; }

    /// <summary>
	/// チュートリアルがクリアされたかどうかを取得
    /// </summary>
	/// <returns>ショットチュートリアルがクリアされていればtrue</returns>
    bool IsShotTutorialCleared() const { return m_isShotTutorialCleared; }

    /// <summary>
	/// タックルチュートリアルがクリアされたかどうかを取得
    /// </summary>
	/// <returns>タックルチュートリアルがクリアされていればtrue</returns>
    bool IsTackleTutorialCleared() const { return m_isTackleTutorialCleared; }

    /// <summary>
    /// 生存している敵の数を取得
    /// </summary>
    /// <returns>生存している敵の数</returns>
    int GetAliveEnemyCount() const;

    /// <summary>
    /// チュートリアル用の敵をスポーンさせる
    /// </summary>
    /// <param name="tutorialWaveId">チュートリアルウェーブのID</param>
    void SpawnTutorialWave(int tutorialWaveId);

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
	std::shared_ptr<EnemyBase> CreateEnemy(const std::string& enemyType, const VECTOR& spawnPos);

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
	// 敵のリストとテンプレート
    std::vector<WaveData> m_waveDataList;
    std::vector<EnemySpawnInfo> m_spawnInfoList;
    std::vector<std::shared_ptr<EnemyBase>> m_enemyList;

    // 敵のパラメータを保持
    std::vector<ObjectTransformData> m_enemyData; 

	// 敵のプール
    std::vector<std::shared_ptr<EnemyNormal>> m_enemyNormalPool;
    std::vector<std::shared_ptr<EnemyRunner>> m_enemyRunnerPool;
    std::vector<std::shared_ptr<EnemyAcid>>   m_enemyAcidPool;

	// 敵のプールから取得
    std::shared_ptr<EnemyNormal> GetPooledNormalEnemy();
    std::shared_ptr<EnemyRunner> GetPooledRunnerEnemy();
    std::shared_ptr<EnemyAcid>   GetPooledAcidEnemy();

    // コールバック
    std::function<void(const VECTOR&)>      m_onEnemyDeathCallback; // 敵の死亡時コールバック
    std::function<void(EnemyBase::HitPart)> m_onEnemyHitCallback;   // 部位情報付き

    // Road_floorオブジェクトの範囲
    VECTOR m_roadFloorMin; // 最小位置
    VECTOR m_roadFloorMax; // 最大位置

    // 敵管理
    int m_totalSpawnedCount; // 累計出現数

	// チュートリアル関連
	bool m_isWave1Loaded;           // ウェーブ1の敵がロードされたかどうか
	bool m_isWave1EnemySpawned;     // ウェーブ1の敵が実際に出現したかどうか
	bool m_isShotTutorialCleared;   // ショットチュートリアルがクリアされたかどうか
	bool m_isTackleTutorialCleared; // タックルチュートリアルがクリアされたかどうか

    // ステージ情報
	bool m_isRoadFloorBoundsSet; // 範囲が設定されているかどうか

	// ウェーブ管理関連
    int m_currentWave;          // 現在のWave番号
    int m_currentSpawnIndex;    // 現在の出現インデックス
    float m_waveTimer;          // ウェーブのタイマー
    float m_spawnTimer;         // 敵の出現タイマー
    float m_waveIntervalTimer;  // ウェーブ間インターバル用タイマー
    bool m_isWaveActive;        // 現在のウェーブがアクティブかどうか
    bool m_isAllWavesCompleted; // すべてのウェーブが完了したかどうか

	// ウェーブ画像アニメーション関連
    int  m_waveImages[3];                    // 1,2,3ウェーブ用画像ハンドル
    int  m_waveImageAnimTimer;               // ウェーブ画像アニメーションタイマー
    int  m_waveImageAnimDuration;            // ウェーブ画像アニメーションの総時間
    int  m_waveImageAnimHoldDuration;        // ウェーブ画像アニメーションのホールド時間
    int  m_waveImageAnimInitialHoldDuration; // ウェーブ画像アニメーションの初期ホールド時間
    bool m_isWaveImageAnimating;             // ウェーブ画像アニメーション中フラグ
};