#include "WaveManager.h"
#include "Bullet.h"
#include "CollisionGrid.h" // 追加
#include "EffekseerForDXLib.h"
#include "EnemyAcid.h"
#include "EnemyBase.h"
#include "EnemyBoss.h"
#include "EnemyNormal.h"
#include "EnemyRunner.h"
#include "Game.h"
#include "Player.h"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <fstream>
#include <map>
#include <random>
#include <sstream>

namespace {
// プレイヤーからの最大アクティブ距離
constexpr float kMaxActiveDistance = 1200.0f;

// 地面の最小最大値座標
constexpr VECTOR kRoadFloorMin = {-1000.0f, 0.0f, -1000.0f}; // 床の最小座標
constexpr VECTOR kRoadFloorMax = {1000.0f, 0.0f, 1000.0f};   // 床の最大座標

// プレイヤーからの最小距離
constexpr float kMinSpawnDistance = 200.0f;

// 出現位置の最大試行回数
constexpr int kMaxSpawnAttempts = 100;

// ウェーブ画像の描画幅
constexpr int kWaveImageDrawWidth = 150;

// 範囲が設定されていない場合のデフォルト位置
constexpr VECTOR kDefaultRoadFloorPos = {0.0f, -0.5f, 3.0f};

// デバック情報の表示位置
constexpr int kDebugInfoPosY = 10;     // 高さ
constexpr int kDebugInfoSpacing = 100; // 項目間の間隔
constexpr int kDebugInfoPosX = 10;     // 左端からのX座標
constexpr int kFontSize = 16;          // フォントサイズ

constexpr float kFrameTime = 1.0f / 60.0f; // フレーム時間
} // namespace

bool WaveManager::s_isDrawSpawnAreas = false;
bool WaveManager::s_isShowActiveEnemyCount = false;
bool WaveManager::s_isShowDrawnEnemyCount = false;

WaveManager::WaveManager()
    : m_currentWave(1), m_waveTimer(0.0f), m_spawnTimer(0.0f),
      m_currentSpawnIndex(0), m_isWaveActive(false),
      m_isAllWavesCompleted(false), m_isWave1Loaded(false),
      m_isWave1EnemySpawned(false), m_roadFloorMin(kRoadFloorMin),
      m_roadFloorMax(kRoadFloorMax), m_isRoadFloorBoundsSet(false),
      m_onEnemyDeathCallback(nullptr), m_waveIntervalTimer(0.0f),
      m_totalSpawnedCount(0), m_isShotTutorialCleared(false),
      m_isTackleTutorialCleared(false), m_waveImageAnimTimer(0),
      m_waveImageAnimDuration(45), m_waveImageAnimHoldDuration(30),
      m_waveImageAnimInitialHoldDuration(30), m_isWaveImageAnimating(false) {
  // 敵のモデルをロード
  EnemyNormal::LoadModel();
  EnemyRunner::LoadModel();
  EnemyAcid::LoadModel();
  EnemyBoss::LoadModel();

  // ウェーブ画像の読み込み
  m_waveImages[0] = LoadGraph("data/image/wave1.png");
  m_waveImages[1] = LoadGraph("data/image/wave2.png");
  m_waveImages[2] = LoadGraph("data/image/wave3.png");
  m_waveImages[3] = LoadGraph("data/image/wave4.png");
  m_waveImages[4] = LoadGraph("data/image/wave5.png");
}

WaveManager::~WaveManager() {
  // 敵のモデルを解放
  EnemyNormal::DeleteModel();
  EnemyRunner::DeleteModel();
  EnemyAcid::DeleteModel();
  EnemyBoss::DeleteModel();

  // 画像の解放
  // 画像の解放
  for (int i = 0; i < 5; ++i) {
    if (m_waveImages[i] >= 0) {
      DeleteGraph(m_waveImages[i]);
      m_waveImages[i] = -1;
    }
  }
}

