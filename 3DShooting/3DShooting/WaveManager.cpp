#include "EnemyBase.h"
#include "Bullet.h"
#include "Player.h"
#include "WaveManager.h"
#include "EnemyNormal.h"
#include "EnemyRunner.h"
#include "EnemyAcid.h"
#include "EffekseerForDXLib.h"
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
	constexpr VECTOR kRoadFloorMin = { -1000.0f, 0.0f, -1000.0f }; // 床の最小座標
	constexpr VECTOR kRoadFloorMax = { 1000.0f, 0.0f, 1000.0f };   // 床の最大座標

    // プレイヤーからの最小距離
	constexpr float kMinSpawnDistance = 200.0f;

    // 出現位置の最大試行回数
	constexpr int kMaxSpawnAttempts = 100;

    // ウェーブ画像の描画幅
    constexpr int kWaveImageDrawWidth = 100;

    // 範囲が設定されていない場合のデフォルト位置
	constexpr VECTOR kDefaultRoadFloorPos = { 0.0f, -0.5f, 3.0f };

    // デバック情報の表示位置
	constexpr int kDebugInfoPosY    = 10;  // 高さ
	constexpr int kDebugInfoSpacing = 100; // 項目間の間隔
	constexpr int kDebugInfoPosX    = 10;  // 左端からのX座標
	constexpr int kFontSize         = 16;  // フォントサイズ

	constexpr float kFrameTime = 1.0f / 60.0f; // フレーム時間
}

WaveManager::WaveManager() :
    m_currentWave(1),
    m_waveTimer(0.0f),
    m_spawnTimer(0.0f),
    m_currentSpawnIndex(0),
    m_isWaveActive(false),
    m_isAllWavesCompleted(false),
    m_isWave1Loaded(false),
    m_isWave1EnemySpawned(false),
    m_roadFloorMin(kRoadFloorMin),
    m_roadFloorMax(kRoadFloorMax),
    m_isRoadFloorBoundsSet(false),
    m_onEnemyDeathCallback(nullptr),
    m_waveIntervalTimer(0.0f),
    m_totalSpawnedCount(0),
    m_isShotTutorialCleared(false),
    m_isTackleTutorialCleared(false),
    m_waveImageAnimTimer(0),
    m_waveImageAnimDuration(45),
    m_waveImageAnimHoldDuration(30),
    m_waveImageAnimInitialHoldDuration(30),
    m_isWaveImageAnimating(false)
{
    // 敵のモデルをロード
    EnemyNormal::LoadModel();
    EnemyRunner::LoadModel();
    EnemyAcid::LoadModel();

    // ウェーブ画像の読み込み
    m_waveImages[0] = LoadGraph("data/image/wave1.png");
    m_waveImages[1] = LoadGraph("data/image/wave2.png");
    m_waveImages[2] = LoadGraph("data/image/wave3.png");
}

