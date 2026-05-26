#pragma once
#include "CollisionGrid.h"
#include "DxLib.h"
#include "EnemyBase.h"
#include "SpawnAreaInfo.h"
#include "TransformDataLoader.h"
#include "WaveData.h"
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

/// <summary>
/// ゲーム実行時に生成される個別の敵出現予定。
/// WaveData の count の数だけ作成され、WaveManager がスポーンタイマーと照合して敵を生成する。
/// </summary>
struct EnemySpawnInfo
{
    std::string enemyType;                     // 敵の種類識別子
    VECTOR      spawnPos          = {};        // 出現座標
    float       spawnTime         = 0.0f;     // スポーンタイマー到達時刻（秒）
    bool        isSpawned         = false;    // 出現済みフラグ
    int         spawnLocationType = 0;         // スポーン位置タイプ（0: ランダム, 1: 下段, 2: 中段, 3: 上段）
    bool        hasShield         = false;    // シールドを持って出現するかどうか
};

/// <summary>
/// ウェーブ制御クラス。
/// CSV から読み込んだウェーブデータを元に敵をスポーンし、ウェーブの進行・完了を管理する。
/// 敵オブジェクトプール・スポーンエリア選択・空間分割グリッドも一括して保持する。
/// </summary>
class WaveManager
{
public:
    /// <summary>ウェーブの進行状態</summary>
    enum class WaveState
    {
        Interval,   // ウェーブ間の待機中
        Starting,   // 開始演出中
        Active,     // 敵が出現・戦闘中
        Completed   // 全ウェーブ完了
    };

    /// <summary>敵モデルを一括ロードして初期化する</summary>
    WaveManager();

    /// <summary>敵モデルを一括解放する</summary>
    ~WaveManager();

    /// <summary>CSV を読み込み、敵プールとグリッドを初期化する</summary>
    void Init();

    /// <summary>ウェーブ進行状態をリセットし、全プールの敵を非アクティブ化する</summary>
    void Reset();

    /// <summary>毎フレームのウェーブ進行・スポーン処理を行う</summary>
    void Update();

    /// <summary>現在のウェーブ番号を返す</summary>
    int GetCurrentWave() const { return m_currentWave; }

    /// <summary>アクティブな敵リストへの参照を返す</summary>
    std::vector<std::shared_ptr<EnemyBase>>& GetEnemyList() { return m_enemyList; }

    /// <summary>空間分割グリッドへの参照を返す</summary>
    CollisionGrid& GetCollisionGrid() { return m_collisionGrid; }

    /// <summary>敵が死亡したときに呼び出されるコールバックを設定する</summary>
    /// <param name="callback">死亡座標を受け取るコールバック関数</param>
    void SetOnEnemyDeathCallback(std::function<void(const VECTOR&)> callback);

    /// <summary>敵がヒットしたときに呼び出されるコールバックを設定する</summary>
    /// <param name="cb">ヒット部位とダメージ値を受け取るコールバック関数</param>
    void SetOnEnemyHitCallback(std::function<void(EnemyBase::HitPart, float)> cb);

    /// <summary>RoadFloor オブジェクトの AABB 範囲を設定し、グリッドを再初期化する</summary>
    /// <param name="minPos">床の最小座標</param>
    /// <param name="maxPos">床の最大座標</param>
    void SetRoadFloorBounds(const VECTOR& minPos, const VECTOR& maxPos);

    /// <summary>ステージの衝突ポリゴンを空間分割グリッドに登録する</summary>
    /// <param name="collisionData">ステージの衝突データリスト</param>
    void RegisterStageToGrid(const std::vector<Stage::StageCollisionData>& collisionData);

    /// <summary>全アクティブ敵を一括更新する</summary>
    /// <param name="bullets">弾リスト（敵との当たり判定に使用）</param>
    /// <param name="tackleInfo">プレイヤーのタックル情報</param>
    /// <param name="player">プレイヤー参照</param>
    /// <param name="collisionData">ステージ衝突データ</param>
    /// <param name="pEffect">エフェクト管理オブジェクト</param>
    void UpdateEnemies(std::vector<Bullet>& bullets,
                       const Player::TackleInfo& tackleInfo, Player& player,
                       const std::vector<Stage::StageCollisionData>& collisionData,
                       Effect* pEffect);

    /// <summary>全アクティブ敵を一括描画する</summary>
    /// <param name="collisionData">ステージ衝突データ（グリッド描画に使用）</param>
    /// <param name="isTutorial">チュートリアルモードかどうか</param>
    void DrawEnemies(const std::vector<Stage::StageCollisionData>& collisionData, bool isTutorial = false);

    /// <summary>衝突グリッドのデバッグ UI を描画する</summary>
    void DrawDebugUI();

