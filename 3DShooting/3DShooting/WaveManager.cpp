#include "WaveManager.h"
#include "Bullet.h"
#include "CollisionGrid.h"
#include "EffekseerWarningSuppress.h"
#include "EnemyAcid.h"
#include "EnemyBase.h"
#include "EnemyBoss.h"
#include "EnemyNormal.h"
#include "EnemyRunner.h"
#include "Game.h"
#include "Player.h"
#include "WaveDataLoader.h"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <map>
#include <random>

namespace
{
    constexpr float kMinSpawnDistance   = 200.0f;  // プレイヤーからの最小スポーン距離
    constexpr int   kMaxSpawnAttempts   = 100;     // スポーン位置の最大試行回数
    constexpr float kGridCellSize       = 100.0f;  // 空間分割グリッドのセルサイズ
    constexpr float kWaveStartAnimFrames = 105.0f; // Starting 演出の継続フレーム数
    constexpr int   kMaxActiveEnemies   = 40;      // 同時アクティブ敵数の上限
    constexpr float kDefaultWaveInterval = 3.0f;   // 次ウェーブまでのデフォルトインターバル（秒）
    constexpr float kLongRangeThreshold  = 500.0f; // 遠距離敵に適用するスポーンエリア距離閾値

    // 床範囲が未設定の場合のフォールバック値
    constexpr VECTOR kDefaultRoadFloorPos = { 0.0f, -0.5f, 3.0f };
    constexpr VECTOR kDefaultRoadFloorMin = { -1000.0f, 0.0f, -1000.0f };
    constexpr VECTOR kDefaultRoadFloorMax = {  1000.0f, 0.0f,  1000.0f };
}

bool WaveManager::s_shouldDrawSpawnAreas       = false;
bool WaveManager::s_shouldShowActiveEnemyCount = false;
bool WaveManager::s_shouldShowDrawnEnemyCount  = false;

// コンストラクタでモデルを静的ロードする。
// WaveManager は各敵種の静的リソース（モデルデータ）の生存期間を管理するため、
// 生成時に LoadModel・破棄時に DeleteModel を呼ぶ責務を持つ。
WaveManager::WaveManager()
    : m_state(WaveState::Interval)
    , m_currentWave(1)
    , m_waveTimer(0.0f)
    , m_spawnTimer(0.0f)
    , m_currentSpawnIndex(0)
    , m_isWaveActive(false)
    , m_haveAllWavesCompleted(false)
    , m_hasLoadedWave1(false)
    , m_hasSpawnedWave1Enemy(false)
    , m_roadFloorMin(kDefaultRoadFloorMin)
    , m_roadFloorMax(kDefaultRoadFloorMax)
    , m_hasSetRoadFloorBounds(false)
    , m_onEnemyDeathCallback(nullptr)
    , m_waveIntervalTimer(0.0f)
    , m_totalSpawnedCount(0)
    , m_hasClearedShotTutorial(false)
    , m_hasClearedTackleTutorial(false)
    , m_isTutorialMode(false)
{
    EnemyNormal::LoadModel();
    EnemyRunner::LoadModel();
    EnemyAcid::LoadModel();
    EnemyBoss::LoadModel();
}

WaveManager::~WaveManager()
{
    EnemyNormal::DeleteModel();
    EnemyRunner::DeleteModel();
    EnemyAcid::DeleteModel();
    EnemyBoss::DeleteModel();
}

