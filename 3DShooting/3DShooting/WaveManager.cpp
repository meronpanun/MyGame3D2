#include "EnemyBase.h"
#include "Bullet.h"
#include "Player.h"
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
#include <cassert>

namespace
{
    // プレイヤーからの最大アクティブ距離
	constexpr float kMaxActiveDistance = 1200.0f; 

    // 地面の最小最大値座標
	constexpr VECTOR kRoadFloorMin = { -500.0f, 0.0f, -500.0f }; // 床の最小座標
	constexpr VECTOR kRoadFloorMax = { 500.0f, 0.0f, 500.0f };   // 床の最大座標

    // プレイヤーからの最小距離
	constexpr float kMinSpawnDistance = 400.0f;

    // 出現位置の最大試行回数
	constexpr int kMaxSpawnAttempts = 100;

    // ウェーブ画像の描画幅
    constexpr int kWaveImageDrawWidth = 100;
}

WaveManager::WaveManager() :
    m_currentWave(1),
    m_waveTimer(0.0f),
    m_spawnTimer(0.0f),
    m_currentSpawnIndex(0),
    m_isWaveActive(false),
    m_isAllWavesCompleted(false),
    m_roadFloorMin(kRoadFloorMin),
    m_roadFloorMax(kRoadFloorMax),
    m_isRoadFloorBoundsSet(false),
    m_onEnemyDeathCallback(nullptr)
{
    // 敵のテンプレートを作成
    m_pEnemyNormalTemplate = std::make_shared<EnemyNormal>();
    m_pEnemyNormalTemplate->Init();

    m_pEnemyRunnerTemplate = std::make_shared<EnemyRunner>();
    m_pEnemyRunnerTemplate->Init();

    m_pEnemyAcidTemplate = std::make_shared<EnemyAcid>();
    m_pEnemyAcidTemplate->Init();

    // ウェーブ画像の読み込み
    m_waveImages[0] = LoadGraph("data/image/wave1.png");
    m_waveImages[1] = LoadGraph("data/image/wave2.png");
    m_waveImages[2] = LoadGraph("data/image/wave3.png");
}

WaveManager::~WaveManager()
{
	// 画像の解放
	for (int i = 0; i < 3; ++i)
	{
		if (m_waveImages[i] >= 0)
		{
			DeleteGraph(m_waveImages[i]);
			m_waveImages[i] = -1;
		}
	}
}

void WaveManager::Init()
{
    m_enemyList.clear();
    m_spawnInfoList.clear();

    // 敵のテンプレートを初期化
    if (m_pEnemyNormalTemplate)
    {
        m_pEnemyNormalTemplate->Init();
    }
    if (m_pEnemyRunnerTemplate)
    {
        m_pEnemyRunnerTemplate->Init();
    }
    if (m_pEnemyAcidTemplate)
    {
        m_pEnemyAcidTemplate->Init();
    }

    LoadWaveData();

    // 各敵種ごとの最大同時出現数を計算
    int maxNormal = 0, maxRunner = 0, maxAcid = 0;
    for (const auto& wave : m_waveDataList)
    {
        if (wave.enemyType == "NormalEnemy") maxNormal = (std::max)(maxNormal, wave.count);
        if (wave.enemyType == "RunnerEnemy") maxRunner = (std::max)(maxRunner, wave.count);
        if (wave.enemyType == "AcidEnemy")   maxAcid = (std::max)(maxAcid, wave.count);
    }

    // その数だけ事前にプール
    for (int i = m_enemyNormalPool.size(); i < maxNormal; ++i) 
    {
        auto pEnemy = std::make_shared<EnemyNormal>();
        if (m_pEnemyNormalTemplate) pEnemy->SetModelHandle(m_pEnemyNormalTemplate->GetModelHandle());
        pEnemy->Init();
        pEnemy->SetActive(false);
        m_enemyNormalPool.push_back(pEnemy);
    }
    for (int i = m_enemyRunnerPool.size(); i < maxRunner; ++i) 
    {
        auto pEnemy = std::make_shared<EnemyRunner>();
        if (m_pEnemyRunnerTemplate) pEnemy->SetModelHandle(m_pEnemyRunnerTemplate->GetModelHandle());
        pEnemy->Init();
        pEnemy->SetActive(false);
        m_enemyRunnerPool.push_back(pEnemy);
    }
    for (int i = m_enemyAcidPool.size(); i < maxAcid; ++i) 
    {
        auto pEnemy = std::make_shared<EnemyAcid>();
        if (m_pEnemyAcidTemplate) pEnemy->SetModelHandle(m_pEnemyAcidTemplate->GetModelHandle());
        pEnemy->Init();
        pEnemy->SetActive(false);
        m_enemyAcidPool.push_back(pEnemy);
    }
}

