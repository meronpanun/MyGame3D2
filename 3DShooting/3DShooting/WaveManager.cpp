#include "WaveManager.h"
#include "Bullet.h"
#include "CollisionGrid.h"
#include "EffekseerForDXLib.h"
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
    // プレイヤーからの最小距離
    constexpr float kMinSpawnDistance = 200.0f;

    // 出現位置の最大試行回数
    constexpr int kMaxSpawnAttempts = 100;

    // 範囲が設定されていない場合のデフォルト位置
    constexpr VECTOR kDefaultRoadFloorPos = { 0.0f, -0.5f, 3.0f };

    // 地面の最小最大値座標
    constexpr VECTOR kDefaultRoadFloorMin = { -1000.0f, 0.0f, -1000.0f }; // 床の最小座標
    constexpr VECTOR kDefaultRoadFloorMax = { 1000.0f, 0.0f, 1000.0f };   // 床の最大座標
}

bool WaveManager::s_isDrawSpawnAreas = false;
bool WaveManager::s_isShowActiveEnemyCount = false;
bool WaveManager::s_isShowDrawnEnemyCount = false;

WaveManager::WaveManager()
    : m_currentWave(1)
    , m_waveTimer(0.0f)
    , m_spawnTimer(0.0f)
    , m_currentSpawnIndex(0)
    , m_isWaveActive(false)
    , m_isAllWavesCompleted(false)
    , m_isWave1Loaded(false)
    , m_isWave1EnemySpawned(false)
    , m_roadFloorMin(kDefaultRoadFloorMin)
    , m_roadFloorMax(kDefaultRoadFloorMax)
    , m_isRoadFloorBoundsSet(false)
    , m_onEnemyDeathCallback(nullptr)
    , m_waveIntervalTimer(0.0f)
    , m_totalSpawnedCount(0)
    , m_isShotTutorialCleared(false)
    , m_isTackleTutorialCleared(false)
    , m_isTutorialMode(false)
{

    // UI管理クラスの生成
    m_pWaveUI = std::make_unique<WaveUI>();

    // 敵のモデルをロード
    EnemyNormal::LoadModel();
    EnemyRunner::LoadModel();
    EnemyAcid::LoadModel();
    EnemyBoss::LoadModel();
}

WaveManager::~WaveManager()
{
    // 敵のモデルを解放
    EnemyNormal::DeleteModel();
    EnemyRunner::DeleteModel();
    EnemyAcid::DeleteModel();
    EnemyBoss::DeleteModel();
}

void WaveManager::Init()
{
    m_enemyData = TransformDataLoader::LoadDataCSV("data/CSV/CharacterTransfromData.csv");
    m_enemyList.clear();
    m_spawnInfoList.clear();

    // UI初期化
    m_pWaveUI->Init();

    // ウェーブデータをロード (WaveDataLoaderを使用)
    m_waveDataList = WaveDataLoader::LoadWaveData("data/CSV/WaveData.csv");
    // スポーンエリアデータをロード (WaveDataLoaderを使用)
    m_spawnAreaList = WaveDataLoader::LoadSpawnAreaData("data/CSV/SpawnAreaData.csv");

    // グリッド初期化
    m_collisionGrid.Init(m_roadFloorMin, m_roadFloorMax, 150.0f);

    // 敵プール初期化
    InitEnemyPools();

    // チュートリアル達成判定コルバック
    auto deathTypeCallback = [this](const VECTOR& pos, AttackType type) {
        if (m_currentWave == 1)
        {
            if (type == AttackType::Shoot) m_isShotTutorialCleared = true;
            if (type == AttackType::Tackle) m_isTackleTutorialCleared = true;
        }
    };

    // 全敵プールにコールバック設定
    auto setCallback = [&](auto& pool) {
        for (auto& enemy : pool) enemy->SetOnDeathWithTypeCallback(deathTypeCallback);
    };
    setCallback(m_enemyNormalPool);
    setCallback(m_enemyRunnerPool);
    setCallback(m_enemyAcidPool);
    setCallback(m_enemyBossPool);

    // 敵の死亡時コールバックを設定
    SetOnEnemyDeathCallback([this](const VECTOR& pos) {
        // 死亡した敵を特定
        // m_enemyListから検索すればプールを全検索する必要はない
        // ただしOnEnemyDeathはm_enemyListから削除される前に呼ばれる必要がある
        // 現状の仕様ではUpdate内で削除されるので、ここでm_enemyListを使うのは安全

        for (auto& enemy : m_enemyList)
        {
            if (enemy->GetPos().x == pos.x && enemy->GetPos().y == pos.y && enemy->GetPos().z == pos.z)
            {
                if (!enemy) continue;
                // チュートリアル達成判定
                if (m_currentWave == 1)
                {
                    if (enemy->GetLastAttackType() == AttackType::Shoot) m_isShotTutorialCleared = true;
                    if (enemy->GetLastAttackType() == AttackType::Tackle) m_isTackleTutorialCleared = true;
                }
                break;
            }
        }
    });
}