void WaveManager::Init()
{
    // CSVから敵の変換データ・ウェーブデータ・スポーンエリアデータをロード
    m_enemyData = TransformDataLoader::LoadDataCSV("data/CSV/CharacterTransfromData.csv");
    m_enemyList.clear();
    m_spawnInfoList.clear();

    m_waveDataList  = WaveDataLoader::LoadWaveData("data/CSV/WaveData.csv");
    m_spawnAreaList = WaveDataLoader::LoadSpawnAreaData("data/CSV/SpawnAreaData.csv");

    // 空間分割グリッドをステージ範囲で初期化（敵の近傍検索・衝突最適化に使用）
    m_collisionGrid.Init(m_roadFloorMin, m_roadFloorMax, kGridCellSize);

    InitEnemyPools();

    // チュートリアル達成判定コールバックを全プールの敵に事前登録する。
    // プールの敵は Init() 後に再利用されるので、ここで一括設定しておく。
    // このコールバックは「どの攻撃種別で敵を倒したか」を WaveManager に通知する。
    auto deathTypeCallback = [this](const VECTOR& pos, AttackType type) {
        if (m_currentWave == 1)
        {
            if (type == AttackType::Shoot)  m_hasClearedShotTutorial   = true;
            if (type == AttackType::Tackle) m_hasClearedTackleTutorial = true;
        }
    };

    auto setCallback = [&](auto& pool) {
        for (auto& enemy : pool) enemy->SetOnDeathWithTypeCallback(deathTypeCallback);
    };
    setCallback(m_enemyNormalPool);
    setCallback(m_enemyRunnerPool);
    setCallback(m_enemyAcidPool);
    setCallback(m_enemyBossPool);

    // Wave1のチュートリアル達成フラグを敵死亡時に更新するコールバックを設定する。
    // 死亡した敵の座標で m_enemyList から対象を特定し、最後に受けた攻撃種別を確認する。
    // ※ ItemDropManager からも SetOnEnemyDeathCallback が呼ばれる場合は上書きされる点に注意。
    //   チュートリアル達成判定は上記の SetOnDeathWithTypeCallback 側で完結しているので問題ない。
    SetOnEnemyDeathCallback([this](const VECTOR& pos) {
        for (auto& enemy : m_enemyList)
        {
            if (enemy->GetPos().x == pos.x && enemy->GetPos().y == pos.y && enemy->GetPos().z == pos.z)
            {
                if (!enemy) continue;
                if (m_currentWave == 1)
                {
                    if (enemy->GetLastAttackType() == AttackType::Shoot)  m_hasClearedShotTutorial   = true;
                    if (enemy->GetLastAttackType() == AttackType::Tackle) m_hasClearedTackleTutorial = true;
                }
                break;
            }
        }
    });
}

void WaveManager::InitEnemyPools()
{
    // ウェーブデータから各敵種の「ウェーブ単位の最大同時出現数」を集計し、
    // プールサイズを事前に確保することで動的生成を最小化する（オブジェクトプール最適化）
    std::map<int, int> normalPerWave, runnerPerWave, acidPerWave, bossPerWave;
    for (const auto& wave : m_waveDataList)
    {
        if (wave.enemyType == "NormalEnemy") normalPerWave[wave.wave] += wave.count;
        if (wave.enemyType == "RunnerEnemy") runnerPerWave[wave.wave] += wave.count;
        if (wave.enemyType == "AcidEnemy")   acidPerWave[wave.wave]   += wave.count;
        if (wave.enemyType == "Boss")        bossPerWave[wave.wave]   += wave.count;
    }

    // 全ウェーブのなかで最も多い出現数をプールサイズとして採用する
    int maxNormal = 0, maxRunner = 0, maxAcid = 0, maxBoss = 0;
    for (const auto& [wave, cnt] : normalPerWave) maxNormal = (std::max)(maxNormal, cnt);
    for (const auto& [wave, cnt] : runnerPerWave) maxRunner = (std::max)(maxRunner, cnt);
    for (const auto& [wave, cnt] : acidPerWave)   maxAcid   = (std::max)(maxAcid,   cnt);
    for (const auto& [wave, cnt] : bossPerWave)   maxBoss   = (std::max)(maxBoss,   cnt);

    // 現在のプールサイズが不足している場合のみ追加生成する（既存エントリは上書きしない）
    // ※ 実行時に GetPooled* からも動的拡張されるので、ここでは最低限のサイズだけ確保する
    auto ensurePoolSize = []<typename T>(std::vector<std::shared_ptr<T>>& pool, int size) {
        for (int i = static_cast<int>(pool.size()); i < size; ++i)
        {
            auto pEnemy = std::make_shared<T>();
            pEnemy->Init();
            pEnemy->SetActive(false);
            pool.push_back(pEnemy);
        }
    };

    ensurePoolSize(m_enemyNormalPool, maxNormal);
    ensurePoolSize(m_enemyRunnerPool, maxRunner);
    ensurePoolSize(m_enemyAcidPool,   maxAcid);
    ensurePoolSize(m_enemyBossPool,   maxBoss);
}