void WaveManager::Update()
{
	// すべてのウェーブが終了している場合は何もしない
    if (m_isAllWavesCompleted)
    {
        return;
    }

    // 現在のウェーブが終了しているかチェック
    if (m_isWaveActive && IsCurrentWaveCleared())
    {
        NextWave();
    }

    // ウェーブが開始されていない場合は開始
    if (!m_isWaveActive && m_currentWave <= 3)
    {
        // プレイヤーの位置を取得
        VECTOR playerPos = VGet(0.0f, 0.0f, 0.0f);
        StartCurrentWave(playerPos);
    }

    // 敵の出現処理
    if (m_isWaveActive && m_currentSpawnIndex < m_spawnInfoList.size())
    {
        m_spawnTimer += 1.0f / 60.0f;

        // 1フレームに1体だけ生成
        EnemySpawnInfo& spawnInfo = m_spawnInfoList[m_currentSpawnIndex];
        if (m_spawnTimer >= spawnInfo.spawnTime && !spawnInfo.isSpawned)
        {
            std::shared_ptr<EnemyBase> pEnemy = CreateEnemy(spawnInfo.enemyType);
            if (pEnemy)
            {
                pEnemy->SetPos(spawnInfo.spawnPos);
                m_enemyList.push_back(pEnemy);
                spawnInfo.isSpawned = true;
            }
            m_currentSpawnIndex++;
        }
    }

    // 死亡した敵をリストから削除
    m_enemyList.erase(
        std::remove_if(m_enemyList.begin(), m_enemyList.end(),
            [](const std::shared_ptr<EnemyBase>& pEnemy) {
                return !pEnemy->IsAlive();
            }),
        m_enemyList.end()
    );
}

// GetEnemyListをアクティブな敵のみ返すようにする
void WaveManager::UpdateEnemies(std::vector<Bullet>& bullets, const Player::TackleInfo& tackleInfo, const Player& player)
{
    VECTOR playerPos = player.GetPos();
    for (auto& pEnemy : m_enemyNormalPool)
    {
		// 敵がアクティブで生存しているかチェック
        if (!pEnemy->IsActive() || !pEnemy->IsAlive()) continue;

        VECTOR toPlayer = VSub(pEnemy->GetPos(), playerPos); // プレイヤーとの距離を計算
		float distSq = toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y + toPlayer.z * toPlayer.z; // 距離の二乗を計算

		// プレイヤーからの距離が最大アクティブ距離を超えている場合は更新しない
        if (distSq > kMaxActiveDistance * kMaxActiveDistance) continue;
        pEnemy->Update(bullets, tackleInfo, player); 
    }
    for (auto& pEnemy : m_enemyRunnerPool) 
    {
		// 敵がアクティブで生存しているかチェック
        if (!pEnemy->IsActive() || !pEnemy->IsAlive()) continue;

		VECTOR toPlayer = VSub(pEnemy->GetPos(), playerPos); // プレイヤーとの距離を計算
		float distSq = toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y + toPlayer.z * toPlayer.z; // 距離の二乗を計算

		// プレイヤーからの距離が最大アクティブ距離を超えている場合は更新しない
        if (distSq > kMaxActiveDistance * kMaxActiveDistance) continue;
        pEnemy->Update(bullets, tackleInfo, player);
    }
    for (auto& pEnemy : m_enemyAcidPool) 
    {
		// 敵がアクティブで生存しているかチェック
        if (!pEnemy->IsActive() || !pEnemy->IsAlive()) continue;

		VECTOR toPlayer = VSub(pEnemy->GetPos(), playerPos); // プレイヤーとの距離を計算
		float distSq = toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y + toPlayer.z * toPlayer.z; // 距離の二乗を計算

		// プレイヤーからの距離が最大アクティブ距離を超えている場合は更新しない
        if (distSq > kMaxActiveDistance * kMaxActiveDistance) continue;
        pEnemy->Update(bullets, tackleInfo, player);
    }
}

