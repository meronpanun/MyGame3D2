#pragma once
#include "AnimationManager.h"
#include "EnemyBase.h"
#include "Effect.h"
#include "EnemySharedConstants.h"
#include <memory>
#include "EnemyState.h"

class Bullet;
class Player;
class Collider;
class SphereCollider;
class CapsuleCollider;

namespace EnemyNormalConstants
{
    // EnemyRunner と共通の定数は EnemySharedConstants から取り込む
    using EnemySharedConstants::kHeadShotPositionOffset;
    using EnemySharedConstants::kBodyColliderRadius;
    using EnemySharedConstants::kBodyColliderHeight;
    using EnemySharedConstants::kRotateSpeedPerFrame;
    using EnemySharedConstants::kWanderTimerInterval;
    using EnemySharedConstants::kWanderMinDist;
    using EnemySharedConstants::kWanderDistRange;
    using EnemySharedConstants::kPushBackEpsilon;
    using EnemySharedConstants::kDrawDistanceSq;
    using EnemySharedConstants::kDrawNearDistanceSq;
    using EnemySharedConstants::kDrawDotThreshold;

    // アニメーション名
    constexpr char kAttackAnimName[] = "ATK";  // 攻撃アニメーション名
    constexpr char kWalkAnimName[]   = "WALK"; // 歩行アニメーション名
    constexpr char kDeadAnimName[]   = "DEAD"; // 死亡アニメーション名

    // コライダーサイズ（EnemyNormal 固有）
    constexpr float kHeadRadius = 12.0f; // 頭の球コライダー半径

    // 近接攻撃関連
    constexpr int   kAttackCooldownMax   = 45;    // 攻撃クールダウン時間（フレーム数）
    constexpr float kAttackHitRadius     = 45.0f; // 攻撃の当たり判定半径
    constexpr float kAttackRangeRadius   = 120.0f; // 攻撃範囲の半径
    constexpr int   kAttackEndDelay      = 20;    // 攻撃後の硬直時間（フレーム数）
    constexpr float kAttackHitStartRatio = 0.5f;  // 攻撃ヒット判定の開始タイミング比率
    constexpr float kAttackHitEndRatio   = 0.7f;  // 攻撃ヒット判定の終了タイミング比率

    // 追跡・移動関連（EnemyNormal 固有）
    constexpr float kChaseStopDistance = 50.0f; // 追跡停止距離
    constexpr int   kTargetOffsetRange = 100;   // 追跡目標へのランダムオフセット範囲（±この値）

    // ダメージ・ノックバック関連
    constexpr int   kDamageDuration          = 30;    // ダメージ（怯み）の持続時間（フレーム数）
    constexpr int   kShortDamageDuration     = 15;    // 短い怯み時間（アサルトライフル用、フレーム数）
    constexpr float kKnockbackInitialSpeed   = 15.0f; // 死亡吹き飛びの初速度
    constexpr float kKnockbackDeceleration   = 0.5f;  // 死亡吹き飛びの減速度（毎フレーム）
    constexpr float kDamageKnockbackSpeed    = 2.0f;  // 怯み時のノックバック速度
    constexpr int   kDamageKnockbackMinTimer = 10;    // ノックバックを適用する怯みタイマーの閾値

    // ダメージ計算関連
    constexpr float kHeadshotMultiplier = 2.0f; // ヘッドショット時のダメージ倍率

    // 当たり判定関連（EnemyNormal 固有）
    constexpr float kBroadPhaseDistSq = 300.0f * 300.0f; // 弾の当たり判定ブロードフェーズ閾値

    // 環境ボイス関連
    constexpr int   kVoiceTimerMin  = 180;     // 環境ボイス再生間隔の最小値（フレーム数）
    constexpr int   kVoiceTimerRand = 420;     // 環境ボイス再生間隔のランダム幅（フレーム数）
    constexpr float kVoiceMaxDist   = 2000.0f; // 環境ボイスが聞こえる最大距離（最大音量はSoundList.csvのVolumeから取得）

    // シールド関連
    constexpr float kShieldMaxHp          = 50.0f;  // シールドの最大耐久値
    constexpr float kShieldColliderRadius = 80.0f;  // シールドコライダーの半径
    constexpr float kShieldYRatio         = 0.5f;   // シールド位置のY軸比率（ボディ高さに対する割合）
    constexpr float kShieldRotationSpeed  = 0.3f;   // シールドの1フレームあたりの回転速度（度）
    constexpr float kShieldEffectDuration = 240.0f; // シールドエフェクトの総再生時間（フレーム数）
    constexpr float kShieldFadeInDuration = 30.0f;  // シールドエフェクトのフェードイン時間（フレーム数）
    constexpr float kShieldEffectScale    = 0.3f;   // シールドエフェクトの表示スケール
    constexpr float kChainBreakRadius     = 400.0f; // 連鎖破壊の影響半径
    constexpr int   kChainBreakDelay      = 12;     // 連鎖破壊の遅延時間（フレーム数）