void WaveManager::Reset()
{
    // Wave番号・タイマー類・スポーン状態をすべて初期値に戻す
    m_currentWave        = 1;
    m_waveTimer          = 0.0f;
    m_spawnTimer         = 0.0f;
    m_currentSpawnIndex  = 0;
    m_waveIntervalTimer  = 0.0f;
    m_isWaveActive       = false;
    m_haveAllWavesCompleted = false;

    // アクティブな敵リストとスポーン予約をクリア
    m_enemyList.clear();
    m_spawnInfoList.clear();

    // プールの全敵を非アクティブにする（プール自体は破棄しない）
    for (auto& enemy : m_enemyNormalPool) enemy->SetActive(false);
    for (auto& enemy : m_enemyRunnerPool) enemy->SetActive(false);
    for (auto& enemy : m_enemyAcidPool)   enemy->SetActive(false);
    for (auto& enemy : m_enemyBossPool)   enemy->SetActive(false);

    // チュートリアルモードで使用していた場合は通常ウェーブデータに戻す
    if (m_isTutorialMode)
    {
        m_waveDataList = WaveDataLoader::LoadWaveData("data/CSV/WaveData.csv");
    }

    m_hasClearedShotTutorial   = false;
    m_hasClearedTackleTutorial = false;
    m_isTutorialMode           = false;
}

