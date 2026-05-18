#pragma once
#include "AnimationManager.h"
#include "EnemyBase.h"
#include "DxLib.h"
#include <memory>
#include <vector>
#include "EnemyState.h"

/// <summary>
/// EnemyBoss 専用の定数（複数の .cpp から参照できるよう .h に定義）
/// </summary>
namespace EnemyBossConstants
{
    // アニメーション名
    constexpr char kWalkAnimName[]            = "Armature|Run";
    constexpr char kCloseAttackAnimName[]     = "Armature|CloseRangeAttack"; // 近接範囲攻撃アニメーション名
    constexpr char kLongRangeAttackAnimName[] = "Armature|LongRangeAttack";  // 遠距離攻撃アニメーション名
    constexpr char kDeadAnimName[]            = "Armature|Death";             // 死亡アニメーション名

    // コライダーサイズ
    constexpr float kBodyColliderRadius = 40.0f;  // 体のカプセルコライダー半径
    constexpr float kBodyColliderHeight = 350.0f; // 体のカプセルコライダー高さ
    constexpr float kHeadRadius         = 25.0f;  // 頭の球コライダー半径
    constexpr float kAttackRangeRadius  = 450.0f; // 近接攻撃範囲の球コライダー半径
    constexpr float kShieldColliderRadius = 300.0f; // シールドの球コライダー半径
    constexpr float kShieldYRatio         = 0.6f;   // シールド位置のY軸比率（ボディ高さに対する割合）
    constexpr float kMinDistSqThreshold   = 0.0001f; // ゼロベクトル判定用の最小距離の二乗閾値

    // 近接攻撃関連
    constexpr int   kAttackCooldownMax          = 60;  // 近接攻撃クールダウン時間（フレーム数）
    constexpr int   kAttackEndDelay             = 30;  // 攻撃後の硬直時間（フレーム数）
    constexpr float kCloseAttackHitStartRatio   = 0.4f; // 近接攻撃ヒット判定の開始タイミング比率
    constexpr float kCloseAttackHitEndRatio     = 0.6f; // 近接攻撃ヒット判定の終了タイミング比率
    constexpr float kCloseAttackEffectRatio     = 0.3f; // 近接攻撃エフェクト再生のタイミング比率
    constexpr int   kAttackEffectTimerDuration  = 100;  // 近接攻撃エフェクトの最大持続フレーム数

    // 遠距離攻撃（ホーミング弾）関連
    constexpr float kLongRangeAttackMinDist     = 400.0f;  // 遠距離攻撃を行う最小プレイヤー距離
    constexpr float kLongRangeAttackMaxDist     = 1000.0f; // 遠距離攻撃を行う最大プレイヤー距離
    constexpr int   kLongRangeAttackCooldownMax = 120;     // 遠距離攻撃クールダウン時間（フレーム数）
    constexpr float kLongRangeAttackShotRatio   = 0.3f;    // 遠距離攻撃における弾発射のタイミング比率
    constexpr float kHomingBulletSpeed          = 6.0f;    // ホーミング弾の速度
    constexpr float kHomingTurnRate             = 0.02f;   // ホーミング弾の旋回性能（フレームあたりのラジアン）
    constexpr float kHomingBulletMaxDist        = 1800.0f; // ホーミング弾の最大飛距離
    constexpr float kHomingBulletDamage         = 20.0f;   // ホーミング弾のダメージ量
    constexpr float kHomingBulletRadius         = 15.0f;   // ホーミング弾の当たり判定半径
    constexpr float kParabolicGravity           = 0.3f;    // 放物線弾の重力加速度

    // スタン関連
    constexpr int kStunDuration = 120; // スタンの総持続時間（フレーム数）

    // パリィ関連
    constexpr float kParryRadiusMultiplier          = 1.5f; // パリィコライダーの半径倍率
    constexpr float kReflectedBulletSpeedMultiplier = 1.5f; // パリィ反射後の弾速倍率
    constexpr float kReflectedBulletDamageMultiplier = 2.0f; // パリィ反射弾のダメージ倍率
    constexpr float kParryTimeScale                 = 0.1f; // パリィ成功時のタイムスケール
    constexpr float kParryTimeScaleDuration         = 1.0f; // パリィタイムスケールの持続時間（秒）

    // 移動・回転関連
    constexpr float kRotateSpeedPerFrame = 0.05f; // フレームあたりの旋回速度（ラジアン）
    constexpr float kWalkAnimSpeed       = 0.9f;  // 歩行アニメーション再生速度

