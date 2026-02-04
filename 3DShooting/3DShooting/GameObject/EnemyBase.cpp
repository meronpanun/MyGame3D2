#include "EnemyBase.h"
#include "Bullet.h"
#include "Collider.h"
#include "Collision.h"
#include "Effect.h"
#include "Stage.h"
#include "Game.h"
#include "TaskTutorialManager.h"

namespace {
constexpr int kDefaultHitDisplayDuration = 60; // 1秒間表示
constexpr float kDefaultInitialHP = 100.0f;    // デフォルトの初期体力
constexpr float kDefaultCooldownMax = 60;      // 攻撃クールダウンの最大値
constexpr float kDefaultAttackPower = 10.0f;   // 攻撃力
} // namespace

int EnemyBase::s_drawCount = 0;
bool EnemyBase::s_showDamage = false;
float EnemyBase::s_debugLastDamage = 0.0f;
int EnemyBase::s_debugDamageTimer = 0;
std::string EnemyBase::s_debugHitInfo = "";

EnemyBase::EnemyBase()
    : m_pos{0, 0, 0}, m_modelHandle(-1), m_pTargetPlayer(nullptr),
      m_hp(kDefaultInitialHP), m_maxHp(kDefaultInitialHP),
      m_lastHitPart(HitPart::None), m_lastTackleId(-1), m_hitDisplayTimer(0),
      m_isAlive(true), m_isTackleHit(false), m_attackCooldown(0),
      m_attackCooldownMax(kDefaultCooldownMax),
      m_attackPower(kDefaultAttackPower), m_attackHitFrame(0),
      m_isAttacking(false), m_isActive(true), m_verticalVelocity(0.0f),
      m_isGrounded(false), m_updateFrameCount(0), m_aiUpdateInterval(1),
      m_isSimpleMode(false), m_shouldUpdateAI(true) {}

void EnemyBase::CheckHitAndDamage(std::vector<Bullet> &bullets,
                                  Effect *pEffect) {
  // 最も近いヒット情報を保持
  int hitBulletIndex = -1;
  float minHitDistSq = FLT_MAX;              // 最も近い衝突までの距離の2乗
  HitPart determinedHitPart = HitPart::None; // 最終的に決定されたヒット部位

  for (int i = 0; i < bullets.size(); ++i) {
    auto &bullet = bullets[i];
    if (!bullet.IsActive())
      continue;

    // 弾のRay情報を取得
    VECTOR rayStart = bullet.GetPrevPos();
    VECTOR rayEnd = bullet.GetPos();

    // どこに当たったのかをチェック
    // CheckHitPartは最も近い衝突点を考慮してHitPartを返すようにする
    // ここでは距離の情報も内部で利用するようにする
    VECTOR currentHitPos;
    float currentHitDistSq;
    HitPart part =
        CheckHitPart(rayStart, rayEnd, currentHitPos,
                     currentHitDistSq); // 距離情報も受け取るように変更

    if (part != HitPart::None) {
      if (currentHitDistSq < minHitDistSq) {
        minHitDistSq = currentHitDistSq;
        hitBulletIndex = i;
        determinedHitPart = part; // 最も近いヒットの部位を保持
      }
    }
  }

  // 最も近い弾でダメージ処理を行う
  if (hitBulletIndex != -1) {
    auto &bullet = bullets[hitBulletIndex];
    float damage = CalcDamage(bullet.GetDamage(), determinedHitPart);
    TakeDamage(damage, bullet.GetAttackType()); // 攻撃種別を渡す

    // デバッグ表示用更新
    if (s_showDamage) {
      s_debugLastDamage = damage;
      s_debugDamageTimer = 120;
      if (determinedHitPart == HitPart::Head) s_debugHitInfo = "(Head)";
      else if (determinedHitPart == HitPart::Body) s_debugHitInfo = "(Body)";
      else s_debugHitInfo = "(None)";
    }

    m_lastHitPart = determinedHitPart;
    m_hitDisplayTimer = kDefaultHitDisplayDuration;

    // 弾が当たった位置で出血エフェクトを生成
    if (pEffect) {
      VECTOR hitPos = bullet.GetPos(); // 弾の現在位置を衝突位置として使用
      pEffect->PlayLossOfBlood(hitPos.x, hitPos.y, hitPos.z, 0.0f, 0.0f, 0.0f);
    }

    bullet.Deactivate(); // 敵に当たった弾は非アクティブにする

    // ヒット時コールバック（ヒットマーク用）
    if (m_onHitCallback) {
      // 距離を計算して渡す
      float hitDist = sqrtf(minHitDistSq);
      m_onHitCallback(determinedHitPart, hitDist);
    }
  }
}

