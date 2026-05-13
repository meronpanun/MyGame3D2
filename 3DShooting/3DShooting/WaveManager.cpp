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
    // 繝励Ξ繧､繝､繝ｼ縺九ｉ縺ｮ譛蟆剰ｷ晞屬
    constexpr float kMinSpawnDistance = 200.0f;

    // 蜃ｺ迴ｾ菴咲ｽｮ縺ｮ譛螟ｧ隧ｦ陦悟屓謨ｰ
    constexpr int kMaxSpawnAttempts = 100;

    // 遽・峇縺瑚ｨｭ螳壹＆繧後※縺・↑縺・ｴ蜷医・繝・ヵ繧ｩ繝ｫ繝井ｽ咲ｽｮ
    constexpr VECTOR kDefaultRoadFloorPos = { 0.0f, -0.5f, 3.0f };

    // 蝨ｰ髱｢縺ｮ譛蟆乗怙螟ｧ蛟､蠎ｧ讓・
    constexpr VECTOR kDefaultRoadFloorMin = { -1000.0f, 0.0f, -1000.0f }; // 蠎翫・譛蟆丞ｺｧ讓・
    constexpr VECTOR kDefaultRoadFloorMax = { 1000.0f, 0.0f, 1000.0f };   // 蠎翫・譛螟ｧ蠎ｧ讓・
}

bool WaveManager::s_shouldDrawSpawnAreas = false;
bool WaveManager::s_shouldShowActiveEnemyCount = false;
bool WaveManager::s_shouldShowDrawnEnemyCount = false;

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
    // 謨ｵ縺ｮ繝｢繝・Ν繧偵Ο繝ｼ繝・
    EnemyNormal::LoadModel();
    EnemyRunner::LoadModel();
    EnemyAcid::LoadModel();
    EnemyBoss::LoadModel();
}

WaveManager::~WaveManager()
{
    // 謨ｵ縺ｮ繝｢繝・Ν繧定ｧ｣謾ｾ
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

    // 繧ｦ繧ｧ繝ｼ繝悶ョ繝ｼ繧ｿ繧偵Ο繝ｼ繝・(WaveDataLoader繧剃ｽｿ逕ｨ)
    m_waveDataList = WaveDataLoader::LoadWaveData("data/CSV/WaveData.csv");
    // 繧ｹ繝昴・繝ｳ繧ｨ繝ｪ繧｢繝・・繧ｿ繧偵Ο繝ｼ繝・(WaveDataLoader繧剃ｽｿ逕ｨ)
    m_spawnAreaList = WaveDataLoader::LoadSpawnAreaData("data/CSV/SpawnAreaData.csv");

    // 繧ｰ繝ｪ繝・ラ蛻晄悄蛹・(繧ｻ繝ｫ繧ｵ繧､繧ｺ繧・00縺ｫ邵ｮ蟆上＠縺ｦ蛻・牡繧定ｦ九ｄ縺吶￥縺吶ｋ)
    m_collisionGrid.Init(m_roadFloorMin, m_roadFloorMax, 100.0f);

    // 謨ｵ繝励・繝ｫ蛻晄悄蛹・
    InitEnemyPools();

    // 繝√Η繝ｼ繝医Μ繧｢繝ｫ驕疲・蛻､螳壹さ繝ｫ繝舌ャ繧ｯ
    auto deathTypeCallback = [this](const VECTOR& pos, AttackType type) {
        if (m_currentWave == 1)
        {
            if (type == AttackType::Shoot) m_hasClearedShotTutorial = true;
            if (type == AttackType::Tackle) m_hasClearedTackleTutorial = true;
        }
    };

    // 蜈ｨ謨ｵ繝励・繝ｫ縺ｫ繧ｳ繝ｼ繝ｫ繝舌ャ繧ｯ險ｭ螳・
    auto setCallback = [&](auto& pool) {
        for (auto& enemy : pool) enemy->SetOnDeathWithTypeCallback(deathTypeCallback);
    };
    setCallback(m_enemyNormalPool);
    setCallback(m_enemyRunnerPool);
    setCallback(m_enemyAcidPool);
    setCallback(m_enemyBossPool);

    // 謨ｵ縺ｮ豁ｻ莠｡譎ゅさ繝ｼ繝ｫ繝舌ャ繧ｯ繧定ｨｭ螳・
    SetOnEnemyDeathCallback([this](const VECTOR& pos) {
        // 豁ｻ莠｡縺励◆謨ｵ繧堤音螳・
        for (auto& enemy : m_enemyList)
        {
            if (enemy->GetPos().x == pos.x && enemy->GetPos().y == pos.y && enemy->GetPos().z == pos.z)
            {
                if (!enemy) continue;
                // 繝√Η繝ｼ繝医Μ繧｢繝ｫ驕疲・蛻､螳・
                if (m_currentWave == 1)
                {
                    if (enemy->GetLastAttackType() == AttackType::Shoot) m_hasClearedShotTutorial = true;
                    if (enemy->GetLastAttackType() == AttackType::Tackle) m_hasClearedTackleTutorial = true;
                }
                break;
            }
        }
    });
}

