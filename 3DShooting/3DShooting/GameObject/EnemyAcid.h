#pragma once
#include "AnimationManager.h"
#include "EnemyBase.h"
#include "Stage.h"
#include <functional>
#include <memory>
#include <vector>
#include "EnemyState.h"

class Player;
class Bullet;
class Effect;
class CapsuleCollider;
class SphereCollider;
class Collider;

namespace EnemyAcidConstants
{
    // アニメーション関連
    constexpr char kAttackAnimName[] = "Armature|ATK"; // 攻撃アニメーション
    constexpr char kWalkAnimName[]   = "Armature|WALK"; // 歩くアニメーション
    constexpr char kBackAnimName[]   = "Armature|BACK"; // 後退アニメーション
    constexpr char kDeadAnimName[]   = "Armature|DEAD"; // 死亡アニメーション

    inline const VECTOR kHeadShotPositionOffset = {0.0f, 0.0f, 0.0f}; // ヘッドショット判定コライダーの中心補正値

    // コライダーサイズ
    constexpr float kBodyColliderRadius = 40.0f; // 体のコライダー半径
    constexpr float kBodyColliderHeight = 50.0f; // 体のコライダー高さ
    constexpr float kHeadRadius         = 18.0f; // 頭のコライダー半径

    // 攻撃関連（遠距離攻撃に特化）
    constexpr int   kAttackCooldownMax    = 160;     // 攻撃クールダウン時間（フレーム数）
    constexpr float kAttackRangeRadius    = 1500.0f; // 攻撃範囲の半径
    constexpr float kAcidBulletSpeed      = 5.0f;    // 酸弾の速度
    constexpr float kAcidBallRadius       = 12.0f;   // 酸弾の当たり判定半径
    constexpr float kParabolicGravity     = 0.3f;    // 放物線弾の重力加速度
    constexpr float kAttackTimingRatio    = 0.3f;    // 弾を発射する攻撃アニメーションの進行比率
    constexpr int   kAttackEndDelayFrames = 20;      // 攻撃後の行動再開までの硬直フレーム数

    // 追跡関連
    constexpr float kOptimalAttackDistanceMin = 500.0f; // 後退を開始するプレイヤーとの最小距離
    constexpr float kRotateSpeedPerFrame      = 0.05f;  // フレームあたりの旋回速度（ラジアン）
    constexpr int   kTargetOffsetRange        = 400;    // ターゲットオフセットの範囲（±この値でランダム）

    // スタン関連
    constexpr int   kStunDuration       = 120;  // スタンの総持続時間（フレーム数）
    constexpr float kStunAnimFrameLimit = 60.0f; // スタンアニメーションの再生上限フレーム

    // パリィ関連
    constexpr float kParryNotifyDistance         = 100.0f; // チュートリアルにパリィ通知を送るプレイヤーからの距離
    constexpr float kParryRadiusMultiplier        = 1.5f;  // パリィコライダーの半径倍率
    constexpr float kReflectedBallSpeedMultiplier = 1.5f;  // パリィ反射後の弾速倍率
    constexpr float kParryBodyCenterOffsetY       = 50.0f; // 反射先として使う敵体の中心Y補正値
    constexpr float kParryTimeScale               = 0.1f;  // パリィ成功時のタイムスケール
    constexpr float kParryTimeScaleDuration       = 1.0f;  // パリィタイムスケールの持続時間（秒）

    // 酸弾の消滅判定
    constexpr float kAcidBallBoundaryDistance = 1500.0f; // 酸弾がプレイヤーから離れすぎたら非アクティブにする距離

    // 視錐台カリング用描画距離
    constexpr float kDrawDistanceSq     = 5000.0f * 5000.0f; // 最大描画距離の二乗
    constexpr float kDrawNearDistanceSq = 300.0f * 300.0f;   // 常に描画する近距離の二乗
    constexpr float kDrawDotThreshold   = 0.4f;              // 視野内判定に使う内積閾値

    // 押し出し
    constexpr float kPushBackEpsilon = 0.0001f; // コライダー押し出し時のゼロ除算防止閾値