void WaveManager::Init() {
  m_enemyData =
      TransformDataLoader::LoadDataCSV("data/CSV/CharacterTransfromData.csv");
  m_enemyList.clear();
  m_spawnInfoList.clear();

  // ウェーブデータをロード
  LoadWaveData();
  // ウェーブデータをロード
  LoadWaveData();
  // スポーンエリアデータをロード
  LoadSpawnAreaData();

  // グリッド初期化
  // World size is roughly 2000x2000 based on road floor
  m_collisionGrid.Init(m_roadFloorMin, m_roadFloorMax, 150.0f); // Cell size 150

  // 各敵種ごとに全ウェーブで同時に出現する最大数を計算
  std::map<int, int> normalPerWave, runnerPerWave, acidPerWave, bossPerWave;
  for (const auto &wave : m_waveDataList) {
    if (wave.enemyType == "NormalEnemy")
      normalPerWave[wave.wave] += wave.count;
    if (wave.enemyType == "RunnerEnemy")
      runnerPerWave[wave.wave] += wave.count;
    if (wave.enemyType == "AcidEnemy")
      acidPerWave[wave.wave] += wave.count;
    if (wave.enemyType == "Boss")
      bossPerWave[wave.wave] += wave.count;
  }
  int maxNormal = 0, maxRunner = 0, maxAcid = 0, maxBoss = 0;

  // 各ウェーブでの最大出現数を計算
  for (const auto &[wave, cnt] : normalPerWave)
    maxNormal = (std::max)(maxNormal, cnt);
  for (const auto &[wave, cnt] : runnerPerWave)
    maxRunner = (std::max)(maxRunner, cnt);
  for (const auto &[wave, cnt] : acidPerWave)
    maxAcid = (std::max)(maxAcid, cnt);
  for (const auto &[wave, cnt] : bossPerWave)
    maxBoss = (std::max)(maxBoss, cnt);

  // その数だけ各プールを確保
  for (int i = m_enemyAcidPool.size(); i < maxAcid; ++i) {
    auto pEnemy = std::make_shared<EnemyAcid>();
    pEnemy->Init();
    pEnemy->SetActive(false);
    m_enemyAcidPool.push_back(pEnemy);
  }
  for (int i = m_enemyBossPool.size(); i < maxBoss; ++i) {
    auto pEnemy = std::make_shared<EnemyBoss>();
    pEnemy->Init();
    pEnemy->SetActive(false);
    m_enemyBossPool.push_back(pEnemy);
  }

  // チュートリアル達成判定用コールバック
  auto deathTypeCallback = [this](const VECTOR &pos, AttackType type) {
    if (m_currentWave == 1) {
      if (type == AttackType::Shoot)
        m_isShotTutorialCleared = true;
      if (type == AttackType::Tackle)
        m_isTackleTutorialCleared = true;
    }
  };
  // 各敵プールの死亡時コールバックを設定
  for (auto &enemy : m_enemyNormalPool)
    enemy->SetOnDeathWithTypeCallback(deathTypeCallback);
  for (auto &enemy : m_enemyRunnerPool)
    enemy->SetOnDeathWithTypeCallback(deathTypeCallback);
  for (auto &enemy : m_enemyAcidPool)
    enemy->SetOnDeathWithTypeCallback(deathTypeCallback);
  for (auto &enemy : m_enemyBossPool)
    enemy->SetOnDeathWithTypeCallback(deathTypeCallback);

  // 敵の死亡時コールバックを設定
  SetOnEnemyDeathCallback([this](const VECTOR &pos) {
    // 死亡した敵を特定
    auto checkAndSet = [this](EnemyBase *enemy) {
      if (!enemy)
        return false;
      // チュートリアル達成判定
      if (m_currentWave == 1) {
        if (enemy->GetLastAttackType() == AttackType::Shoot)
          m_isShotTutorialCleared = true;
        if (enemy->GetLastAttackType() == AttackType::Tackle)
          m_isTackleTutorialCleared = true;
      }
      return true;
    };
    // 敵のプールから位置が一致する敵を探して死亡処理
    for (auto &enemy : m_enemyNormalPool) {
      if (enemy->GetPos().x == pos.x && enemy->GetPos().y == pos.y &&
          enemy->GetPos().z == pos.z) {
        if (checkAndSet(enemy.get()))
          break;
      }
    }
    for (auto &enemy : m_enemyRunnerPool) {
      if (enemy->GetPos().x == pos.x && enemy->GetPos().y == pos.y &&
          enemy->GetPos().z == pos.z) {
        if (checkAndSet(enemy.get()))
          break;
      }
    }
    for (auto &enemy : m_enemyAcidPool) {
      if (enemy->GetPos().x == pos.x && enemy->GetPos().y == pos.y &&
          enemy->GetPos().z == pos.z) {
        if (checkAndSet(enemy.get()))
          break;
      }
    }
    for (auto &enemy : m_enemyBossPool) {
      if (enemy->GetPos().x == pos.x && enemy->GetPos().y == pos.y &&
          enemy->GetPos().z == pos.z) {
        if (checkAndSet(enemy.get()))
          break;
      }
    }
  });
}

void WaveManager::Reset() {
  m_currentWave = 1;
  m_waveTimer = 0.0f;
  m_spawnTimer = 0.0f;
  m_currentSpawnIndex = 0;
  m_waveIntervalTimer = 0.0f;
  m_isWaveActive = false;
  m_isAllWavesCompleted = false;

  m_enemyList.clear();
  m_spawnInfoList.clear(); // スポーン予定もクリア

  // 全プールの敵を非アクティブ化
  for (auto &enemy : m_enemyNormalPool)
    enemy->SetActive(false);
  for (auto &enemy : m_enemyRunnerPool)
    enemy->SetActive(false);
  for (auto &enemy : m_enemyAcidPool)
    enemy->SetActive(false);
  for (auto &enemy : m_enemyBossPool)
    enemy->SetActive(false);

  // チュートリアル関連フラグのリセット
  m_isShotTutorialCleared = false;
  m_isTackleTutorialCleared = false;
}