    // シールドエフェクト関連
    constexpr float kShieldEffectDuration = 240.0f; // シールドエフェクトの総再生時間（フレーム数）
    constexpr float kShieldFadeInDuration = 30.0f;  // シールドエフェクトのフェードイン時間（フレーム数）
    constexpr float kShieldRotationSpeed  = 0.3f;   // シールドの1フレームあたりの回転速度（度）

    // ダメージ計算関連
    constexpr float kBossHeadshotMultiplier   = 1.5f; // ヘッドショット時のダメージ倍率
    constexpr float kBossBodyDamageMultiplier = 0.8f; // ボディヒット時のダメージ倍率（やや硬い）
    constexpr int   kBossScoreMultiplier      = 10;   // ボス撃破時のスコア倍率

    // 描画関連
    constexpr float kDrawDistanceSq     = 10000.0f * 10000.0f; // 最大描画距離の二乗
    constexpr float kDrawNearDistanceSq = 600.0f * 600.0f;     // 常に描画する近距離の二乗
    constexpr float kDrawDotThreshold   = 0.0f;                // 視野内判定に使う内積閾値（ボスは常に描画）

    // シールド関連
    constexpr float kShieldMaxHp = 200.0f; // シールドの最大耐久値
}


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

    /// <summary>
    /// モデルをロードする（共有）
    /// </summary>
    static void LoadModel();

    /// <summary>
    /// モデルを解放する（共有）
    /// </summary>
    static void DeleteModel();

    static void SetDrawCollision(bool draw) { s_shouldDrawCollision = draw; }
    static bool ShouldDrawCollision() { return s_shouldDrawCollision; }

    static void SetDrawAttackHit(bool draw) { s_shouldDrawAttackHit = draw; }
    static bool IsDrawAttackHit() { return s_shouldDrawAttackHit; }

    static void SetDrawShieldCollision(bool draw) { s_shouldDrawShieldCollision = draw; }
    static bool IsDrawShieldCollision() { return s_shouldDrawShieldCollision; }

    void Init() override;
    void Update(const EnemyUpdateContext& context) override;
    void Draw() override;

    void OnDeath() override;
    bool IsBoss() const override { return true; }

    /// <summary>
    /// ダメージを受ける処理（シールド破壊ロジックを含む）
    /// </summary>
    /// <param name="damage">受けるダメージ量</param>
    /// <param name="type">攻撃の種類</param>
    void TakeDamage(float damage, AttackType type) override;

    /// <summary>
    /// タックルダメージを受ける処理
    /// </summary>
    /// <param name="damage">受けるダメージ量</param>
    void TakeTackleDamage(float damage) override;

    /// <summary>
    /// パリィされた時に呼び出される
    /// </summary>
    void OnParried();

    /// <summary>
    /// ボディコライダーを取得する
    /// </summary>
    std::shared_ptr<CapsuleCollider> GetBodyCollider() const override;

    void ResetTackleHitFlag() override { m_hasTakenTackleDamage = false; }

    /// <summary>
    /// 弾のRayが当たった部位を判定する（シールド優先）
    /// </summary>
    HitPart CheckHitPart(const VECTOR& rayStart, const VECTOR& rayEnd, VECTOR& outHtPos, float& outHtDistSq) const override;

    /// <summary>
    /// シールドが破壊済みかどうかを返す
    /// </summary>
    bool IsShieldBroken() const { return m_isShieldBroken; }

    /// <summary>
    /// シールドコライダーを取得する
    /// </summary>
    std::shared_ptr<SphereCollider> GetShieldCollider() const { return m_pShieldCollider; }

protected:
    // シールド関連メンバ
    bool m_isShieldBroken;  // シールドが破壊されたか
    bool m_hasPlayedShieldBreakableEffect; // シールド破壊可能エフェクトを再生したか
    float m_shieldHp;       // シールドHP
    std::shared_ptr<SphereCollider> m_pShieldCollider; // シールドコライダー

protected:
    // ダメージ計算
    float CalcDamage(float bulletDamage, HitPart part) const override;
    
    // ダメージ適用（シールドヒット時はエフェクトを変えるためオーバーライド）
    void ApplyBulletDamage(Bullet& bullet, HitPart part, float distSq, Effect* pEffect) override;

    // デバッグ描画
    void DrawCollisionDebug() const override;

