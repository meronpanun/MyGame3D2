#include "EnemyBoss.h"
#include "Bullet.h"
#include "EffekseerForDXLib.h"
#include "DebugUtil.h"
#include "SceneMain.h"
#include "ScoreManager.h"
#include "SphereCollider.h"
#include "CapsuleCollider.h"
#include "TransformDataLoader.h" // Added
#include <cassert>
#include <algorithm>
#include <cmath>

namespace
{
    // アニメーション名
    constexpr char kIdleAnimName[]        = "IDLE"; 
    constexpr char kWalkAnimName[]        = "WALK"; 
    constexpr char kCloseAttackAnimName[] = "Armature|CloseRangeAttack"; // 近接範囲攻撃
    constexpr char kDeadAnimName[]        = "DEAD";

    // kDefaultInitialHP and kChaseSpeed are no longer needed as defaults if CSV is primary, but good as fallback.
    constexpr float kDefaultInitialHP = 5000.0f; // ボスの初期体力
    constexpr float kChaseSpeed       = 1.5f;    // 追跡速度

    // コライダーサイズ
    constexpr float kBodyColliderRadius = 40.0f;
    constexpr float kBodyColliderHeight = 200.0f;
    constexpr float kHeadRadius         = 20.0f;
    constexpr float kAttackRangeRadius  = 150.0f; // 指定された近接範囲
    constexpr float kAttackHitRadius    = 60.0f;  // 攻撃自体の当たり判定

    constexpr int kAttackCooldownMax = 60;
    constexpr int kAttackEndDelay    = 30; // 攻撃後の硬直
}

EnemyBoss::EnemyBoss() :
    m_currentAnimState(AnimState::Idle),
    m_isDeadAnimPlaying(false),
    m_animTime(0.0f),
    m_chaseSpeed(kChaseSpeed),
    m_attackEndDelayTimer(0),
    m_isAttackHit(false)
{
    m_modelHandle = MV1LoadModel("data/model/Boss.mv1");
    // モデル読み込み失敗時はアサート
    // assert(m_modelHandle != -1); 
    // ※実機でアサートすると止まるので、エラーログを出して続行制御などは本来必要
    if (m_modelHandle == -1)
    {
        // 読み込み失敗時のフォールバックやエラー処理
        // ここでは一旦そのままスルー（描画されないだけ）
    }

    // コライダー初期化
    m_pBodyCollider        = std::make_shared<CapsuleCollider>();
    m_pHeadCollider        = std::make_shared<SphereCollider>();
    m_pAttackRangeCollider = std::make_shared<SphereCollider>();
    m_pAttackHitCollider   = std::make_shared<CapsuleCollider>();
}

EnemyBoss::~EnemyBoss()
{
    MV1DeleteModel(m_modelHandle);
}

void EnemyBoss::Init()
{
    m_hp = kDefaultInitialHP;
    m_chaseSpeed = kChaseSpeed;

    // CSVからデータをロード
    auto dataList = TransformDataLoader::LoadDataCSV("data/CSV/CharacterTransfromData.csv");
    for (const auto& data : dataList)
    {
        if (data.name == "Boss")
        {
            MV1SetRotationXYZ(m_modelHandle, data.rot);
            MV1SetScale(m_modelHandle, data.scale);
            m_attackPower = data.attack; // EnemyBaseにm_attackPowerがある前提
            m_hp = data.hp;
            m_chaseSpeed = data.chaseSpeed; // データ定義にあるspeedを使用
            break;
        }
    }

    m_isAlive = true;
    m_isDeadAnimPlaying = false;
    m_animTime = 0.0f;
    
    // 位置はWaveManagerでセットされるが、初期値として
    m_pos = VGet(0.0f, 0.0f, 1000.0f); 
    MV1SetPosition(m_modelHandle, m_pos);
    
    // 攻撃範囲コライダー設定
    m_pAttackRangeCollider->SetRadius(kAttackRangeRadius);

    ChangeAnimation(AnimState::Walk, true); // 最初は歩いて近づく
}

