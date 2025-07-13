#include "WaveManager.h"
#include "EnemyNormal.h"
#include "EnemyRunner.h"
#include "EnemyAcid.h"
#include "DxLib.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <random>
#include <chrono>
#include <map>

WaveManager::WaveManager() :
    m_currentWave(1),
    m_waveTimer(0.0f),
    m_spawnTimer(0.0f),
    m_currentSpawnIndex(0),
    m_isWaveActive(false),
    m_isAllWavesCompleted(false),
    m_roadFloorMin(VGet(-500.0f, 0.0f, -500.0f)),
    m_roadFloorMax(VGet(500.0f, 0.0f, 500.0f)),
    m_isRoadFloorBoundsSet(false),
    m_onEnemyDeathCallback(nullptr)
{
    // 敵のテンプレートを作成
    m_enemyNormalTemplate = std::make_shared<EnemyNormal>();
    m_enemyNormalTemplate->Init();
    
    m_enemyRunnerTemplate = std::make_shared<EnemyRunner>();
    m_enemyRunnerTemplate->Init();
    
    m_enemyAcidTemplate = std::make_shared<EnemyAcid>();
    m_enemyAcidTemplate->Init();
}

WaveManager::~WaveManager()
{
}

void WaveManager::Init()
{
    m_currentWave = 1;
    m_waveTimer = 0.0f;
    m_spawnTimer = 0.0f;
    m_currentSpawnIndex = 0;
    m_isWaveActive = false;
    m_isAllWavesCompleted = false;
    
    m_enemyList.clear();
    m_spawnInfoList.clear();
    // m_precomputedSpawns.clear();
    
    // 敵のテンプレートを初期化
    if (m_enemyNormalTemplate)
    {
        m_enemyNormalTemplate->Init();
    }
    if (m_enemyRunnerTemplate)
    {
        m_enemyRunnerTemplate->Init();
    }
    if (m_enemyAcidTemplate)
    {
        m_enemyAcidTemplate->Init();
    }
    
    LoadWaveData();

    // 各敵種ごとの最大同時出現数を計算
    int maxNormal = 0, maxRunner = 0, maxAcid = 0;
    for (const auto& wave : m_waveDataList) {
        if (wave.enemyType == "NormalEnemy") maxNormal = (std::max)(maxNormal, wave.count);
        if (wave.enemyType == "RunnerEnemy") maxRunner = (std::max)(maxRunner, wave.count);
        if (wave.enemyType == "AcidEnemy")   maxAcid   = (std::max)(maxAcid,   wave.count);
    }
    // その数だけ事前にプール
    for (int i = m_enemyNormalPool.size(); i < maxNormal; ++i) {
        auto enemy = std::make_shared<EnemyNormal>();
        if (m_enemyNormalTemplate) enemy->SetModelHandle(m_enemyNormalTemplate->GetModelHandle());
        enemy->Init();
        enemy->SetActive(false);
        m_enemyNormalPool.push_back(enemy);
    }
    for (int i = m_enemyRunnerPool.size(); i < maxRunner; ++i) {
        auto enemy = std::make_shared<EnemyRunner>();
        if (m_enemyRunnerTemplate) enemy->SetModelHandle(m_enemyRunnerTemplate->GetModelHandle());
        enemy->Init();
        enemy->SetActive(false);
        m_enemyRunnerPool.push_back(enemy);
    }
    for (int i = m_enemyAcidPool.size(); i < maxAcid; ++i) {
        auto enemy = std::make_shared<EnemyAcid>();
        if (m_enemyAcidTemplate) enemy->SetModelHandle(m_enemyAcidTemplate->GetModelHandle());
        enemy->Init();
        enemy->SetActive(false);
        m_enemyAcidPool.push_back(enemy);
    }
}

