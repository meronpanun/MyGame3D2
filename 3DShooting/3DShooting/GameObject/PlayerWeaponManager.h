#pragma once
#include "AttackType.h"
// 外部ライブラリ（Effekseer）内部で発生する警告を抑制
// 4244: 型変換, 26495: 未初期化メンバ変数
#pragma warning(push)
#pragma warning(disable: 4244 26495)
#include "EffekseerWarningSuppress.h"
#pragma warning(pop)
#include "Stage.h"
#include <vector>
#include <tuple>
#include <deque>

class Bullet;
class Camera;
class Effect;
class AnimationManager;
class ShellCasing;
class EnemyBase;

/// <summary>
/// 武器の種類列挙型
/// </summary>
enum class WeaponType
{
    AssaultRifle, // アサルトライフル
    Shotgun       // ショットガン
};

/// <summary>
/// プレイヤーの武器管理クラス
/// </summary>
class PlayerWeaponManager
{
public:
    PlayerWeaponManager();
    ~PlayerWeaponManager();

    /// <summary>
    /// 初期化処理
    /// </summary>
    /// <param name="arInitAmmo">AR初期弾数</param>
    /// <param name="sgInitAmmo">SG初期弾数</param>
    /// <param name="arMaxAmmo">AR最大弾数</param>
    /// <param name="sgMaxAmmo">SG最大弾数</param>
    /// <param name="bulletPower">AR弾威力</param>
    /// <param name="sgBulletPower">SG弾威力</param>
    /// <param name="arShootRate">AR射撃レート（省略時はデフォルト値を使用）</param>
    void Init(int arInitAmmo, int sgInitAmmo, int arMaxAmmo, int sgMaxAmmo,
        float bulletPower, float sgBulletPower, float arShootRate = kARShootRate);

    // コンテキスト構造体
    struct UpdateContext
    {
        float deltaTime;
        const VECTOR& playerPos;
        Camera* pCamera;
        bool isGuarding;
        bool isDead;
        bool isTackling;
        bool isLockingOn;
        bool isSwitchingWeapon;
        AttackType allowedAttackType;
        bool isInfiniteAmmo;
        const std::vector<EnemyBase*>& enemyList;
        const std::vector<Stage::StageCollisionData>& collisionData;
    };

    struct DrawContext
    {
        const VECTOR& playerPos;
        Camera* pCamera;
        const VECTOR& gunSwayOffset;
        const VECTOR& gunShakeOffset;
        const VECTOR& gunSwayRotOffset;
        float guardAnimTimer;
        float guardAnimDuration;
        bool isSwitchingWeapon;
        float weaponSwitchTimer;
        float weaponSwitchDuration;
        WeaponType previousWeaponType;
        bool isTryingToGuard;
        bool isTackling;
        float startAnimOffsetY;
    };

    /// <summary>
    /// 毎フレームの更新処理
    /// </summary>
    /// <param name="context">更新コンテキスト</param>
    void Update(const UpdateContext& context);

    /// <summary>
    /// 3D描画処理
    /// </summary>
    /// <param name="context">描画コンテキスト</param>
    void Draw3D(const DrawContext& context);

    /// <summary>
    /// アサルトライフルの射撃レートを取得する
    /// </summary>
    /// <returns>アサルトライフルの射撃レート</returns>
    static float GetARShootRate() { return kARShootRate; }

    /// <summary>
    /// 武器を切り替える
    /// </summary>
    /// <param name="weaponType">切り替える武器の種類</param>
    void SwitchWeapon(WeaponType weaponType);

    /// <summary>
    /// 現在の武器の種類を取得する
    /// </summary>
    /// <returns>現在の武器の種類</returns>
    WeaponType GetCurrentWeaponType() const { return m_currentWeaponType; }

    /// <summary>
    /// 射撃処理
    /// </summary>
    /// <param name="bullets">弾の管理配列</param>
    /// <param name="playerPos">プレイヤーの位置ベクトル</param>
    /// <param name="pCamera">カメラポインタ</param>
    /// <param name="pEffect">エフェクトポインタ</param>
    /// <param name="pAnimManager">アニメーションマネージャーポインタ</param>
    /// <param name="shellCasings">薬莢の管理配列</param>
    void Shoot(std::vector<Bullet>& bullets, const VECTOR& playerPos,
        Camera* pCamera, Effect* pEffect, AnimationManager* pAnimManager,
        std::vector<ShellCasing>& shellCasings);

    /// <summary>
    /// 射撃可能かどうかを取得する
    /// </summary>
    /// <returns>射撃可能ならtrue</returns>
    bool CanShoot() const;