// 敵の一括描画
void WaveManager::DrawEnemies()
{
    // ウェーブ中は常に画像を表示
    if (!m_isAllWavesCompleted && m_currentWave >= 1 && m_currentWave <= 3)
    {
        int img = m_waveImages[m_currentWave - 1];
        int imgW = 0, imgH = 0;
        GetGraphSize(img, &imgW, &imgH);
        int screenW = 0, screenH = 0;
        GetScreenState(&screenW, &screenH, NULL);
        // 定数で指定した幅、高さは縦横比維持で計算
        int drawW = kWaveImageDrawWidth;
        int drawH = imgH * drawW / imgW;
        int x = (screenW - drawW) / 2;
        int y = 0; // 画面上部中央
        DrawExtendGraph(x, y, x + drawW, y + drawH, img, true);
    }

	// 敵の描画
    for (auto& pEnemy : m_enemyNormalPool) 
    {
        if (!pEnemy->IsActive() || !pEnemy->IsAlive()) continue;
        pEnemy->Draw();
    }
    for (auto& pEnemy : m_enemyRunnerPool) 
    {
        if (!pEnemy->IsActive() || !pEnemy->IsAlive()) continue;
        pEnemy->Draw();
    }
    for (auto& pEnemy : m_enemyAcidPool) 
    {
        if (!pEnemy->IsActive() || !pEnemy->IsAlive()) continue;
        pEnemy->Draw();
    }
}

// 敵の死亡時コールバック
void WaveManager::SetOnEnemyDeathCallback(std::function<void(const VECTOR&)> callback)
{
    m_onEnemyDeathCallback = callback;
}

// Road_floorオブジェクトの範囲を設定
void WaveManager::SetRoadFloorBounds(const VECTOR& minPos, const VECTOR& maxPos)
{
    m_roadFloorMin = minPos;
    m_roadFloorMax = maxPos;
    m_isRoadFloorBoundsSet = true;
}

// ウェーブデータを読み込む
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

	// CSVファイルの各行を読み込む
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

// ランダムな出現位置を生成
VECTOR WaveManager::GenerateRandomSpawnPos(const VECTOR& playerPos)
{
	// Road_floorの範囲が設定されていない場合はデフォルト位置を返す
    if (!m_isRoadFloorBoundsSet)
    {
        return VGet(0.0f, -0.5f, 3.0f);
    }

	// 乱数生成器の初期化
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 gen(seed);

    VECTOR spawnPos;
    int attempts = 0;

	// プレイヤーからの最小距離を確保するためのループ
    do {
        std::uniform_real_distribution<float> xDist(m_roadFloorMin.x, m_roadFloorMax.x);
        std::uniform_real_distribution<float> zDist(m_roadFloorMin.z, m_roadFloorMax.z);

        float x = xDist(gen);
        float z = zDist(gen);

        spawnPos = VGet(x, 0.0f, z);

        VECTOR toPlayer = VSub(playerPos, spawnPos);
        toPlayer.y = 0.0f;
        float distanceToPlayer = sqrtf(toPlayer.x * toPlayer.x + toPlayer.z * toPlayer.z);

		if (distanceToPlayer >= kMinSpawnDistance) 
            break; 

        attempts++;
    } while (attempts < kMaxSpawnAttempts);

    return spawnPos;
}

// 敵を生成
std::shared_ptr<EnemyBase> WaveManager::CreateEnemy(const std::string& enemyType)
{
    std::shared_ptr<EnemyBase> pEnemy = nullptr;

	// 敵の種類に応じてプールから取得または新規生成
    if (enemyType == "NormalEnemy")
    {
        auto pPooled = GetPooledNormalEnemy();
        pPooled->SetActive(true);
        pPooled->Init();
        pEnemy = pPooled;
    }
    else if (enemyType == "RunnerEnemy")
    {
        auto pPooled = GetPooledRunnerEnemy();
        pPooled->SetActive(true);
        pPooled->Init();
        pEnemy = pPooled;
    }
    else if (enemyType == "AcidEnemy")
    {
        auto pPooled = GetPooledAcidEnemy();
        pPooled->SetActive(true);
        pPooled->Init();
        pEnemy = pPooled;
    }
    else
    {
        printf("Warning: Unknown enemy type: %s\n", enemyType.c_str());
        return nullptr;
    }

    if (pEnemy)
    {
        // 死亡コールバック
        pEnemy->SetOnDeathCallback([this](const VECTOR& pos) {
            OnEnemyDeath(pos);
            });

        // アイテムドロップコールバック
        if (m_onEnemyDeathCallback) 
        {
            pEnemy->SetOnDropItemCallback(m_onEnemyDeathCallback);
        }

        // ヒット時コールバック(ヒットマーク用)
        if (m_onEnemyHitCallback) 
        {
            pEnemy->SetOnHitCallback(m_onEnemyHitCallback);
        }
        printf("Created enemy: %s at position (%.2f, %.2f, %.2f)\n",
            enemyType.c_str(), pEnemy->GetPos().x, pEnemy->GetPos().y, pEnemy->GetPos().z);
    }

    return pEnemy;
}