void EnemyBoss::Update(std::vector<Bullet>& bullets, const Player::TackleInfo& tackleInfo, const Player& player, const std::vector<EnemyBase*>& enemyList, const std::vector<Stage::StageCollisionData>& collisionData, Effect* pEffect)
{
    UpdateStageCollision(collisionData);

    if (m_hp <= 0.0f) 
    {
        if (!m_isDeadAnimPlaying) 
        {
            ChangeAnimation(AnimState::Dead, false);
            m_isDeadAnimPlaying = true;
            m_animTime = 0.0f;
            m_isAlive = true; 
        }
        
        if (m_animationManager.GetCurrentAttachedAnimHandle(m_modelHandle) != -1)
        {
            m_animTime += 1.0f;
            m_animationManager.UpdateAnimationTime(m_modelHandle, m_animTime);
        }
        
        float currentAnimTotalTime = m_animationManager.GetAnimationTotalTime(m_modelHandle, kDeadAnimName);
        if (m_animTime >= currentAnimTotalTime) 
        {
            if (m_animationManager.GetCurrentAttachedAnimHandle(m_modelHandle) != -1) 
            {
                MV1DetachAnim(m_modelHandle, 0);
                m_animationManager.ResetAttachedAnimHandle(m_modelHandle);
            }
            if (m_onDeathCallback) 
            {
                m_onDeathCallback(m_pos);
                m_onDeathCallback = nullptr;
            }
            m_isAlive = false; 
            SetActive(false);
        } 
        return;
    }

    MV1SetPosition(m_modelHandle, m_pos);

    // プレイヤーの位置・コライダー
    VECTOR playerPos = player.GetPos();
    std::shared_ptr<CapsuleCollider> playerBodyCollider = player.GetBodyCollider();

    // 攻撃範囲コライダー更新
    VECTOR attackRangeCenter = m_pos;
    attackRangeCenter.y += (kBodyColliderHeight * 0.5f);
    m_pAttackRangeCollider->SetCenter(attackRangeCenter);

    // 状態遷移ロジック
    if (m_currentAnimState == AnimState::Attack)
    {
        // 攻撃中
        float currentAnimTotalTime = m_animationManager.GetAnimationTotalTime(m_modelHandle, kCloseAttackAnimName);
        
        // 攻撃アニメーション終了判定
        if (m_animTime > currentAnimTotalTime)
        {
             if (m_attackEndDelayTimer <= 0)
             {
                 m_attackEndDelayTimer = kAttackEndDelay;
             }
        }

        if (m_attackEndDelayTimer > 0)
        {
            --m_attackEndDelayTimer;
            if (m_attackEndDelayTimer == 0)
            {
                m_isAttackHit = false;
                // 攻撃終了後、範囲内にいれば再度攻撃、いなければ移動へ
                if (CanAttackPlayer(player))
                {
                    ChangeAnimation(AnimState::Attack, false);
                }
                else
                {
                    ChangeAnimation(AnimState::Walk, true);
                }
            }
        }
    }
    else // Idle or Walk
    {
        // 移動処理 (Walk)
        if (m_currentAnimState == AnimState::Walk)
        {
             VECTOR toPlayer = VSub(playerPos, m_pos);
             toPlayer.y = 0.0f;
             float disToPlayer = VSize(toPlayer);

             // 向き変更
             if (disToPlayer > 0.1f)
             {
                 float yaw = atan2f(toPlayer.x, toPlayer.z);
                 yaw += DX_PI_F; // モデルの向き補正が必要なら調整
                 MV1SetRotationXYZ(m_modelHandle, VGet(0.0f, yaw, 0.0f));
             }

             // 攻撃範囲に入ったら攻撃へ遷移
             if (CanAttackPlayer(player))
             {
                 m_isAttackHit = false;
                 ChangeAnimation(AnimState::Attack, false);
             }
             else
             {
                 // 範囲外なら近づく
                 VECTOR dir = VNorm(toPlayer);
                 // 攻撃範囲手前くらいまで近づくイメージだが、ここではシンプルに近づく
                 // 押し出し処理があるので重なってもある程度大丈夫
                 m_pos = VAdd(m_pos, VScale(dir, m_chaseSpeed));
             }
        }
    }

    // アニメーション更新
    if (m_animationManager.GetCurrentAttachedAnimHandle(m_modelHandle) != -1) 
    {
        m_animTime += 1.0f;
        const char* animName = nullptr;
        if (m_currentAnimState == AnimState::Walk) animName = kWalkAnimName;
        else if (m_currentAnimState == AnimState::Attack) animName = kCloseAttackAnimName;
        else if (m_currentAnimState == AnimState::Dead) animName = kDeadAnimName;
        else animName = kIdleAnimName;

        if (animName)
        {
            float totalTime = m_animationManager.GetAnimationTotalTime(m_modelHandle, animName);
            // ループ処理
            if (m_currentAnimState == AnimState::Walk || m_currentAnimState == AnimState::Idle)
            {
                m_animTime = fmodf(m_animTime, totalTime);
            }
            m_animationManager.UpdateAnimationTime(m_modelHandle, m_animTime);
        }
    }

    // コライダー更新(Body)
    VECTOR bodyCapA = VAdd(m_pos, VGet(0, kBodyColliderRadius, 0));
    VECTOR bodyCapB = VAdd(m_pos, VGet(0, kBodyColliderHeight - kBodyColliderRadius, 0));
    m_pBodyCollider->SetSegment(bodyCapA, bodyCapB);
    m_pBodyCollider->SetRadius(kBodyColliderRadius);

    // コライダー更新(Head)
    int headIndex = MV1SearchFrame(m_modelHandle, "mixamorig:HeadTop_End");
    if (headIndex != -1)
    {
        VECTOR headPos = MV1GetFramePosition(m_modelHandle, headIndex);
        m_pHeadCollider->SetCenter(headPos);
        m_pHeadCollider->SetRadius(kHeadRadius);
    }
    else
    {
        // 見つからない場合は"Head"で再検索
        headIndex = MV1SearchFrame(m_modelHandle, "Head");
        if (headIndex != -1)
        {
             VECTOR headPos = MV1GetFramePosition(m_modelHandle, headIndex);
             m_pHeadCollider->SetCenter(headPos);
             m_pHeadCollider->SetRadius(kHeadRadius);
        }
        else
        {
             // 頭が見つからない場合のフォールバック（体の上の方）
             m_pHeadCollider->SetCenter(VAdd(m_pos, VGet(0, kBodyColliderHeight, 0)));
             m_pHeadCollider->SetRadius(kHeadRadius);
        }
    }


    // プレイヤーとの押し出しなど
    if (m_pBodyCollider->IsIntersects(playerBodyCollider.get()))
    {
         // 簡易押し出し
         VECTOR diff = VSub(m_pos, player.GetPos());
         diff.y = 0.0f;
         if (VSize(diff) > 0.001f)
         {
             VECTOR pushDir = VNorm(diff);
             float pushDist = 2.0f; // 適当な押し出し係数
             m_pos = VAdd(m_pos, VScale(pushDir, pushDist));
         }
    }


    // 攻撃判定発生
    if (m_currentAnimState == AnimState::Attack && !m_isAttackHit)
    {
        // アニメーションの特定タイミングでのみヒット判定を出す
        float totalTime = m_animationManager.GetAnimationTotalTime(m_modelHandle, kCloseAttackAnimName);
        // 判定期間を少し広げる or 調整
        // 攻撃の出始めから終わり際まで判定があっても良いならそうするが、
        // 以前のロジック(20%~70%)を維持する
        if (m_animTime > totalTime * 0.4f && m_animTime < totalTime * 0.6f)
        {
            // ユーザー要望：デバッグで表示している範囲内（m_pAttackRangeCollider）はすべてダメージが入るように
            // 範囲攻撃なので、攻撃範囲コライダーとプレイヤーが接触していればヒットとする
            if (m_pAttackRangeCollider->IsIntersects(playerBodyCollider.get()))
            {
                 // ダメージを与える (CSVから読み込んだ攻撃力を使用)
                 const_cast<Player&>(player).TakeDamage(static_cast<float>(m_attackPower), m_pos); 
                 m_isAttackHit = true;
                 
                 // デバッグ用ヒット表示も更新しておく（青色表示用）
                 // カプセルとして設定（球として扱う）
                 m_pAttackHitCollider->SetSegment(m_pAttackRangeCollider->GetCenter(), m_pAttackRangeCollider->GetCenter());
                 m_pAttackHitCollider->SetRadius(m_pAttackRangeCollider->GetRadius());
            }
        }
    }

    CheckHitAndDamage(bullets, pEffect);
    
    // タックル判定
    if (tackleInfo.isTackling && m_hp > 0.0f)
    {
        if (!m_isTackleHit) // まだヒットしてない場合のみ
        {
             CapsuleCollider tackleCol(tackleInfo.capA, tackleInfo.capB, tackleInfo.radius);
             if (m_pBodyCollider->IsIntersects(&tackleCol))
             {
                 TakeTackleDamage(tackleInfo.damage);
                 // m_isTackleHit = true; // TakeTackleDamage内でフラグ管理されるかもだが念のため
                 // EnemyNormalでは m_lastTackleId で管理していたのでそちらに合わせるのもありだが、
                 // Baseクラスの仕様に従う
             }
        }
    }
}