void WaveManager::InitEnemyPools()
{
    // 各敵種ごとに全ウェーブで同時に出現する最大数を計算
    std::map<int, int> normalPerWave, runnerPerWave, acidPerWave, bossPerWave;
    for (const auto& wave : m_waveDataList)
    {
        if (wave.enemyType == "NormalEnemy") normalPerWave[wave.wave] += wave.count;
        if (wave.enemyType == "RunnerEnemy") runnerPerWave[wave.wave] += wave.count;
        if (wave.enemyType == "AcidEnemy") acidPerWave[wave.wave] += wave.count;
        if (wave.enemyType == "Boss") bossPerWave[wave.wave] += wave.count;
    }
    int maxNormal = 0, maxRunner = 0, maxAcid = 0, maxBoss = 0;

    // 各ウェーブでの最大出現数を計算
    for (const auto& [wave, cnt] : normalPerWave) maxNormal = (std::max)(maxNormal, cnt);
    for (const auto& [wave, cnt] : runnerPerWave) maxRunner = (std::max)(maxRunner, cnt);
    for (const auto& [wave, cnt] : acidPerWave) maxAcid = (std::max)(maxAcid, cnt);
    for (const auto& [wave, cnt] : bossPerWave) maxBoss = (std::max)(maxBoss, cnt);

    auto ensurePoolSize = []<typename T>(std::vector<std::shared_ptr<T>>& pool, int size) {
        for (int i = pool.size(); i < size; ++i)
        {
            auto pEnemy = std::make_shared<T>();
            pEnemy->Init();
            pEnemy->SetActive(false);
            pool.push_back(pEnemy);
        }
    };

    ensurePoolSize(m_enemyNormalPool, maxNormal);
    ensurePoolSize(m_enemyRunnerPool, maxRunner);
    ensurePoolSize(m_enemyAcidPool, maxAcid);
    ensurePoolSize(m_enemyBossPool, maxBoss);
}

void WaveManager::Reset()
{
    m_currentWave = 1;
    m_waveTimer = 0.0f;
    m_spawnTimer = 0.0f;
    m_currentSpawnIndex = 0;
    m_waveIntervalTimer = 0.0f;
    m_isWaveActive = false;
    m_isAllWavesCompleted = false;

    m_enemyList.clear();
    m_spawnInfoList.clear();

    // 全プールの敵を非アクティブ化
    for (auto& enemy : m_enemyNormalPool) enemy->SetActive(false);
    for (auto& enemy : m_enemyRunnerPool) enemy->SetActive(false);
    for (auto& enemy : m_enemyAcidPool) enemy->SetActive(false);
    for (auto& enemy : m_enemyBossPool) enemy->SetActive(false);

    if (m_isTutorialMode)
    {
        m_waveDataList = WaveDataLoader::LoadWaveData("data/CSV/WaveData.csv");
    }

    m_isShotTutorialCleared = false;
    m_isTackleTutorialCleared = false;
    m_isTutorialMode = false;
}

