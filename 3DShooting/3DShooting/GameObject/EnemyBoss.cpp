//#include "EnemyBoss.h"
//#include "Bullet.h"
//#include "EffekseerForDXLib.h"
//#include "DebugUtil.h"
//#include "SceneMain.h"
//#include "ScoreManager.h"
//#include <cassert>
//#include <algorithm>
//#include <cmath>
//
//namespace
//{
//    constexpr char kDeadAnimName[] = "DEAD"; // 死亡アニメーション
//    constexpr float kDefaultInitialHP = 1000.0f; // ボスの初期体力
//}
//
//EnemyBoss::EnemyBoss() :
//    m_currentAnimState(AnimState::Idle),
//    m_isDeadAnimPlaying(false),
//    m_animTime(0.0f)
//{
//    m_modelHandle = MV1LoadModel("data/model/Boss.mv1"); // 仮のモデル
//    assert(m_modelHandle != -1);
//}
//
//EnemyBoss::~EnemyBoss()
//{
//    MV1DeleteModel(m_modelHandle);
//}
//
//void EnemyBoss::Init()
//{
//    m_hp = kDefaultInitialHP;
//    m_isAlive = true;
//    m_isDeadAnimPlaying = false;
//    m_animTime = 0.0f;
//    m_pos = VGet(0.0f, 0.0f, 500.0f); // 仮の位置
//    MV1SetPosition(m_modelHandle, m_pos);
//    ChangeAnimation(AnimState::Idle, true); // 初期アニメーション
//}
//
//void EnemyBoss::Update(std::vector<Bullet>& bullets, const Player::TackleInfo& tackleInfo, const Player& player, const std::vector<EnemyBase*>& enemyList, const std::vector<Stage::StageCollisionData>& collisionData, Effect* pEffect)
//{
//    if (m_hp <= 0.0f) 
//    {
//        if (!m_isDeadAnimPlaying) 
//        {
//            ChangeAnimation(AnimState::Dead, false);
//            m_isDeadAnimPlaying = true;
//            m_animTime = 0.0f; // アニメーション時間をリセット
//            m_isAlive = true; // 死亡アニメーション中はtrueのまま
//        }
//        
//        // 死亡アニメーション中もアニメーション時間を更新
//        if (m_animationManager.GetCurrentAttachedAnimHandle(m_modelHandle) != -1)
//        {
//            m_animTime += 1.0f;
//            m_animationManager.UpdateAnimationTime(m_modelHandle, m_animTime);
//        }
//        
//        float currentAnimTotalTime = m_animationManager.GetAnimationTotalTime(m_modelHandle, kDeadAnimName);
//        if (m_animTime >= currentAnimTotalTime) 
//        {
//            if (m_animationManager.GetCurrentAttachedAnimHandle(m_modelHandle) != -1) 
//            {
//                MV1DetachAnim(m_modelHandle, 0);
//                m_animationManager.ResetAttachedAnimHandle(m_modelHandle);
//            }
//            m_isAlive = false; // 死亡アニメーション終了時のみfalseにする
//            SetActive(false);  // プールに戻す
//        } 
//        return;
//    }
//
//    // ボスの通常のロジック（移動、攻撃など）はここに実装
//    // 現状は死亡アニメーションのテストのため省略
//    MV1SetPosition(m_modelHandle, m_pos);
//
//    CheckHitAndDamage(bullets);
//    if (tackleInfo.isTackling)
//    {
//        // タックルダメージ処理
//        TakeTackleDamage(tackleInfo.damage);
//    }
//}
//
//void EnemyBoss::Draw()
//{
//    if (!m_isAlive)
//    {
//        float currentAnimTotalTime = m_animationManager.GetAnimationTotalTime(m_modelHandle, kDeadAnimName);
//        if (m_animTime < currentAnimTotalTime)
//        {
//            MV1DrawModel(m_modelHandle);
//        }
//        return;
//    }
//    MV1DrawModel(m_modelHandle);
//}
//
//void EnemyBoss::ChangeAnimation(AnimState newAnimState, bool loop)
//{
//    if (m_currentAnimState == newAnimState)
//    {
//        return;
//    }
//
//    const char* animName = nullptr;
//    switch (newAnimState)
//    {
//    case AnimState::Idle:
//        // 仮のIdleアニメーション名
//        animName = "IDLE"; 
//        break;
//    case AnimState::Dead:
//        animName = kDeadAnimName;
//        break;
//    }
//
//    if (animName)
//    {
//        m_animationManager.PlayAnimation(m_modelHandle, animName, loop);
//        m_animTime = 0.0f;
//    }
//    m_currentAnimState = newAnimState;
//}
//
//void EnemyBoss::TakeDamage(float damage)
//{
//    EnemyBase::TakeDamage(damage);
//    if (m_hp <= 0.0f)
//    {
//        if (m_lastHitPart == HitPart::None) m_lastHitPart = HitPart::Body;
//        bool isHeadShot = (m_lastHitPart == HitPart::Head);
//        int addScore = ScoreManager::Instance().AddScore(isHeadShot);
//        if (SceneMain::Instance()) 
//        {
//            SceneMain::Instance()->AddScorePopup(addScore, isHeadShot, ScoreManager::Instance().GetCombo());
//        }
//    }
//}
//
//void EnemyBoss::TakeTackleDamage(float damage)
//{
//    EnemyBase::TakeTackleDamage(damage);
//}