    /// <summary>
    /// 現在の弾薬数を取得する
    /// </summary>
    /// <returns>現在の弾数</returns>
    int GetCurrentAmmo() const;

    /// <summary>
    /// 最大弾薬数を取得する
    /// </summary>
    /// <returns>最大弾数</returns>
    int GetMaxAmmo() const;

    /// <summary>
    /// アサルトライフルの弾を追加する
    /// </summary>
    /// <param name="value">追加する弾数</param>
    void AddARAmmo(int value);

    /// <summary>
    /// ショットガンの弾を追加する
    /// </summary>
    /// <param name="value">追加する弾数</param>
    void AddSGAmmo(int value);

    /// <summary>
    /// 弾薬を消費する
    /// </summary>
    void ConsumeAmmo();

    /// <summary>
    /// 武器切り替え中かどうかを取得する
    /// </summary>
    /// <returns>切り替え中ならtrue</returns>
    bool IsSwitchingWeapon() const { return m_isSwitchingWeapon; }

    /// <summary>
    /// 武器切り替え中かどうかを設定する
    /// </summary>
    /// <param name="switching">切り替え中にするならtrue</param>
    void SetSwitchingWeapon(bool switching) { m_isSwitchingWeapon = switching; }

    /// <summary>
    /// 武器切り替えタイマーを取得する
    /// </summary>
    /// <returns>切り替えタイマーの値</returns>
    float GetWeaponSwitchTimer() const { return m_weaponSwitchTimer; }

    /// <summary>
    /// 武器切り替えアニメーション時間を取得する
    /// </summary>
    /// <returns>切り替えアニメーション時間</returns>
    float GetWeaponSwitchDuration() const { return m_weaponSwitchDuration; }

    /// <summary>
    /// 前の武器の種類を取得する
    /// </summary>
    /// <returns>前の武器の種類</returns>
    WeaponType GetPreviousWeaponType() const { return m_previousWeaponType; }

    /// <summary>
    /// 前の武器が弾薬少ない状態だったかどうかを取得する
    /// </summary>
    /// <returns>弾薬が少ない状態ならtrue</returns>
    bool GetPrevWeaponHadLowAmmo() const { return m_prevWeaponHadLowAmmo; }

    /// <summary>
    /// 前の武器が弾切れだったかどうかを取得する
    /// </summary>
    /// <returns>弾切れならtrue</returns>
    bool GetPrevWeaponHadNoAmmo() const { return m_prevWeaponHadNoAmmo; }

    /// <summary>
    /// 銃のシェイクオフセットを取得する
    /// </summary>
    /// <returns>銃のシェイクオフセットベクトル</returns>
    VECTOR GetGunShakeOffset() const { return m_gunShakeOffset; }

    /// <summary>
    /// 弾薬少ない警告状態かどうかを取得する
    /// </summary>
    /// <returns>弾薬が少ない警告ならtrue</returns>
    bool IsLowAmmo() const { return m_isLowAmmo; }

    /// <summary>
    /// 弾切れ警告状態かどうかを取得する
    /// </summary>
    /// <returns>弾切れ警告ならtrue</returns>
    bool IsNoAmmoWarning() const { return m_isShowingNoAmmoWarning; }

    /// <summary>
    /// 弾薬警告の点滅タイマーを取得する
    /// </summary>
    /// <returns>点滅タイマーの値</returns>
    float GetLowAmmoBlinkTimer() const { return m_lowAmmoBlinkTimer; }

    /// <summary>
    /// 無限弾薬モードを設定する
    /// </summary>
    /// <param name="infinite">無限弾薬にするならtrue</param>
    void SetInfiniteAmmo(bool infinite) { m_isInfiniteAmmo = infinite; }

    /// <summary>
    /// 無限弾薬かどうかを取得する
    /// </summary>
    /// <returns>無限弾薬ならtrue</returns>
    bool IsInfiniteAmmo() const { return m_isInfiniteAmmo; }

    /// <summary>
    /// 銃の位置を取得する
    /// </summary>
    /// <param name="playerPos">プレイヤーの位置</param>
    /// <param name="pCamera">カメラのポインタ</param>
    /// <returns>銃の位置ベクトル</returns>
    VECTOR GetGunPos(const VECTOR& playerPos, Camera* pCamera) const;

    /// <summary>
    /// 銃の向きベクトルを取得する
    /// </summary>
    /// <param name="pCamera">カメラのポインタ</param>
    /// <returns>銃の向きベクトル</returns>
    VECTOR GetGunRot(Camera* pCamera) const;

    /// <summary>
    /// 薬莢排出口の位置を取得する
    /// </summary>
    /// <returns>薬莢排出口の位置ベクトル</returns>
    VECTOR GetEjectionPortPos() const;

