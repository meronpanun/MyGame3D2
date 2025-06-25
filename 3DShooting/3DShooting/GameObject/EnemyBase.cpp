#include "Player.h"
#include "EnemyBase.h"
#include "Bullet.h"
#include "Collider.h"

namespace
{
	constexpr int   kDefaultHitDisplayDuration = 60;     // 1秒間表示
	constexpr float kDefaultInitialHP = 100.0f; // デフォルトの初期体力 

	constexpr float kDefaultCooldownMax = 60;     // 攻撃クールダウンの最大値
	constexpr float kDefaultAttackPower = 10.0f;  // 攻撃力
}

EnemyBase::EnemyBase() :
	m_pos{ 0, 0, 0 },
	m_modelHandle(-1),
	// m_colRadius(1.0f), // 削除
	m_targetPlayer(nullptr),
	m_hp(kDefaultInitialHP),
	m_lastHitPart(HitPart::None),
	m_hitDisplayTimer(0),
	m_isAlive(true),
	m_isTackleHit(false),
	m_attackCooldown(0),
	m_attackCooldownMax(kDefaultCooldownMax),
	m_attackPower(kDefaultAttackPower),
	m_attackHitFrame(0),
	m_isAttacking(false)
{
}

void EnemyBase::CheckHitAndDamage(std::vector<Bullet>& bullets)
{
    // 最も近いヒット情報を保持
    int hitBulletIndex = -1;
    float minHitDistSq = FLT_MAX; // 最も近い衝突までの距離の2乗
    HitPart determinedHitPart = HitPart::None; // 最終的に決定されたヒット部位

    for (int i = 0; i < bullets.size(); ++i)
    {
        auto& bullet = bullets[i];
        if (!bullet.IsActive()) continue;

        // 弾のRay情報を取得 (前フレーム位置 -> 現在位置)
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
        TakeDamage(damage);

        m_lastHitPart = determinedHitPart;
        m_hitDisplayTimer = kDefaultHitDisplayDuration;

        bullet.Deactivate(); // 敵に当たった弾は非アクティブにする
    }
}

// 敵がダメージを受ける処理
void EnemyBase::TakeDamage(float damage)
{
	m_hp -= damage;
	if (m_hp <= 0.0f)
	{
		m_hp = 0.0f;
		m_isAlive = false;
	}
}

// 敵がタックルダメージを受ける処理
void EnemyBase::TakeTackleDamage(float damage)
{
	TakeDamage(damage); // デフォルトは通常ダメージと同じ

	// ヒット表示を体ヒットとして更新
	m_lastHitPart = HitPart::Body;
	m_hitDisplayTimer = kDefaultHitDisplayDuration; // 1秒間表示
}