#pragma once
#include "AnimationManager.h"
#include "EnemyBase.h"
#include "DxLib.h"
#include <memory>
#include "EnemyState.h"

class Bullet;
class Player;
class Collider;
class CapsuleCollider;

namespace EnemyRunnerConstants
{
    // アニメーション名
    constexpr char kAttackAnimName[] = "Armature|Attack"; // 攻撃アニメーション名
    constexpr char kRunAnimName[]    = "Armature|Run";    // 走行アニメーション名
    constexpr char kDeadAnimName[]   = "Armature|Death";  // 死亡アニメーション名

    inline const VECTOR kHeadShotPositionOffset = { 0.0f, 0.0f, 0.0f }; // ヘッドショット判定コライダーの中心補正値

    // コライダーサイズ
    constexpr float kBodyColliderRadius = 20.0f;  // 体のカプセルコライダー半径
    constexpr float kBodyColliderHeight = 110.0f; // 体のカプセルコライダー高さ
    constexpr float kHeadRadius         = 15.0f;  // 頭の球コライダー半径

    // 攻撃関連
    constexpr int   kAttackCooldownMax   = 30;     // 攻撃クールダウン時間（フレーム数）
    constexpr float kAttackHitRadius     = 60.0f;  // 攻撃の当たり判定半径
    constexpr float kAttackTriggerRadius = 30.0f;  // 攻撃開始判定半径（通常より小さめ）
    constexpr float kAttackRangeRadius   = 100.0f; // 攻撃範囲の半径
    constexpr int   kAttackEndDelay      = 10;     // 攻撃後の硬直時間（フレーム数）
    constexpr float kAttackHitStartRatio = 0.3f;   // 攻撃ヒット判定の開始タイミング比率
    constexpr float kAttackHitEndRatio   = 0.6f;   // 攻撃ヒット判定の終了タイミング比率

    // 追跡・移動関連
    constexpr float kRotateSpeedPerFrame = 0.05f; // フレームあたりの旋回速度（ラジアン）
    constexpr int   kTargetOffsetRange   = 40;    // 追跡目標へのランダムオフセット範囲（±この値）

    // 徘徊関連
    constexpr int   kWanderTimerInterval = 120;    // 徘徊位置更新間隔（フレーム数）
    constexpr float kWanderMinDist       = 300.0f; // 徘徊時の最小距離
    constexpr int   kWanderDistRange     = 400;    // 徘徊時の距離のランダム幅

    // 回避関連
    constexpr float kEvasionMarginRadius = 80.0f; // 回避判定のマージン半径
    constexpr float kEvasionSwitchTime   = 60.0f; // 回避方向を切り替える間隔（フレーム数）

    // サウンド関連
    constexpr float kVoiceMaxDist        = 2000.0f; // 攻撃ボイス・ダメージSEが聞こえる最大距離（最大音量はSoundList.csvのVolumeから取得）
    constexpr float kDamageSECooldownMax = 45.0f;   // ダメージSE再生クールタイム（フレーム数）

    // 当たり判定関連
    constexpr float kPushBackEpsilon = 0.0001f; // ゼロ除算防止のための最小距離の二乗閾値

    // 描画関連
    constexpr float kDrawDistanceSq     = 5000.0f * 5000.0f; // 最大描画距離の二乗
    constexpr float kDrawNearDistanceSq = 300.0f * 300.0f;   // 常に描画する近距離の二乗
    constexpr float kDrawDotThreshold   = 0.4f;              // 視野内判定に使う内積閾値
}
class SphereCollider;

/// <summary>
/// 走る敵クラス
/// </summary>
class EnemyRunner : public EnemyBase
{
public:
    EnemyRunner();
    virtual ~EnemyRunner();

    void Init() override;
    void Update(const EnemyUpdateContext& context) override;
    void Draw() override;

    void OnDeath() override;

    /// <summary>
    /// ボディコライダーを取得する
    /// </summary>
    /// <returns>ボディコライダー</returns>
    std::shared_ptr<CapsuleCollider> GetBodyCollider() const override;