    /// <summary>現在のウェーブ状態を返す</summary>
    WaveState GetState() const { return m_state; }

    /// <summary>現在のウェーブがアクティブかどうかを返す</summary>
    bool IsWaveActive() const { return m_isWaveActive; }

    /// <summary>全ウェーブが完了しているかどうかを返す</summary>
    bool IsAllWavesCompleted() const { return m_haveAllWavesCompleted; }

    /// <summary>ウェーブ 1 の敵データがロード済みかどうかを返す</summary>
    bool IsWave1Loaded() const { return m_hasLoadedWave1; }

    /// <summary>ウェーブ 1 の最初の敵が実際に出現したかどうかを返す</summary>
    bool IsWave1EnemySpawned() const { return m_hasSpawnedWave1Enemy; }

    /// <summary>現在のスポーンタイマー値（秒）を返す</summary>
    float GetSpawnTimer() const { return m_spawnTimer; }

    /// <summary>ショットチュートリアルがクリアされたかどうかを返す</summary>
    bool IsShotTutorialCleared() const { return m_hasClearedShotTutorial; }

    /// <summary>タックルチュートリアルがクリアされたかどうかを返す</summary>
    bool IsTackleTutorialCleared() const { return m_hasClearedTackleTutorial; }

    /// <summary>現在生存している敵の数を返す</summary>
    int GetAliveEnemyCount() const;

    /// <summary>指定チュートリアルウェーブの敵をスポーンさせる</summary>
    /// <param name="tutorialWaveId">チュートリアルウェーブ番号</param>
    void SpawnTutorialWave(int tutorialWaveId);

    // --- デバッグ表示切り替え（後方互換性のため維持）---

    /// <summary>スポーンエリアのデバッグ描画を有効/無効にする</summary>
    static void SetDrawSpawnAreas(bool isDraw) { s_shouldDrawSpawnAreas = isDraw; }
    /// <summary>スポーンエリアのデバッグ描画が有効かどうかを返す</summary>
    static bool IsDrawSpawnAreas() { return s_shouldDrawSpawnAreas; }

    /// <summary>アクティブ敵数の表示を有効/無効にする</summary>
    static void SetShowActiveEnemyCount(bool isShow) { s_shouldShowActiveEnemyCount = isShow; }
    /// <summary>アクティブ敵数の表示が有効かどうかを返す</summary>
    static bool IsShowActiveEnemyCount() { return s_shouldShowActiveEnemyCount; }

    /// <summary>描画済み敵数の表示を有効/無効にする</summary>
    static void SetShowDrawnEnemyCount(bool isShow) { s_shouldShowDrawnEnemyCount = isShow; }
    /// <summary>描画済み敵数の表示が有効かどうかを返す</summary>
    static bool IsShowDrawnEnemyCount() { return s_shouldShowDrawnEnemyCount; }

    // --- 敵プール取得 ---

    /// <summary>プールから非アクティブな EnemyNormal を取得する（なければ新規生成）</summary>
    std::shared_ptr<EnemyNormal> GetPooledNormalEnemy();
    /// <summary>プールから非アクティブな EnemyRunner を取得する（なければ新規生成）</summary>
    std::shared_ptr<EnemyRunner> GetPooledRunnerEnemy();
    /// <summary>プールから非アクティブな EnemyAcid を取得する（なければ新規生成）</summary>
    std::shared_ptr<EnemyAcid>  GetPooledAcidEnemy();
    /// <summary>プールから非アクティブな EnemyBoss を取得する（なければ新規生成）</summary>
    std::shared_ptr<EnemyBoss>  GetPooledBossEnemy();

    /// <summary>敵種別文字列から対応するプール済み敵を取得する</summary>
    /// <param name="type">敵種別識別子</param>
    std::shared_ptr<EnemyBase> GetPooledEnemy(const std::string& type);

    /// <summary>ウェーブデータから各敵種の最大同時出現数を算出してプールを確保する</summary>
    void InitEnemyPools();

    /// <summary>
    /// スポーン位置タイプとプレイヤー位置に基づいてスポーンエリアを選択する。
    /// 候補が存在しない場合は nullptr を返す。
    /// </summary>
    /// <param name="type">エリア種別（0: メイン, 1: チュートリアル）</param>
    /// <param name="enemyType">敵種別識別子（遠距離判定に使用）</param>
    /// <param name="playerPos">現在のプレイヤー座標</param>
    /// <param name="spawnLocationType">高さ層タイプ（0: ランダム, 1〜3: 層指定）</param>
    const SpawnAreaInfo* SelectSpawnArea(int type, const std::string& enemyType,
                                         const VECTOR& playerPos, int spawnLocationType);