    /// <summary>
    /// 銃のシェイクを開始する
    /// </summary>
    /// <param name="power">シェイクの強さ</param>
    /// <param name="duration">シェイクの持続時間</param>
    void ShakeGun(float power, float duration);

    /// <summary>
    /// アサルトライフルモデルハンドルを取得する
    /// </summary>
    /// <returns>アサルトライフルモデルハンドル</returns>
    int GetARHandle() const { return m_arHandle; }

    /// <summary>
    /// ショットガンモデルハンドルを取得する
    /// </summary>
    /// <returns>ショットガンモデルハンドル</returns>
    int GetSGHandle() const { return m_sgHandle; }

    /// <summary>
    /// ショットガンアニメーションを更新する
    /// </summary>
    /// <param name="pAnimManager">アニメーションマネージャーのポインタ</param>
    /// <param name="deltaTime">経過時間</param>
    void UpdateSGAnimation(AnimationManager* pAnimManager, float deltaTime);

    /// <summary>
    /// 武器モデルのスケールを設定する
    /// </summary>
    /// <param name="scale">スケールベクトル</param>
    void SetWeaponScale(const VECTOR& scale);

    /// <summary>
    /// 武器モデルの回転を設定する
    /// </summary>
    /// <param name="rot">回転ベクトル</param>
    void SetWeaponRotation(const VECTOR& rot);

    /// <summary>
    /// 射撃反動のスケールを取得する（0.0f〜1.0f）
    /// </summary>
    /// <returns>反動スケール</returns>
    float GetRecoilScale() const;

    static constexpr float kARShootRate = 10.0f; // ARのデフォルト射撃レート

private:
    /// <summary>
    /// 銃のモデルが近くの障害物に接触しているか計算し、引き込み量を返す
    /// </summary>
    float CalculatePullBackOffset(
        const VECTOR& playerPos, Camera* pCamera,
        const std::vector<EnemyBase*>& enemyList,
        const std::vector<Stage::StageCollisionData>& collisionData) const;

    // 武器モデルハンドル
    int m_arHandle;           // アサルトライフルモデル
    int m_sgHandle;           // ショットガンモデル
    int m_ejectionPortFrame;  // 薬莢排出口フレームインデックス
    int m_currentWeaponIndex; // 現在の武器インデックス

    // 弾薬関連
    int   m_arAmmo;       // ARの現在弾数
    int   m_sgAmmo;       // SGの現在弾数
    int   m_arMaxAmmo;    // ARの最大弾数
    int   m_sgMaxAmmo;    // SGの最大弾数
    float m_bulletPower;  // ARの弾威力
    float m_sgBulletPower;// SGの弾威力

    // 射撃関連
    float m_shootCooldown;      // 1発あたりのクールダウン時間
    float m_shootCooldownTimer; // 現在のクールダウンタイマー
    float m_arShootRate;        // ARの射撃レート

    // 武器切り替え
    WeaponType              m_currentWeaponType;   // 現在の武器種類
    WeaponType              m_previousWeaponType;  // 前の武器種類
    std::vector<WeaponType> m_weaponTypes;         // 武器リスト
    bool  m_isSwitchingWeapon;    // 武器切り替えアニメーション中フラグ
    float m_weaponSwitchTimer;    // 切り替えアニメーションタイマー
    float m_weaponSwitchDuration; // 切り替えアニメーション時間
    bool  m_prevWeaponHadLowAmmo; // 前の武器が弾薬少ない状態だったか
    bool  m_prevWeaponHadNoAmmo;  // 前の武器が弾切れだったか

    // 弾薬警告
    bool  m_isLowAmmo;              // 弾薬少ない警告フラグ
    bool  m_isShowingNoAmmoWarning; // 弾切れ警告表示フラグ
    float m_lowAmmoBlinkTimer;      // 警告点滅タイマー

    // 無限弾薬
    bool m_isInfiniteAmmo; // 無限弾薬モードフラグ

    // 銃のシェイク
    VECTOR m_gunShakeOffset; // シェイクによる位置オフセット
    float  m_gunShakeTimer;  // シェイクタイマー
    float  m_gunShakePower;  // シェイクの強さ

    // ショットガンアニメーション
    bool  m_isSGAnimPlaying;              // SGアニメーション再生中フラグ
    float m_sgAnimTime;                   // SGアニメーション経過時間
    float m_sgPumpTimer;                  // ポンプアクション音の遅延タイマー
    std::deque<float> m_arCartridgeQueue; // AR薬莢SE再生予約キュー
    std::deque<float> m_sgCartridgeQueue; // SG薬莢SE再生予約キュー

    // 引き込みオフセット
    float m_pullBackOffset; // 銃の引き込み量（障害物回避用）
};