void WaveManager::Update()
{
    // グリッドをクリア（敵のみ。ステージデータは保持）
    m_collisionGrid.ClearEnemies();
    m_collisionGrid.ResetAccessFlags();
    m_collisionGrid.ResetStats();
    m_collisionGrid.SetTotalEnemies(0);

    for (auto& pEnemy : m_enemyList)
    {
        if (pEnemy->IsActive() && pEnemy->IsAlive())
        {
            m_collisionGrid.RegisterEnemy(pEnemy.get());
        }
    }

    // Starting 演出中はタイマーを進めて Active へ遷移する
    if (m_state == WaveState::Starting)
    {
        m_waveTimer += 1.0f * Game::GetTimeScale();
        if (m_waveTimer >= kWaveStartAnimFrames)
        {
            m_state     = WaveState::Active;
            m_waveTimer = 0.0f;
        }
    }

    if (m_haveAllWavesCompleted) return;

    if (m_isWaveActive)
    {
        // ウェーブ内の全敵を倒したかつスポーン済みかで Wave クリアを判定
        if (IsCurrentWaveCleared())
        {
            // チュートリアルモードでは自動的に次のWaveに進まない（SceneMain が制御する）
            if (!m_isTutorialMode) NextWave();
        }
        else
        {
            // Starting 演出中はスポーンしない
            if (m_currentSpawnIndex < m_spawnInfoList.size() && m_state == WaveState::Active)
            {
                // スポーンタイマーを 60FPS 相当で進める
                m_spawnTimer += (1.0f / 60.0f) * Game::GetTimeScale();

                // スポーン時間に達した敵を順番に生成する（同時アクティブ数が上限を超えたら待機）
                while (m_currentSpawnIndex < m_spawnInfoList.size() && GetAliveEnemyCount() < kMaxActiveEnemies)
                {
                    EnemySpawnInfo& spawnInfo = m_spawnInfoList[m_currentSpawnIndex];
                    if (m_spawnTimer >= spawnInfo.spawnTime && !spawnInfo.isSpawned)
                    {
                        // スポーン直前にプレイヤー位置を取得し、プレイヤーから離れた位置に出現させる
                        VECTOR currentPlayerPos = VGet(0.0f, 0.0f, 0.0f);
                        if (Game::GetPlayer()) currentPlayerPos = Game::GetPlayer()->GetPos();

                        spawnInfo.spawnPos = GenerateSpawnPos(
                            m_isTutorialMode ? 1 : 0,
                            spawnInfo.enemyType,
                            currentPlayerPos,
                            spawnInfo.spawnLocationType);

                        std::shared_ptr<EnemyBase> pEnemy = CreateEnemy(
                            spawnInfo.enemyType, spawnInfo.spawnPos, spawnInfo.hasShield);
                        if (pEnemy)
                        {
                            m_enemyList.push_back(pEnemy);
                            spawnInfo.isSpawned = true;
                        }
                        m_currentSpawnIndex++;
                    }
                    else
                    {
                        break; // 次の敵のスポーン時間がまだ来ていないので終了
                    }
                }
            }
        }
    }
    else
    {
        // Wave 間インターバル中: タイマーをカウントダウンし、0 以下になったら次の Wave を開始する
        if (m_waveIntervalTimer > 0.0f)
        {
            m_waveIntervalTimer -= (1.0f / 60.0f) * Game::GetTimeScale();
        }
        else
        {
            if (m_currentWave <= 5)
            {
                StartCurrentWave(VGet(0.0f, 0.0f, 0.0f));
            }
        }
    }

    // 非アクティブ（死亡アニメーション終了など）な敵をリストから除去する
    m_enemyList.erase(
        std::remove_if(m_enemyList.begin(), m_enemyList.end(),
                       [](const std::shared_ptr<EnemyBase>& pEnemy) { return !pEnemy->IsActive(); }),
        m_enemyList.end());
}

void WaveManager::UpdateEnemies(
    std::vector<Bullet>& bullets,
    const Player::TackleInfo& tackleInfo,
    const Player& player,
    const std::vector<Stage::StageCollisionData>& collisionData,
    Effect* pEffect)
{
    // 生存中のアクティブ敵のリストを構築し、敵同士の衝突解決で近傍探索に使用する
    std::vector<EnemyBase*> activeEnemies;
    for (const auto& pEnemy : m_enemyList)
    {
        if (pEnemy->IsActive() && pEnemy->IsAlive())
        {
            activeEnemies.push_back(pEnemy.get());
        }
    }

    // 空間分割グリッドは Update() 内で構築済みのため、ここでは再構築しない
    EnemyUpdateContext context = { bullets, tackleInfo, player, activeEnemies, collisionData, pEffect, &m_collisionGrid };

    // アクティブな敵を全て更新する（死亡アニメーション中の敵も IsActive() の間は更新対象）
    for (auto& pEnemy : m_enemyList)
    {
        if (pEnemy->IsActive()) pEnemy->Update(context);
    }
}

void WaveManager::DrawEnemies(const std::vector<Stage::StageCollisionData>& collisionData, bool isTutorial)
{
    // アクティブな敵を描画する（距離・視野角によるカリングは各敵の Draw() 内で行う）
    for (const auto& pEnemy : m_enemyList)
    {
        if (pEnemy->IsActive()) pEnemy->Draw();
    }

    // 空間分割グリッドのデバッグ描画（リリースビルドでは描画なし）
    m_collisionGrid.Draw(collisionData);
}

void WaveManager::DrawDebugUI()
{
    m_collisionGrid.DrawUI();
}

void WaveManager::SetOnEnemyDeathCallback(std::function<void(const VECTOR&)> callback)
{
    // 敵が死亡した際に座標を引数として呼ばれるコールバックを登録する。
    // 単一コールバックのため、後から呼ぶと前の登録が上書きされる点に注意。
    m_onEnemyDeathCallback = callback;
}