void WaveManager::Update()
{
    // UI更新
    m_pWaveUI->Update();

    if (m_isAllWavesCompleted)
    {
        return;
    }

    if (m_isWaveActive)
    {
        if (IsCurrentWaveCleared())
        {
            if (!m_isTutorialMode)
            {
                NextWave();
            }
        }
        else
        {
            // アニメーション中はスポーンしない
            if (m_currentSpawnIndex < m_spawnInfoList.size() && !m_pWaveUI->IsAnimating())
            {
                m_spawnTimer += (1.0f / 60.0f) * Game::GetTimeScale();
                while (m_currentSpawnIndex < m_spawnInfoList.size())
                {
                    EnemySpawnInfo& spawnInfo = m_spawnInfoList[m_currentSpawnIndex];
                    if (m_spawnTimer >= spawnInfo.spawnTime && !spawnInfo.isSpawned)
                    {
                        VECTOR currentPlayerPos = VGet(0.0f, 0.0f, 0.0f);
                        if (Game::m_pPlayer)
                        {
                            currentPlayerPos = Game::m_pPlayer->GetPos();
                        }
                        spawnInfo.spawnPos = GenerateSpawnPos(m_isTutorialMode ? 1 : 0, spawnInfo.enemyType, currentPlayerPos, spawnInfo.spawnLocationType);

                        std::shared_ptr<EnemyBase> pEnemy = CreateEnemy(spawnInfo.enemyType, spawnInfo.spawnPos);
                        if (pEnemy)
                        {
                            m_enemyList.push_back(pEnemy);
                            spawnInfo.isSpawned = true;
                        }
                        m_currentSpawnIndex++;
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }
    }
    else
    {
        if (m_waveIntervalTimer > 0.0f)
        {
            m_waveIntervalTimer -= (1.0f / 60.0f) * Game::GetTimeScale();
        }
        else
        {
            if (m_currentWave <= 5)
            {
                VECTOR playerPos = VGet(0.0f, 0.0f, 0.0f);
                StartCurrentWave(playerPos);
            }
        }
    }

    // 非アクティブな敵をリストから削除
    m_enemyList.erase(std::remove_if(m_enemyList.begin(), m_enemyList.end(), [](const std::shared_ptr<EnemyBase>& pEnemy) {
                          return !pEnemy->IsActive();
                      }),
                      m_enemyList.end());
}

void WaveManager::UpdateEnemies(
    std::vector<Bullet>& bullets,
    const Player::TackleInfo& tackleInfo,
    const Player& player,
    const std::vector<Stage::StageCollisionData>& collisionData,
    Effect* pEffect)
{
    std::vector<EnemyBase*> activeEnemies;
    for (const auto& pEnemy : m_enemyList)
    {
        if (pEnemy->IsActive() && pEnemy->IsAlive())
        {
            activeEnemies.push_back(pEnemy.get());
        }
    }

    m_collisionGrid.Clear();
    for (auto* enemy : activeEnemies)
    {
        m_collisionGrid.RegisterEnemy(enemy);
    }

    EnemyUpdateContext context = { bullets, tackleInfo, player, activeEnemies, collisionData, pEffect, &m_collisionGrid };

    for (auto& pEnemy : m_enemyList)
    {
        if (pEnemy->IsActive())
        {
            pEnemy->Update(context);
        }
    }
}

void WaveManager::DrawEnemies(bool isTutorial)
{
    for (const auto& pEnemy : m_enemyList)
    {
        if (pEnemy->IsActive())
        {
            pEnemy->Draw();
        }
    }

    // デバッグ表示
    // (UIクラスに移譲、フラグチェックはWaveManagerが持つか、UI側で持つか)
    // ここでは互換性のためs_isDrawSpawnAreasを使用し、UIクラスに渡す
    if (s_isDrawSpawnAreas)
    {
        m_pWaveUI->DrawDebugSpawnAreas(m_spawnAreaList, isTutorial || m_isTutorialMode);
    }
}

void WaveManager::DrawWaveUI()
{
    if (!m_isTutorialMode)
    {
        m_pWaveUI->DrawWaveUI(m_currentWave, m_isWaveActive, m_isAllWavesCompleted);
    }
}

void WaveManager::DrawDebugInfo()
{
    float nextSpawnTime = 0.0f;
    int remainingEnemiesInWave = 0;
    if (m_currentSpawnIndex < m_spawnInfoList.size())
    {
        nextSpawnTime = m_spawnInfoList[m_currentSpawnIndex].spawnTime;
        remainingEnemiesInWave = static_cast<int>(m_spawnInfoList.size() - m_currentSpawnIndex);
    }

    m_pWaveUI->DrawDebugInfo(m_currentWave, GetAliveEnemyCount(), m_totalSpawnedCount, m_waveTimer, m_spawnTimer, nextSpawnTime, remainingEnemiesInWave);
}

void WaveManager::SetOnEnemyDeathCallback(std::function<void(const VECTOR&)> callback)
{
    m_onEnemyDeathCallback = callback;
}

void WaveManager::SetOnEnemyHitCallback(std::function<void(EnemyBase::HitPart, float)> cb)
{
    m_onEnemyHitCallback = cb;
}

void WaveManager::SetRoadFloorBounds(const VECTOR& minPos, const VECTOR& maxPos)
{
    m_roadFloorMin = minPos;
    m_roadFloorMax = maxPos;
    m_isRoadFloorBoundsSet = true;
}

VECTOR WaveManager::GenerateRandomSpawnPos(const VECTOR& playerPos)
{
    if (!m_isRoadFloorBoundsSet)
    {
        return kDefaultRoadFloorPos;
    }

    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 gen(seed);

    VECTOR spawnPos;
    int attempts = 0;
    bool found = false;
    do
    {
        std::uniform_real_distribution<float> xDist(m_roadFloorMin.x, m_roadFloorMax.x);
        std::uniform_real_distribution<float> zDist(m_roadFloorMin.z, m_roadFloorMax.z);

        float x = xDist(gen);
        float z = zDist(gen);
        spawnPos = VGet(x, 0.0f, z);

        VECTOR toPlayer = VSub(playerPos, spawnPos);
        toPlayer.y = 0.0f;
        float distanceToPlayer = sqrtf(toPlayer.x * toPlayer.x + toPlayer.z * toPlayer.z);

        if (distanceToPlayer >= kMinSpawnDistance)
        {
            found = true;
            break;
        }
        attempts++;
    } while (attempts < kMaxSpawnAttempts);

    if (!found)
    {
        std::uniform_real_distribution<float> xDist(m_roadFloorMin.x, m_roadFloorMax.x);
        std::uniform_real_distribution<float> zDist(m_roadFloorMin.z, m_roadFloorMax.z);
        spawnPos = VGet(xDist(gen), 0.0f, zDist(gen));
    }
    return spawnPos;
}

VECTOR WaveManager::GenerateSpawnPos(int type, const std::string& enemyType, const VECTOR& playerPos, int spawnLocationType)
{
    const SpawnAreaInfo* pArea = SelectSpawnArea(type, enemyType, playerPos, spawnLocationType);
    if (!pArea)
    {
        return GenerateRandomSpawnPos(playerPos);
    }
    return CalculateRandomSpawnPos(*pArea);
}

const SpawnAreaInfo* WaveManager::SelectSpawnArea(int type, const std::string& enemyType, const VECTOR& playerPos, int spawnLocationType)
{
    std::vector<SpawnAreaInfo> candidates;

    float targetY = -999.0f;
    if (spawnLocationType == 1) targetY = 200.0f;
    else if (spawnLocationType == 2) targetY = 562.0f;
    else if (spawnLocationType == 3) targetY = 962.0f;

    if ((m_currentWave == 1 && type == 0) || (spawnLocationType > 0 && type == 0))
    {
        float checkY = (targetY != -999.0f) ? targetY : 200.0f;
        for (const auto& area : m_spawnAreaList)
        {
            if (area.type == type && std::abs(area.center.y - checkY) < 10.0f)
            {
                candidates.push_back(area);
            }
        }

        std::vector<SpawnAreaInfo> validCandidates;
        for (const auto& area : candidates)
        {
            float halfSizeX = area.size.x * 0.5f;
            float halfSizeZ = area.size.z * 0.5f;
            bool isInsideX = playerPos.x >= (area.center.x - halfSizeX) && playerPos.x <= (area.center.x + halfSizeX);
            bool isInsideZ = playerPos.z >= (area.center.z - halfSizeZ) && playerPos.z <= (area.center.z + halfSizeZ);
            if (isInsideX && isInsideZ) continue;

            VECTOR diff = VSub(area.center, playerPos);
            float dist = VSize(diff);
            float safeDistance = (std::max)(area.size.x, area.size.z) + 200.0f;
            if (dist >= safeDistance) validCandidates.push_back(area);
        }
        if (!validCandidates.empty())
        {
            std::sort(validCandidates.begin(), validCandidates.end(), [&](const SpawnAreaInfo& a, const SpawnAreaInfo& b) {
                return VSize(VSub(a.center, playerPos)) > VSize(VSub(b.center, playerPos));
            });
            return &validCandidates[0]; // 最も遠い有効なエリア
        }
        else
        {
            if (!candidates.empty())
            {
                std::sort(candidates.begin(), candidates.end(), [&](const SpawnAreaInfo& a, const SpawnAreaInfo& b) {
                    return VSize(VSub(a.center, playerPos)) > VSize(VSub(b.center, playerPos));
                });
                return &candidates[0]; // フォールバック
            }
        }
    }
    else
    {
        for (const auto& area : m_spawnAreaList)
        {
            if (area.type == type) candidates.push_back(area);
        }
    }

    if (candidates.empty()) return nullptr;

    std::vector<SpawnAreaInfo> filteredCandidates;
    if ((m_currentWave == 1 && type == 0) || (spawnLocationType > 0 && type == 0))
    {
        filteredCandidates = candidates;
    }
    else
    {
        bool isLongRange = (enemyType == "AcidEnemy");
        const float kLongRangeThreshold = 500.0f;
        if (isLongRange)
        {
            for (const auto& area : candidates)
            {
                if (VSize(VSub(area.center, playerPos)) >= kLongRangeThreshold) filteredCandidates.push_back(area);
            }
            if (filteredCandidates.empty()) filteredCandidates = candidates;
        }
        else
        {
            filteredCandidates = candidates;
        }
    }

    return &filteredCandidates[GetRand(static_cast<int>(filteredCandidates.size()) - 1)];
}

VECTOR WaveManager::CalculateRandomSpawnPos(const SpawnAreaInfo& area)
{
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 gen(seed);
    std::uniform_real_distribution<float> xDist(-0.5f, 0.5f);
    std::uniform_real_distribution<float> zDist(-0.5f, 0.5f);

    VECTOR offset = VGet(xDist(gen) * area.size.x, 0, zDist(gen) * area.size.z);
    return VAdd(area.center, offset);
}

std::shared_ptr<EnemyBase> WaveManager::CreateEnemy(const std::string& enemyType, const VECTOR& spawnPos)
{
    std::shared_ptr<EnemyBase> pEnemy = nullptr;
    pEnemy = GetPooledEnemy(enemyType);

    if (pEnemy)
    {
        pEnemy->SetPos(spawnPos);
        pEnemy->SetActive(true);
        pEnemy->Init();
        pEnemy->SetOnDeathCallback(m_onEnemyDeathCallback);
        pEnemy->SetOnHitCallback(m_onEnemyHitCallback);
        m_totalSpawnedCount++;

        // パラメータ適用
        for (const auto& data : m_enemyData)
        {
            if (data.name == enemyType)
            {
                break;
            }
        }
    }
    return pEnemy;
}

std::shared_ptr<EnemyNormal> WaveManager::GetPooledNormalEnemy()
{
    for (auto& enemy : m_enemyNormalPool)
    {
        if (!enemy->IsActive()) return enemy;
    }
    auto pNew = std::make_shared<EnemyNormal>();
    pNew->Init();
    m_enemyNormalPool.push_back(pNew);
    return pNew;
}

std::shared_ptr<EnemyRunner> WaveManager::GetPooledRunnerEnemy()
{
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
    if (type == "NormalEnemy") return GetPooledNormalEnemy();
    if (type == "RunnerEnemy") return GetPooledRunnerEnemy();
    if (type == "AcidEnemy") return GetPooledAcidEnemy();
    if (type == "Boss") return GetPooledBossEnemy();
    return nullptr;
}

void WaveManager::StartCurrentWave(const VECTOR& playerPos)
{
    m_isWaveActive = true;
    m_waveTimer = 0.0f;
    m_spawnTimer = 0.0f;
    m_currentSpawnIndex = 0;
    m_spawnInfoList.clear();

    // WaveDataから現在のWaveのデータを抽出してSpawnInfoを作成
    for (const auto& waveData : m_waveDataList)
    {
        if (waveData.wave == m_currentWave)
        {
            float interval = waveData.spawnInterval;
            float startTime = waveData.startTime;
            int count = waveData.count;

            for (int i = 0; i < count; ++i)
            {
                EnemySpawnInfo info;
                info.enemyType = waveData.enemyType;
                info.spawnTime = startTime + i * interval;
                info.isSpawned = false;
                info.spawnLocationType = waveData.spawnLocationType;
                m_spawnInfoList.push_back(info);
            }
            if (waveData.waveInterval > 0)
            {
                // このWaveの後にインターバル設定（ここでは単純に最初に見つかったものを採用など）
                // 実際にはNextWaveでインターバル設定するので、ここではデータ保持のみか
                // 元のコードではデータメンバになかったので、NextWave時に計算または固定値だった可能性
            }
        }
    }

    // スポーン時間順にソート
    std::sort(m_spawnInfoList.begin(), m_spawnInfoList.end(), [](const EnemySpawnInfo& a, const EnemySpawnInfo& b) {
        return a.spawnTime < b.spawnTime;
    });

    // アニメーション開始
    if (!m_isTutorialMode)
    {
        m_pWaveUI->StartWaveAnimation();
    }
}

void WaveManager::NextWave()
{
    m_isWaveActive = false;
    m_currentWave++;

    // 次のWaveがあるか確認
    bool hasNextWave = false;
    float nextInterval = 3.0f; // デフォルト

    for (const auto& wave : m_waveDataList)
    {
        if (wave.wave == m_currentWave)
        {
            hasNextWave = true;
        }
        if (wave.wave == m_currentWave - 1)
        {
            // 前のウェーブのデータからインターバルを取得
            if (wave.waveInterval > 0) nextInterval = wave.waveInterval;
        }
    }

    if (!hasNextWave)
    {
        m_isAllWavesCompleted = true;
    }
    else
    {
        m_waveIntervalTimer = nextInterval;
    }
}

bool WaveManager::IsCurrentWaveCleared()
{
    // 全て出現済みか
    if (m_currentSpawnIndex < m_spawnInfoList.size()) return false;

    // アクティブな敵がいないか
    if (GetAliveEnemyCount() > 0) return false;

    return true;
}

void WaveManager::OnEnemyDeath(const VECTOR& pos)
{
    if (m_onEnemyDeathCallback)
    {
        m_onEnemyDeathCallback(pos);
    }
}

int WaveManager::GetAliveEnemyCount() const
{
    int count = 0;
    for (const auto& enemy : m_enemyList)
    {
        if (enemy->IsActive() && enemy->IsAlive()) count++;
    }
    return count;
}

void WaveManager::SpawnTutorialWave(int tutorialWaveId)
{
    m_isTutorialMode = true;
    // チュートリアル用のWaveデータをロード
    m_waveDataList = WaveDataLoader::LoadWaveData("data/CSV/TutorialWaves.csv");

    m_currentWave = tutorialWaveId;

    VECTOR playerPos = VGet(0, 0, 0);
    if (Game::m_pPlayer)
    {
        playerPos = Game::m_pPlayer->GetPos();
    }

    StartCurrentWave(playerPos);
}
