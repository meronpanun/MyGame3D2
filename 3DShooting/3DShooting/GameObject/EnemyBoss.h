//#pragma once
//#include "EnemyBase.h"
//#include "AnimationManager.h"
//#include <vector>
//
//class Bullet;
//class Player;
//
///// <summary>
///// ボスクラス
///// </summary>
//class EnemyBoss : public EnemyBase
//{
//public:
//	EnemyBoss();
//	virtual ~EnemyBoss();
//	
//	void Init() override;
//	void Update(std::vector<Bullet>& bullets, const Player::TackleInfo& tackleInfo, const Player& player, const std::vector<EnemyBase*>& enemyList, const std::vector<Stage::StageCollisionData>& collisionData, Effect* pEffect = nullptr) override;
//	void Draw() override;
//
//	// ダメージ処理
//    void TakeDamage(float damage) override;
//    void TakeTackleDamage(float damage) override;
//
//private:
//	/// <summary>
//	/// アニメーションを変更する
//	/// </summary>
//	/// <param name="newAnimState">新しいアニメーション状態</param>
//	/// <param name="loop">ループ再生するかどうか</param>
//    void ChangeAnimation(AnimState newAnimState, bool loop);
//
//private:
//    AnimationManager m_animationManager; // アニメーション管理
//    AnimState m_currentAnimState;        // 現在のアニメーション状態
//    bool m_isDeadAnimPlaying;            // 死亡アニメーション再生中フラグ
//    float m_animTime;                    // アニメーションの経過時間
//};
//
