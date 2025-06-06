#pragma once
#include "DxLib.h"
#include <memory>
#include <vector>

class Player;
class Bullet;

/// <summary>
/// 敵の基底クラス
/// </summary>
class EnemyBase abstract
{
public:
	EnemyBase();
	virtual ~EnemyBase() = default;

	virtual void Init() abstract;
	virtual void Update(const std::vector<Bullet>& bullets) abstract; // 弾リストを受け取る
	virtual void Draw() abstract;
	

	/// <summary>
	/// 弾や攻撃を受ける処理(基底で共通処理) 
	/// </summary>
	/// <param name="bullets">弾のリスト</param>
	virtual void CheckHitAndDamage(std::vector<Bullet>& bullets) abstract; 

	/// <summary>
	/// 敵が弾に当たったかどうかをチェックする関数
	/// </summary>
	/// <param name="bullet">弾の情報</param>
	/// <returns>当たったかどうか</returns>
	virtual bool IsHit(const Bullet& bullet) const abstract;

	/// <summary>
	/// 敵がダメージを受ける処理
	/// </summary>
	/// <param name="damage">受けるダメージ量</param>
	virtual void TakeDamage(float damage) abstract;

	/// <summary>
	/// デバッグ用の当たり判定を描画する関数
	/// </summary>
	virtual void DrawCollisionDebug() const {}

	// 当たり判定の部位
	enum class HitPart {
		None,
		Body,
		Head
	};

	/// <summary>
	/// 派生クラスでどこに当たったか判定する仮想関数
	/// </summary>
	/// <param name="bullet">弾の情報</param>
	/// <returns>当たった部位</returns>
	virtual HitPart CheckHitPart(const Bullet& bullet) const { return HitPart::None; }

protected:
	VECTOR m_pos; // 位置

	std::shared_ptr<Player> m_targetPlayer;  // ターゲットプレイヤー

	HitPart m_lastHitPart; // 最後に当たった部位

	int   m_modelHandle;     // モデルハンドル
	int   m_hitDisplayTimer; // ヒット表示タイマー
	float m_colRadius;       // 当たり判定用半径
	float m_hp;              // 体力

};