void WaveManager::Update()
{
    if (m_isAllWavesCompleted)
    {
        return;
    }

    // 現在のwaveが終了しているかチェック
    if (m_isWaveActive && IsCurrentWaveCleared())
    {
        NextWave();
    }

    // waveが開始されていない場合は開始
    if (!m_isWaveActive && m_currentWave <= 3)
    {
        // プレイヤーの位置を取得（デフォルト位置を使用）
        VECTOR playerPos = VGet(0.0f, 0.0f, 0.0f);
        StartCurrentWave(playerPos);
    }

    // 敵の出現処理
    if (m_isWaveActive && m_currentSpawnIndex < m_spawnInfoList.size())
    {
        m_spawnTimer += 1.0f / 60.0f; // 60FPSを想定

        // 1フレームに1体だけ生成
        EnemySpawnInfo& spawnInfo = m_spawnInfoList[m_currentSpawnIndex];
        if (m_spawnTimer >= spawnInfo.spawnTime && !spawnInfo.isSpawned)
        {
            std::shared_ptr<EnemyBase> enemy = CreateEnemy(spawnInfo.enemyType);
            if (enemy)
            {
                enemy->SetPos(spawnInfo.spawnPosition);
                m_enemyList.push_back(enemy);
                spawnInfo.isSpawned = true;
            }
            m_currentSpawnIndex++;
        }
    }

    // 死亡した敵をリストから削除
    m_enemyList.erase(
        std::remove_if(m_enemyList.begin(), m_enemyList.end(),
            [](const std::shared_ptr<EnemyBase>& enemy) {
                return !enemy->IsAlive();
            }),
        m_enemyList.end()
    );
}

// GetEnemyListをアクティブな敵のみ返すようにする（もしくはUpdate/Drawでアクティブ判定）
void WaveManager::UpdateEnemies(std::vector<Bullet>& bullets, const Player::TackleInfo& tackleInfo, const Player& player)
{
    const float kMaxActiveDistance = 1200.0f;
    VECTOR playerPos = player.GetPos();
    for (auto& enemy : m_enemyNormalPool) {
        if (!enemy->IsActive() || !enemy->IsAlive()) continue;
        VECTOR toPlayer = VSub(enemy->GetPos(), playerPos);
        float distSq = toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y + toPlayer.z * toPlayer.z;
        if (distSq > kMaxActiveDistance * kMaxActiveDistance) continue;
        enemy->Update(bullets, tackleInfo, player);
    }
    for (auto& enemy : m_enemyRunnerPool) {
        if (!enemy->IsActive() || !enemy->IsAlive()) continue;
        VECTOR toPlayer = VSub(enemy->GetPos(), playerPos);
        float distSq = toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y + toPlayer.z * toPlayer.z;
        if (distSq > kMaxActiveDistance * kMaxActiveDistance) continue;
        enemy->Update(bullets, tackleInfo, player);
    }
    for (auto& enemy : m_enemyAcidPool) {
        if (!enemy->IsActive() || !enemy->IsAlive()) continue;
        VECTOR toPlayer = VSub(enemy->GetPos(), playerPos);
        float distSq = toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y + toPlayer.z * toPlayer.z;
        if (distSq > kMaxActiveDistance * kMaxActiveDistance) continue;
        enemy->Update(bullets, tackleInfo, player);
    }
}

void WaveManager::DrawEnemies()
{
    for (auto& enemy : m_enemyNormalPool) {
        if (!enemy->IsActive() || !enemy->IsAlive()) continue;
        enemy->Draw();
    }
    for (auto& enemy : m_enemyRunnerPool) {
        if (!enemy->IsActive() || !enemy->IsAlive()) continue;
        enemy->Draw();
    }
    for (auto& enemy : m_enemyAcidPool) {
        if (!enemy->IsActive() || !enemy->IsAlive()) continue;
        enemy->Draw();
    }
}

void WaveManager::Draw()
{
    // ここでは何もしない（DrawEnemiesで描画）
}

void WaveManager::SetOnEnemyDeathCallback(std::function<void(const VECTOR&)> callback)
{
    m_onEnemyDeathCallback = callback;
}

void WaveManager::SetRoadFloorBounds(const VECTOR& minPos, const VECTOR& maxPos)
{
    m_roadFloorMin = minPos;
    m_roadFloorMax = maxPos;
    m_isRoadFloorBoundsSet = true;
}

void WaveManager::LoadWaveData()
{
    m_waveDataList.clear();
    
    std::ifstream file("data/CSV/WaveData.csv");
    if (!file.is_open())
    {
        return;
    }

    std::string line;
    // ヘッダー行をスキップ
    std::getline(file, line);

    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string token;
        WaveData waveData;

        // Wave
        std::getline(ss, token, ',');
        waveData.wave = std::stoi(token);

        // Type
        std::getline(ss, token, ',');
        waveData.enemyType = token;

        // Count
        std::getline(ss, token, ',');
        waveData.count = std::stoi(token);

        // SpawnInterval
        std::getline(ss, token, ',');
        waveData.spawnInterval = std::stof(token);

        // delay
        std::getline(ss, token, ',');
        waveData.delay = std::stof(token);

        m_waveDataList.push_back(waveData);
    }
}