// 敵がダメージを受ける処理
void EnemyBase::TakeDamage(float damage, AttackType type) {
  m_lastAttackType = type;
  m_hp -= damage;
  if (m_hp <= 0.0f) {
    m_hp = 0.0f;
    if (m_isAlive) {
      m_isAlive = false;
      TaskTutorialManager::GetInstance()->NotifyEnemyKilled(m_lastAttackType);
      if (m_onDeathWithTypeCallback)
        m_onDeathWithTypeCallback(m_pos, m_lastAttackType);
      OnDeath(); // 敵が死亡した際にOnDeathを呼び出す
    }
  }

  // デバッグ表示用
  if (s_showDamage) {
    s_debugLastDamage = damage;
    s_debugDamageTimer = 120;
    switch (type) {
    case AttackType::Shoot:
      s_debugHitInfo = "(Shot)"; // 後で詳細(Head/Body)で上書きされる可能性あり
      break;
    case AttackType::Tackle:
      s_debugHitInfo = "(Tackle)";
      break;
    case AttackType::ShieldThrow:
      s_debugHitInfo = "(Shield)";
      break;
    case AttackType::Parry:
      s_debugHitInfo = "(Parry)";
      break;
    default:
      s_debugHitInfo = "(Unknown)";
      break;
    }
  }
}

// 敵がタックルダメージを受ける処理
void EnemyBase::TakeTackleDamage(float damage) {
  TakeDamage(damage, AttackType::Tackle);
  m_lastHitPart = HitPart::Body;
  m_hitDisplayTimer = kDefaultHitDisplayDuration;
  
  // デバッグ表示用
  if (s_showDamage) {
    s_debugLastDamage = damage;
    s_debugDamageTimer = 120;
    s_debugHitInfo = "(Tackle)";
  }
}

void EnemyBase::UpdateStageCollision(
    const std::vector<Stage::StageCollisionData> &collisionData) {
  // 敵のコライダーのサイズ（仮の値。敵の種類ごとに調整が必要）
  constexpr float kCapsuleHeight = 100.0f;
  constexpr float kCapsuleRadius = 30.0f;
  constexpr float kColliderYOffset = 60.0f;

  // 重力定数 (PlayerMovementと同じ値を使用)
  constexpr float kGravity = 0.35f;

  // 重力適用
  m_verticalVelocity -= kGravity;
  m_pos.y += m_verticalVelocity;

  CollisionResult result = Collision::CheckStageCollision(
      m_pos, kCapsuleHeight, kCapsuleRadius, kColliderYOffset, collisionData);
  m_isGrounded = result.isGrounded;

  // Y=0 平面（地面）との判定
  if (m_pos.y <= 0.0f) {
    m_pos.y = 0.0f;
    m_isGrounded = true;
  }

  if (m_isGrounded && m_verticalVelocity < 0.0f) {
    m_verticalVelocity = 0.0f;
  }
}

bool EnemyBase::IsTargetVisible(
    const VECTOR &startPos, const VECTOR &targetPos,
    const std::vector<Stage::StageCollisionData> &stageCollision) {
  VECTOR dir = VSub(targetPos, startPos);
  float dist = VSize(dir);
  if (dist < 0.001f)
    return true;
  dir = VNorm(dir);

  // レイキャスト判定
  for (const auto &col : stageCollision) {
    float t;
    if (Collision::IntersectRayTriangle(startPos, dir, col.v1, col.v2, col.v3,
                                        t)) {
      if (t < dist) {
        return false; // 遮蔽物あり
      }
    }
  }

  return true;
}