void EnemyBoss::Draw()
{
    if (!m_isAlive && m_animationManager.GetCurrentAttachedAnimHandle(m_modelHandle) == -1) return;

    MV1DrawModel(m_modelHandle);

#ifdef _DEBUG
    DrawCollisionDebug();
#endif
}

void EnemyBoss::ChangeAnimation(AnimState newAnimState, bool loop)
{
    if (m_currentAnimState == newAnimState && newAnimState != AnimState::Attack)
    {
        return;
    }

    const char* animName = nullptr;
    switch (newAnimState)
    {
    case AnimState::Idle:
        animName = kIdleAnimName;
        break;
    case AnimState::Walk:
        animName = kWalkAnimName;
        break;
    case AnimState::Attack:
        animName = kCloseAttackAnimName;
        break;
    case AnimState::Dead:
        animName = kDeadAnimName;
        break;
    }

    if (animName)
    {
        m_animationManager.PlayAnimation(m_modelHandle, animName, loop);
        m_animTime = 0.0f;
    }
    m_currentAnimState = newAnimState;
}

void EnemyBoss::TakeDamage(float damage, AttackType type)
{
    EnemyBase::TakeDamage(damage, type);
    
    // UIへのスコア加算などの演出はEnemyNormal同様にあれば追加
    if (m_hp <= 0.0f)
    {
        bool isHeadShot = (m_lastHitPart == HitPart::Head);
        int addScore = ScoreManager::Instance().AddScore(isHeadShot) * 10; // ボスなのでスコア高め
        if (SceneMain::Instance()) 
        {
            SceneMain::Instance()->AddScorePopup(addScore, isHeadShot, ScoreManager::Instance().GetCombo());
        }
    }
}

