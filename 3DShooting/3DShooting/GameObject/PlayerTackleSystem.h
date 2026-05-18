#pragma once
#include "DxLib.h"
#include <vector>
#include "Stage.h"

class EnemyBase;
class Camera;
class Effect;
class PlayerMovement;
class Player;

/// <summary>
/// プレイヤーのタックルアクションを管理するクラス
/// </summary>
class PlayerTackleSystem
{
public:
    PlayerTackleSystem();
    ~PlayerTackleSystem() = default;

    /// <summary>
    /// 初期化処理
    /// </summary>
    /// <param name="cooldownMax">クールダウンの最大値</param>
    /// <param name="speed">タックルの移動速度</param>
    /// <param name="damage">タックルのダメージ量</param>
    void Init(float cooldownMax, float speed, float damage);

    /// <summary>
    /// 毎フレームの更新処理
    /// </summary>
    /// <param name="deltaTime">経過時間</param>
    /// <param name="isLockingOn">ロックオン中かどうか</param>
    /// <param name="lockedOnEnemy">ロックオン中の敵ポインタ</param>
    /// <param name="playerPos">プレイヤー位置（参照渡し）</param>
    /// <param name="movement">プレイヤー移動コンポーネント</param>
    /// <param name="pCamera">カメラポインタ</param>
    /// <param name="pEffect">エフェクトポインタ</param>
    /// <param name="enemyList">敵リスト</param>
    /// <param name="collisionData">ステージコリジョンデータ</param>
    /// <param name="pPlayer">プレイヤーポインタ</param>
    void Update(float deltaTime, bool isLockingOn, EnemyBase* lockedOnEnemy,
                VECTOR& playerPos, PlayerMovement& movement, Camera* pCamera,
                Effect* pEffect, const std::vector<EnemyBase*>& enemyList,
                const std::vector<Stage::StageCollisionData>& collisionData,
                Player* pPlayer);

    /// <summary>
    /// クールダウンのみ更新（制限中用）
    /// </summary>
    void UpdateCooldownOnly(float deltaTime);

    /// <summary>
    /// タックル中かどうかを返す
    /// </summary>
    /// <returns>タックル中ならtrue</returns>
    bool IsTackling() const { return m_isTackling; }

    /// <summary>
    /// クールダウンタイマーを取得する
    /// </summary>
    float GetCooldown() const { return m_cooldownTimer; }

    /// <summary>
    /// タックルIDを取得する
    /// </summary>
    int GetTackleId() const { return m_tackleId; }

    /// <summary>
    /// タックルのダメージ量を取得する
    /// </summary>
    float GetDamage() const { return m_damage; }

    /// <summary>
    /// タックルの移動方向を取得する
    /// </summary>
    VECTOR GetDir() const { return m_tackleDir; }

    /// <summary>
    /// クールダウンをリセットする
    /// </summary>
    void ResetCooldown() { m_cooldownTimer = 0.0f; }

private:
    bool  m_isTackling;                    // タックル中フラグ
    float m_cooldownTimer;                 // 現在のクールダウンタイマー
    float m_cooldownMax;                   // クールダウンの最大値
    float m_speed;                         // タックルの移動速度
    float m_damage;                        // タックルのダメージ量
    float m_tackleFrame;                   // タックル残りフレーム数
    int   m_tackleId;                      // タックルの識別ID（敵ヒット重複防止用）
    int   m_concentrationLineEffectHandle; // 集中線エフェクトハンドル
    int   m_hitSECooldownTimer;            // ヒットSEのクールダウンタイマー（フレーム数）
    VECTOR m_tackleDir;                    // タックルの移動方向

    static constexpr int   kTackleDuration                 = 35;     // タックルの持続フレーム数
    static constexpr float kTackleStopMargin               = 45.0f;  // 敵との当たり判定マージン
    static constexpr float kTackleFov                      = 100.0f; // タックル中のFOV（度）
    static constexpr float kTackleCameraZOffset            = 30.0f;  // タックル中のカメラZオフセット
    static constexpr float kConcentrationLineEffectZOffset = 15.0f;  // 集中線エフェクトのカメラ前方オフセット
};