void WaveManager::Update() {
  if (m_isAllWavesCompleted) {
    return;
  }

  // ウェーブがアクティブな場合、敵のスポーンとウェーブクリア判定を行う
  if (m_isWaveActive) {
    if (IsCurrentWaveCleared()) {

      NextWave();
    } else {

      if (m_currentSpawnIndex < m_spawnInfoList.size() &&
          !m_isWaveImageAnimating) {
        m_spawnTimer += (1.0f / 60.0f) * Game::GetTimeScale();
        while (m_currentSpawnIndex < m_spawnInfoList.size()) {
          EnemySpawnInfo &spawnInfo = m_spawnInfoList[m_currentSpawnIndex];
          if (m_spawnTimer >= spawnInfo.spawnTime && !spawnInfo.isSpawned) {
            // スポーン位置を現在のプレイヤー位置に基づいて再計算
            VECTOR currentPlayerPos = VGet(0.0f, 0.0f, 0.0f);
            if (Game::m_pPlayer) {
              currentPlayerPos = Game::m_pPlayer->GetPos();
            }
            // 0: Main Stage
            spawnInfo.spawnPos =
                GenerateSpawnPos(0, spawnInfo.enemyType, currentPlayerPos,
                                 spawnInfo.spawnLocationType);

            std::shared_ptr<EnemyBase> pEnemy =
                CreateEnemy(spawnInfo.enemyType, spawnInfo.spawnPos);
            if (pEnemy) {
              m_enemyList.push_back(pEnemy);
              spawnInfo.isSpawned = true;
            }
            m_currentSpawnIndex++;
          } else {
            break;
          }
        }
      }
    }
  }
  // ウェーブがアクティブでない場合、次のウェーブ開始までのインターバルをカウントダウン
  else {
      if (m_waveIntervalTimer > 0.0f) {
          m_waveIntervalTimer -= (1.0f / 60.0f) * Game::GetTimeScale();
      }
      else {
      if (m_currentWave <= 5) {
        VECTOR playerPos = VGet(0.0f, 0.0f, 0.0f); // プレイヤー位置の初期値
        StartCurrentWave(playerPos);
      }
    }
  }

  // ウェーブ画像アニメーションの更新
  if (m_isWaveImageAnimating) {
    m_waveImageAnimTimer++;
    if (m_waveImageAnimTimer >= m_waveImageAnimInitialHoldDuration +
                                    m_waveImageAnimDuration +
                                    m_waveImageAnimHoldDuration) {
      m_isWaveImageAnimating = false;
    }
  }

  m_enemyList.erase(
      std::remove_if(m_enemyList.begin(), m_enemyList.end(),
                     [](const std::shared_ptr<EnemyBase> &pEnemy) {
                       return !pEnemy->IsActive();
                     }),
      m_enemyList.end());
}

// GetEnemyListをアクティブな敵のみ返すようにする
void WaveManager::UpdateEnemies(
    std::vector<Bullet> &bullets, const Player::TackleInfo &tackleInfo,
    const Player &player,
    const std::vector<Stage::StageCollisionData> &collisionData,
    Effect *pEffect) {

  std::vector<EnemyBase *> activeEnemies;
  for (auto &pEnemy : m_enemyNormalPool) {
    if (pEnemy->IsActive() && pEnemy->IsAlive())
      activeEnemies.push_back(pEnemy.get());
  }
  for (auto &pEnemy : m_enemyRunnerPool) {
    if (pEnemy->IsActive() && pEnemy->IsAlive())
      activeEnemies.push_back(pEnemy.get());
  }
  for (auto &pEnemy : m_enemyAcidPool) {
    if (pEnemy->IsActive() && pEnemy->IsAlive())
      activeEnemies.push_back(pEnemy.get());
  }
  for (auto &pEnemy : m_enemyBossPool) {
    if (pEnemy->IsActive() && pEnemy->IsAlive())
      activeEnemies.push_back(pEnemy.get());
  }

  // 1. Gridをクリアして再構築
  m_collisionGrid.Clear();
  for (auto *enemy : activeEnemies) {
    m_collisionGrid.RegisterEnemy(enemy);
  }

  // コンテキストを作成
  EnemyUpdateContext context = {
      bullets,         tackleInfo,    player,
      activeEnemies,   collisionData, pEffect,
      &m_collisionGrid // Gridを渡す
  };

  for (auto &pEnemy : m_enemyNormalPool) {
    if (!pEnemy->IsActive())
      continue;
    pEnemy->Update(context);
  }
  for (auto &pEnemy : m_enemyRunnerPool) {
    if (!pEnemy->IsActive())
      continue;
    pEnemy->Update(context);
  }
  for (auto &pEnemy : m_enemyAcidPool) {
    if (!pEnemy->IsActive())
      continue;
    pEnemy->Update(context);
  }
  for (auto &pEnemy : m_enemyBossPool) {
    if (!pEnemy->IsActive())
      continue;
    pEnemy->Update(context);
  }
}

// 敵の一括描画
void WaveManager::DrawEnemies(bool isTutorial) {
  // 敵の描画
  for (auto &pEnemy : m_enemyNormalPool) {
    if (!pEnemy->IsActive())
      continue;
    pEnemy->Draw();
  }
  for (auto &pEnemy : m_enemyRunnerPool) {
    if (!pEnemy->IsActive())
      continue;
    pEnemy->Draw();
  }
  for (auto &pEnemy : m_enemyAcidPool) {
    if (!pEnemy->IsActive())
      continue;
    pEnemy->Draw();
  }
  for (auto &pEnemy : m_enemyBossPool) {
    if (!pEnemy->IsActive())
      continue;
    pEnemy->Draw();
  }

  // デバッグ表示：スポーンエリア
  DrawDebugSpawnAreas(isTutorial);
}

// ウェーブUIの描画
void WaveManager::DrawWaveUI() {
  // ウェーブ中は常に画像を表示
  if (!m_isAllWavesCompleted && m_currentWave >= 1 && m_currentWave <= 5 &&
      (m_isWaveImageAnimating || m_isWaveActive)) {
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

    if (m_isWaveImageAnimating) {
      float t;
      if (m_waveImageAnimTimer < m_waveImageAnimInitialHoldDuration) {
        t = 0.0f; // 初期ホールド中は補間を0に固定
      } else if (m_waveImageAnimTimer <
                 m_waveImageAnimInitialHoldDuration + m_waveImageAnimDuration) {
        t = (float)(m_waveImageAnimTimer - m_waveImageAnimInitialHoldDuration) /
            m_waveImageAnimDuration;
      } else {
        t = 1.0f; // ホールド中は補間を終了位置に固定
      }
      t = (std::min)(1.0f, t); // 0.0fから1.0fにクランプ

      // 線形補間
      currentX = static_cast<int>(startX + (targetX - startX) * t);
      currentY = static_cast<int>(startY + (targetY - startY) * t);
      currentDrawW =
          static_cast<int>(startDrawW + (targetDrawW - startDrawW) * t);
      currentDrawH =
          static_cast<int>(startDrawH + (targetDrawH - startDrawH) * t);
    } else {
      currentX = targetX;
      currentY = targetY;
      currentDrawW = targetDrawW;
      currentDrawH = targetDrawH;
    }

    DrawExtendGraph(currentX, currentY, currentX + currentDrawW,
                    currentY + currentDrawH, img, true);
  }
}