VECTOR WaveManager::GenerateRandomSpawnPosition(const VECTOR& playerPos)
{
    if (!m_isRoadFloorBoundsSet)
    {
        return VGet(0.0f, -0.5f, 3.0f);
    }

    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 gen(seed);

    float minDistanceFromPlayer = 400.0f; // プレイヤーから最低300ユニット離す
    VECTOR spawnPos;
    int attempts = 0;
    const int maxAttempts = 100;

    do {
        std::uniform_real_distribution<float> xDist(m_roadFloorMin.x, m_roadFloorMax.x);
        std::uniform_real_distribution<float> zDist(m_roadFloorMin.z, m_roadFloorMax.z);

        float x = xDist(gen);
        float z = zDist(gen);

        spawnPos = VGet(x, 0.0f, z);

        VECTOR toPlayer = VSub(playerPos, spawnPos);
        toPlayer.y = 0.0f;
        float distanceToPlayer = sqrtf(toPlayer.x * toPlayer.x + toPlayer.z * toPlayer.z);

        if (distanceToPlayer >= minDistanceFromPlayer)
            break;

        attempts++;
    } while (attempts < maxAttempts);

    return spawnPos;
}

std::shared_ptr<EnemyBase> WaveManager::CreateEnemy(const std::string& enemyType)
{
    std::shared_ptr<EnemyBase> enemy = nullptr;
    
    if (enemyType == "NormalEnemy")
    {
        auto pooled = GetPooledNormalEnemy();
        pooled->SetActive(true);
        pooled->Init();
        enemy = pooled;
    }
    else if (enemyType == "RunnerEnemy")
    {
        auto pooled = GetPooledRunnerEnemy();
        pooled->SetActive(true);
        pooled->Init();
        enemy = pooled;
    }
    else if (enemyType == "AcidEnemy")
    {
        auto pooled = GetPooledAcidEnemy();
        pooled->SetActive(true);
        pooled->Init();
        enemy = pooled;
    }
    else
    {
        printf("Warning: Unknown enemy type: %s\n", enemyType.c_str());
        return nullptr;
    }
    
    if (enemy)
    {
        // 死亡コールバック
        enemy->SetOnDeathCallback([this](const VECTOR& pos) {
            OnEnemyDeath(pos);
        });
        // アイテムドロップコールバック
        if (m_onEnemyDeathCallback) {
            enemy->SetOnDropItemCallback(m_onEnemyDeathCallback);
        }
        // ヒット時コールバック（ヒットマーク用）
        if (m_onEnemyHitCallback) {
            enemy->SetOnHitCallback(m_onEnemyHitCallback);
        }
        printf("Created enemy: %s at position (%.2f, %.2f, %.2f)\n", 
               enemyType.c_str(), enemy->GetPos().x, enemy->GetPos().y, enemy->GetPos().z);
    }
    
    return enemy;
}

std::shared_ptr<EnemyNormal> WaveManager::GetPooledNormalEnemy() {
    for (auto& enemy : m_enemyNormalPool) {
        if (!enemy->IsActive()) {
            return enemy;
        }
    }
    // プールに空きがなければ新規生成
    auto enemy = std::make_shared<EnemyNormal>();
    if (m_enemyNormalTemplate) {
        enemy->SetModelHandle(m_enemyNormalTemplate->GetModelHandle());
    }
    enemy->Init();
    m_enemyNormalPool.push_back(enemy);
    return enemy;
}
std::shared_ptr<EnemyRunner> WaveManager::GetPooledRunnerEnemy() {
    for (auto& enemy : m_enemyRunnerPool) {
        if (!enemy->IsActive()) {
            return enemy;
        }
    }
    auto enemy = std::make_shared<EnemyRunner>();
    if (m_enemyRunnerTemplate) {
        enemy->SetModelHandle(m_enemyRunnerTemplate->GetModelHandle());
    }
    enemy->Init();
    m_enemyRunnerPool.push_back(enemy);
    return enemy;
}
std::shared_ptr<EnemyAcid> WaveManager::GetPooledAcidEnemy() {
    for (auto& enemy : m_enemyAcidPool) {
        if (!enemy->IsActive()) {
            return enemy;
        }
    }
    auto enemy = std::make_shared<EnemyAcid>();
    if (m_enemyAcidTemplate) {
        enemy->SetModelHandle(m_enemyAcidTemplate->GetModelHandle());
    }
    enemy->Init();
    m_enemyAcidPool.push_back(enemy);
    return enemy;
}

