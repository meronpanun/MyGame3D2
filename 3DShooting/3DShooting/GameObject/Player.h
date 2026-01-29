#pragma once
#include "AttackType.h"
#include "Camera.h"
#include "EffekseerForDXLib.h"
#include "PlayerEffectManager.h"
#include "PlayerMovement.h"
#include "PlayerShieldSystem.h"
#include "PlayerUI.h"
#include "PlayerWeaponManager.h"
#include "Stage.h"
#include <vector>

class Camera;
class Effect;
class Bullet;
class EnemyBase;
class EnemyNormal;
class CapsuleCollider;
class DirectionIndicator;
class ShellCasing;
class AnimationManager;

/// <summary>
/// プレイヤークラス
/// </summary>
class Player {
public:
  Player();
  virtual ~Player();

  void Init(bool isTutorial = false);
  void Update(const std::vector<EnemyBase *> &enemyList,
              const std::vector<Stage::StageCollisionData> &collisionData);
  void Draw3D();
  void DrawShield();
  void DrawUI();

  /// <summary>
  /// タックル情報構造体
  /// </summary>
  struct TackleInfo {
    VECTOR capA = {0, 0, 0}; // タックル判定カプセルのA点
    VECTOR capB = {0, 0, 0}; // タックル判定カプセルのB点
    float radius = 0.0f;     // タックル判定カプセルの半径
    float damage = 0.0f;     // タックルのダメージ量
    bool isTackling = false; // タックル中かどうか
    int tackleId = 0;        // タックルID
  };

  /// <summary>
  /// カメラを取得する
  /// </summary>
  /// <returns>カメラの共有ポインタ</returns>
  const std::shared_ptr<Camera> &GetCamera() const { return m_pCamera; }

  /// <summary>
  /// 弾を撃つ
  /// </summary>
  /// <param name="bullets">弾のベクター</param>
  void Shoot(std::vector<Bullet> &bullets);

  /// <summary>
  /// プレイヤーがダメージを受ける
  /// </summary>
  /// <param name="damage">受けるダメージ量</param>
  /// <param name="attackerPos">攻撃者の位置（オプション）</param>
  /// <param name="isParryable">パリィ可能かどうか（デフォルトtrue）</param>
  void TakeDamage(float damage, const VECTOR &attackerPos = VGet(0, 0, 0),
                  bool isParryable = true);

  /// <summary>
  /// プレイヤーの位置を取得する
  /// </summary>
  /// <returns>プレイヤーの位置</returns>
  VECTOR GetPos() const { return m_modelPos; }

  /// <summary>
  /// プレイヤーの位置を設定する
  /// </summary>
  /// <param name="pos">設定する位置</param>
  void SetPos(const VECTOR &pos) { m_pos = pos; }

  /// <summary>
  /// 弾の取得
  /// </summary>
  /// <returns>弾のベクター</returns>
  std::vector<Bullet> &GetBullets();

  /// <summary>
  ///  プレイヤーがショット可能かどうか
  /// </summary>
  /// <returns>ショット可能ならtrue</returns>
  bool HasShot();

  /// <summary>
  /// タックル情報を取得する
  /// </summary>
  /// <returns>タックル情報</returns>
  TackleInfo GetTackleInfo() const;

  /// <summary>
  /// プレイヤーのカプセルコライダー情報を取得する
  /// </summary>
  /// <param name="capA">カプセルのA点</param>
  /// <param name="capB">カプセルのB点</param>
  /// <param name="radius">カプセルの半径</param>
  void GetCapsuleInfo(VECTOR &capA, VECTOR &capB, float &radius) const;

  /// <summary>
  /// 直前にガードが成功したかどうか
  /// </summary>
  /// <returns>ガード成功ならtrue</returns>
  bool IsJustGuarded() const;

  /// <summary>
  /// 体力を加算する
  /// </summary>
  /// <param name="value">加算する体力値</param>
  void AddHp(float value);

  /// <summary>
  /// プレイヤーがガード中かどうか
  /// </summary>
  /// <returns>ガード中ならtrue</returns>
  bool IsGuarding() const { return m_shieldSystem.IsGuarding(); }

  /// <summary>
  /// 現在の体力を取得する
  /// </summary>
  /// <returns>現在の体力</returns>
  float GetHealth() const { return m_health; }

  /// <summary>
  /// 最大体力を取得する
  /// </summary>
  /// <returns>最大体力</returns>
  float GetMaxHealth() const { return m_maxHealth; }

  /// <summary>
  /// AR弾薬回復用関数
  /// </summary>
  /// <param name="value">弾薬数</param>
  void AddARAmmo(int value);

  /// <summary>
  /// SG弾薬回復用関数
  /// </summary>
  /// <param name="value">弾薬数</param>
  void AddSGAmmo(int value);