WaveManager::~WaveManager()
{
    // 敵のモデルを解放
    EnemyNormal::DeleteModel();
    EnemyRunner::DeleteModel();
    EnemyAcid::DeleteModel();

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
    m_enemyData = TransformDataLoader::LoadDataCSV("data/CSV/CharacterTransfromData.csv");
    m_enemyList.clear();
    m_spawnInfoList.clear();

    // ウェーブデータをロード
    LoadWaveData();

    // 各敵種ごとに全ウェーブで同時に出現する最大数を計算
    std::map<int, int> normalPerWave, runnerPerWave, acidPerWave;
    for (const auto& wave : m_waveDataList)
    {
        if (wave.enemyType == "NormalEnemy") normalPerWave[wave.wave] += wave.count;
        if (wave.enemyType == "RunnerEnemy") runnerPerWave[wave.wave] += wave.count;
        if (wave.enemyType == "AcidEnemy")   acidPerWave[wave.wave] += wave.count;
    }
    int maxNormal = 0, maxRunner = 0, maxAcid = 0;
    // 各ウェーブでの最大出現数を計算
    for (const auto& [wave, cnt] : normalPerWave) maxNormal = (std::max)(maxNormal, cnt);
    for (const auto& [wave, cnt] : runnerPerWave) maxRunner = (std::max)(maxRunner, cnt);
    for (const auto& [wave, cnt] : acidPerWave)   maxAcid = (std::max)(maxAcid, cnt);

    // その数だけ各プールを確保
    for (int i = m_enemyNormalPool.size(); i < maxNormal; ++i)
    {
        auto pEnemy = std::make_shared<EnemyNormal>();
        pEnemy->Init();
        pEnemy->SetActive(false);
        m_enemyNormalPool.push_back(pEnemy);
    }
    for (int i = m_enemyRunnerPool.size(); i < maxRunner; ++i)
    {
        auto pEnemy = std::make_shared<EnemyRunner>();
        pEnemy->Init();
        pEnemy->SetActive(false);
        m_enemyRunnerPool.push_back(pEnemy);
    }
    for (int i = m_enemyAcidPool.size(); i < maxAcid; ++i)
    {
        auto pEnemy = std::make_shared<EnemyAcid>();
        pEnemy->Init();
        pEnemy->SetActive(false);
        m_enemyAcidPool.push_back(pEnemy);
    }

    // チュートリアル達成判定用コールバック
    auto deathTypeCallback = [this](const VECTOR& pos, AttackType type) {
        if (m_currentWave == 1) 
        {
            if (type == AttackType::Shoot)   m_isShotTutorialCleared   = true;
            if (type == AttackType::Tackle) m_isTackleTutorialCleared = true;
        }
    };
	// 各敵プールの死亡時コールバックを設定
    for (auto& enemy : m_enemyNormalPool) enemy->SetOnDeathWithTypeCallback(deathTypeCallback);
    for (auto& enemy : m_enemyRunnerPool) enemy->SetOnDeathWithTypeCallback(deathTypeCallback);
    for (auto& enemy : m_enemyAcidPool)   enemy->SetOnDeathWithTypeCallback(deathTypeCallback);

	// 敵の死亡時コールバックを設定
    SetOnEnemyDeathCallback([this](const VECTOR& pos) {
        // 死亡した敵を特定
        auto checkAndSet = [this](EnemyBase* enemy) {
            if (!enemy) return false;
			// チュートリアル達成判定
            if (m_currentWave == 1) 
            {
                if (enemy->GetLastAttackType() == AttackType::Shoot)   m_isShotTutorialCleared   = true;
                if (enemy->GetLastAttackType() == AttackType::Tackle) m_isTackleTutorialCleared = true;
            }
            return true;
        };
		// 敵のプールから位置が一致する敵を探して死亡処理
        for (auto& enemy : m_enemyNormalPool) 
        {
            if (enemy->GetPos().x == pos.x && enemy->GetPos().y == pos.y && enemy->GetPos().z == pos.z) 
            {
                if (checkAndSet(enemy.get())) break;
            }
        }
        for (auto& enemy : m_enemyRunnerPool) 
        {
            if (enemy->GetPos().x == pos.x && enemy->GetPos().y == pos.y && enemy->GetPos().z == pos.z) 
            {
                if (checkAndSet(enemy.get())) break;
            }
        }
        for (auto& enemy : m_enemyAcidPool) 
        {
            if (enemy->GetPos().x == pos.x && enemy->GetPos().y == pos.y && enemy->GetPos().z == pos.z) 
            {
                if (checkAndSet(enemy.get())) break;
            }
        }
    });
}