void EnemyBoss::TakeTackleDamage(float damage)
{
    EnemyBase::TakeTackleDamage(damage);
}

std::shared_ptr<CapsuleCollider> EnemyBoss::GetBodyCollider() const
{
    return m_pBodyCollider;
}

float EnemyBoss::CalcDamage(float bulletDamage, HitPart part) const
{
    // ボスは硬い、あるいは弱点だけ効くなどの調整が可能
    if (part == HitPart::Head)
    {
        return bulletDamage * 1.5f; 
    }
    return bulletDamage * 0.8f; // ボディは少し硬い
}

void EnemyBoss::DrawCollisionDebug() const
{
    // 体
    if (m_pBodyCollider)
        DebugUtil::DrawCapsule(m_pBodyCollider->GetSegmentA(), m_pBodyCollider->GetSegmentB(), m_pBodyCollider->GetRadius(), 16, 0xff0000); // 赤
    
    // 頭
    if (m_pHeadCollider)
        DebugUtil::DrawSphere(m_pHeadCollider->GetCenter(), m_pHeadCollider->GetRadius(), 16, 0x00ff00); // 緑

    // 攻撃範囲（指定されたデバッグ表示）
    // 黄色やオレンジでわかりやすく
    if (m_pAttackRangeCollider)
        DebugUtil::DrawSphere(m_pAttackRangeCollider->GetCenter(), m_pAttackRangeCollider->GetRadius(), 32, 0xffaa00); 

    // 攻撃判定（攻撃中のみ青などで）
    if (m_currentAnimState == AnimState::Attack && m_pAttackHitCollider)
    {
         DebugUtil::DrawCapsule(m_pAttackHitCollider->GetSegmentA(), m_pAttackHitCollider->GetSegmentB(), m_pAttackHitCollider->GetRadius(), 16, 0x0000ff);
    }
}

bool EnemyBoss::CanAttackPlayer(const Player& player)
{
    // 攻撃範囲コライダー内にプレイヤーがいるか
    auto playerCol = player.GetBodyCollider();
    return m_pAttackRangeCollider->IsIntersects(playerCol.get());
}

EnemyBase::HitPart EnemyBoss::CheckHitPart(const VECTOR& rayStart, const VECTOR& rayEnd, VECTOR& outHtPos, float& outHtDistSq) const
{
    HitPart part = HitPart::None;
    float minDistSq = FLT_MAX;
    VECTOR hitPos = VGet(0, 0, 0);

    // 頭との判定
    if (m_pHeadCollider)
    {
        VECTOR tmpHitPos;
        float tmpDistSq;
        if (m_pHeadCollider->IsIsIntersectsRay(rayStart, rayEnd, tmpHitPos, tmpDistSq))
        {
            if (tmpDistSq < minDistSq)
            {
                minDistSq = tmpDistSq;
                hitPos = tmpHitPos;
                part = HitPart::Head;
            }
        }
    }

    // 体との判定
    if (m_pBodyCollider)
    {
        VECTOR tmpHitPos;
        float tmpDistSq;
        if (m_pBodyCollider->IsIsIntersectsRay(rayStart, rayEnd, tmpHitPos, tmpDistSq))
        {
            if (tmpDistSq < minDistSq)
            {
                minDistSq = tmpDistSq;
                hitPos = tmpHitPos;
                part = HitPart::Body;
            }
        }
    }

    if (part != HitPart::None)
    {
        outHtPos = hitPos;
        outHtDistSq = minDistSq;
    }

    return part;
}