// 敵の死亡時コールバック
void WaveManager::SetOnEnemyDeathCallback(
    std::function<void(const VECTOR &)> callback) {
  m_onEnemyDeathCallback = callback;
}

// 敵ヒット時のコールバックを設定
void WaveManager::SetOnEnemyHitCallback(
    std::function<void(EnemyBase::HitPart, float)> cb) {
  m_onEnemyHitCallback = cb;
}

// Road_floorオブジェクトの範囲を設定
void WaveManager::SetRoadFloorBounds(const VECTOR &minPos,
                                     const VECTOR &maxPos) {
  m_roadFloorMin = minPos;
  m_roadFloorMax = maxPos;
  m_isRoadFloorBoundsSet = true;
}

// ウェーブデータを読み込む
void WaveManager::LoadWaveData() {
  m_waveDataList.clear();

  std::ifstream file("data/CSV/WaveData.csv");
  if (!file.is_open()) {
    printf("Error: Cannot open WaveData.csv\n");
    return;
  }

  std::string line;
  // ヘッダー行をスキップ
  std::getline(file, line);

  // CSVファイルの各行を読み込む
  while (std::getline(file, line)) {
    std::stringstream ss(line);
    std::string token;
    WaveData waveData;

    // Wave
    if (!std::getline(ss, token, ','))
      continue;
    waveData.wave = std::stoi(token);

    // EnemyType
    if (!std::getline(ss, token, ','))
      continue;
    waveData.enemyType = token;

    // Count
    if (!std::getline(ss, token, ','))
      continue;
    waveData.count = std::stoi(token);

    // SpawnInterval
    if (!std::getline(ss, token, ','))
      continue;
    waveData.spawnInterval = std::stof(token);

    // StartTime
    if (!std::getline(ss, token, ','))
      continue;
    waveData.startTime = std::stof(token);

    // WaveInterval
    if (std::getline(ss, token, ',')) {
      waveData.waveInterval = std::stof(token);
    } else {
      waveData.waveInterval = 0.0f;
    }

    // SpawnLocation (Optional, default 0)
    if (std::getline(ss, token, ',')) {
      waveData.spawnLocationType = std::stoi(token);
    } else {
      waveData.spawnLocationType = 0;
    }

    m_waveDataList.push_back(waveData);

    // デバッグ出力
    printf("Loaded: Wave %d, %s, Count %d, Interval %.1f, Start %.1f, Loc %d\n",
           waveData.wave, waveData.enemyType.c_str(), waveData.count,
           waveData.spawnInterval, waveData.startTime,
           waveData.spawnLocationType);
  }
}

// SpawnAreaData.csvを読み込む
void WaveManager::LoadSpawnAreaData() {
  m_spawnAreaList.clear();

  std::ifstream file("data/CSV/SpawnAreaData.csv");
  if (!file.is_open()) {
    printf("Error: Cannot open SpawnAreaData.csv\n");
    return;
  }

  std::string line;
  // ヘッダー行をスキップ
  std::getline(file, line);

  // CSVファイルの各行を読み込む
  while (std::getline(file, line)) {
    std::stringstream ss(line);
    std::string token;
    SpawnAreaInfo info;

    // Type
    if (!std::getline(ss, token, ','))
      continue;
    info.type = std::stoi(token);

    // PosX, PosY, PosZ
    float x, y, z;
    if (!std::getline(ss, token, ','))
      continue;
    x = std::stof(token);
    if (!std::getline(ss, token, ','))
      continue;
    y = std::stof(token);
    if (!std::getline(ss, token, ','))
      continue;
    z = std::stof(token);

    // Unity座標系からの変換（100倍）
    info.center = VGet(x * 100.0f, y * 100.0f, z * 100.0f);

    // ScaleX, ScaleY, ScaleZ
    float sx, sy, sz;
    if (!std::getline(ss, token, ','))
      continue;
    sx = std::stof(token);
    if (!std::getline(ss, token, ','))
      continue;
    sy = std::stof(token);
    if (!std::getline(ss, token, ','))
      continue;
    sz = std::stof(token);

    // スケールも100倍する
    info.size = VGet(sx * 100.0f, sy * 100.0f, sz * 100.0f);

    m_spawnAreaList.push_back(info);

    // デバッグ出力
    printf("Loaded SpawnArea: Type %d, Pos(%.1f, %.1f, %.1f), Size(%.1f, %.1f, "
           "%.1f)\n",
           info.type, info.center.x, info.center.y, info.center.z, info.size.x,
           info.size.y, info.size.z);
  }
}