void WaveManager::SetOnEnemyHitCallback(std::function<void(EnemyBase::HitPart, float)> cb)
{
    // プレイヤーの弾が敵にヒットした際に呼ばれるコールバックを登録する（ヒットマーク表示用）
    m_onEnemyHitCallback = cb;
}

void WaveManager::SetRoadFloorBounds(const VECTOR& minPos, const VECTOR& maxPos)
{
    // ステージのフロア範囲を設定し、空間分割グリッドを再初期化する。
    // ステージロード後に呼ぶことで、グリッドがマップ範囲に合わせて再構築される。
    m_roadFloorMin         = minPos;
    m_roadFloorMax         = maxPos;
    m_hasSetRoadFloorBounds = true;

    m_collisionGrid.Init(m_roadFloorMin, m_roadFloorMax, kGridCellSize);
}

void WaveManager::RegisterStageToGrid(const std::vector<Stage::StageCollisionData>& collisionData)
{
    // ステージの全ポリゴンをグリッドに登録する。
    // 敵の高さ計算（CollisionGrid::CalculateHeights）はこの後に呼ぶ必要がある。
    for (const auto& tri : collisionData)
    {
        m_collisionGrid.RegisterStageTriangle(tri);
    }
}

VECTOR WaveManager::GenerateRandomSpawnPos(const VECTOR& playerPos)
{
    // フロア範囲が未設定の場合はフォールバック座標を返す
    if (!m_hasSetRoadFloorBounds) return kDefaultRoadFloorPos;

    // 毎回異なるシードで乱数エンジンを初期化し、同じ位置に連続スポーンするのを防ぐ
    unsigned int seed = static_cast<unsigned int>(
        std::chrono::system_clock::now().time_since_epoch().count());
    std::mt19937 gen(seed);

    std::uniform_real_distribution<float> xDist(m_roadFloorMin.x, m_roadFloorMax.x);
    std::uniform_real_distribution<float> zDist(m_roadFloorMin.z, m_roadFloorMax.z);

    // プレイヤーから kMinSpawnDistance 以上離れた座標を最大 kMaxSpawnAttempts 回試みる
    VECTOR spawnPos = {};
    bool   found    = false;
    for (int attempts = 0; attempts < kMaxSpawnAttempts; ++attempts)
    {
        float x = xDist(gen);
        float z = zDist(gen);
        spawnPos = VGet(x, 0.0f, z);

        VECTOR toPlayer = VSub(playerPos, spawnPos);
        toPlayer.y = 0.0f;
        float dist = sqrtf(toPlayer.x * toPlayer.x + toPlayer.z * toPlayer.z);
        if (dist >= kMinSpawnDistance)
        {
            found = true;
            break;
        }
    }

    // 試行上限に達しても見つからない場合はランダム座標をそのまま使用する
    if (!found) spawnPos = VGet(xDist(gen), 0.0f, zDist(gen));
    return spawnPos;
}

VECTOR WaveManager::GenerateSpawnPos(int type, const std::string& enemyType,
                                      const VECTOR& playerPos, int spawnLocationType)
{
    // スポーンエリアを選択し、そのエリア内のランダム座標を返す。
    // 候補エリアが存在しない場合はフロア全体からランダム生成にフォールバックする。
    const SpawnAreaInfo* pArea = SelectSpawnArea(type, enemyType, playerPos, spawnLocationType);
    if (!pArea) return GenerateRandomSpawnPos(playerPos);
    return CalculateRandomSpawnPos(*pArea);
}