  /// <summary>
  /// 現在の武器の弾薬数を取得する
  /// </summary>
  /// <returns>現在の弾薬数</returns>
  int GetCurrentAmmo() const;

  /// <summary>
  /// 現在の武器の最大弾薬数を取得する
  /// </summary>
  /// <returns>最大弾薬数</returns>
  int GetMaxAmmo() const;

  /// <summary>
  /// 弾薬無限モードを設定する
  /// </summary>
  /// <param name="isInfinite">無限にするかどうか</param>
  void SetInfiniteAmmo(bool isInfinite) { m_isInfiniteAmmo = isInfinite; }

  /// <summary>
  /// 弾薬無限モードかどうかを取得する
  /// </summary>
  /// <returns>弾薬無限モードならtrue</returns>
  bool IsInfiniteAmmo() const { return m_isInfiniteAmmo; }

  // プレイヤーのカプセルコライダー取得
  std::shared_ptr<CapsuleCollider> GetBodyCollider() const;

  /// <summary>
  /// 接地しているオブジェクト名を取得する
  /// </summary>
  /// <returns>接地しているオブジェクト名</returns>
  std::string GetGroundedObjectName() const;

  /// <summary>
  /// 回復SEハンドルを取得する
  /// </summary>
  int GetRecoverySEHandle() const { return m_recoverySEHandle; }

  /// <summary>
  /// 弾薬アイテムSEハンドルを取得する
  /// </summary>
  int GetAmmoItemSEHandle() const { return m_ammoItemSEHandle; }

  /// <summary>
  /// 無敵モードを設定する
  /// </summary>
  /// <param name="isInvincible">無敵にするかどうか</param>
  void SetInvincible(bool isInvincible) { m_isInvincible = isInvincible; }

  /// <summary>
  /// 無敵モードかどうかを取得する
  /// </summary>
  /// <returns>無敵モードならtrue</returns>
  bool IsInvincible() const { return m_isInvincible; }

  /// <summary>
  /// 飛行モードを設定する
  /// </summary>
  /// <param name="isFlightMode">飛行モードにするかどうか</param>
  void SetFlightMode(bool isFlightMode) { m_isFlightMode = isFlightMode; }

  /// <summary>
  /// 飛行モードかどうかを取得する
  /// </summary>
  /// <returns>飛行モードならtrue</returns>
  bool IsFlightMode() const { return m_isFlightMode; }

  /// <summary>
  /// ロックオン可能な敵がいるかどうかを取得する
  /// </summary>
  /// <returns>ロックオン可能な敵がいるならtrue</returns>
  bool IsTargetAvailable() const { return m_isTargetAvailable; }

  /// <summary>
  /// 敵に照準が合っているかどうかを取得する
  /// </summary>
  /// <returns>照準が合っているならtrue</returns>
  bool IsAimingAtEnemy() const;

  /// <summary>
  /// 体力が低下しているかどうかを取得する
  /// </summary>
  /// <returns>体力が低下しているならtrue</returns>
  bool IsLowHealth() const { return m_isLowHealth; }

  /// <summary>	/// 攻撃制限を設定する
  /// </summary>
  /// <param name="allowedAttack">許可する攻撃タイプ</param>
  void SetAttackRestrictions(AttackType allowedAttack);

  /// <summary>
  /// プレイヤーが死亡しているかどうか
  /// </summary>
  /// <returns>死亡しているならtrue</returns>
  bool IsDead() const { return m_isDead; }

  /// <summary>
  /// 方向インジケーターを設定する
  /// </summary>
  /// <param name="directionIndicator">方向インジケーターのポインタ</param>
  void SetDirectionIndicator(DirectionIndicator *directionIndicator) {
    m_pDirectionIndicator = directionIndicator;
  }

  /// <summary>
  /// 銃を揺らす
  /// </summary>
  /// <param name="power">揺れの強さ</param>
  /// <param name="duration">揺れの持続時間</param>
  void ShakeGun(float power, float duration);

  /// <summary>
  /// エフェクトを設定する
  /// </summary>
  /// <param name="pEffect">エフェクトのポインタ</param>
  void SetEffect(Effect *pEffect) { m_pEffect = pEffect; }

  /// <summary>
  /// アニメーションマネージャーを設定する
  /// </summary>
  /// <param name="pAnimManager">アニメーションマネージャーのポインタ</param>
  void SetAnimationManager(AnimationManager *pAnimManager) {
    m_pAnimManager = pAnimManager;
  }

  WeaponType GetCurrentWeaponType() const;

private:
  AttackType m_allowedAttackType = AttackType::None;
  /// <summary>
  /// 死亡時の更新処理
  /// </summary>
  void DeathUpdate();

