#pragma once
#include "DxLib.h"
#include <vector>
#include "Stage.h"

class EnemyBase;
class Camera;

/// <summary>
/// プレイヤーのロックオンシステムを管理するクラス
/// </summary>
class PlayerLockOnSystem
{
public:
    PlayerLockOnSystem();
    ~PlayerLockOnSystem() = default;

    void Update(const VECTOR& playerPos, Camera* pCamera, const std::vector<EnemyBase*>& enemyList, const std::vector<Stage::StageCollisionData>& collisionData, bool isGuarding, float tackleCooldown);

    // Getters
    bool IsLockingOn() const { return m_isLockingOn; }
    EnemyBase* GetLockedOnEnemy() const { return m_lockedOnEnemy; }
    bool IsTargetAvailable() const { return m_isTargetAvailable; }
    bool IsAimingAtEnemy() const { return m_isAimingAtEnemy; }

private:
    bool CheckLineOfSight(const VECTOR& start, const VECTOR& end, const std::vector<Stage::StageCollisionData>& collisionData) const;

private:
    bool m_isLockingOn;
    bool m_isTargetAvailable;
    bool m_isAimingAtEnemy;
    EnemyBase* m_lockedOnEnemy;
    int  m_aimCheckSkipCounter;  // 照準チェックのフレームスキップ用カウンタ

    static constexpr float kLockOnAngleCos = 0.966f; // cos(15度)
    static constexpr float kLockOnMaxScreenOffsetY = 100.0f;
    static constexpr float kTackleMaxReachSq = 1800.0f * 1800.0f;
    static constexpr int   kAimCheckInterval  = 2;   // 照準チェックを N フレームに1回だけ実行
    static constexpr int   kMaxAimTargets     = 10;  // 照準・ロックオン判定の対象敵数上限
};