private:
    static bool s_shouldDrawCollision;
    static bool s_shouldDrawAttackHit;
    static bool s_shouldDrawShieldCollision;

private:
    /// <summary>
    /// アニメーションを変更する
    /// </summary>
    /// <param name="newAnimState">新しいアニメーション状態</param>
    /// <param name="loop">ループ再生するかどうか</param>
    void ChangeAnimation(AnimState newAnimState, bool loop);

    /// <summary>
    /// ステートを変更する
    /// </summary>
    /// <param name="newState">新しいステートオブジェクト</param>
    void ChangeState(std::shared_ptr<EnemyState<EnemyBoss>> newState);

    /// <summary>
    /// 攻撃可能か判定
    /// </summary>
    bool CanAttackPlayer(const Player& player);

private:
    static int s_modelHandle;

    AnimationManager m_animationManager; // アニメーション管理
    AnimState m_currentAnimState;        // 現在のアニメーション状態
    std::shared_ptr<EnemyState<EnemyBoss>> m_pCurrentState; // 現在のAIステート
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
    bool m_hasAttackHit;        // 攻撃がヒットしたか

    /// <summary>
    /// ホーミング弾の構造体
    /// </summary>
    struct HomingBullet
    {
        VECTOR pos;           // 弾の現在位置
        VECTOR dir;           // 弾の進行方向（正規化済み）
        float  speed;         // 弾の速度
        bool   active;        // アクティブ状態かどうか
        float  damage;        // ダメージ量
        int    effectHandle;  // エフェクトハンドル
        float  distTraveled;  // 累計移動距離（最大飛距離の判定に使用）
        float  turnRate;      // ホーミングの旋回性能（フレームあたりのラジアン）
        bool   isReflected;   // パリィで反射されたか
        bool   isParryable;   // パリィ可能かどうか
        EnemyBase* owner;     // この弾の所有者

        // 放物線弾用
        bool   isParabolic;   // 放物線軌道で飛ぶかどうか
        VECTOR velocity;      // 放物線弾の現在速度ベクトル
        float  gravity;       // 放物線弾の重力加速度

        HomingBullet()
            : pos(VGet(0, 0, 0))
            , dir(VGet(0, 0, 0))
            , speed(0)
            , active(false)
            , damage(0)
            , effectHandle(-1)
            , distTraveled(0)
            , turnRate(EnemyBossConstants::kHomingTurnRate)
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
    bool m_isFirstUpdate;             // 初回更新フラグ

    bool m_shouldDrawParryCollider = false; // パリィコライダーを描画するか
    VECTOR m_debugParryCapA = { 0,0,0 };    // デバッグ用パリィカプセルのA点
    VECTOR m_debugParryCapB = { 0,0,0 };    // デバッグ用パリィカプセルのB点
    float m_debugParryRadius = 0.0f;        // デバッグ用パリィカプセルの半径

    // EnemyBase::UpdateStandard から呼び出される内部処理
    void UpdateAI(const EnemyUpdateContext& context) override;
    void UpdateAnimation(const EnemyUpdateContext& context) override;
    void UpdateDeath(const EnemyUpdateContext& context) override;
    void UpdateSimpleMode(const EnemyUpdateContext& context) override;

    /// <summary>
    /// ホーミング弾の移動・パリィ・ヒット判定（EnemyBossHomingBullet.cpp で実装）
    /// </summary>
    void UpdateHomingBullets(const EnemyUpdateContext& context);

    /// <summary>
    /// シールドエフェクトの生成・回転・色変化（EnemyBossShield.cpp で実装）
    /// </summary>
    void UpdateShieldEffect(const EnemyUpdateContext& context);

    /// <summary>
    /// シールドによるプレイヤー押し出し処理（EnemyBossShield.cpp で実装）
    /// </summary>
    void UpdateShieldPushout();


    std::vector<int> m_shieldEffectHandles; // シールドエフェクトハンドル(複数管理用)
    float m_maxShieldHp = 0.0f;    // シールド最大耐久値
    float m_shieldRotation = 0.0f; // シールドの回転角度
    float m_shieldEffectTimer = 0.0f; // シールドエフェクトの再生タイマー

    // ステートクラスからのアクセスを許可
    friend class EnemyBossStateWalk;
    friend class EnemyBossStateAttack;
    friend class EnemyBossStateLongRange;
    friend class EnemyBossStateStunned;
    friend class EnemyBossStateDead;
};