  /// <summary>
  /// 銃の位置を取得する
  /// </summary>
  /// <returns>銃の位置</returns>
  VECTOR GetGunPos() const;
  VECTOR GetGunRot() const;

  /// <summary>
  /// 薬莢排出口の位置を取得する
  /// </summary>
  /// <returns>薬莢排出口の位置</returns>
  VECTOR GetEjectionPortPos() const;

  /// <summary>
  /// 武器を切り替える
  /// </summary>
  /// <param name="weaponType">切り替える武器の種類</param>
  void SwitchWeapon(WeaponType weaponType);

private:
  // コンポーネント
  PlayerWeaponManager m_weaponManager;
  PlayerMovement m_movement;
  PlayerShieldSystem m_shieldSystem;
  PlayerUI m_ui;
  PlayerEffectManager m_effectManager;

  std::vector<ShellCasing> m_shellCasings;
  std::shared_ptr<Camera> m_pCamera; // カメラのポインタ
  std::vector<Bullet> m_bullets;     // 弾の管理
  Effect *m_pEffect;                 // エフェクトのポインタ
  AnimationManager *m_pAnimManager;  // アニメーションマネージャーのポインタ

  // プレイヤーの位置を保持するメンバー変数
  VECTOR m_pos;
  VECTOR m_modelPos;
  VECTOR m_tackleDir;
  VECTOR m_scale;

  unsigned char m_prevKeyState[256]{}; // 前回のキー入力状態
  bool m_prevIsGuarding = false;       // 前フレームのガード状態

  int m_arInitAmmo;                    // ARの初期弾薬数
  int m_sgInitAmmo;                    // SGの初期弾薬数
  int m_arMaxAmmo;                     // ARの最大弾薬数
  int m_sgMaxAmmo;                     // SGの最大弾薬数
  int m_playerHitSEHandle;             // 被弾SEのハンドル
  int m_tackleSEHandle;                // タックルSEのハンドル
  int m_recoverySEHandle;              // 回復アイテムSEのハンドル
  int m_tackleFrame;                   // タックルのフレーム数
  int m_tackleCooldown;                // タックルのクールダウンタイマー
  int m_tackleId;                      // タックルID
  int m_concentrationLineEffectHandle; // 集中線エフェクトハンドル
  int m_ammoItemSEHandle;              // 弾薬アイテムSEのハンドル
  int m_landingSEHandle;               // 着地SEのハンドル
  bool m_isLowHealth;                  // 体力が少ないかどうかのフラグ
  float m_lowHealthBlinkTimer;         // 体力低下UIの点滅タイマー
  float m_health;                      // 現在の体力
  float m_healthBarAnim;               // HPバーのアニメーション用体力値
  float m_healthBarAnimTimer;          // HPバーアニメーション用タイマー
  float m_maxHealth;                   // 最大体力
  float m_bulletPower;                 // 弾の威力
  float m_sgBulletPower;               // SG弾の威力
  float m_maxShieldDurability;         // 盾の最大耐久値
  float m_tackleCooldownMax;           // タックルクールタイム
  float m_tackleSpeed;                 // タックル時の速度
  float m_tackleDamage;                // タックルダメージ
  float m_moveSpeed;                   // 移動速度
  float m_runSpeed;                    // 走る速度
  bool m_hasShot;                      // プレイヤーがショット可能かどうか
  bool m_isTackling;                   // タックル中かどうか
  bool m_isInvincible;                 // 無敵モードかどうか
  bool m_isInfiniteAmmo;               // 弾薬無限モードかどうか
  bool m_isFlightMode;                 // 飛行モードかどうか
  float m_shieldRegenRate;             // 盾の回復速度
  float m_ammoTextFlashTimer;          // 弾薬テキストのフラッシュタイマー

  // Sway管理
  float m_idleSwayTimer;     // 待機時の揺れタイマー
  VECTOR m_gunSwayOffset;    // 銃の揺れオフセット
  VECTOR m_gunSwayRotOffset; // 銃の回転揺れオフセット

  // 死亡関連
  bool m_isDead;      // 死亡フラグ
  float m_deathTimer; // 死亡アニメーションタイマー

  // 方向インジケーター
  DirectionIndicator *m_pDirectionIndicator;

  // ロックオン関連
  bool m_isLockingOn;         // ロックオン中か
  EnemyBase *m_lockedOnEnemy; // ロックオンした敵
  bool m_isTargetAvailable;   // ロックオン可能な敵がいるか
  bool m_isAimingAtEnemy;     // 敵に照準が合っているか

  // ガード関連
  bool m_ignoreGuardInput; // ガード入力を無視するか

  // チュートリアル関連
  bool m_isTutorial;
};