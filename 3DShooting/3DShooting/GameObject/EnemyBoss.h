#pragma once
#include "AnimationManager.h"
#include "EnemyBase.h"
#include "DxLib.h"
#include <memory>
#include <vector>

class Bullet;
class Player;
class SphereCollider;
class CapsuleCollider;

/// <summary>
/// ボスクラス
/// </summary>
class EnemyBoss : public EnemyBase
{
public:
    EnemyBoss();
    virtual ~EnemyBoss();

    // モデルロード・アンロード
    static void LoadModel();
    static void DeleteModel();

    static void SetDrawCollision(bool draw) { s_drawCollision = draw; }
    static bool IsDrawCollision() { return s_drawCollision; }

    static void SetDrawAttackHit(bool draw) { s_drawAttackHit = draw; }
    static bool IsDrawAttackHit() { return s_drawAttackHit; }

    void Init() override;
    void Update(const EnemyUpdateContext& context) override;
    void Draw() override;
    bool IsBoss() const override { return true; }

    // ダメージ処理
    void TakeDamage(float damage, AttackType type) override;
    void TakeTackleDamage(float damage) override;

    /// <summary>
    /// パリィされた時に呼び出される
    /// </summary>
    void OnParried();

    // コライダー取得
    std::shared_ptr<CapsuleCollider> GetBodyCollider() const override;

    // タックルヒットフラグのリセット
    void ResetTackleHitFlag() override { m_isTackleHit = false; }

    // 衝突判定
    HitPart CheckHitPart(const VECTOR& rayStart, const VECTOR& rayEnd, VECTOR& outHtPos, float& outHtDistSq) const override;

protected:
    // ダメージ計算
    float CalcDamage(float bulletDamage, HitPart part) const override;

    // デバッグ描画
    void DrawCollisionDebug() const override;

private:
    static bool s_drawCollision;
    static bool s_drawAttackHit;

private:
    /// <summary>
    /// アニメーションを変更する
    /// </summary>
    /// <param name="newAnimState">新しいアニメーション状態</param>
    /// <param name="loop">ループ再生するかどうか</param>
    void ChangeAnimation(AnimState newAnimState, bool loop);

    /// <summary>
    /// 攻撃可能か判定
    /// </summary>
    bool CanAttackPlayer(const Player& player);

private:
    static int s_modelHandle;

    AnimationManager m_animationManager; // アニメーション管理
    AnimState m_currentAnimState;        // 現在のアニメーション状態
    bool m_isDeadAnimPlaying;            // 死亡アニメーション再生中フラグ
    float m_animTime;                    // アニメーションの経過時間

    // フレームインデックスキャッシュ
    int m_headNodeIndex;
    int m_headTopEndNodeIndex;

    std::shared_ptr<CapsuleCollider> m_pBodyCollider;       // 体の当たり判定
    std::shared_ptr<SphereCollider> m_pHeadCollider;        // 頭の当たり判定
    std::shared_ptr<SphereCollider> m_pAttackRangeCollider; // 攻撃範囲
    std::shared_ptr<CapsuleCollider> m_pAttackHitCollider;  // 攻撃判定(腕など)
    std::shared_ptr<SphereCollider> m_pWeakCollider;        // 弱点

    int m_attackEndDelayTimer; // 攻撃後の硬直タイマー
    bool m_isAttackHit;        // 攻撃がヒットしたか

    // 遠距離攻撃用
    struct HomingBullet
    {
        VECTOR pos;
        VECTOR dir;
        float speed;
        bool active;
        float damage;
        int effectHandle;
        float distTraveled; // 移動距離

        // パラメータ
        float turnRate; // 旋回性能

        // パリィ・反射関連
        bool isReflected; // パリィで反射されたか
        bool isParryable; // パリィ可能かどうか
        EnemyBase* owner; // この弾の所有者

        // 放物線用
        bool isParabolic = false;
        VECTOR velocity = { 0, 0, 0 };
        float gravity = 0.0f;

        HomingBullet()
            : pos(VGet(0, 0, 0))
            , dir(VGet(0, 0, 0))
            , speed(0)
            , active(false)
            , damage(0)
            , effectHandle(-1)
            , distTraveled(0)
            , turnRate(0.05f)
            , isReflected(false)
            , isParryable(true)
            , owner(nullptr)
            , isParabolic(false)
            , velocity(VGet(0, 0, 0))
            , gravity(0.0f)
        {
        }
    };

    std::vector<HomingBullet> m_homingBullets;
    int m_longRangeAttackCooldown;
    bool m_hasShotLongRange;

    bool m_isNextAttackNormal; // 次の攻撃が通常弾かどうか

    int m_currentEffectHandle; // 再生中の近接攻撃エフェクトハンドル
    int m_effectTimer;         // エフェクト再生タイマー

    bool m_isStunned; // 怯み状態か
    int m_stunTimer;  // 怯みタイマー
    bool m_hasPlayedCloseRangeEffect; // 近接攻撃エフェクト再生済みか

    bool m_shouldDrawParryCollider = false; // パリィコライダーを描画するか
    VECTOR m_debugParryCapA = { 0,0,0 };    // デバッグ用パリィカプセルのA点
    VECTOR m_debugParryCapB = { 0,0,0 };    // デバッグ用パリィカプセルのB点
    float m_debugParryRadius = 0.0f;        // デバッグ用パリィカプセルの半径

    // リファクタリング用メソッド
    void UpdateDeath(const std::vector<Stage::StageCollisionData>& stageCollision);

    int m_shieldEffectHandle = -1; // シールドエフェクトハンドル
};