void WaveManager::Update()
{
    if (m_isAllWavesCompleted)
    {
        return;
    }

	// ウェーブがアクティブな場合、敵のスポーンとウェーブクリア判定を行う
    if (m_isWaveActive)
    {
        if (IsCurrentWaveCleared())
        {
			
            NextWave();
        }
        else
        {
    
            if (m_currentSpawnIndex < m_spawnInfoList.size() && !m_isWaveImageAnimating)
            {
                m_spawnTimer += 1.0f / 60.0f;
                while (m_currentSpawnIndex < m_spawnInfoList.size())
                {
                    EnemySpawnInfo& spawnInfo = m_spawnInfoList[m_currentSpawnIndex];
                    if (m_spawnTimer >= spawnInfo.spawnTime && !spawnInfo.isSpawned)
                    {
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
	// ウェーブがアクティブでない場合、次のウェーブ開始までのインターバルをカウントダウン
    else
    {
        if (m_waveIntervalTimer > 0.0f)
        {
            m_waveIntervalTimer -= 1.0f / 60.0f;
        }
        else
        {
            if (m_currentWave <= 3)
            {
				VECTOR playerPos = VGet(0.0f, 0.0f, 0.0f); // プレイヤー位置の初期値
                StartCurrentWave(playerPos);
            }
        }
    }

    // ウェーブ画像アニメーションの更新
    if (m_isWaveImageAnimating)
    {
        m_waveImageAnimTimer++;
        if (m_waveImageAnimTimer >= m_waveImageAnimInitialHoldDuration + m_waveImageAnimDuration + m_waveImageAnimHoldDuration)
        {
            m_isWaveImageAnimating = false;
        }
    }

    m_enemyList.erase(
        std::remove_if(m_enemyList.begin(), m_enemyList.end(),
            [](const std::shared_ptr<EnemyBase>& pEnemy) {
                return !pEnemy->IsActive();
            }),
        m_enemyList.end()
    );
}

// GetEnemyListをアクティブな敵のみ返すようにする
void WaveManager::UpdateEnemies(std::vector<Bullet>& bullets, const Player::TackleInfo& tackleInfo, const Player& player, Effect* pEffect)
{
    VECTOR playerPos = player.GetPos();
    // アクティブな敵リストを作成
    std::vector<EnemyBase*> activeEnemies;
    for (auto& pEnemy : m_enemyNormalPool) 
    {
        if (pEnemy->IsActive() && pEnemy->IsAlive()) activeEnemies.push_back(pEnemy.get());
    }
    for (auto& pEnemy : m_enemyRunnerPool) 
    {
        if (pEnemy->IsActive() && pEnemy->IsAlive()) activeEnemies.push_back(pEnemy.get());
    }
    // NormalEnemy
    for (auto& pEnemy : m_enemyNormalPool)
    {
		// アクティブで生存している敵のみ更新
        if (!pEnemy->IsActive() || !pEnemy->IsAlive()) continue;
        // 自分以外の敵リストを作成
        std::vector<EnemyBase*> others;
        for (auto* e : activeEnemies) if (e != pEnemy.get()) others.push_back(e);
        pEnemy->Update(bullets, tackleInfo, player, others, pEffect);
    }
    // RunnerEnemy
    for (auto& pEnemy : m_enemyRunnerPool)
    {
        if (!pEnemy->IsActive() || !pEnemy->IsAlive()) continue;
        std::vector<EnemyBase*> others;
        for (auto* e : activeEnemies) if (e != pEnemy.get()) others.push_back(e);
        pEnemy->Update(bullets, tackleInfo, player, others, pEffect);
    }
    // AcidEnemy
    for (auto& pEnemy : m_enemyAcidPool)
    {
        if (!pEnemy->IsActive() || !pEnemy->IsAlive()) continue;
        std::vector<EnemyBase*> others;
        for (auto* e : activeEnemies) if (e != pEnemy.get()) others.push_back(e);
        pEnemy->Update(bullets, tackleInfo, player, others, pEffect);
    }
}

// 敵の一括描画
void WaveManager::DrawEnemies()
{
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

// ウェーブUIの描画
void WaveManager::DrawWaveUI()
{
    // ウェーブ中は常に画像を表示
    if (!m_isAllWavesCompleted && m_currentWave >= 1 && m_currentWave <= 3 && (m_isWaveImageAnimating || m_isWaveActive))
    {
        int img = m_waveImages[m_currentWave - 1];
        int imgW = 0, imgH = 0;
        GetGraphSize(img, &imgW, &imgH);
        int screenW = 0, screenH = 0;
        GetScreenState(&screenW, &screenH, NULL);

        // 最終的な位置とサイズ
        int targetDrawW = kWaveImageDrawWidth;
        int targetDrawH = imgH * targetDrawW / imgW;
        int targetX = (screenW - targetDrawW) * 0.5f;
        int targetY = 0; // 画面上部中央

        // 初期位置とサイズ (拡大して中央)
        int startDrawW = static_cast<int>(screenW * 0.4f);
        int startDrawH = imgH * startDrawW / imgW;
        int startX = (screenW - startDrawW) * 0.5f; // 画面中央
        int startY = (screenH - startDrawH) * 0.5f; // 画面中央

        int currentX, currentY, currentDrawW, currentDrawH;

        if (m_isWaveImageAnimating)
        {
            float t;
            if (m_waveImageAnimTimer < m_waveImageAnimInitialHoldDuration)
            {
                t = 0.0f; // 初期ホールド中は補間を0に固定
            }
            else if (m_waveImageAnimTimer < m_waveImageAnimInitialHoldDuration + m_waveImageAnimDuration)
            {
                t = (float)(m_waveImageAnimTimer - m_waveImageAnimInitialHoldDuration) / m_waveImageAnimDuration;
            }
            else
            {
                t = 1.0f; // ホールド中は補間を終了位置に固定
            }
            t = (std::min)(1.0f, t); // 0.0fから1.0fにクランプ

            // 線形補間
            currentX = static_cast<int>(startX + (targetX - startX) * t);
            currentY = static_cast<int>(startY + (targetY - startY) * t);
            currentDrawW = static_cast<int>(startDrawW + (targetDrawW - startDrawW) * t);
            currentDrawH = static_cast<int>(startDrawH + (targetDrawH - startDrawH) * t);
        }
        else
        {
            currentX = targetX;
            currentY = targetY;
            currentDrawW = targetDrawW;
            currentDrawH = targetDrawH;
        }

        DrawExtendGraph(currentX, currentY, currentX + currentDrawW, currentY + currentDrawH, img, true);
    }
}

// 敵の死亡時コールバック
void WaveManager::SetOnEnemyDeathCallback(std::function<void(const VECTOR&)> callback)
{
    m_onEnemyDeathCallback = callback;
}

// 敵ヒット時のコールバックを設定
void WaveManager::SetOnEnemyHitCallback(std::function<void(EnemyBase::HitPart)> cb)
{
    m_onEnemyHitCallback = cb;
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
        printf("Error: Cannot open WaveData.csv\n");
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
        if (!std::getline(ss, token, ',')) continue;
        waveData.wave = std::stoi(token);

        // EnemyType
        if (!std::getline(ss, token, ',')) continue;
        waveData.enemyType = token;

        // Count
        if (!std::getline(ss, token, ',')) continue;
        waveData.count = std::stoi(token);

        // SpawnInterval
        if (!std::getline(ss, token, ',')) continue;
        waveData.spawnInterval = std::stof(token);

        // StartTime
        if (!std::getline(ss, token, ',')) continue;
        waveData.startTime = std::stof(token);

        // WaveInterval
        if (std::getline(ss, token, ',')) 
        {
            waveData.waveInterval = std::stof(token);
        }
        else 
        {
            waveData.waveInterval = 0.0f;
        }

        m_waveDataList.push_back(waveData);

        // デバッグ出力
        printf("Loaded: Wave %d, %s, Count %d, Interval %.1f, Start %.1f\n",
            waveData.wave, waveData.enemyType.c_str(), waveData.count,
            waveData.spawnInterval, waveData.startTime);
    }
}

// ランダムな出現位置を生成
VECTOR WaveManager::GenerateRandomSpawnPos(const VECTOR& playerPos)
{
	// Road_floorの範囲が設定されていない場合はデフォルト位置を返す
    if (!m_isRoadFloorBoundsSet)
    {
		return kDefaultRoadFloorPos;
    }

	// 乱数生成器の初期化
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 gen(seed);

    VECTOR spawnPos;
    int attempts = 0;
    bool found = false;
    do {
		// 範囲内のランダムな座標を生成
        std::uniform_real_distribution<float> xDist(m_roadFloorMin.x, m_roadFloorMax.x);
        std::uniform_real_distribution<float> zDist(m_roadFloorMin.z, m_roadFloorMax.z);

        float x = xDist(gen);
        float z = zDist(gen);

        spawnPos = VGet(x, 0.0f, z);

		// プレイヤーとの距離を計算
        VECTOR toPlayer = VSub(playerPos, spawnPos);
        toPlayer.y = 0.0f;
        float distanceToPlayer = sqrtf(toPlayer.x * toPlayer.x + toPlayer.z * toPlayer.z);

		// プレイヤーからの距離が最小距離以上なら成功
        if (distanceToPlayer >= kMinSpawnDistance) 
        {
            found = true;
            break;
        }

        attempts++;
    } while (attempts < kMaxSpawnAttempts);

    if (!found) 
    {
        // 条件を満たさなかった場合は範囲内のランダムな点を返す
        std::uniform_real_distribution<float> xDist(m_roadFloorMin.x, m_roadFloorMax.x);
        std::uniform_real_distribution<float> zDist(m_roadFloorMin.z, m_roadFloorMax.z);
        float x = xDist(gen);
        float z = zDist(gen);
        spawnPos = VGet(x, 0.0f, z);
    }
    return spawnPos;
}

// 敵を生成
std::shared_ptr<EnemyBase> WaveManager::CreateEnemy(const std::string& enemyType, const VECTOR& spawnPos)
{
    // ウェーブ1の敵が初めて出現したタイミングでフラグを立てる
    if (m_currentWave == 1 && !m_isWave1EnemySpawned) m_isWave1EnemySpawned = true;
    else if (m_currentWave != 1) m_isWave1EnemySpawned = false;

    std::shared_ptr<EnemyBase> pEnemy = nullptr;

    // CSVデータから該当する敵のデータを検索
    ObjectTransformData* enemyData = nullptr;
    for (auto& data : m_enemyData)
    {
        if (data.name == enemyType)
        {
            enemyData = &data;
            break;
        }
    }

	// 敵の種類に応じてプールから取得または新規生成
    if (enemyType == "NormalEnemy")
    {
        auto pPooled = GetPooledNormalEnemy();
        pPooled->SetActive(true);
        pPooled->Init();
        if (enemyData) pPooled->SetHp(enemyData->hp);
        pPooled->SetPos(spawnPos);
        pEnemy = pPooled;
    }
    else if (enemyType == "RunnerEnemy")
    {
        auto pPooled = GetPooledRunnerEnemy();
        pPooled->SetActive(true);
        pPooled->Init();
        if (enemyData) pPooled->SetHp(enemyData->hp);
        pPooled->SetPos(spawnPos);
        pEnemy = pPooled;
    }
    else if (enemyType == "AcidEnemy")
    {
        auto pPooled = GetPooledAcidEnemy();
        pPooled->SetActive(true);
        pPooled->Init();
        if (enemyData) pPooled->SetHp(enemyData->hp);
        pPooled->SetPos(spawnPos);
        pEnemy = pPooled;
    }
    else
    {
        return nullptr;
    }

    if (pEnemy)
    {
        // 死亡コールバック
        pEnemy->SetOnDeathCallback([this](const VECTOR& pos) { OnEnemyDeath(pos); });

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
        m_totalSpawnedCount++;
    }

    return pEnemy;
}

// プールから空きのあるNormalEnemyを取得または新規生成
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
    pEnemy->Init();
    m_enemyNormalPool.push_back(pEnemy);
    return pEnemy;
}

// プールから空きのあるRunnerEnemyを取得または新規生成
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
    pEnemy->Init();
    m_enemyRunnerPool.push_back(pEnemy);
    return pEnemy;
}

// プールから空きのあるAcidEnemyを取得または新規生成
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
    m_enemyList.clear(); // 敵リストをクリア

    // ウェーブ画像アニメーションを開始
    m_isWaveImageAnimating = true;
    m_waveImageAnimTimer = 0;

    // ウェーブ1の敵ロードフラグ
    if (m_currentWave == 1)
    {
        m_isWave1Loaded = true;
    }
    else
    {
        m_isWave1Loaded = false;
    }

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
    for (const WaveData& waveData : currentWaveData)
    {
        printf("Processing %s: Count %d, StartTime %.1f, Interval %.1f\n",
            waveData.enemyType.c_str(), waveData.count, waveData.startTime, waveData.spawnInterval);

		for (int i = 0; i < waveData.count; ++i) // 出現数分ループ
        {
			// 出現位置を生成
            EnemySpawnInfo spawnInfo;
            spawnInfo.enemyType = waveData.enemyType;
            spawnInfo.spawnPos  = GenerateRandomSpawnPos(playerPos);
            spawnInfo.spawnTime = waveData.startTime + (i * waveData.spawnInterval);
            spawnInfo.isSpawned = false;

            m_spawnInfoList.push_back(spawnInfo);
        }
    }

    // spawnTimeで昇順ソート
    std::sort(m_spawnInfoList.begin(), m_spawnInfoList.end(), [](const EnemySpawnInfo& a, const EnemySpawnInfo& b) {
        return a.spawnTime < b.spawnTime;
        });

    // デバッグ出力：スポーンスケジュール
    printf("Wave %d spawn schedule (%d enemies total):\n", m_currentWave, static_cast<int>(m_spawnInfoList.size()));
    for (size_t i = 0; i < m_spawnInfoList.size(); ++i) {
        const auto& info = m_spawnInfoList[i];
        printf("  [%d] %s at %.2fs (%.2f, %.2f, %.2f)\n",
            static_cast<int>(i), info.enemyType.c_str(), info.spawnTime,
            info.spawnPos.x, info.spawnPos.y, info.spawnPos.z);
    }
}

void WaveManager::NextWave()
{
    // 現在完了したウェーブのインターバル値の中から最大値を取得する
    float maxInterval = 0.0f;
    for (const auto& waveData : m_waveDataList)
    {
        if (waveData.wave == m_currentWave)
        {
            if (waveData.waveInterval > maxInterval)
            {
                maxInterval = waveData.waveInterval;
            }
        }
    }

    // 次のウェーブへの移行準備
    m_isWaveActive = false;
    m_waveIntervalTimer = maxInterval;

    // ウェーブ番号を更新
    m_currentWave++;
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
    // フォントサイズを設定
    SetFontSize(kFontSize);

    // 現在のwave情報
    char waveInfo[256];
    sprintf_s(waveInfo, "Wave:%d/3", m_currentWave);
    DrawString(kDebugInfoPosX, kDebugInfoPosY, waveInfo, 0xffffff);

    // 経過時間
    char timeInfo[256];
    sprintf_s(timeInfo, "Timer:%.1fs", m_spawnTimer);
    DrawString(kDebugInfoPosX + kDebugInfoSpacing, kDebugInfoPosY, timeInfo, 0xffffff);

    // 敵の出現情報
    char spawnInfo[256];
    sprintf_s(spawnInfo, "Spawn:%d/%d", m_currentSpawnIndex, static_cast<int>(m_spawnInfoList.size()));
    DrawString(kDebugInfoPosX + kDebugInfoSpacing * 2, kDebugInfoPosY, spawnInfo, 0xffffff);

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
    DrawString(kDebugInfoPosX + kDebugInfoSpacing * 3, kDebugInfoPosY, enemyInfo, 0xffffff);

    // 総敵数
    char totalEnemyInfo[256];
    sprintf_s(totalEnemyInfo, "Total:%d", m_totalSpawnedCount);
    DrawString(kDebugInfoPosX + kDebugInfoSpacing * 4, kDebugInfoPosY, totalEnemyInfo, 0xffffff);
}

int WaveManager::GetAliveEnemyCount() const
{
    int aliveCount = 0;
    for (const auto& pEnemy : m_enemyNormalPool)
    {
        if (pEnemy->IsActive() && pEnemy->IsAlive())
        {
            aliveCount++;
        }
    }
    for (const auto& pEnemy : m_enemyRunnerPool)
    {
        if (pEnemy->IsActive() && pEnemy->IsAlive())
        {
            aliveCount++;
        }
    }
    for (const auto& pEnemy : m_enemyAcidPool)
    {
        if (pEnemy->IsActive() && pEnemy->IsAlive())
        {
            aliveCount++;
        }
    }
    return aliveCount;
}

void WaveManager::SpawnTutorialWave(int tutorialWaveId)
{
    // 既存の敵をすべて非アクティブ化し、m_enemyListをクリア
    for (auto& pEnemy : m_enemyNormalPool) pEnemy->SetActive(false);
    for (auto& pEnemy : m_enemyRunnerPool) pEnemy->SetActive(false);
    for (auto& pEnemy : m_enemyAcidPool)   pEnemy->SetActive(false);
    m_enemyList.clear();

    std::ifstream file("data/CSV/TutorialWaves.csv");
    if (!file.is_open())
    {
        return;
    }

    std::string line;
    std::getline(file, line); // ヘッダー行をスキップ

    std::vector<WaveData> tutorialWaves;
    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string token;
        WaveData waveData;
        if (!std::getline(ss, token, ',')) continue;
        waveData.wave = std::stoi(token);
        if (!std::getline(ss, token, ',')) continue;
        waveData.enemyType = token;
        if (!std::getline(ss, token, ',')) continue;
        waveData.count = std::stoi(token);
        if (!std::getline(ss, token, ',')) continue;
        waveData.spawnInterval = std::stof(token);
        if (!std::getline(ss, token, ',')) continue;
        waveData.startTime = std::stof(token);
        if (std::getline(ss, token, ',')) waveData.waveInterval = std::stof(token);
        else waveData.waveInterval = 0.0f;
        tutorialWaves.push_back(waveData);
    }

    for (const auto& waveData : tutorialWaves)
    {
        if (waveData.wave == tutorialWaveId)
        {
            for (int i = 0; i < waveData.count; ++i)
            {
                VECTOR playerPos = VGet(0,0,0); // プレイヤー位置を仮定
                VECTOR spawnPos = GenerateRandomSpawnPos(playerPos);
                std::shared_ptr<EnemyBase> pEnemy = CreateEnemy(waveData.enemyType, spawnPos);
                // CreateEnemyはプールから敵を取得し、アクティブに設定する
                // m_enemyListには後でアクティブな敵をすべて追加する
            }
        }
    }

    // すべてのプールからアクティブな敵をm_enemyListに追加
    for (auto& pEnemy : m_enemyNormalPool) { if (pEnemy->IsActive()) m_enemyList.push_back(pEnemy); }
    for (auto& pEnemy : m_enemyRunnerPool) { if (pEnemy->IsActive()) m_enemyList.push_back(pEnemy); }
    for (auto& pEnemy : m_enemyAcidPool)   { if (pEnemy->IsActive()) m_enemyList.push_back(pEnemy); }
}