    // シールド破壊時のカメラシェイク関連
    constexpr float kCameraShakeMaxDist   = 1000.0f; // カメラシェイクの影響が届く最大距離
    constexpr float kCameraShakeIntensity = 20.0f;   // カメラシェイクの基本強度
    constexpr int   kCameraShakeDuration  = 10;      // カメラシェイクの持続フレーム数
    constexpr float kCameraShakeMinRatio  = 0.2f;    // カメラシェイク強度の最小割合
}

/// <summary>
/// 通常の敵クラス
/// </summary>
class EnemyNormal : public EnemyBase
{
public:
    EnemyNormal();
    virtual ~EnemyNormal();

    void Init() override;
    void Update(const EnemyUpdateContext& context) override;
    void Draw() override;

    /// <summary>
    /// デバッグ用の当たり判定を描画する
    /// </summary>
    virtual void DrawCollisionDebug() const override;

    /// <summary>
    /// 死亡時に呼ばれる処理
    /// </summary>
    void OnDeath() override;

    /// <summary>
    /// モデルの読み込み(共有)
    /// </summary>
    static void LoadModel();

    /// <summary>
    /// ボディコライダーを取得する
    /// </summary>
    /// <returns>ボディコライダー</returns>
    std::shared_ptr<CapsuleCollider> GetBodyCollider() const override;

    /// <summary>
    /// モデルの解放(共有)
    /// </summary>
    static void DeleteModel();

    static void SetDrawCollision(bool draw) { s_shouldDrawCollision = draw; }
    static bool ShouldDrawCollision() { return s_shouldDrawCollision; }

    static void SetDrawShieldCollision(bool draw) { s_shouldDrawShieldCollision = draw; }
    static bool IsDrawShieldCollision() { return s_shouldDrawShieldCollision; }

private:
    static bool s_shouldDrawCollision;
    static bool s_shouldDrawShieldCollision;

    /// <summary>
    /// どこに当たったかを判定する
    /// </summary>
    /// <param name="rayStart">弾のRayの始点</param>
    /// <param name="rayEnd">弾のRayの終点</param>
    /// <returns>当たった部位</returns>
    HitPart CheckHitPart(const VECTOR& rayStart, const VECTOR& rayEnd,
                         VECTOR& outHtPos, float& outHtDistSq) const override;

    void ResetTackleHitFlag() override { m_hasTakenTackleDamage = false; }

    /// <summary>
    /// アイテムドロップ時のコールバック関数を設定する
    /// </summary>
    /// <param name="cb">コールバック関数</param>
    void SetOnDropItemCallback(std::function<void(const VECTOR&)> cb);

    /// <summary>
    /// ダメージを受ける処理
    /// </summary>
    /// <param name="damage">受けるダメージ量</param>
    void TakeDamage(float damage, AttackType type) override;

    /// <summary>
    /// タックルダメージを受ける処理
    /// </summary>
    /// <param name="damage">受けるダメージ量</param>
    void TakeTackleDamage(float damage) override;


public:

    /// <summary>
    /// シールドを所持するかどうかを設定し、初期化する
    /// </summary>
    void SetHasShield(bool hasShield);
    void TriggerShieldChainBreak(int delayFrames);

    /// <summary>
    /// シールド状態取得
    /// </summary>
    bool IsShieldBroken() const { return m_isShieldBroken; }
    std::shared_ptr<SphereCollider> GetShieldCollider() const { return m_pShieldCollider; }

    // === State 向けインターフェース ===
    // ステートクラスから利用される限定的な操作群。
    // ステートはこれらの公開メソッドのみを通じて EnemyNormal を操作し、
    // 直接 private メンバへ触れないようにする（friend を不要にするための窓口）。

    /// <summary>
    /// アニメーションを変更する
    /// </summary>
    void ChangeAnimation(AnimState newAnimState, bool loop);

    /// <summary>
    /// ステートを変更する
    /// </summary>
    void ChangeState(std::shared_ptr<EnemyState<EnemyNormal>> newState);

    /// <summary>
    /// 今フレームに AI 更新を行うべきかどうか
    /// </summary>
    bool ShouldUpdateAI() const { return m_shouldUpdateAI; }

    /// <summary>
    /// プレイヤーに攻撃可能かどうか（攻撃ヒットコライダー判定）
    /// </summary>
    bool CanAttackPlayer(const Player& player);

    /// <summary>
    /// プレイヤーが攻撃範囲（範囲コライダー）内にいるかどうか
    /// </summary>
    bool IsPlayerInAttackRange(const Player& player) const;

    /// <summary>
    /// 攻撃ヒットフラグをリセット
    /// </summary>
    void ResetAttackHitFlag() { m_hasAttackHit = false; }