    // デバッグ描画用パラメータ
    constexpr int kDebugSphereDiv        = 16;       // 球の分割数
    constexpr int kDebugBodyColor        = 0xff00ff; // 体のコライダーの色（マゼンタ）
    constexpr int kDebugHeadColor        = 0xffff00; // 頭のコライダーの色（黄色）
    constexpr int kDebugAttackRangeColor = 0x00ffff; // 攻撃範囲コライダーの色（シアン）
    constexpr int kDebugParryColor       = 0x0000ff; // パリィコライダーの色（青）
}

/// <summary>
/// 遠距離型ゾンビクラス
/// </summary>
class EnemyAcid : public EnemyBase
{
public:
    EnemyAcid();
    virtual ~EnemyAcid();

    void Init() override;
    void Update(const EnemyUpdateContext& context) override;
    void Draw() override;

    /// <summary>
    /// モデルの読み込み（共有）
    /// </summary>
    static void LoadModel();

    /// <summary>
    /// モデルの解放(共有)
    /// </summary>
    static void DeleteModel();

    std::shared_ptr<CapsuleCollider> GetBodyCollider() const override;

    /// <summary>
    /// デバッグ用の当たり判定を描画する
    /// </summary>
    virtual void DrawCollisionDebug() const override;

    static void SetDrawCollision(bool draw) { s_shouldDrawCollision = draw; }
    static bool ShouldDrawCollision() { return s_shouldDrawCollision; }

private:

    /// <summary>
    /// どこに当たったかを判定する
    /// </summary>
    /// <param name="rayStart">弾のRayの始点</param>
    /// <param name="rayEnd">弾のRayの終点</param>
    /// <returns>当たった部位</returns>
    HitPart CheckHitPart(const VECTOR& rayStart, const VECTOR& rayEnd, VECTOR& outHtPos, float& outHtDistSq) const override;

    /// <summary>
    /// タックルダメージを受けたかどうかのフラグをリセットする
    /// </summary>
    void ResetTackleHitFlag() override { m_hasTakenTackleDamage = false; }

    /// <summary>
    /// アイテムドロップ時のコールバック関数を設定する
    /// </summary>
    /// <param name="cb">コールバック関数</param>
    void SetOnDropItemCallback(std::function<void(const VECTOR&)> cb);

    /// <summary>
    /// モデルハンドルを取得する
    /// </summary>
    /// <returns>モデルハンドル</returns>
    int GetModelHandle() const { return m_modelHandle; }

    /// <summary>
    /// ダメージを受ける処理
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
    /// 敵が死亡した際に呼ばれる処理
    /// </summary>
    void OnDeath() override;

    /// <summary>
    /// 酸の弾構造体
    /// </summary>
    struct AcidBall
    {
        VECTOR pos;          // 弾の位置
        VECTOR dir;          // 弾の進行方向

        bool  active       = false;                               // アクティブ状態かどうか
        float radius       = EnemyAcidConstants::kAcidBallRadius; // 当たり判定半径
        float damage       = 0.0f;                                // ダメージ量
        float speed        = EnemyAcidConstants::kAcidBulletSpeed; // 弾の速度
        int   effectHandle = -1;                                  // エフェクトハンドル
        bool isReflected   = false;                               // パリィで反射されたか
        bool isParryable   = true;                                // パリィ可能かどうか
        EnemyBase* owner   = nullptr;                             // この弾の所有者

        // 放物線用
        bool  isParabolic = false;
        VECTOR velocity   = { 0, 0, 0 };
        float gravity     = 0.0f;

        AcidBall() : pos(VGet(0,0,0)), dir(VGet(0,0,1)), active(false), radius(EnemyAcidConstants::kAcidBallRadius), damage(0.0f), speed(EnemyAcidConstants::kAcidBulletSpeed), effectHandle(-1), isReflected(false), isParryable(true), owner(nullptr), isParabolic(false), velocity(VGet(0,0,0)), gravity(0.0f) {}