const SpawnAreaInfo* WaveManager::SelectSpawnArea(int type, const std::string& enemyType,
                                                    const VECTOR& playerPos, int spawnLocationType)
{
    // スポーンエリアの選択ルール:
    //   type == 0 かつ Wave1 またはスポーン高さ指定あり → 指定高さ層のエリアから選択し、プレイヤーから遠い順に優先
    //   それ以外 → type が一致する全エリアからランダム選択（AcidEnemy は遠距離エリア優先）
    std::vector<const SpawnAreaInfo*> candidates;

    // 高さ層タイプに対応する Y 座標（下段: 200, 中段: 500, 上段: 962）
    float targetY = -999.0f;
    if      (spawnLocationType == 1) targetY = 200.0f;
    else if (spawnLocationType == 2) targetY = 500.0f;
    else if (spawnLocationType == 3) targetY = 962.0f;

    if ((m_currentWave == 1 && type == 0) || (spawnLocationType > 0 && type == 0))
    {
        float checkY = (targetY != -999.0f) ? targetY : 200.0f;
        for (const auto& area : m_spawnAreaList)
        {
            if (area.type == type && std::abs(area.center.y - checkY) < 10.0f)
            {
                candidates.push_back(&area);
            }
        }

        // プレイヤーから十分離れているエリアを優先する
        std::vector<const SpawnAreaInfo*> validCandidates;
        for (const auto* area : candidates)
        {
            float halfSizeX = area->size.x * 0.5f;
            float halfSizeZ = area->size.z * 0.5f;
            bool inX = playerPos.x >= (area->center.x - halfSizeX) && playerPos.x <= (area->center.x + halfSizeX);
            bool inZ = playerPos.z >= (area->center.z - halfSizeZ) && playerPos.z <= (area->center.z + halfSizeZ);
            if (inX && inZ) continue;

            float safeDistance = (std::max)(area->size.x, area->size.z) + 200.0f;
            if (VSize(VSub(area->center, playerPos)) >= safeDistance) validCandidates.push_back(area);
        }

        auto byDistDesc = [&](const SpawnAreaInfo* a, const SpawnAreaInfo* b) {
            return VSize(VSub(a->center, playerPos)) > VSize(VSub(b->center, playerPos));
        };

        if (!validCandidates.empty())
        {
            std::sort(validCandidates.begin(), validCandidates.end(), byDistDesc);
            return validCandidates[0];
        }
        if (!candidates.empty())
        {
            std::sort(candidates.begin(), candidates.end(), byDistDesc);
            return candidates[0];
        }
    }
    else
    {
        for (const auto& area : m_spawnAreaList)
        {
            if (area.type == type) candidates.push_back(&area);
        }
    }

    if (candidates.empty()) return nullptr;

    // 遠距離敵（AcidEnemy）はプレイヤーから距離のあるエリアを優先する
    std::vector<const SpawnAreaInfo*> filteredCandidates;
    if ((m_currentWave == 1 && type == 0) || (spawnLocationType > 0 && type == 0))
    {
        filteredCandidates = candidates;
    }
    else
    {
        bool isLongRange = (enemyType == "AcidEnemy");
        if (isLongRange)
        {
            for (const auto* area : candidates)
            {
                if (VSize(VSub(area->center, playerPos)) >= kLongRangeThreshold)
                    filteredCandidates.push_back(area);
            }
            if (filteredCandidates.empty()) filteredCandidates = candidates;
        }
        else
        {
            filteredCandidates = candidates;
        }
    }

    return filteredCandidates[GetRand(static_cast<int>(filteredCandidates.size()) - 1)];
}

VECTOR WaveManager::CalculateRandomSpawnPos(const SpawnAreaInfo& area)
{
    unsigned int seed = static_cast<unsigned int>(
        std::chrono::system_clock::now().time_since_epoch().count());
    std::mt19937 gen(seed);
    std::uniform_real_distribution<float> xDist(-0.5f, 0.5f);
    std::uniform_real_distribution<float> zDist(-0.5f, 0.5f);

    VECTOR offset = VGet(xDist(gen) * area.size.x, 0.0f, zDist(gen) * area.size.z);
    return VAdd(area.center, offset);
}