// ランダムな出現位置を生成
VECTOR WaveManager::GenerateRandomSpawnPos(const VECTOR &playerPos) {
  // Road_floorの範囲が設定されていない場合はデフォルト位置を返す
  if (!m_isRoadFloorBoundsSet) {
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
    std::uniform_real_distribution<float> xDist(m_roadFloorMin.x,
                                                m_roadFloorMax.x);
    std::uniform_real_distribution<float> zDist(m_roadFloorMin.z,
                                                m_roadFloorMax.z);

    float x = xDist(gen);
    float z = zDist(gen);

    spawnPos = VGet(x, 0.0f, z);

    // プレイヤーとの距離を計算
    VECTOR toPlayer = VSub(playerPos, spawnPos);
    toPlayer.y = 0.0f;
    float distanceToPlayer =
        sqrtf(toPlayer.x * toPlayer.x + toPlayer.z * toPlayer.z);

    // プレイヤーからの距離が最小距離以上なら成功
    if (distanceToPlayer >= kMinSpawnDistance) {
      found = true;
      break;
    }

    attempts++;
  } while (attempts < kMaxSpawnAttempts);

  if (!found) {
    // 条件を満たさなかった場合は範囲内のランダムな点を返す
    std::uniform_real_distribution<float> xDist(m_roadFloorMin.x,
                                                m_roadFloorMax.x);
    std::uniform_real_distribution<float> zDist(m_roadFloorMin.z,
                                                m_roadFloorMax.z);
    float x = xDist(gen);
    float z = zDist(gen);
    spawnPos = VGet(x, 0.0f, z);
  }
  return spawnPos;
}

// 出現位置を生成（エリア定義があればそれを使用、なければランダム）
VECTOR WaveManager::GenerateSpawnPos(int type, const std::string &enemyType,
                                     const VECTOR &playerPos,
                                     int spawnLocationType) {
  // タイプに一致するエリアを検索
  std::vector<SpawnAreaInfo> candidates;

  // 指定された高さタイプに基づいてフィルタリング
  float targetY = -999.0f;
  if (spawnLocationType == 1)
    targetY = 200.0f; // 下段
  else if (spawnLocationType == 2)
    targetY = 562.0f; // 中段
  else if (spawnLocationType == 3)
    targetY = 962.0f; // 上段

  // Wave 1 (Main Stage) の特別処理 または 高さ指定がある場合
  if ((m_currentWave == 1 && type == 0) ||
      (spawnLocationType > 0 && type == 0)) {
    // ターゲットY座標が決まっている場合はそれでフィルタリング、なければWave1デフォルトの200.0f
    float checkY = (targetY != -999.0f) ? targetY : 200.0f;

    for (const auto &area : m_spawnAreaList) {
      if (area.type == type && std::abs(area.center.y - checkY) < 10.0f) {
        candidates.push_back(area);
      }
    }

    // プレイヤーとの距離判定
    std::vector<SpawnAreaInfo> validCandidates;
    for (const auto &area : candidates) {
      // プレイヤーがスポーンエリア内にいるかどうかを判定 (AABB)
      float halfSizeX = area.size.x * 0.5f;
      float halfSizeZ = area.size.z * 0.5f;

      bool isInsideX = playerPos.x >= (area.center.x - halfSizeX) &&
                       playerPos.x <= (area.center.x + halfSizeX);
      bool isInsideZ = playerPos.z >= (area.center.z - halfSizeZ) &&
                       playerPos.z <= (area.center.z + halfSizeZ);

      // エリア内にいる場合は除外
      if (isInsideX && isInsideZ) {
        continue;
      }

      // さらに一定距離（例えばエリアの最大半径 + 余裕）離れているかをチェック
      VECTOR diff = VSub(area.center, playerPos);
      float dist = VSize(diff);
      float safeDistance =
          (std::max)(area.size.x, area.size.z) + 200.0f; // サイズ + 余白

      if (dist >= safeDistance) {
        validCandidates.push_back(area);
      }
    }

    // 有効な候補があれば、それらの中で最も遠いエリアを優先する
    if (!validCandidates.empty()) {
      // 距離の降順でソート
      std::sort(validCandidates.begin(), validCandidates.end(),
                [&](const SpawnAreaInfo &a, const SpawnAreaInfo &b) {
                  float distA = VSize(VSub(a.center, playerPos));
                  float distB = VSize(VSub(b.center, playerPos));
                  return distA > distB;
                });
      // 最も遠いエリア1つに絞る
      candidates.clear();
      candidates.push_back(validCandidates[0]);
    } else {
      // 有効な候補がない場合（全てのエリアが近すぎる場合）
      // 緊急回避：
      // 指定された高さの中で最も距離が遠いエリアを選択する
      if (!candidates.empty()) {
        std::sort(candidates.begin(), candidates.end(),
                  [&](const SpawnAreaInfo &a, const SpawnAreaInfo &b) {
                    float distA = VSize(VSub(a.center, playerPos));
                    float distB = VSize(VSub(b.center, playerPos));
                    return distA > distB;
                  });
        std::vector<SpawnAreaInfo> fallback;
        fallback.push_back(candidates[0]); // 一番遠いものだけ残す
        candidates = fallback;
      }
      // 指定高さのエリア自体が見つからない場合は、全エリア検索などのフォールバックが必要だが
      // SpawnData設定ミス以外では起きないはずなので、ここでは空のままにしてランダムスポーンへ
    }
  } else {
    // 通常の処理
    for (const auto &area : m_spawnAreaList) {
      if (area.type == type) {
        candidates.push_back(area);
      }
    }
  }

  // 候補がなければ従来のランダムスポーン
  if (candidates.empty()) {
    // 指定高さのエリアがない場合でも、とりあえずどこかには湧かせる
    printf("Warning: No valid spawn area found for LocationType %d. Using "
           "random spawn.\n",
           spawnLocationType);
    return GenerateRandomSpawnPos(playerPos);
  }

  // 敵の種類に応じたロジック
  std::vector<SpawnAreaInfo> filteredCandidates;

  // Wave 1 または 高さ指定がある場合は既に選定済みなのでそのまま使用
  if ((m_currentWave == 1 && type == 0) ||
      (spawnLocationType > 0 && type == 0)) {
    filteredCandidates = candidates;
  } else {
    // AcidEnemy（遠距離）はプレイヤーから離れたエリアを優先
    bool isLongRange = (enemyType == "AcidEnemy");
    const float kLongRangeThreshold = 500.0f; // 閾値（5m相当）

    if (isLongRange) {
      for (const auto &area : candidates) {
        // 中心点との距離で判定（簡易的）
        VECTOR diff = VSub(area.center, playerPos);
        float dist = VSize(diff);
        if (dist >= kLongRangeThreshold) {
          filteredCandidates.push_back(area);
        }
      }
      // 条件を満たすエリアがなければ、全ての候補を使用（フォールバック）
      if (filteredCandidates.empty()) {
        filteredCandidates = candidates;
      }
    } else {
      // 近接などは全ての候補から選択
      filteredCandidates = candidates;
    }
  }

  unsigned seed =
      (unsigned)std::chrono::system_clock::now().time_since_epoch().count();
  std::mt19937 gen(seed);
  std::uniform_int_distribution<size_t> distIndex(0, filteredCandidates.size() -
                                                         1);

  const SpawnAreaInfo &area = filteredCandidates[distIndex(gen)];

  // エリア内のランダムな位置を生成
  std::uniform_real_distribution<float> xDist(
      area.center.x - area.size.x * 0.5f, area.center.x + area.size.x * 0.5f);
  std::uniform_real_distribution<float> yDist(
      area.center.y - area.size.y * 0.5f, area.center.y + area.size.y * 0.5f);
  std::uniform_real_distribution<float> zDist(
      area.center.z - area.size.z * 0.5f, area.center.z + area.size.z * 0.5f);

  return VGet(xDist(gen), yDist(gen), zDist(gen));
}

