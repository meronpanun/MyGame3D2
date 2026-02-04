#pragma once
#include "AnimationManager.h"
#include "EnemyBase.h"
#include <functional>
#include <memory>
#include <string>
#include <vector>

class Bullet;
class Player;
class Collider;
class SphereCollider;
class CapsuleCollider;

/// <summary>
/// 通常の敵クラス
/// </summary>
class EnemyNormal : public EnemyBase {
public:
  EnemyNormal();
  virtual ~EnemyNormal();

  void Init() override;
  void Update(const EnemyUpdateContext &context) override;
  void Draw() override;

  /// <summary>
  /// デバッグ用の当たり判定を描画する
  /// </summary>
  virtual void DrawCollisionDebug() const override;

  /// <summary>
  /// どこに当たったかを判定する
  /// </summary>
  /// <param name="rayStart">弾のRayの始点</param>
  /// <param name="rayEnd">弾のRayの終点</param>
  /// <returns>当たった部位</returns>
  HitPart CheckHitPart(const VECTOR &rayStart, const VECTOR &rayEnd,
                       VECTOR &outHtPos, float &outHtDistSq) const override;

  /// <summary>
  /// ダメージを計算する
  /// </summary>
  /// <param name="bulletDamage">弾のダメージ量</param>
  /// <param name="part">当たった部位</param>
  /// <returns>計算されたダメージ</returns>
  float CalcDamage(float bulletDamage, HitPart part) const override;

  /// <summary>
  /// タックルダメージを受ける処理
  /// </summary>
  void ResetTackleHitFlag() { m_isTackleHit = false; }

  /// <summary>
  /// アイテムドロップ時のコールバック関数を設定する
  /// </summary>
  /// <param name="cb">コールバック関数</param>
  void SetOnDropItemCallback(std::function<void(const VECTOR &)> cb);

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

  /// <summary>
  /// モデルの読み込み(共有)
  /// </summary>
  static void LoadModel();

  /// <summary>
  /// モデルの解放(共有)
  /// </summary>
  static void DeleteModel();

  /// <summary>
  /// ボディコライダーを取得する
  /// </summary>
  /// <returns>ボディコライダー</returns>
  std::shared_ptr<CapsuleCollider> GetBodyCollider() const override;

private:
  /// <summary>
  /// アニメーションを変更する
  /// </summary>
  /// <param name="newAnimState">新しいアニメーション状態</param>
  /// <param name="loop">ループ再生するかどうか</param>
  void ChangeAnimation(AnimState newAnimState, bool loop);

  /// <summary>
  /// プレイヤーに攻撃可能かどうかを判定する
  /// </summary>
  /// <param name="player">プレイヤーオブジェクト</param>
  /// <returns>攻撃可能ならtrue</returns>
  bool CanAttackPlayer(const Player &player);

private:
  VECTOR m_headPosOffset; // ヘッドショット判定用オフセット座標

  std::shared_ptr<CapsuleCollider> m_pBodyCollider; // 体のコライダー
  std::shared_ptr<SphereCollider> m_pHeadCollider;  // 頭のコライダー
  std::shared_ptr<SphereCollider>
      m_pAttackRangeCollider; // 攻撃範囲のコライダー
  std::shared_ptr<CapsuleCollider>
      m_pAttackHitCollider; // 攻撃ヒット判定用のコライダー

  // アイテムドロップ時のコールバック関数
  std::function<void(const VECTOR &)> m_onDropItem;

  AnimState m_currentAnimState; // 現在のアニメーション状態
  AnimationManager
      m_animationManager; // EnemyNormal가 애니메이션マネージャーを所有

  int m_attackEndDelayTimer; // 攻撃終了までの遅延タイマー
  int m_damageTimer;         // ダメージ（怯み）タイマー

  float m_animTime;   // アニメーションの経過時間
  float m_chaseSpeed; // 追跡速度

  bool m_isTackleHit;       // 1フレームで複数回ダメージを受けないためのフラグ
  bool m_isAttackHit;       // 攻撃がヒットしたかどうか
  bool m_isDeadAnimPlaying; // 死亡アニメーション再生中フラグ
  bool m_isItemDropped;     // アイテムドロップ済みフラグ

  // 徘徊挙動用
  int m_wanderTimer;     // 徘徊位置更新タイマー
  VECTOR m_wanderOffset; // 徘徊位置オフセット

  static int s_modelHandle; // 共有モデルハンドル
};