std::shared_ptr<EnemyNormal> WaveManager::GetPooledNormalEnemy() 
{
    // プールから空きのある敵を探す
	for (auto& pEnemy : m_enemyNormalPool) 
    {
        if (!pEnemy->IsActive()) 
        {
            return pEnemy;
        }
    }

    // プールに空きがなければ新規生成
    auto pEnemy = std::make_shared<EnemyNormal>();
    if (m_pEnemyNormalTemplate) 
    {
        pEnemy->SetModelHandle(m_pEnemyNormalTemplate->GetModelHandle());
    }
    pEnemy->Init();
    m_enemyNormalPool.push_back(pEnemy);
    return pEnemy;
}

std::shared_ptr<EnemyRunner> WaveManager::GetPooledRunnerEnemy()
{
	// プールから空きのある敵を探す
    for (auto& pEnemy : m_enemyRunnerPool) 
    {
        if (!pEnemy->IsActive()) 
        {
            return pEnemy;
        }
    }

	// プールに空きがなければ新規生成
    auto pEnemy = std::make_shared<EnemyRunner>();
    if (m_pEnemyRunnerTemplate) 
    {
        pEnemy->SetModelHandle(m_pEnemyRunnerTemplate->GetModelHandle());
    }
    pEnemy->Init();
    m_enemyRunnerPool.push_back(pEnemy);
    return pEnemy;
}
std::shared_ptr<EnemyAcid> WaveManager::GetPooledAcidEnemy() 
{
	// プールから空きのある敵を探す
    for (auto& pEnemy : m_enemyAcidPool) 
    {
        if (!pEnemy->IsActive()) 
        {
            return pEnemy;
        }
    }

	// プールに空きがなければ新規生成
    auto pEnemy = std::make_shared<EnemyAcid>();
    if (m_pEnemyAcidTemplate) 
    {
        pEnemy->SetModelHandle(m_pEnemyAcidTemplate->GetModelHandle());
    }
    pEnemy->Init();
    m_enemyAcidPool.push_back(pEnemy);
    return pEnemy;
}

// 現在のウェーブを開始
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
            spawnInfo.spawnPos = GenerateRandomSpawnPos(playerPos);
            spawnInfo.spawnTime = currentTime + waveData.delay + (i * waveData.spawnInterval);
            spawnInfo.isSpawned = false;

            m_spawnInfoList.push_back(spawnInfo);
        }
    }

    // デバッグ出力
    printf("Starting Wave %d with %zu enemies\n", m_currentWave, m_spawnInfoList.size());
}

// 次のウェーブに進む
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

// 現在のウェーブの敵がすべて倒されたかチェック
bool WaveManager::IsCurrentWaveCleared()
{
    // 現在のwaveの敵がすべて出現済みで、生存している敵がいない場合
    if (m_currentSpawnIndex >= m_spawnInfoList.size())
    {
        for (const std::shared_ptr<EnemyBase>& pEnemy : m_enemyList)
        {
            if (pEnemy->IsAlive())
            {
                return false;
            }
        }
        return true;
    }

    return false;
}

// 敵の死亡処理
void WaveManager::OnEnemyDeath(const VECTOR& position)
{
    // デバッグ出力
    if (m_onEnemyDeathCallback)
    {
        m_onEnemyDeathCallback(position);
    }
}

// デバッグ情報の表示
void WaveManager::DrawDebugInfo()
{
    // デバッグ情報の表示位置
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
    for (const std::shared_ptr<EnemyBase>& pEnemy : m_enemyList)
    {
        if (pEnemy->IsAlive())
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