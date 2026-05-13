#include "PlayerLockOnSystem.h"
#include "EnemyBase.h"
#include "Camera.h"
#include "Game.h"
#include "WaveManager.h"
#include "CollisionGrid.h"
#include "Collision.h"
#include <algorithm>

PlayerLockOnSystem::PlayerLockOnSystem()
    : m_isLockingOn(false)
    , m_isTargetAvailable(false)
    , m_isAimingAtEnemy(false)
    , m_lockedOnEnemy(nullptr)
    , m_aimCheckSkipCounter(0)
{
}

void PlayerLockOnSystem::Update(const VECTOR& playerPos, Camera* pCamera, const std::vector<EnemyBase*>& enemyList, const std::vector<Stage::StageCollisionData>& collisionData, bool isGuarding, float tackleCooldown)
{
    if (!pCamera) return;

    VECTOR camPos = pCamera->GetPos();
    VECTOR camDir = VNorm(VSub(pCamera->GetTarget(), camPos));
    VECTOR rayEnd = VAdd(camPos, VScale(camDir, 2000.0f));

    m_isTargetAvailable = false;

    // ── 近傍の敵を取得（グリッド利用）────────────────────────────
    std::vector<EnemyBase*> nearbyEnemies;
    if (Game::m_pWaveManager)
    {
        Game::m_pWaveManager->GetCollisionGrid().GetNeighbors(playerPos,                             nearbyEnemies, false);
        Game::m_pWaveManager->GetCollisionGrid().GetNeighbors(VAdd(playerPos, VScale(camDir, 500.0f)), nearbyEnemies, false);
    }

    // 重複除去（2回の GetNeighbors で同一敵が混入する場合がある）
    std::sort(nearbyEnemies.begin(), nearbyEnemies.end());
    nearbyEnemies.erase(std::unique(nearbyEnemies.begin(), nearbyEnemies.end()), nearbyEnemies.end());

    // フォールバック：グリッド外にいる場合のみ全敵リストを使用
    std::vector<EnemyBase*>& targetList = nearbyEnemies.empty() ?
        const_cast<std::vector<EnemyBase*>&>(enemyList) : nearbyEnemies;

    // プレイヤーからの距離でソートし、上位 kMaxAimTargets 体に絞る
    // （密集時でもレイキャスト回数を上限に固定する）
    std::sort(targetList.begin(), targetList.end(), [&playerPos](const EnemyBase* a, const EnemyBase* b)
    {
        float daSq = VSquareSize(VSub(a->GetPos(), playerPos));
        float dbSq = VSquareSize(VSub(b->GetPos(), playerPos));
        return daSq < dbSq;
    });
    if ((int)targetList.size() > kMaxAimTargets)
    {
        targetList.resize(kMaxAimTargets);
    }
    // ── 近傍ステージ三角形を1回だけ取得してキャッシュ ────────────
    // CheckLineOfSight で毎回 collisionData 全件を走査する代わりに、
    // グリッドで絞り込んだ近傍ポリゴンのみを使う。
    // sort+unique は GetNearbyTriangles 内で1回だけ実行される。
    std::vector<const Stage::StageCollisionData*> nearbyTriangles;
    if (Game::m_pWaveManager)
    {
        Game::m_pWaveManager->GetCollisionGrid().GetNearbyTriangles(camPos, nearbyTriangles);
    }
    // レティクル色変更のみに影響。33ms程度の遅延は体感不可能。
    if (++m_aimCheckSkipCounter >= kAimCheckInterval)
    {
        m_aimCheckSkipCounter = 0;
        m_isAimingAtEnemy = false;

        for (const auto& enemy : targetList)
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

    // ── ロックオン処理（タックル判定）：毎フレーム実行 ───────────
    if (isGuarding && tackleCooldown <= 0)
    {
        m_isLockingOn = true;
        m_lockedOnEnemy = nullptr;
        float minScreenDistSq = -1.0f;

        for (EnemyBase* enemy : targetList)
        {
            if (!enemy || !enemy->IsAlive()) continue;

            VECTOR diff = VSub(playerPos, enemy->GetPos());
            if (VSquareSize(diff) > kTackleMaxReachSq) continue;

            VECTOR enemyTargetPos = enemy->GetPos();
            enemyTargetPos.y += 70.0f;
            VECTOR toEnemyDir = VNorm(VSub(enemyTargetPos, camPos));

            if (VDot(camDir, toEnemyDir) > kLockOnAngleCos)
            {
                VECTOR screenPos = ConvWorldPosToScreenPos(enemyTargetPos);
                if (screenPos.z > 0)
                {
                    float dx = screenPos.x - (Game::GetScreenWidth() * 0.5f);
                    float dy = screenPos.y - (Game::GetScreenHeight() * 0.5f);

                    if (fabs(dy) < kLockOnMaxScreenOffsetY)
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
    // ── ────────────────────────────────────────────────────────

    m_isTargetAvailable = (m_lockedOnEnemy != nullptr);
}

bool PlayerLockOnSystem::CheckLineOfSight(const VECTOR& start, const VECTOR& end, const std::vector<const Stage::StageCollisionData*>& triangles) const
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
