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

    /// <summary>
    /// 毎フレームの更新処理
    /// </summary>
    /// <param name="playerPos">プレイヤーの位置</param>
    /// <param name="pCamera">カメラのポインタ</param>
    /// <param name="enemyList">敵リスト</param>
    /// <param name="collisionData">ステージコリジョンデータ</param>
    /// <param name="isGuarding">ガード中かどうか</param>
    /// <param name="tackleCooldown">タックルのクールダウン残量</param>
    void Update(const VECTOR& playerPos, Camera* pCamera,
        const std::vector<EnemyBase*>& enemyList,
        const std::vector<Stage::StageCollisionData>& collisionData,
        bool isGuarding, float tackleCooldown);

    /// <summary>
    /// ロックオン中かどうかを返す
    /// </summary>
    bool IsLockingOn() const { return m_isLockingOn; }

    /// <summary>
    /// ロックオン中の敵を返す
    /// </summary>
    EnemyBase* GetLockedOnEnemy() const { return m_lockedOnEnemy; }

    /// <summary>
    /// タックル可能なターゲットが存在するかどうかを返す
    /// </summary>
    bool IsTargetAvailable() const { return m_isTargetAvailable; }

    /// <summary>
    /// 敵に照準を合わせているかどうかを返す
    /// </summary>
    bool IsAimingAtEnemy() const { return m_isAimingAtEnemy; }

private:
    /// <summary>
    /// 視線がステージに遮られていないか確認する
    /// </summary>
    bool CheckLineOfSight(const VECTOR& start, const VECTOR& end,
        const std::vector<const Stage::StageCollisionData*>& triangles) const;

private:
    bool       m_isLockingOn;         // ロックオン中かどうか
    bool       m_isTargetAvailable;   // タックル可能なターゲットが存在するか
    bool       m_isAimingAtEnemy;     // 敵に照準を合わせているか
    EnemyBase* m_lockedOnEnemy;       // ロックオン中の敵
    int        m_aimCheckSkipCounter; // 照準チェックのフレームスキップ用カウンタ

    static constexpr float kLockOnAngleCos         = 0.966f;              // ロックオン判定の視野角（cos(15度)）
    static constexpr float kLockOnMaxScreenOffsetY  = 100.0f;             // 画面中央からの垂直方向の最大オフセット
    static constexpr float kTackleMaxReachSq        = 1800.0f * 1800.0f;  // タックルの最大距離の二乗
    static constexpr int   kAimCheckInterval        = 2;                  // 照準チェックを N フレームに1回だけ実行
    static constexpr int   kMaxAimTargets           = 10;                 // 照準・ロックオン判定の対象敵数上限
};