        void Update(float timeScale);
    };

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
    void ChangeState(std::shared_ptr<EnemyState<EnemyAcid>> newState);

    /// <summary>
    /// プレイヤーに攻撃可能かどうかを判定する
    /// </summary>
    /// <param name="player">プレイヤーオブジェクト</param>
    /// <returns>攻撃可能ならtrue</returns>
    bool CanAttackPlayer(const Player& player);

    /// <summary>
    /// 酸を吐く攻撃を行う
    /// </summary>
    /// <param name="bullets">弾のリスト</param>
    /// <param name="player">プレイヤーオブジェクト</param>
    /// <param name="pEffect">エフェクトオブジェクト</param>
    void ShootAcidBullet(std::vector<Bullet>& bullets, const Player& player, Effect* pEffect, const std::vector<Stage::StageCollisionData>& stageCollision, const class CollisionGrid* pGrid = nullptr);

    // EnemyBase::UpdateStandard から呼び出される内部処理
    void UpdateAI(const EnemyUpdateContext& context) override;
    void UpdateAnimation(const EnemyUpdateContext& context) override;
    void UpdateDeath(const EnemyUpdateContext& context) override;
    void UpdateSimpleMode(const EnemyUpdateContext& context) override;

    /// <summary>
    /// 酸弾の更新・パリィ・ヒット判定（EnemyAcidState.cpp で実装）
    /// </summary>
    void UpdateAcidBalls(const EnemyUpdateContext& context);

    /// <summary>
    /// 移動・攻撃範囲判定 AI（EnemyAcidState.cpp で実装）
    /// </summary>
    void UpdateMovementAI(const EnemyUpdateContext& context);



private:
    VECTOR m_headPosOffset;              // ヘッドショット判定用オフセット座標
    VECTOR m_acidBulletSpawnOffset;      // 酸を吐く場所のオフセット
    AnimationManager m_animationManager; // アニメーション管理
    AnimState m_currentAnimState;        // 現在のアニメーション状態
    std::shared_ptr<EnemyState<EnemyAcid>> m_pCurrentState; // 現在のAIステート
    float m_distToPlayer;         // プレイヤーとの距離
    std::shared_ptr<CapsuleCollider> m_pBodyCollider; // 体のコライダー
    std::shared_ptr<SphereCollider> m_pHeadCollider;  // 頭のコライダー
    std::shared_ptr<SphereCollider> m_pAttackRangeCollider; // 攻撃範囲のコライダー
    std::vector<AcidBall> m_acidBalls;
    std::function<void(const VECTOR&)> m_onDropItem; // アイテムドロップコールバック

    int m_attackEndDelayTimer; // 攻撃後の硬直時間
    int m_backAnimCount;       // 後退アニメーションのカウント

    bool m_isNextAttackNormal; // 次の攻撃が通常弾かどうか

    float m_animTime;   // 現在のアニメーション再生時間

    bool m_hasAttacked;       // 攻撃アニメーション中に一度だけ攻撃ヒット判定を行うためのフラグ
    bool m_isDeadAnimPlaying; // 死亡アニメーション再生中フラグ
    bool m_hasDroppedItem;    // アイテムドロップ済みフラグ
    bool m_isStunned;         // 怯み状態か
    int m_stunTimer;          // 怯みタイマー

    static int s_modelHandle; // 共有モデルハンドル
    static bool s_shouldDrawCollision;   // 当たり判定を描画するか

    bool m_shouldDrawParryCollider = false; // パリィコライダーを描画するか
    VECTOR m_debugParryCapA = { 0,0,0 };    // デバッグ用パリィカプセルのA点
    VECTOR m_debugParryCapB = { 0,0,0 };    // デバッグ用パリィカプセルのB点
    float m_debugParryRadius = 0.0f;        // デバッグ用パリィカプセルの半径

    // ステートクラスからのアクセスを許可
    friend class EnemyAcidStateWalk;
    friend class EnemyAcidStateBack;
    friend class EnemyAcidStateAttack;
    friend class EnemyAcidStateStunned;
    friend class EnemyAcidStateDead;
};