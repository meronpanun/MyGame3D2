#include "EnemyBase.h"
#include "Bullet.h"
#include "Collider.h"
#include "Effect.h"
#include "TaskTutorialManager.h"
#include "Collision.h"
#include "Stage.h"

namespace
{
	constexpr int   kDefaultHitDisplayDuration = 60;     // 1秒間表示
	constexpr float kDefaultInitialHP          = 100.0f; // デフォルトの初期体力 
	constexpr float kDefaultCooldownMax        = 60;     // 攻撃クールダウンの最大値
	constexpr float kDefaultAttackPower        = 10.0f;  // 攻撃力
}

EnemyBase::EnemyBase() :
	m_pos{ 0, 0, 0 },
	m_modelHandle(-1),
	m_pTargetPlayer(nullptr),
	m_hp(kDefaultInitialHP),
	m_maxHp(kDefaultInitialHP),
	m_lastHitPart(HitPart::None),
	m_lastTackleId(-1),
	m_hitDisplayTimer(0),
	m_isAlive(true),
	m_isTackleHit(false),
	m_attackCooldown(0),
	m_attackCooldownMax(kDefaultCooldownMax),
	m_attackPower(kDefaultAttackPower),
	m_attackHitFrame(0),
	m_isAttacking(false),
	m_isActive(true),
	m_verticalVelocity(0.0f),
	m_isGrounded(false)
{
}

void EnemyBase::CheckHitAndDamage(std::vector<Bullet>& bullets, Effect* pEffect)
{
    // 最も近いヒット情報を保持
    int hitBulletIndex = -1;
    float minHitDistSq = FLT_MAX; // 最も近い衝突までの距離の2乗
    HitPart determinedHitPart = HitPart::None; // 最終的に決定されたヒット部位

    for (int i = 0; i < bullets.size(); ++i)
    {
        auto& bullet = bullets[i];
        if (!bullet.IsActive()) continue;

        // 弾のRay情報を取得
        VECTOR rayStart = bullet.GetPrevPos();
        VECTOR rayEnd = bullet.GetPos();

        // どこに当たったのかをチェック
        // CheckHitPartは最も近い衝突点を考慮してHitPartを返すようにする
        // ここでは距離の情報も内部で利用するようにする
        VECTOR currentHitPos;
        float currentHitDistSq;
        HitPart part = CheckHitPart(rayStart, rayEnd, currentHitPos, currentHitDistSq); // 距離情報も受け取るように変更

        if (part != HitPart::None)
        {
            if (currentHitDistSq < minHitDistSq)
            {
                minHitDistSq = currentHitDistSq;
                hitBulletIndex = i;
                determinedHitPart = part; // 最も近いヒットの部位を保持
            }
        }
    }

    // 最も近い弾でダメージ処理を行う
    if (hitBulletIndex != -1)
    {
        auto& bullet = bullets[hitBulletIndex];
        float damage = CalcDamage(bullet.GetDamage(), determinedHitPart);
        TakeDamage(damage, bullet.GetAttackType()); // 攻撃種別を渡す

        m_lastHitPart = determinedHitPart;
        m_hitDisplayTimer = kDefaultHitDisplayDuration;

        // 弾が当たった位置で出血エフェクトを生成
        if (pEffect)
        {
            VECTOR hitPos = bullet.GetPos(); // 弾の現在位置を衝突位置として使用
            pEffect->PlayLossOfBlood(hitPos.x, hitPos.y, hitPos.z, 0.0f, 0.0f, 0.0f);
        }

        bullet.Deactivate(); // 敵に当たった弾は非アクティブにする

        // ヒット時コールバック（ヒットマーク用）
        if (m_onHitCallback) 
        {
            printf("EnemyBase: Hit callback triggered with part: %d\n", static_cast<int>(determinedHitPart));
            m_onHitCallback(determinedHitPart);
        }
    }
}

// 敵がダメージを受ける処理
void EnemyBase::TakeDamage(float damage, AttackType type)
{
    m_lastAttackType = type;
    m_hp -= damage;
    if (m_hp <= 0.0f)
    {
        m_hp = 0.0f;
        if (m_isAlive)
        {
            m_isAlive = false;
            TaskTutorialManager::GetInstance()->NotifyEnemyKilled(m_lastAttackType);
            if (m_onDeathWithTypeCallback) m_onDeathWithTypeCallback(m_pos, m_lastAttackType);
            OnDeath(); // 敵が死亡した際にOnDeathを呼び出す
        }
    }
}

// 敵がタックルダメージを受ける処理
void EnemyBase::TakeTackleDamage(float damage)
{
    TakeDamage(damage, AttackType::Tackle);
    m_lastHitPart = HitPart::Body;
    m_hitDisplayTimer = kDefaultHitDisplayDuration;
}

void EnemyBase::UpdateStageCollision(const std::vector<Stage::StageCollisionData>& collisionData)
{
    // 敵のコライダーのサイズ（仮の値。敵の種類ごとに調整が必要）
    constexpr float kCapsuleHeight = 100.0f;
    constexpr float kCapsuleRadius = 30.0f;
    constexpr float kColliderYOffset = 60.0f;

    // 重力定数 (PlayerMovementと同じ値を使用)
    constexpr float kGravity = 0.35f;

    // 重力適用
    m_verticalVelocity -= kGravity;
    m_pos.y += m_verticalVelocity;

    CollisionResult result = Collision::CheckStageCollision(m_pos, kCapsuleHeight, kCapsuleRadius, kColliderYOffset, collisionData);
    m_isGrounded = result.isGrounded;

    // Y=0 平面（地面）との判定
    if (m_pos.y <= 0.0f)
    {
        m_pos.y = 0.0f;
        m_isGrounded = true;
    }

    // 接地時は落下速度をリセット
    if (m_isGrounded && m_verticalVelocity < 0.0f)
    {
        m_verticalVelocity = 0.0f;
    }
}