    /// <summary>
    /// デバッグ用の当たり判定を描画する
    /// </summary>
    virtual void DrawCollisionDebug() const override;

    /// <summary>
    /// モデルの読み込み（共有）
    /// </summary>
    static void LoadModel();

    /// <summary>
    /// モデルの解放（共有）
    /// </summary>
    static void DeleteModel();

    static void SetDrawCollision(bool draw) { s_shouldDrawCollision = draw; }
    static bool ShouldDrawCollision() { return s_shouldDrawCollision; }

private:
    static bool s_shouldDrawCollision;

    /// <summary>
    /// どこに当たったかを判定する
    /// </summary>
    /// <param name="rayStart">弾のRayの始点</param>
    /// <param name="rayEnd">弾のRayの終点</param>
    /// <returns>当たった部位</returns>
    HitPart CheckHitPart(const VECTOR& rayStart, const VECTOR& rayEnd, VECTOR& outHtPos, float& outHtDistSq) const override;

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
    void ChangeState(std::shared_ptr<EnemyState<EnemyRunner>> newState);

    /// <summary>
    /// プレイヤーに攻撃可能かどうかを判定
    /// </summary>
    /// <param name="player">プレイヤーオブジェクト</param>
    /// <param name="checkRadius">判定半径(-1の場合はデフォルト)</param>
    /// <returns>攻撃可能ならtrue</returns>
    bool CanAttackPlayer(const Player& player, float checkRadius = -1.0f);

    // EnemyBase::UpdateStandard から呼び出される内部処理
    void UpdateAI(const EnemyUpdateContext& context) override;
    void UpdateAnimation(const EnemyUpdateContext& context) override;
    void UpdateDeath(const EnemyUpdateContext& context) override;
    void UpdateSimpleMode(const EnemyUpdateContext& context) override;

private:
    VECTOR m_headPosOffset; // ヘッドショット判定コライダーの中心補正値

    std::shared_ptr<CapsuleCollider> m_pBodyCollider;      // 体のコライダー
    std::shared_ptr<SphereCollider> m_pHeadCollider;       // 頭のコライダー
    std::shared_ptr<SphereCollider> m_pAttackRangeCollider; // 攻撃範囲のコライダー
    std::shared_ptr<CapsuleCollider> m_pAttackHitCollider;  // 攻撃ヒット判定用のコライダー

    // アイテムドロップ時のコールバック関数
    std::function<void(const VECTOR&)> m_onDropItem;

    AnimState m_currentAnimState;        // 現在のアニメーション状態
    std::shared_ptr<EnemyState<EnemyRunner>> m_pCurrentState; // 現在のAIステート

    AnimationManager m_animationManager; // EnemyRunnerがアニメーションマネージャーを所有

    int m_attackEndDelayTimer; // 攻撃終了までの遅延タイマー

    float m_animTime;   // アニメーションの経過時間

    bool m_hasTakenTackleDamage;       // 1フレームで複数回ダメージを受けないためのフラグ
    bool m_hasAttackHit;       // 攻撃がヒットしたかどうか
    bool m_isDeadAnimPlaying; // 死亡アニメーション再生中フラグ
    bool m_hasDroppedItem;     // アイテムドロップ済みフラグ

    // 徘徊挙動用
    int m_wanderTimer;     // 徘徊位置更新タイマー
    VECTOR m_wanderOffset; // 徘徊位置オフセット

    // 回避挙動用
    float m_evadeSwitchTimer; // 回避方向切り替えタイマー
    bool m_isEvadingRight;    // 現在右に避けているか


    float m_distToPlayer;         // プレイヤーとの距離
    float m_damageSECooldown;    // ダメージSE再生用クールタイム
    static int s_modelHandle; // 共有モデルハンドル

    // ステートクラスからのアクセスを許可
    friend class EnemyRunnerStateRun;
    friend class EnemyRunnerStateAttack;
    friend class EnemyRunnerStateDead;
};