// 敵を生成
std::shared_ptr<EnemyBase>
WaveManager::CreateEnemy(const std::string &enemyType, const VECTOR &spawnPos) {
  // ウェーブ1の敵が初めて出現したタイミングでフラグを立てる
  if (m_currentWave == 1 && !m_isWave1EnemySpawned)
    m_isWave1EnemySpawned = true;
  else if (m_currentWave != 1)
    m_isWave1EnemySpawned = false;

  std::shared_ptr<EnemyBase> pEnemy = nullptr;

  // CSVデータから該当する敵のデータを検索
  ObjectTransformData *enemyData = nullptr;
  for (auto &data : m_enemyData) {
    if (data.name == enemyType) {
      enemyData = &data;
      break;
    }
  }

  // 敵の種類に応じてプールから取得または新規生成
  if (enemyType == "NormalEnemy") {
    auto pPooled = GetPooledNormalEnemy();
    pPooled->SetActive(true);
    pPooled->Init();
    if (enemyData)
      pPooled->SetHp(enemyData->hp);
    pPooled->SetPos(spawnPos);
    pEnemy = pPooled;
  } else if (enemyType == "RunnerEnemy") {
    auto pPooled = GetPooledRunnerEnemy();
    pPooled->SetActive(true);
    pPooled->Init();
    if (enemyData)
      pPooled->SetHp(enemyData->hp);
    pPooled->SetPos(spawnPos);
    pEnemy = pPooled;
  } else if (enemyType == "AcidEnemy") {
    auto pPooled = GetPooledAcidEnemy();
    pPooled->SetActive(true);
    pPooled->Init();
    if (enemyData)
      pPooled->SetHp(enemyData->hp);
    pPooled->SetPos(spawnPos);
    pEnemy = pPooled;
  } else if (enemyType == "Boss") {
    auto pPooled = GetPooledBossEnemy();
    pPooled->SetActive(true);
    pPooled->Init();
    // パラメータはInitでCSVから読み込まれるが、WaveManager側でのパラメータセットもサポート
    if (enemyData) {
      pPooled->SetHp(enemyData->hp);
    }
    pPooled->SetPos(spawnPos);
    pEnemy = pPooled;
  } else {
    return nullptr;
  }

  if (pEnemy) {
    // 死亡コールバック
    pEnemy->SetOnDeathCallback(
        [this](const VECTOR &pos) { OnEnemyDeath(pos); });

    // アイテムドロップコールバック
    if (m_onEnemyDeathCallback) {
      pEnemy->SetOnDropItemCallback(m_onEnemyDeathCallback);
    }

    // ヒット時コールバック(ヒットマーク用)
    if (m_onEnemyHitCallback) {
      pEnemy->SetOnHitCallback(m_onEnemyHitCallback);
    }
    printf("Created enemy: %s at position (%.2f, %.2f, %.2f)\n",
           enemyType.c_str(), pEnemy->GetPos().x, pEnemy->GetPos().y,
           pEnemy->GetPos().z);
    m_totalSpawnedCount++;
  }

  return pEnemy;
}