void WaveManager::InitEnemyPools()
{
    // 蜷・雰遞ｮ縺斐→縺ｫ蜈ｨ繧ｦ繧ｧ繝ｼ繝悶〒蜷梧凾縺ｫ蜃ｺ迴ｾ縺吶ｋ譛螟ｧ謨ｰ繧定ｨ育ｮ・
    std::map<int, int> normalPerWave, runnerPerWave, acidPerWave, bossPerWave;
    for (const auto& wave : m_waveDataList)
    {
        if (wave.enemyType == "NormalEnemy") normalPerWave[wave.wave] += wave.count;
        if (wave.enemyType == "RunnerEnemy") runnerPerWave[wave.wave] += wave.count;
        if (wave.enemyType == "AcidEnemy") acidPerWave[wave.wave] += wave.count;
        if (wave.enemyType == "Boss") bossPerWave[wave.wave] += wave.count;
    }
    int maxNormal = 0, maxRunner = 0, maxAcid = 0, maxBoss = 0;

    // 蜷・え繧ｧ繝ｼ繝悶〒縺ｮ譛螟ｧ蜃ｺ迴ｾ謨ｰ繧定ｨ育ｮ・
    for (const auto& [wave, cnt] : normalPerWave) maxNormal = (std::max)(maxNormal, cnt);
    for (const auto& [wave, cnt] : runnerPerWave) maxRunner = (std::max)(maxRunner, cnt);
    for (const auto& [wave, cnt] : acidPerWave) maxAcid = (std::max)(maxAcid, cnt);
    for (const auto& [wave, cnt] : bossPerWave) maxBoss = (std::max)(maxBoss, cnt);

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
    m_haveAllWavesCompleted = false;

    m_enemyList.clear();
    m_spawnInfoList.clear();

    // 蜈ｨ繝励・繝ｫ縺ｮ謨ｵ繧帝撼繧｢繧ｯ繝・ぅ繝門喧
    for (auto& enemy : m_enemyNormalPool) enemy->SetActive(false);
    for (auto& enemy : m_enemyRunnerPool) enemy->SetActive(false);
    for (auto& enemy : m_enemyAcidPool) enemy->SetActive(false);
    for (auto& enemy : m_enemyBossPool) enemy->SetActive(false);

    if (m_isTutorialMode)
    {
        m_waveDataList = WaveDataLoader::LoadWaveData("data/CSV/WaveData.csv");
    }

    m_hasClearedShotTutorial = false;
    m_hasClearedTackleTutorial = false;
    m_isTutorialMode = false;
}