    /// <summary>
    /// 追跡・徘徊・回転・移動をまとめて行う（Walk ステート用）
    /// </summary>
    void UpdateChaseBehavior(const Player& player);

    /// <summary>
    /// 攻撃アニメーション中のヒット判定とダメージ適用を行う（Attack ステート用）
    /// </summary>
    void UpdateAttackHitDetection(Player& player);

    /// <summary>
    /// 攻撃アニメーションが終わったか
    /// </summary>
    bool IsAttackAnimationFinished() const;

    /// <summary>
    /// 攻撃後ディレイタイマーを開始する（まだ動作中でなければセット）
    /// </summary>
    void StartAttackEndDelay();

    /// <summary>
    /// 攻撃後ディレイタイマーを進める。タイマーが今フレームで終了した場合 true を返す
    /// </summary>
    bool TickAttackEndDelay();

    /// <summary>
    /// ダメージタイマーを進める。タイマーが今フレームで終了した場合 true を返す
    /// </summary>
    bool TickDamageTimer();

    /// <summary>
    /// 怯み中のノックバック移動を適用する
    /// </summary>
    void ApplyDamageKnockback(const Player& player);

protected:
    // ダメージ計算と適用
    void CheckHitAndDamage(std::vector<Bullet>& bullets, Effect* pEffect) override;
    float CalcDamage(float bulletDamage, HitPart part) const override;
    void ApplyBulletDamage(Bullet& bullet, HitPart part, float distSq, Effect* pEffect) override;

private:
    void BreakShield(const EnemyUpdateContext* context = nullptr);

    // EnemyBase::UpdateStandard から呼び出される内部処理
    void UpdateAI(const EnemyUpdateContext& context) override;
    void UpdateAnimation(const EnemyUpdateContext& context) override;
    void UpdateDeath(const EnemyUpdateContext& context) override;
    void UpdateSimpleMode(const EnemyUpdateContext& context) override;

    /// <summary>
    /// シールドの更新処理（EnemyNormalShield.cpp で実装）
    /// </summary>
    void UpdateShield(const EnemyUpdateContext& context);

private:
    VECTOR m_headPosOffset; // ヘッドショット判定用オフセット座標

    std::shared_ptr<CapsuleCollider> m_pBodyCollider;      // 体のコライダー
    std::shared_ptr<SphereCollider> m_pHeadCollider;       // 頭のコライダー
    std::shared_ptr<SphereCollider> m_pAttackRangeCollider; // 攻撃範囲のコライダー
    std::shared_ptr<CapsuleCollider> m_pAttackHitCollider;  // 攻撃ヒット判定用のコライダー

    // アイテムドロップ時のコールバック関数
    std::function<void(const VECTOR&)> m_onDropItem;

    AnimState m_currentAnimState; // 現在のアニメーション状態
    std::shared_ptr<EnemyState<EnemyNormal>> m_pCurrentState; // 現在のAIステート

    AnimationManager m_animationManager; // EnemyNormalがアニメーションマネージャーを所有

    int m_attackEndDelayTimer; // 攻撃終了までの遅延タイマー
    int m_damageTimer;         // ダメージ（怯み）タイマー

    float m_animTime;   // アニメーションの経過時間

    bool m_hasTakenTackleDamage;       // 1フレームで複数回ダメージを受けないためのフラグ
    bool m_hasAttackHit;       // 攻撃がヒットしたかどうか
    bool m_isDeadAnimPlaying; // 死亡アニメーション再生中フラグ
    bool m_hasDroppedItem;     // アイテムドロップ済みフラグ

    // 徘徊挙動用
    int m_wanderTimer;     // 徘徊位置更新タイマー
    VECTOR m_wanderOffset; // 徘徊位置オフセット

    // 死亡時吹き飛び用
    bool m_isBlownAway;          // 吹き飛ばされて死亡したか
    VECTOR m_deathKnockbackDir;  // 吹き飛び方向
    float m_deathKnockbackSpeed; // 吹き飛び速度

    // シールド関連メンバ
    bool m_hasShieldConfigured;    // シールドを持っているか
    bool m_isShieldBroken;         // シールドが破壊されたか
    float m_shieldHp;              // シールドHP
    float m_maxShieldHp;           // シールド最大耐久値
    std::shared_ptr<SphereCollider> m_pShieldCollider; // シールドコライダー
    std::vector<int> m_shieldEffectHandles;            // シールドエフェクトハンドル
    float m_shieldRotation;        // シールドの回転角度
    float m_shieldEffectTimer;     // シールドエフェクトの再生タイマー
    bool m_hasPlayedShieldBreakableEffect; // シールド破壊可能エフェクト再生済みフラグ
    int m_shieldChainBreakTimer;   // 連鎖破壊タイマー

    int m_voiceTimer;               // 環境ボイス再生用タイマー
    float m_distToPlayer;           // プレイヤーとの距離
    static int s_modelHandle; // 共有モデルハンドル
};
