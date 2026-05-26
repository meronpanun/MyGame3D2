#include "PlayerLockOnSystem.h"
#include "EnemyBase.h"
#include "Camera.h"
#include "Game.h"
#include "WaveManager.h"
#include "CollisionGrid.h"
#include "Collision.h"
#include <algorithm>

namespace
{
    constexpr float kRayCastLength        = 2000.0f; // レイキャストの最大長
    constexpr float kGridLookAheadDist    = 500.0f;  // グリッド取得時の前方参照距離
    constexpr float kEnemyTargetHeightOffset = 70.0f; // 敵ターゲット位置の高さオフセット
}

PlayerLockOnSystem::PlayerLockOnSystem()
    : m_isLockingOn(false)
    , m_isTargetAvailable(false)
    , m_isAimingAtEnemy(false)
    , m_lockedOnEnemy(nullptr)
    , m_aimCheckSkipCounter(0)
{
}

void PlayerLockOnSystem::Update(const VECTOR& playerPos, Camera* pCamera,
    const std::vector<EnemyBase*>& enemyList,
    const std::vector<Stage::StageCollisionData>& /*collisionData*/,
    bool isGuarding, float tackleCooldown)
{
    if (!pCamera) return;

    VECTOR camPos = pCamera->GetPos();
    VECTOR camDir = VNorm(VSub(pCamera->GetTarget(), camPos));
    VECTOR rayEnd = VAdd(camPos, VScale(camDir, kRayCastLength));

    m_isTargetAvailable = false;

    // 近傍の敵を取得（グリッド利用）
    std::vector<EnemyBase*> nearbyEnemies;
    if (Game::GetWaveManager())
    {
        Game::GetWaveManager()->GetCollisionGrid().GetNeighbors(playerPos,                                          nearbyEnemies, false);
        Game::GetWaveManager()->GetCollisionGrid().GetNeighbors(VAdd(playerPos, VScale(camDir, kGridLookAheadDist)), nearbyEnemies, false);
    }

    // 重複除去（2回の GetNeighbors で同一敵が混入する場合がある）
    std::sort(nearbyEnemies.begin(), nearbyEnemies.end());
    nearbyEnemies.erase(std::unique(nearbyEnemies.begin(), nearbyEnemies.end()), nearbyEnemies.end());

    // フォールバック：グリッド外にいる場合のみ全敵リストをコピーして使用
    // （enemyList は const なのでローカルコピーを作成してソート対象にする）
    if (nearbyEnemies.empty())
    {
        nearbyEnemies.assign(enemyList.begin(), enemyList.end());
    }

    // プレイヤーからの距離でソートし、上位 kMaxAimTargets 体に絞る
    // （密集時でもレイキャスト回数を上限に固定する）
    std::sort(nearbyEnemies.begin(), nearbyEnemies.end(), [&playerPos](const EnemyBase* a, const EnemyBase* b)
    {
        float daSq = VSquareSize(VSub(a->GetPos(), playerPos));
        float dbSq = VSquareSize(VSub(b->GetPos(), playerPos));
        return daSq < dbSq;
    });
    if (static_cast<int>(nearbyEnemies.size()) > kMaxAimTargets)
    {
        nearbyEnemies.resize(kMaxAimTargets);
    }

    // 近傍ステージ三角形を1回だけ取得してキャッシュ
    // CheckLineOfSight で毎回 collisionData 全件を走査する代わりに、
    // グリッドで絞り込んだ近傍ポリゴンのみを使う。
    std::vector<const Stage::StageCollisionData*> nearbyTriangles;
    if (Game::GetWaveManager())
    {
        Game::GetWaveManager()->GetCollisionGrid().GetNearbyTriangles(camPos, nearbyTriangles);
    }

    // 照準チェック（レティクル色変更用）: kAimCheckInterval フレームに1回実行
    // 数フレームの遅延は体感不可能
    if (++m_aimCheckSkipCounter >= kAimCheckInterval)
    {
        m_aimCheckSkipCounter = 0;
        m_isAimingAtEnemy = false;

        for (const auto& enemy : nearbyEnemies)
        {
            if (!enemy || !enemy->IsAlive()) continue;

            VECTOR hitPos;
            float hitDistSq;
            EnemyBase::HitPart part = enemy->CheckHitPart(camPos, rayEnd, hitPos, hitDistSq);

            if (part == EnemyBase::HitPart::Body || part == EnemyBase::HitPart::Head)
            {
                if (CheckLineOfSight(camPos, hitPos, nearbyTriangles))
                {
                    m_isAimingAtEnemy = true;
                    break;
                }
            }
        }
    }

    // ロックオン処理（タックル判定）: 毎フレーム実行
    if (isGuarding && tackleCooldown <= 0)
    {
        m_isLockingOn = true;
        m_lockedOnEnemy = nullptr;
        float minScreenDistSq = -1.0f;

        for (EnemyBase* enemy : nearbyEnemies)
        {
            if (!enemy || !enemy->IsAlive()) continue;

            VECTOR diff = VSub(playerPos, enemy->GetPos());
            if (VSquareSize(diff) > kTackleMaxReachSq) continue;

            VECTOR enemyTargetPos = enemy->GetPos();
            enemyTargetPos.y += kEnemyTargetHeightOffset;
            VECTOR toEnemyDir = VNorm(VSub(enemyTargetPos, camPos));

            if (VDot(camDir, toEnemyDir) > kLockOnAngleCos)
            {
                VECTOR screenPos = ConvWorldPosToScreenPos(enemyTargetPos);
                if (screenPos.z > 0)
                {
                    float dx = screenPos.x - (Game::GetScreenWidth()  * 0.5f);
                    float dy = screenPos.y - (Game::GetScreenHeight() * 0.5f);

                    if (fabsf(dy) < kLockOnMaxScreenOffsetY)
                    {
                        if (CheckLineOfSight(camPos, enemyTargetPos, nearbyTriangles))
                        {
                            float distSq = dx * dx + dy * dy;
                            if (minScreenDistSq < 0 || distSq < minScreenDistSq)
                            {
                                minScreenDistSq = distSq;
                                m_lockedOnEnemy = enemy;
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        m_isLockingOn = false;
        m_lockedOnEnemy = nullptr;
    }

    m_isTargetAvailable = (m_lockedOnEnemy != nullptr);
}

bool PlayerLockOnSystem::CheckLineOfSight(const VECTOR& start, const VECTOR& end,
    const std::vector<const Stage::StageCollisionData*>& triangles) const
{
    for (const auto* col : triangles)
    {
        if (!col) continue;
        HITRESULT_LINE result = HitCheck_Line_Triangle(start, end, col->v1, col->v2, col->v3);
        if (result.HitFlag)
        {
            return false;
        }
    }
    return true;
}