std::shared_ptr<EnemyBase> WaveManager::CreateEnemy(const std::string& enemyType,
                                                      const VECTOR& spawnPos, bool hasShield)
{
    // プールから非アクティブな敵を取得して再利用する（プールが満杯なら動的生成）
    std::shared_ptr<EnemyBase> pEnemy = GetPooledEnemy(enemyType);
    if (!pEnemy) return nullptr;

    // 位置・状態を初期化し、コールバックを再登録してからアクティブ化する
    pEnemy->SetPos(spawnPos);
    pEnemy->SetActive(true);
    pEnemy->Init();
    pEnemy->SetOnDeathCallback(m_onEnemyDeathCallback);
    pEnemy->SetOnHitCallback(m_onEnemyHitCallback);
    m_totalSpawnedCount++;

    // NormalEnemy のみシールド有無を外部から制御できる（Wave設定による）
    if (enemyType == "NormalEnemy")
    {
        auto pNormal = std::dynamic_pointer_cast<EnemyNormal>(pEnemy);
        if (pNormal) pNormal->SetHasShield(hasShield);
    }

    return pEnemy;
}

std::shared_ptr<EnemyNormal> WaveManager::GetPooledNormalEnemy()
{
    // 非アクティブな敵をプールから再利用する（オブジェクトプールパターン）
    for (auto& enemy : m_enemyNormalPool)
    {
        if (!enemy->IsActive()) return enemy;
    }
    // プールに空きがない場合のみ動的生成しプールへ追加する（スポーン失敗を防ぐフォールバック）
    auto pNew = std::make_shared<EnemyNormal>();
    pNew->Init();
    m_enemyNormalPool.push_back(pNew);
    return pNew;
}

std::shared_ptr<EnemyRunner> WaveManager::GetPooledRunnerEnemy()
{
    // GetPooledNormalEnemy() と同じパターン（コメントはそちらを参照）
    for (auto& enemy : m_enemyRunnerPool)
    {
        if (!enemy->IsActive()) return enemy;
    }
    auto pNew = std::make_shared<EnemyRunner>();
    pNew->Init();
    m_enemyRunnerPool.push_back(pNew);
    return pNew;
}

std::shared_ptr<EnemyAcid> WaveManager::GetPooledAcidEnemy()
{
    for (auto& enemy : m_enemyAcidPool)
    {
        if (!enemy->IsActive()) return enemy;
    }
    auto pNew = std::make_shared<EnemyAcid>();
    pNew->Init();
    m_enemyAcidPool.push_back(pNew);
    return pNew;
}

std::shared_ptr<EnemyBoss> WaveManager::GetPooledBossEnemy()
{
    for (auto& enemy : m_enemyBossPool)
    {
        if (!enemy->IsActive()) return enemy;
    }
    auto pNew = std::make_shared<EnemyBoss>();
    pNew->Init();
    m_enemyBossPool.push_back(pNew);
    return pNew;
}

std::shared_ptr<EnemyBase> WaveManager::GetPooledEnemy(const std::string& type)
{
    // 敵種別文字列からプール取得関数にディスパッチする（CSV から読み込んだ文字列と対応）
    if (type == "NormalEnemy") return GetPooledNormalEnemy();
    if (type == "RunnerEnemy") return GetPooledRunnerEnemy();
    if (type == "AcidEnemy")   return GetPooledAcidEnemy();
    if (type == "Boss")        return GetPooledBossEnemy();
    return nullptr; // 未知の敵種別の場合は nullptr を返す
}