// プールから空きのあるNormalEnemyを取得または新規生成
std::shared_ptr<EnemyNormal> WaveManager::GetPooledNormalEnemy() {
  // プールから空きのある敵を探す
  for (auto &pEnemy : m_enemyNormalPool) {
    if (!pEnemy->IsActive()) {
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
std::shared_ptr<EnemyRunner> WaveManager::GetPooledRunnerEnemy() {
  // プールから空きのある敵を探す
  for (auto &pEnemy : m_enemyRunnerPool) {
    if (!pEnemy->IsActive()) {
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
std::shared_ptr<EnemyAcid> WaveManager::GetPooledAcidEnemy() {
  // プールから空きのある敵を探す
  for (auto &pEnemy : m_enemyAcidPool) {
    if (!pEnemy->IsActive()) {
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
void WaveManager::StartCurrentWave(const VECTOR &playerPos) {
  m_spawnInfoList.clear();
  m_currentSpawnIndex = 0;
  m_spawnTimer = 0.0f;
  m_isWaveActive = true;
  m_enemyList.clear(); // 敵リストをクリア

  // ウェーブ画像アニメーションを開始
  m_isWaveImageAnimating = true;
  m_waveImageAnimTimer = 0;

  // ウェーブ1の敵ロードフラグ
  if (m_currentWave == 1) {
    m_isWave1Loaded = true;
  } else {
    m_isWave1Loaded = false;
  }

  // 現在のwaveのデータを取得
  std::vector<WaveData> currentWaveData;
  for (const WaveData &waveData : m_waveDataList) {
    if (waveData.wave == m_currentWave) {
      currentWaveData.push_back(waveData);
    }
  }

  // 出現情報を作成
  for (const WaveData &waveData : currentWaveData) {
    printf("Processing %s: Count %d, StartTime %.1f, Interval %.1f\n",
           waveData.enemyType.c_str(), waveData.count, waveData.startTime,
           waveData.spawnInterval);

    for (int i = 0; i < waveData.count; ++i) // 出現数分ループ
    {
      // 出現位置を生成
      EnemySpawnInfo spawnInfo;
      spawnInfo.enemyType = waveData.enemyType;
      // 実際の位置はUpdateで生成時にプレイヤー位置に基づいて再計算する
      spawnInfo.spawnPos = VGet(0, 0, 0);
      spawnInfo.spawnTime = waveData.startTime + (i * waveData.spawnInterval);
      spawnInfo.isSpawned = false;
      spawnInfo.spawnLocationType = waveData.spawnLocationType; // 追加

      m_spawnInfoList.push_back(spawnInfo);
    }
  }

  // spawnTimeで昇順ソート
  std::sort(m_spawnInfoList.begin(), m_spawnInfoList.end(),
            [](const EnemySpawnInfo &a, const EnemySpawnInfo &b) {
              return a.spawnTime < b.spawnTime;
            });

  // デバッグ出力：スポーンスケジュール
  printf("Wave %d spawn schedule (%d enemies total):\n", m_currentWave,
         static_cast<int>(m_spawnInfoList.size()));
  for (size_t i = 0; i < m_spawnInfoList.size(); ++i) {
    const auto &info = m_spawnInfoList[i];
    printf("  [%d] %s at %.2fs (%.2f, %.2f, %.2f)\n", static_cast<int>(i),
           info.enemyType.c_str(), info.spawnTime, info.spawnPos.x,
           info.spawnPos.y, info.spawnPos.z);
  }
}

void WaveManager::NextWave() {
  // 現在完了したウェーブのインターバル値の中から最大値を取得する
  float maxInterval = 0.0f;
  for (const auto &waveData : m_waveDataList) {
    if (waveData.wave == m_currentWave) {
      if (waveData.waveInterval > maxInterval) {
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
bool WaveManager::IsCurrentWaveCleared() {
  // 現在のwaveの敵がすべて出現済みで、生存している敵がいない場合
  if (m_currentSpawnIndex >= m_spawnInfoList.size()) {
    for (const std::shared_ptr<EnemyBase> &pEnemy : m_enemyList) {
      if (pEnemy->IsAlive()) {
        return false;
      }
    }
    return true;
  }
  return false;
}

// 敵の死亡処理
void WaveManager::OnEnemyDeath(const VECTOR &position) {
  // デバッグ出力
  if (m_onEnemyDeathCallback) {
    m_onEnemyDeathCallback(position);
  }
}

// スポーンエリアのデバッグ表示
void WaveManager::DrawDebugSpawnAreas(bool isTutorial) {
  if (!s_isDrawSpawnAreas)
    return;

  for (const auto &area : m_spawnAreaList) {
    // ステージに合致しないエリアはスキップ
    // type 0: Main, type 1: Tutorial
    if (isTutorial && area.type != 1)
      continue;
    if (!isTutorial && area.type != 0)
      continue;

    VECTOR minPos = VSub(area.center, VScale(area.size, 0.5f));
    VECTOR maxPos = VAdd(area.center, VScale(area.size, 0.5f));

    // 色決定 (Typeによって変える)
    unsigned int color;
    if (area.type == 0) {
      color = 0x0000ff;
    } else if (area.type == 1) {
      color = 0xffff00;
    } else {
      color = 0xffffff;
    }

    // ワイヤーフレームで描画
    DrawCube3D(minPos, maxPos, color, color, false);
  }
}

// デバッグ情報の表示
void WaveManager::DrawDebugInfo() {
  // フォントサイズを設定
  SetFontSize(kFontSize);

#ifdef _DEBUG
  // 現在のwave情報
  char waveInfo[256];
  sprintf_s(waveInfo, "Wave:%d/3", m_currentWave);
  DrawString(kDebugInfoPosX, kDebugInfoPosY, waveInfo, 0xffffff);

  // 経過時間
  char timeInfo[256];
  sprintf_s(timeInfo, "Timer:%.1fs", m_spawnTimer);
  DrawString(kDebugInfoPosX + kDebugInfoSpacing, kDebugInfoPosY, timeInfo,
             0xffffff);

  // 敵の出現情報
  char spawnInfo[256];
  sprintf_s(spawnInfo, "Spawn:%d/%d", m_currentSpawnIndex,
            static_cast<int>(m_spawnInfoList.size()));
  DrawString(kDebugInfoPosX + kDebugInfoSpacing * 2, kDebugInfoPosY, spawnInfo,
             0xffffff);

  // 生存している敵の数
  int aliveEnemies = 0;
  for (const std::shared_ptr<EnemyBase> &pEnemy : m_enemyList) {
    if (pEnemy->IsAlive()) {
      aliveEnemies++;
    }
  }
  char enemyInfo[256];
  sprintf_s(enemyInfo, "Alive:%d", aliveEnemies);
  DrawString(kDebugInfoPosX + kDebugInfoSpacing * 3, kDebugInfoPosY, enemyInfo,
             0xffffff);

  // 総敵数
  char totalEnemyInfo[256];
  sprintf_s(totalEnemyInfo, "Total:%d", m_totalSpawnedCount);
  DrawString(kDebugInfoPosX + kDebugInfoSpacing * 4, kDebugInfoPosY,
             totalEnemyInfo, 0xffffff);

  int currentX = kDebugInfoPosX + kDebugInfoSpacing * 5;
#else
  int currentX = kDebugInfoPosX;
#endif

  if (s_isShowActiveEnemyCount) {
    char activeInfo[256];
    sprintf_s(activeInfo, "Active(All):%d", (int)m_enemyList.size());
    DrawString(currentX, kDebugInfoPosY, activeInfo, 0xffffff);
    currentX += kDebugInfoSpacing * 2; // 少し広めにとる
  }

  if (s_isShowDrawnEnemyCount) {
    char drawnInfo[256];
    sprintf_s(drawnInfo, "Drawn:%d", EnemyBase::GetDrawCount());
    DrawString(currentX, kDebugInfoPosY, drawnInfo, 0xffffff);
  }
}

int WaveManager::GetAliveEnemyCount() const {
  int aliveCount = 0;
  for (const auto &pEnemy : m_enemyNormalPool) {
    if (pEnemy->IsActive() && pEnemy->IsAlive()) {
      aliveCount++;
    }
  }
  for (const auto &pEnemy : m_enemyRunnerPool) {
    if (pEnemy->IsActive() && pEnemy->IsAlive()) {
      aliveCount++;
    }
  }
  for (const auto &pEnemy : m_enemyAcidPool) {
    if (pEnemy->IsActive() && pEnemy->IsAlive()) {
      aliveCount++;
    }
  }
  return aliveCount;
}

void WaveManager::SpawnTutorialWave(int tutorialWaveId) {
  // 既存の敵をすべて非アクティブ化し、m_enemyListをクリア
  for (auto &pEnemy : m_enemyNormalPool)
    pEnemy->SetActive(false);
  for (auto &pEnemy : m_enemyRunnerPool)
    pEnemy->SetActive(false);
  for (auto &pEnemy : m_enemyAcidPool)
    pEnemy->SetActive(false);
  m_enemyList.clear();

  std::ifstream file("data/CSV/TutorialWaves.csv");
  if (!file.is_open()) {
    return;
  }

  std::string line;
  std::getline(file, line); // ヘッダー行をスキップ

  std::vector<WaveData> tutorialWaves;
  while (std::getline(file, line)) {
    std::stringstream ss(line);
    std::string token;
    WaveData waveData;
    if (!std::getline(ss, token, ','))
      continue;
    waveData.wave = std::stoi(token);
    if (!std::getline(ss, token, ','))
      continue;
    waveData.enemyType = token;
    if (!std::getline(ss, token, ','))
      continue;
    waveData.count = std::stoi(token);
    if (!std::getline(ss, token, ','))
      continue;
    waveData.spawnInterval = std::stof(token);
    if (!std::getline(ss, token, ','))
      continue;
    waveData.startTime = std::stof(token);
    if (std::getline(ss, token, ','))
      waveData.waveInterval = std::stof(token);
    else
      waveData.waveInterval = 0.0f;
    tutorialWaves.push_back(waveData);
  }

  for (const auto &waveData : tutorialWaves) {
    if (waveData.wave == tutorialWaveId) {
      for (int i = 0; i < waveData.count; ++i) {
        VECTOR playerPos = VGet(0, 0, 0); // プレイヤー位置を仮定
        // 1: Tutorial Stage
        VECTOR spawnPos = GenerateSpawnPos(1, waveData.enemyType, playerPos);
        std::shared_ptr<EnemyBase> pEnemy =
            CreateEnemy(waveData.enemyType, spawnPos);
        // CreateEnemyはプールから敵を取得し、アクティブに設定する
        // m_enemyListには後でアクティブな敵をすべて追加する
      }
    }
  }

  // すべてのプールからアクティブな敵をm_enemyListに追加
  for (auto &pEnemy : m_enemyNormalPool) {
    if (pEnemy->IsActive())
      m_enemyList.push_back(pEnemy);
  }
  for (auto &pEnemy : m_enemyRunnerPool) {
    if (pEnemy->IsActive())
      m_enemyList.push_back(pEnemy);
  }
  for (auto &pEnemy : m_enemyAcidPool) {
    if (pEnemy->IsActive())
      m_enemyList.push_back(pEnemy);
  }
}

// プールから空きのあるBossEnemyを取得または新規生成
std::shared_ptr<EnemyBoss> WaveManager::GetPooledBossEnemy() {
  // プールから空きのある敵を探す
  for (auto &pEnemy : m_enemyBossPool) {
    if (!pEnemy->IsActive()) {
      return pEnemy;
    }
  }

  // プールに空きがなければ新規生成
  auto pEnemy = std::make_shared<EnemyBoss>();
  pEnemy->Init();
  m_enemyBossPool.push_back(pEnemy);
  return pEnemy;
}