void WaveManager::Update()
{
    // 繧ｰ繝ｪ繝・ラ繧偵け繝ｪ繧｢・域雰縺ｮ縺ｿ縲√せ繝・・繧ｸ繝・・繧ｿ縺ｯ菫晄戟・・
    m_collisionGrid.ClearEnemies();
    m_collisionGrid.ResetAccessFlags();
    m_collisionGrid.ResetStats(); // 邨ｱ險医・繝ｪ繧ｻ繝・ヨ
    m_collisionGrid.SetTotalEnemies(0); // 邱乗雰謨ｰ縺ｮ繧ｫ繧ｦ繝ｳ繝育畑繝ｪ繧ｻ繝・ヨ

    for (auto& pEnemy : m_enemyList)
    {
        if (pEnemy->IsActive() && pEnemy->IsAlive())
        {
            m_collisionGrid.RegisterEnemy(pEnemy.get());
        }
    }

    // 繧ｦ繧ｧ繝ｼ繝也憾諷九↓繧医ｋ譖ｴ譁ｰ
    if (m_state == WaveState::Starting)
    {
        // 貍泌・繧ｿ繧､繝槭・縺ｮ譖ｴ譁ｰ (105繝輔Ξ繝ｼ繝 = 邏・.75遘・
        // 譛ｬ譚･縺ｯ繝｡繝ｳ繝仙､画焚縺ｧ繧ｿ繧､繝槭・繧呈戟縺､縺ｹ縺阪□縺後∫ｰ｡譏灘喧縺ｮ縺溘ａ譌｢蟄倥・繧ｿ繧､繝槭・繧呈ｵ∫畑縺吶ｋ縺区眠隕剰ｿｽ蜉縺悟ｿ・ｦ・
        // 莉雁屓縺ｯWaveManager縺ｫ繧ｿ繧､繝槭・縺御ｸ崎ｶｳ縺励※縺・ｋ縺溘ａ縲［_waveTimer繧呈ｵ∫畑
        m_waveTimer += 1.0f * Game::GetTimeScale();
        if (m_waveTimer >= 105.0f)
        {
            m_state = WaveState::Active;
            m_waveTimer = 0.0f;
        }
    }

    if (m_haveAllWavesCompleted)
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
            // Starting迥ｶ諷具ｼ域ｼ泌・荳ｭ・峨・繧ｹ繝昴・繝ｳ縺励↑縺・
            if (m_currentSpawnIndex < m_spawnInfoList.size() && m_state == WaveState::Active)
            {
                m_spawnTimer += (1.0f / 60.0f) * Game::GetTimeScale();
                while (m_currentSpawnIndex < m_spawnInfoList.size() && GetAliveEnemyCount() < 40)
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

                        std::shared_ptr<EnemyBase> pEnemy = CreateEnemy(spawnInfo.enemyType, spawnInfo.spawnPos, spawnInfo.hasShield);
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

    // 髱槭い繧ｯ繝・ぅ繝悶↑謨ｵ繧偵Μ繧ｹ繝医°繧牙炎髯､
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

    // 繧ｰ繝ｪ繝・ラ縺ｯUpdate()縺ｮ譎らせ縺ｧ讒狗ｯ画ｸ医∩縺ｪ縺ｮ縺ｧ縺薙％縺ｧ縺ｯ陦後ｏ縺ｪ縺・

    EnemyUpdateContext context = { bullets, tackleInfo, player, activeEnemies, collisionData, pEffect, &m_collisionGrid };

    for (auto& pEnemy : m_enemyList)
    {
        if (pEnemy->IsActive())
        {
            pEnemy->Update(context);
        }
    }
}

void WaveManager::DrawEnemies(const std::vector<Stage::StageCollisionData>& collisionData, bool isTutorial)
{
    for (const auto& pEnemy : m_enemyList)
    {
        if (pEnemy->IsActive())
        {
            pEnemy->Draw();
        }
    }

    // 遨ｺ髢灘・蜑ｲ繧ｰ繝ｪ繝・ラ縺ｮ謠冗判
    m_collisionGrid.Draw(collisionData);

    // 繝・ヰ繝・げ陦ｨ遉ｺ
    // (UI繧ｯ繝ｩ繧ｹ縺ｫ遘ｻ隴ｲ縲√ヵ繝ｩ繧ｰ繝√ぉ繝・け縺ｯWaveManager縺梧戟縺､縺九ゞI蛛ｴ縺ｧ謖√▽縺・
    // 縺薙％縺ｧ縺ｯ莠呈鋤諤ｧ縺ｮ縺溘ａs_isDrawSpawnAreas繧剃ｽｿ逕ｨ縺励ゞI繧ｯ繝ｩ繧ｹ縺ｫ貂｡縺・
    if (s_shouldDrawSpawnAreas)
    {
        // 繝・ヰ繝・げ謠冗判縺ｯSceneMain縺ｪ縺ｩ縺ｧ驕ｩ蛻・↓陦後≧縺九・
        // 蠢・ｦ√↑繧蔚IManager邨檎罰縺ｧWaveUI縺ｫ謖・､ｺ縺吶ｋ
    }
}

void WaveManager::DrawDebugUI()
{
    m_collisionGrid.DrawUI();
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
    m_hasSetRoadFloorBounds = true;

    // 遽・峇縺悟､画峩縺輔ｌ縺溘・縺ｧ繧ｰ繝ｪ繝・ラ繧貞・蛻晄悄蛹・
    m_collisionGrid.Init(m_roadFloorMin, m_roadFloorMax, 100.0f);
}

void WaveManager::RegisterStageToGrid(const std::vector<Stage::StageCollisionData>& collisionData)
{
    for (const auto& tri : collisionData)
    {
        m_collisionGrid.RegisterStageTriangle(tri);
    }
}

VECTOR WaveManager::GenerateRandomSpawnPos(const VECTOR& playerPos)
{
    if (!m_hasSetRoadFloorBounds)
    {
        return kDefaultRoadFloorPos;
    }

    unsigned int seed = static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count());
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
    std::vector<const SpawnAreaInfo*> candidates;

    float targetY = -999.0f;
    if (spawnLocationType == 1) targetY = 200.0f;
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

        std::vector<const SpawnAreaInfo*> validCandidates;
        for (const auto* area : candidates)
        {
            float halfSizeX = area->size.x * 0.5f;
            float halfSizeZ = area->size.z * 0.5f;
            bool isInsideX = playerPos.x >= (area->center.x - halfSizeX) && playerPos.x <= (area->center.x + halfSizeX);
            bool isInsideZ = playerPos.z >= (area->center.z - halfSizeZ) && playerPos.z <= (area->center.z + halfSizeZ);
            if (isInsideX && isInsideZ) continue;

            VECTOR diff = VSub(area->center, playerPos);
            float dist = VSize(diff);
            float safeDistance = (std::max)(area->size.x, area->size.z) + 200.0f;
            if (dist >= safeDistance) validCandidates.push_back(area);
        }
        if (!validCandidates.empty())
        {
            std::sort(validCandidates.begin(), validCandidates.end(), [&](const SpawnAreaInfo* a, const SpawnAreaInfo* b) {
                return VSize(VSub(a->center, playerPos)) > VSize(VSub(b->center, playerPos));
            });
            return validCandidates[0];
        }
        else
        {
            if (!candidates.empty())
            {
                std::sort(candidates.begin(), candidates.end(), [&](const SpawnAreaInfo* a, const SpawnAreaInfo* b) {
                    return VSize(VSub(a->center, playerPos)) > VSize(VSub(b->center, playerPos));
                });
                return candidates[0];
            }
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

    std::vector<const SpawnAreaInfo*> filteredCandidates;
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
            for (const auto* area : candidates)
            {
                if (VSize(VSub(area->center, playerPos)) >= kLongRangeThreshold) filteredCandidates.push_back(area);
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
    unsigned int seed = static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count());
    std::mt19937 gen(seed);
    std::uniform_real_distribution<float> xDist(-0.5f, 0.5f);
    std::uniform_real_distribution<float> zDist(-0.5f, 0.5f);

    VECTOR offset = VGet(xDist(gen) * area.size.x, 0, zDist(gen) * area.size.z);
    return VAdd(area.center, offset);
}

std::shared_ptr<EnemyBase> WaveManager::CreateEnemy(const std::string& enemyType, const VECTOR& spawnPos, bool hasShield)
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

        for (const auto& data : m_enemyData)
        {
            if (data.name == enemyType)
            {
                break;
            }
        }

        if (enemyType == "NormalEnemy")
        {
            auto pNormalEnemy = std::dynamic_pointer_cast<EnemyNormal>(pEnemy);
            if (pNormalEnemy)
            {
                pNormalEnemy->SetHasShield(hasShield);
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
                info.hasShield = waveData.hasShield;
                m_spawnInfoList.push_back(info);
            }
        }
    }

    std::sort(m_spawnInfoList.begin(), m_spawnInfoList.end(), [](const EnemySpawnInfo& a, const EnemySpawnInfo& b) {
        return a.spawnTime < b.spawnTime;
    });

    if (!m_isTutorialMode)
    {
        m_state = WaveState::Starting;
        m_waveTimer = 0.0f;
    }
    else
    {
        m_state = WaveState::Active;
    }
}

void WaveManager::NextWave()
{
    m_isWaveActive = false;
    m_currentWave++;

    bool hasNextWave = false;
    float nextInterval = 3.0f;

    for (const auto& wave : m_waveDataList)
    {
        if (wave.wave == m_currentWave)
        {
            hasNextWave = true;
        }
        if (wave.wave == m_currentWave - 1)
        {
            if (wave.waveInterval > 0) nextInterval = wave.waveInterval;
        }
    }

    if (!hasNextWave)
    {
        m_haveAllWavesCompleted = true;
    }
    else
    {
        m_waveIntervalTimer = nextInterval;
    }
}

bool WaveManager::IsCurrentWaveCleared()
{
    if (m_currentSpawnIndex < m_spawnInfoList.size()) return false;

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
    m_waveDataList = WaveDataLoader::LoadWaveData("data/CSV/TutorialWaves.csv");

    m_currentWave = tutorialWaveId;

    VECTOR playerPos = VGet(0, 0, 0);
    if (Game::m_pPlayer)
    {
        playerPos = Game::m_pPlayer->GetPos();
    }

    StartCurrentWave(playerPos);
}