void WaveManager::StartCurrentWave(const VECTOR& playerPos)
{
    m_spawnInfoList.clear();
    m_currentSpawnIndex = 0;
    m_spawnTimer = 0.0f;
    m_isWaveActive = true;

    // 現在のwaveのデータを取得
    std::vector<WaveData> currentWaveData;
    for (const WaveData& waveData : m_waveDataList)
    {
        if (waveData.wave == m_currentWave)
        {
            currentWaveData.push_back(waveData);
        }
    }

    // 出現情報を作成
    float currentTime = 0.0f;
    for (const WaveData& waveData : currentWaveData)
    {
        for (int i = 0; i < waveData.count; ++i)
        {
            EnemySpawnInfo spawnInfo;
            spawnInfo.enemyType = waveData.enemyType;
            spawnInfo.spawnPosition = GenerateRandomSpawnPosition(playerPos);
            spawnInfo.spawnTime = currentTime + waveData.delay + (i * waveData.spawnInterval);
            spawnInfo.isSpawned = false;
            
            m_spawnInfoList.push_back(spawnInfo);
        }
    }
    
    // デバッグ出力
    printf("Starting Wave %d with %zu enemies\n", m_currentWave, m_spawnInfoList.size());
}

void WaveManager::NextWave()
{
    printf("Wave %d completed, moving to Wave %d\n", m_currentWave, m_currentWave + 1);
    
    m_currentWave++;
    m_isWaveActive = false;
    
    if (m_currentWave > 3)
    {
        m_isAllWavesCompleted = true;
        printf("All waves completed!\n");
    }
}

bool WaveManager::IsCurrentWaveCleared()
{
    // 現在のwaveの敵がすべて出現済みで、生存している敵がいない場合
    if (m_currentSpawnIndex >= m_spawnInfoList.size())
    {
        for (const std::shared_ptr<EnemyBase>& enemy : m_enemyList)
        {
            if (enemy->IsAlive())
            {
                return false;
            }
        }
        return true;
    }
    
    return false;
}

void WaveManager::OnEnemyDeath(const VECTOR& position)
{
    // デバッグ出力
    printf("Enemy died at position: (%.2f, %.2f, %.2f)\n", position.x, position.y, position.z);
    if (m_onEnemyDeathCallback)
    {
        m_onEnemyDeathCallback(position);
    }
    // 死亡した敵を非アクティブ化（プールに戻す）
    // 呼び出し元でenemy->SetActive(false)を必ず行うようにする
}

void WaveManager::DrawDebugInfo()
{
    // デバッグ情報の表示位置（画面一番上、左寄せでタックルデバッグと被らないように）
    int y = 10;
    int lineHeight = 25;
    int itemSpacing = 100; // 項目間の間隔をさらに狭く
    int startX = 10; // 左寄せで開始
    
    // フォントサイズを設定
    SetFontSize(16);
    
    // 現在のwave情報
    char waveInfo[256];
    sprintf_s(waveInfo, "Wave:%d/3", m_currentWave);
    DrawString(startX, y, waveInfo, GetColor(255, 255, 255));
    
    // 経過時間
    char timeInfo[256];
    sprintf_s(timeInfo, "Timer:%.1fs", m_spawnTimer);
    DrawString(startX + itemSpacing, y, timeInfo, GetColor(255, 255, 255));
    
    // 敵の出現情報
    char spawnInfo[256];
    sprintf_s(spawnInfo, "Spawn:%d/%d", m_currentSpawnIndex, m_spawnInfoList.size());
    DrawString(startX + itemSpacing * 2, y, spawnInfo, GetColor(255, 255, 255));
    
    // 生存している敵の数
    int aliveEnemies = 0;
    for (const std::shared_ptr<EnemyBase>& enemy : m_enemyList)
    {
        if (enemy->IsAlive())
        {
            aliveEnemies++;
        }
    }
    char enemyInfo[256];
    sprintf_s(enemyInfo, "Alive:%d", aliveEnemies);
    DrawString(startX + itemSpacing * 3, y, enemyInfo, GetColor(255, 255, 255));
    
    // 総敵数
    char totalEnemyInfo[256];
    sprintf_s(totalEnemyInfo, "Total:%d", m_enemyList.size());
    DrawString(startX + itemSpacing * 4, y, totalEnemyInfo, GetColor(255, 255, 255));
}