void WaveManager::StartCurrentWave(const VECTOR& playerPos)
{
    // ウェーブ開始時の各種タイマー・インデックスをリセットする
    m_isWaveActive      = true;
    m_waveTimer         = 0.0f;
    m_spawnTimer        = 0.0f;
    m_currentSpawnIndex = 0;
    m_spawnInfoList.clear();

    // CSVから現在Waveに対応するエントリを読み込み、スポーンスケジュールを構築する
    // 同じ種別が count 体いる場合は spawnInterval 間隔でスポーン時刻を算出する
    for (const auto& waveData : m_waveDataList)
    {
        if (waveData.wave != m_currentWave) continue;

        for (int i = 0; i < waveData.count; ++i)
        {
            EnemySpawnInfo info;
            info.enemyType         = waveData.enemyType;
            info.spawnTime         = waveData.startTime + i * waveData.spawnInterval;
            info.spawnLocationType = waveData.spawnLocationType;
            info.hasShield         = waveData.hasShield;
            m_spawnInfoList.push_back(info);
        }
    }

    // スポーン時刻昇順にソートすることで、Update() 内で先頭から順に処理できるようにする
    std::sort(m_spawnInfoList.begin(), m_spawnInfoList.end(),
              [](const EnemySpawnInfo& a, const EnemySpawnInfo& b) { return a.spawnTime < b.spawnTime; });

    // チュートリアルモードでは Starting 演出をスキップして即座に Active へ遷移する
    m_state     = m_isTutorialMode ? WaveState::Active : WaveState::Starting;
    m_waveTimer = 0.0f;
}

void WaveManager::NextWave()
{
    m_isWaveActive = false;
    m_currentWave++;

    bool  hasNextWave   = false;
    float nextInterval  = kDefaultWaveInterval;

    for (const auto& wave : m_waveDataList)
    {
        if (wave.wave == m_currentWave) hasNextWave = true;
        // インターバル時間は「終了した Wave（= m_currentWave - 1）」のデータから取る。
        // waveInterval は「そのWaveが終わった後の待機時間」という意味のフィールドなので、
        // 次のWaveではなく終わったWaveの方から読む。
        if (wave.wave == m_currentWave - 1 && wave.waveInterval > 0.0f) nextInterval = wave.waveInterval;
    }

    if (!hasNextWave)
        m_haveAllWavesCompleted = true; // 次のWaveデータが存在しない → 全Wave完了
    else
        m_waveIntervalTimer = nextInterval; // インターバル待機を開始する
}

bool WaveManager::IsCurrentWaveCleared()
{
    // 未スポーンの敵が残っている間はクリア条件を満たさない
    if (m_currentSpawnIndex < m_spawnInfoList.size()) return false;
    // 全スポーン済みかつ生存中の敵が0体になったらクリア
    if (GetAliveEnemyCount() > 0)                     return false;
    return true;
}

void WaveManager::OnEnemyDeath(const VECTOR& pos)
{
    // 登録済みコールバックを通じてアイテムドロップ等の外部処理に通知する
    if (m_onEnemyDeathCallback) m_onEnemyDeathCallback(pos);
}

int WaveManager::GetAliveEnemyCount() const
{
    // IsActive（アクティブ状態）かつ IsAlive（生存状態）の敵のみをカウントする。
    // 死亡アニメーション中の敵は IsActive=true, IsAlive=false のため計上されない。
    int count = 0;
    for (const auto& enemy : m_enemyList)
    {
        if (enemy->IsActive() && enemy->IsAlive()) count++;
    }
    return count;
}

void WaveManager::SpawnTutorialWave(int tutorialWaveId)
{
    // チュートリアル専用Waveデータに切り替えて指定IDのWaveを開始する。
    // m_isTutorialMode を立てることで、Wave クリア後に自動的に NextWave() を呼ばないようにする。
    // Reset() は不要（SceneMain 側で Init() 後に呼ばれるため）。
    m_isTutorialMode = true;
    m_waveDataList   = WaveDataLoader::LoadWaveData("data/CSV/TutorialWaves.csv");
    m_currentWave    = tutorialWaveId;

    VECTOR playerPos = VGet(0.0f, 0.0f, 0.0f);
    if (Game::GetPlayer()) playerPos = Game::GetPlayer()->GetPos();

    StartCurrentWave(playerPos);
}