    /// <summary>エリア内のランダムな座標を計算して返す</summary>
    /// <param name="area">対象スポーンエリア</param>
    VECTOR CalculateRandomSpawnPos(const SpawnAreaInfo& area);

private:
    /// <summary>RoadFloor の AABB 内でプレイヤーから一定距離以上離れた座標をランダム生成する</summary>
    VECTOR GenerateRandomSpawnPos(const VECTOR& playerPos);

    /// <summary>エリア定義があればエリアから、なければランダムでスポーン座標を生成する</summary>
    VECTOR GenerateSpawnPos(int type, const std::string& enemyType,
                            const VECTOR& playerPos, int spawnLocationType = 0);

    /// <summary>指定種別・位置の敵をプールから取得して初期化し、返す</summary>
    std::shared_ptr<EnemyBase> CreateEnemy(const std::string& enemyType,
                                            const VECTOR& spawnPos, bool hasShield = false);

    /// <summary>次のウェーブへ進む。次ウェーブが存在しない場合は全完了フラグを立てる</summary>
    void NextWave();

    /// <summary>現在のウェーブのスポーン情報を構築して戦闘を開始する</summary>
    void StartCurrentWave(const VECTOR& playerPos = VGet(0.0f, 0.0f, 0.0f));

    /// <summary>現在のウェーブの全敵が倒されたかどうかを返す</summary>
    bool IsCurrentWaveCleared();

    /// <summary>敵が死亡したときのコールバックを呼び出す</summary>
    void OnEnemyDeath(const VECTOR& pos);

private:
    // --- データリスト ---
    std::vector<WaveData>                    m_waveDataList;    // CSV から読み込んだウェーブ設定
    std::vector<EnemySpawnInfo>              m_spawnInfoList;   // 現ウェーブの個別スポーン予定
    std::vector<SpawnAreaInfo>               m_spawnAreaList;   // スポーンエリア定義リスト
    std::vector<std::shared_ptr<EnemyBase>>  m_enemyList;       // 現在アクティブな敵リスト

    CollisionGrid m_collisionGrid; // 空間分割グリッド（敵・ステージ衝突用）

    // --- 敵パラメータ・プール ---
    std::vector<ObjectTransformData>           m_enemyData;        // 敵のトランスフォームデータ
    std::vector<std::shared_ptr<EnemyNormal>>  m_enemyNormalPool;  // 通常敵プール
    std::vector<std::shared_ptr<EnemyRunner>>  m_enemyRunnerPool;  // ランナー敵プール
    std::vector<std::shared_ptr<EnemyAcid>>    m_enemyAcidPool;    // 酸敵プール
    std::vector<std::shared_ptr<EnemyBoss>>    m_enemyBossPool;    // ボスプール

    // --- コールバック ---
    std::function<void(const VECTOR&)>           m_onEnemyDeathCallback; // 敵死亡時コールバック
    std::function<void(EnemyBase::HitPart, float)> m_onEnemyHitCallback; // 敵ヒット時コールバック

    // --- RoadFloor 範囲 ---
    VECTOR m_roadFloorMin; // 床の最小座標（ランダムスポーン範囲）
    VECTOR m_roadFloorMax; // 床の最大座標（ランダムスポーン範囲）

    int m_totalSpawnedCount; // 累計スポーン数

    // --- 状態フラグ ---
    bool m_hasLoadedWave1;           // ウェーブ 1 データロード済み
    bool m_hasSpawnedWave1Enemy;     // ウェーブ 1 の初敵出現済み
    bool m_hasClearedShotTutorial;   // ショットチュートリアルクリア済み
    bool m_hasClearedTackleTutorial; // タックルチュートリアルクリア済み
    bool m_hasSetRoadFloorBounds;    // RoadFloor 範囲設定済み
    bool m_isTutorialMode;           // チュートリアルモード中

    // --- ウェーブ進行管理 ---
    WaveState m_state;              // 現在のウェーブ状態
    int       m_currentWave;        // 現在のウェーブ番号
    int       m_currentSpawnIndex;  // 次にスポーンする SpawnInfo のインデックス
    float     m_waveTimer;          // 演出・遷移用タイマー（フレーム）
    float     m_spawnTimer;         // スポーンタイマー（秒）
    float     m_waveIntervalTimer;  // ウェーブ間インターバルタイマー（秒）
    bool      m_isWaveActive;       // ウェーブ進行中フラグ
    bool      m_haveAllWavesCompleted; // 全ウェーブ完了フラグ

    // --- スタティックデバッグフラグ ---
    static bool s_shouldDrawSpawnAreas;       // スポーンエリア描画
    static bool s_shouldShowActiveEnemyCount; // アクティブ敵数表示
    static bool s_shouldShowDrawnEnemyCount;  // 描画済み敵数表示
};