VECTOR EnemyBase::CalculateParabolicVelocity(const VECTOR &startPos,
                                             const VECTOR &targetPos,
                                             float gravity, float speed) {
  VECTOR toTarget = VSub(targetPos, startPos);

  // 滞空時間を設定 (距離に応じて調整)
  float dist = VSize(toTarget);
  // 直線より速い速度を基準にして放物線を低くする
  float time = dist / (speed * 2.0f);
  if (time < 20.0f)
    time = 20.0f; // 最低保証

  // 初速度計算
  // pos + v*t + 0.5*g*t*t = target
  // v*t = target - pos - 0.5*g*t*t
  VECTOR gravityVec = VGet(0.0f, -gravity, 0.0f);
  VECTOR term1 = VScale(toTarget, 1.0f / time);
  VECTOR term2 = VScale(gravityVec, 0.5f * time);

  return VSub(term1, term2);
}

void EnemyBase::UpdateThrottling(const VECTOR &playerPos) {
  // カメラ情報の取得
  VECTOR camPos = GetCameraPosition();
  VECTOR camTarget = GetCameraTarget();
  VECTOR camDir = VNorm(VSub(camTarget, camPos));

  // プレイヤーとの距離チェック
  VECTOR toPlayer = VSub(playerPos, m_pos);
  float distSq = VSquareSize(toPlayer);

  // デフォルト設定
  m_aiUpdateInterval = 1;
  m_isSimpleMode = false;

  // 1. 距離による更新頻度変更
  if (distSq > 800.0f * 800.0f) {
    m_aiUpdateInterval = 3; // 遠距離: 3フレームに1回
  } else if (distSq > 400.0f * 400.0f) {
    m_aiUpdateInterval = 2; // 中距離: 2フレームに1回
  }

  // 2. 視界判定 (画面外停止)
  // Bossは常にフルパワー
  if (!IsBoss()) {
    VECTOR toEnemy = VSub(m_pos, camPos);
    float enemyDistSq = VSquareSize(toEnemy);

    // カメラからある程度離れている場合のみ判定
    if (enemyDistSq > 300.0f * 300.0f) {
      VECTOR dirToEnemy = VNorm(toEnemy);
      float dot = VDot(camDir, dirToEnemy);

      // 視界外 (視野角 約66度相当) かつ プレイヤーからもある程度離れている
      if (dot < 0.4f && distSq > 400.0f * 400.0f) {
        m_isSimpleMode = true;
      }
    }
  }

  // フレームカウントの更新と実行フラグの設定
  m_updateFrameCount++;
  m_shouldUpdateAI = (m_updateFrameCount % m_aiUpdateInterval == 0);
}

// デバッグ用ダメージ描画
void EnemyBase::DrawDebugDamage() {
  if (s_showDamage && s_debugDamageTimer > 0) {
    s_debugDamageTimer--;

    int screenW = Game::GetScreenWidth();
    int screenH = Game::GetScreenHeight();

    // 表示テキストの整形
    char text[256];
    sprintf_s(text, "Last Damage: %.1f %s", s_debugLastDamage,
              s_debugHitInfo.c_str());

    // テキストサイズ計算 (簡易的に文字数 * 幅と仮定、または等幅フォントなら正確)
    // DxLibのデフォルトフォント前提
    int strLen = static_cast<int>(strlen(text));
    int fontSize = GetFontSize();
    // 全角半角混じりは GetDrawStringWidth が確実
    int strWidth = GetDrawStringWidth(text, strLen);

    // 背景ボックスのサイズと位置
    int paddingX = 20;
    int paddingY = 10;
    int boxW = strWidth + paddingX * 2;
    int boxH = fontSize + paddingY * 2;

    int boxX = (screenW - boxW) / 2;     // 横中央
    int boxY = static_cast<int>(screenH * 0.15f); // 画面上部から15%の位置

    // 半透明背景描画 (黒, alpha=128)
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);
    DrawBox(boxX, boxY, boxX + boxW, boxY + boxH, 0x000000, TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    // テキスト描画 (赤)
    DrawString(boxX + paddingX, boxY + paddingY, text, 0xFF0000);
  }
}