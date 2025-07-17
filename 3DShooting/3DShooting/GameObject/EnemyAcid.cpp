#include "EnemyAcid.h"    
#include "Player.h"      
#include "DxLib.h"       
#include "DebugUtil.h"   
#include "SphereCollider.h" 
#include "CapsuleCollider.h" 
#include "SceneMain.h"
#include <cassert>       
#include <algorithm>     
#include <cmath> 
#include <functional>    

namespace
{
    // アニメーション関連
    constexpr char kAttackAnimName[] = "Armature|ATK";  // 攻撃アニメーション
    constexpr char kWalkAnimName[]   = "Armature|WALK"; // 歩くアニメーション
    constexpr char kBackAnimName[]   = "Armature|BACK"; // 後退アニメーション
    constexpr char kDeadAnimName[]   = "Armature|DEAD"; // 死亡アニメーション

    constexpr VECTOR kInitialPosition = { -50.0f, -30.0f, 300.0f };  // 初期位置
    constexpr VECTOR kHeadShotPositionOffset = { 0.0f, 0.0f, 0.0f }; // オフセット

    // コライダーのサイズを定義
    constexpr float kBodyColliderRadius = 40.0f;  // 体のコライダー半径
    constexpr float kBodyColliderHeight = 50.0f;  // 体のコライダー高さ
    constexpr float kHeadRadius = 18.0f;          // 頭のコライダー半径

    // 体力
    constexpr float kInitialHP = 150.0f; // 初期HP 

    // 攻撃関連（遠距離攻撃に特化）
    constexpr int   kAttackCooldownMax = 160;     // 攻撃クールダウン時間
    constexpr float kAttackPower       = 30.0f;   // 攻撃力
    constexpr float kAttackRangeRadius = 1000.0f; // 攻撃範囲の半径
    constexpr float kAcidBulletSpeed   = 10.0f;   // 酸弾の速度
    constexpr float kAcidBulletRadius  = 10.0f;   // 酸弾の半径
    constexpr int   kAttackEndDelay    = 30;      // 攻撃後の硬直時間

    // 追跡関連（遠距離型なので、近づきすぎたら離れる）
    constexpr float kChaseSpeed = 2.0f;   // 追跡速度
    constexpr float kOptimalAttackDistanceMin = 500.0f; // 攻撃可能最小距離
}

EnemyAcid::EnemyAcid() :
    m_headPosOffset{ kHeadShotPositionOffset },
    m_animTime(0.0f),
    m_currentAnimState(AnimState::Walk),
    m_onDropItem(nullptr),
    m_hasAttacked(false),
    m_attackEndDelayTimer(0),
    m_acidBulletSpawnOffset({ 0.0f, 100.0f, 0.0f }),
	m_backAnimCount(0),
	m_isItemDropped(false)
{
    m_modelHandle = MV1LoadModel("data/model/AcidZombie.mv1");
    assert(m_modelHandle != -1);

    // コライダーの初期化
    m_pBodyCollider = std::make_shared<CapsuleCollider>();
    m_pHeadCollider = std::make_shared<SphereCollider>();
    m_pAttackRangeCollider = std::make_shared<SphereCollider>();

    // AnimationManagerにアニメーション名を登録
    m_animationManager.SetAnimName(AnimState::Attack, kAttackAnimName);
    m_animationManager.SetAnimName(AnimState::Walk, kWalkAnimName);
    m_animationManager.SetAnimName(AnimState::Dead, kDeadAnimName);
    m_animationManager.SetAnimName(AnimState::Back, kBackAnimName);
}

EnemyAcid::~EnemyAcid()
{
    MV1DeleteModel(m_modelHandle);
}

void EnemyAcid::Init()
{
    m_hp = kInitialHP;
    m_attackPower = kAttackPower;
    m_attackCooldownMax = kAttackCooldownMax;
    m_attackCooldown = 0; // 最初は攻撃可能にしておく

    m_isAlive = true;
    m_isDeadAnimPlaying = false;
    m_isItemDropped = false;

    // ここで一度「絶対にRunでない値」にリセット
    m_currentAnimState = AnimState::Dead; // 初期アニメーションを強制的に再生させるため

    // 初期化時に歩行アニメーションを開始
    ChangeAnimation(AnimState::Walk, true); 
}

// アニメーションを変更する
void EnemyAcid::ChangeAnimation(AnimState newAnimState, bool loop)
{
    // 後退アニメーションは同じ状態でも必ず再生し直す
    if (m_currentAnimState == newAnimState)
    {
        if (newAnimState == AnimState::Attack || newAnimState == AnimState::Back)
        {
            m_animationManager.PlayAnimation(m_modelHandle,
                (newAnimState == AnimState::Attack) ? kAttackAnimName : kBackAnimName,
                loop);
            m_animTime = 0.0f;
            if (newAnimState == AnimState::Attack) m_hasAttacked = false;
            if (newAnimState == AnimState::Back) m_backAnimCount = 0;
        }
        return;
    }

    const char* animName = nullptr;
    switch (newAnimState)
    {
    case AnimState::Walk:   
        animName = kWalkAnimName; 
        break;
    case AnimState::Attack: 
        animName = kAttackAnimName; 
        break;
    case AnimState::Dead:   
        animName = kDeadAnimName; 
        break;
    case AnimState::Back:   
        animName = kBackAnimName; 
        break;
    default:
        return;
    }

    if (animName)
    {
        m_animationManager.PlayAnimation(m_modelHandle, animName, loop);
        m_animTime = 0.0f;
        if (newAnimState == AnimState::Attack) m_hasAttacked = false;
        if (newAnimState == AnimState::Back) m_backAnimCount = 0; 
    }

    m_currentAnimState = newAnimState;
}

// プレイヤーに攻撃可能かどうかを判定
bool EnemyAcid::CanAttackPlayer(const Player& player)
{
    VECTOR playerPos = player.GetPos();
    m_pAttackRangeCollider->SetCenter(m_pos);
    m_pAttackRangeCollider->SetRadius(kAttackRangeRadius);

    // プレイヤーのボディコライダーを取得
    std::shared_ptr<CapsuleCollider> playerBodyCollider = player.GetBodyCollider();

    // プレイヤーが攻撃範囲内にいるかどうかのみ判定
    return m_pAttackRangeCollider->Intersects(playerBodyCollider.get());
}

// 酸を吐く攻撃を行う
void EnemyAcid::ShootAcidBullet(std::vector<Bullet>& bullets, const Player& player)
{
    // 発射位置
    int mouthIndex = MV1SearchFrame(m_modelHandle, "mixamorig:Head");
    VECTOR spawnPos = m_pos;
    if (mouthIndex != -1) 
    {
        spawnPos = MV1GetFramePosition(m_modelHandle, mouthIndex);
    }
    spawnPos = VAdd(spawnPos, m_acidBulletSpawnOffset);

    // プレイヤーの位置
    VECTOR target = player.GetPos();

    // 放物線の初速計算
    VECTOR toTarget = VSub(target, spawnPos);
    VECTOR flat = toTarget; flat.y = 0.0f;
    float dist = VSize(flat);
    float speed = 10.0f; 
    float t = dist / speed;
    float gravity = 0.4f;
    float vy = (toTarget.y + 0.5f * gravity * t * t) / t;
    VECTOR vel = VScale(VNorm(flat), speed);
    vel.y = vy;

    AcidBall ball;
    ball.pos = spawnPos;
    ball.dir = vel;
    ball.active = true;
    m_acidBalls.push_back(ball);
}

void EnemyAcid::Update(std::vector<Bullet>& bullets, const Player::TackleInfo& tackleInfo, const Player& player, const std::vector<EnemyBase*>& enemyList)
{
    if (m_hp <= 0.0f) 
    {
        if (!m_isDeadAnimPlaying) 
        {
            // スコア加算処理はTakeDamageで行うのでここでは不要
            ChangeAnimation(AnimState::Dead, false);
            m_isDeadAnimPlaying = true;
            m_animTime = 0.0f; // アニメーション時間をリセット
        }
        
        // 死亡アニメーション中もアニメーション時間を更新
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
            // アイテムドロップと死亡コールバックを呼び出し
            if (!m_isItemDropped && m_onDropItem)
            {
                m_onDropItem(m_pos);
                m_onDropItem = nullptr;
                m_isItemDropped = true;
            }
            if (m_onDeathCallback) 
            {
                m_onDeathCallback(m_pos);
                m_onDeathCallback = nullptr; // 一度だけ呼び出す
            }
            m_isAlive = false; // 死亡アニメーション終了時のみfalseにする
        } 
        else 
        {
            m_isAlive = true; // 死亡アニメーション中はtrueのまま
        }
        return;
    }

    MV1SetPosition(m_modelHandle, m_pos);

    // プレイヤーの方向を向く
    VECTOR playerPos = player.GetPos();
    VECTOR toPlayer = VSub(playerPos, m_pos);
    toPlayer.y = 0.0f;
    float disToPlayer = sqrtf(VSquareSize(toPlayer));
    float yaw = 0.0f;
    if (toPlayer.x != 0.0f || toPlayer.z != 0.0f)
    {
        yaw = atan2f(toPlayer.x, toPlayer.z);
        yaw += DX_PI_F;
    }
    MV1SetRotationXYZ(m_modelHandle, VGet(0.0f, yaw, 0.0f));

    // コライダーの更新
    int hipsIndex = MV1SearchFrame(m_modelHandle, "mixamorig:Hips");
    VECTOR hipsPos = (hipsIndex != -1) ? MV1GetFramePosition(m_modelHandle, hipsIndex) : m_pos;

    VECTOR bodySegmentP1 = VAdd(hipsPos, VGet(0, ::kBodyColliderHeight / 2.0f, 0));
    VECTOR bodySegmentP2 = VAdd(hipsPos, VGet(0, -::kBodyColliderHeight / 2.0f, 0));
    m_pBodyCollider->SetSegment(bodySegmentP1, bodySegmentP2);
    m_pBodyCollider->SetRadius(::kBodyColliderRadius);

	// ヘッドの位置を取得
    int headIndex = MV1SearchFrame(m_modelHandle, "mixamorig:Head");

	// 頭の位置を取得してヘッドコライダーの中心を設定
    VECTOR headModelPos = (headIndex != -1) ? MV1GetFramePosition(m_modelHandle, headIndex) : VAdd(m_pos, m_headPosOffset);
    VECTOR headCenter = VAdd(headModelPos, m_headPosOffset);
    m_pHeadCollider->SetCenter(headCenter);
    m_pHeadCollider->SetRadius(kHeadRadius);

	// 攻撃範囲のコライダーを更新
    VECTOR attackRangeCenter = m_pos;
    attackRangeCenter.y += (::kBodyColliderHeight * 0.5f);
    m_pAttackRangeCollider->SetCenter(attackRangeCenter);
    m_pAttackRangeCollider->SetRadius(kAttackRangeRadius);

    // プレイヤーとの物理衝突判定
    std::shared_ptr<CapsuleCollider> playerBodyCollider = player.GetBodyCollider();
    if (m_pBodyCollider->Intersects(playerBodyCollider.get()))
    {
        VECTOR enemyCenter  = VScale(VAdd(m_pBodyCollider->GetSegmentA(), m_pBodyCollider->GetSegmentB()), 0.5f);
        VECTOR playerCenter = VScale(VAdd(playerBodyCollider->GetSegmentA(), playerBodyCollider->GetSegmentB()), 0.5f);
        VECTOR diff         = VSub(enemyCenter, playerCenter);

        float distSq  = VDot(diff, diff);
        float minDist = ::kBodyColliderRadius + playerBodyCollider->GetRadius();

        // 0.0001fはゼロ除算回避のための閾値
		if (distSq < minDist * minDist && distSq > 0.0001f) 
        {
            float dist = std::sqrt(distSq);
            float pushBack = minDist - dist;

            if (dist > 0)
            {
                VECTOR pushDir = VSub(enemyCenter, playerCenter);
                pushDir.y = 0.0f;
                float horizontalDistSq = VDot(pushDir, pushDir);
                if (horizontalDistSq > 0.0001f)
                {
                    pushDir = VNorm(pushDir);
                    m_pos = VAdd(m_pos, VScale(pushDir, pushBack * 0.5f));
                }
            }
        }
    }

    // プレイヤーが攻撃範囲内か
    bool inAttackRange = m_pAttackRangeCollider->Intersects(playerBodyCollider.get());

    // 攻撃アニメーション中・硬直中は移動や状態遷移を行わない
    if (m_currentAnimState == AnimState::Attack || m_attackEndDelayTimer > 0) 
    {
        // 攻撃アニメーション・硬直中は移動・状態遷移を行わない
    }
    else if (inAttackRange) 
    {
        if (disToPlayer < kOptimalAttackDistanceMin)
        {
            // 最小攻撃距離未満なら後退
            if (m_currentAnimState != AnimState::Back)
            {
                ChangeAnimation(AnimState::Back, true);
            }
            VECTOR dirAway = VNorm(VSub(m_pos, playerPos));
            m_pos.x += dirAway.x * kChaseSpeed;
            m_pos.z += dirAway.z * kChaseSpeed;
        }
        else
        {
            // 攻撃可能距離なら攻撃
            if (m_attackCooldown == 0)
            {
                ChangeAnimation(AnimState::Attack, false);
                m_hasAttacked = false;
                m_attackCooldown = m_attackCooldownMax;
            }
        }
    }
    else 
    {
        // 攻撃範囲外なら追跡
        if (m_currentAnimState != AnimState::Walk)
        {
            ChangeAnimation(AnimState::Walk, true);
        }
        VECTOR dirTowards = VNorm(VSub(playerPos, m_pos));
        m_pos.x += dirTowards.x * kChaseSpeed;
        m_pos.z += dirTowards.z * kChaseSpeed;
    }

    // 攻撃アニメーション中の酸弾発射タイミング
    if (m_currentAnimState == AnimState::Attack)
    {
        float totalAttackAnimTime = m_animationManager.GetAnimationTotalTime(m_modelHandle, kAttackAnimName);
        if (!m_hasAttacked && m_animTime >= totalAttackAnimTime * 0.3f)
        {
            ShootAcidBullet(bullets, player);
            m_hasAttacked = true;
        }
        if (m_animTime >= totalAttackAnimTime)
        {
            m_attackEndDelayTimer = 20;
            m_animTime = 0.0f; // ここでアニメーション時間をリセット
            if (m_animationManager.GetCurrentAttachedAnimHandle(m_modelHandle) != -1)
            {
                MV1DetachAnim(m_modelHandle, 0);
                m_animationManager.ResetAttachedAnimHandle(m_modelHandle);
            }
        }
    }

    // アニメーション時間の更新
    if (m_attackEndDelayTimer == 0)
    {
        m_animTime += 1.0f;
        float animTotal = 0.0f;
        if (m_currentAnimState == AnimState::Back)
        {
            animTotal = m_animationManager.GetAnimationTotalTime(m_modelHandle, kBackAnimName);
        }
        else if (m_currentAnimState == AnimState::Walk)
        {
            animTotal = m_animationManager.GetAnimationTotalTime(m_modelHandle, kWalkAnimName);
        }
        else if (m_currentAnimState == AnimState::Attack)
        {
            animTotal = m_animationManager.GetAnimationTotalTime(m_modelHandle, kAttackAnimName);
        }
        else if (m_currentAnimState == AnimState::Dead)
        {
            animTotal = m_animationManager.GetAnimationTotalTime(m_modelHandle, kDeadAnimName);
        }

        // Back・Walkアニメーションはループ
        if ((m_currentAnimState == AnimState::Back || m_currentAnimState == AnimState::Walk) && animTotal > 0.0f)
        {
            if (m_animTime >= animTotal)
            {
                m_animTime = 0.0f;
            }
        }
        m_animationManager.UpdateAnimationTime(m_modelHandle, m_animTime);
    }

    // 弾との当たり判定・ダメージ処理
    CheckHitAndDamage(bullets);

    // タックルダメージ処理
    if (tackleInfo.isTackling && tackleInfo.tackleId != m_lastTackleId)
    {
        CapsuleCollider playerTackleCollider(tackleInfo.capA, tackleInfo.capB, tackleInfo.radius);
        if (m_pBodyCollider->Intersects(&playerTackleCollider))
        {
            TakeTackleDamage(tackleInfo.damage);
            m_lastTackleId = tackleInfo.tackleId;
        }
    }
    else if (!tackleInfo.isTackling)
    {
        ResetTackleHitFlag();
        m_lastTackleId = -1;
    }

    if (m_hitDisplayTimer > 0)
    {
        m_hitDisplayTimer--;
    }

    // AcidBallの更新とプレイヤーへの当たり判定
    for (auto& ball : m_acidBalls)
    {
        if (!ball.active) continue;
        ball.Update();
        std::shared_ptr<CapsuleCollider> playerCol = player.GetBodyCollider();
        SphereCollider acidCol(ball.pos, ball.radius);
        if (acidCol.Intersects(playerCol.get()))
        {
            const_cast<Player&>(player).TakeDamage(ball.damage);
            ball.active = false;
        }
    }

    // 攻撃クールダウンと攻撃後硬直の減算処理を追加
    if (m_attackCooldown > 0)
    {
        m_attackCooldown--;
    }
    if (m_attackEndDelayTimer > 0) 
    {
        m_attackEndDelayTimer--;
        if (m_attackEndDelayTimer == 0 && m_currentAnimState != AnimState::Walk)
        {
            ChangeAnimation(AnimState::Walk, true);
        }
    }
}

void EnemyAcid::Draw()
{
    // 死亡時も死亡アニメーションが終わるまでは描画する
    if (!m_isAlive)
    {
        float currentAnimTotalTime = m_animationManager.GetAnimationTotalTime(m_modelHandle, kDeadAnimName);
        if (m_animTime < currentAnimTotalTime)
        {
            MV1DrawModel(m_modelHandle);
        }
        return;
    }

    if (m_hp <= 0.0f && m_animationManager.IsAnimationFinished(m_modelHandle))
    {
        // 死亡アニメーションが完全に終了したらモデルを描画しない
        return;
    }

    // AcidBallの描画
    for (const auto& ball : m_acidBalls) 
    {
        if (!ball.active) continue;
        DrawSphere3D(ball.pos, ball.radius, 8, 0x00ff00, 0x008800, true);
    }

    MV1DrawModel(m_modelHandle);


#ifdef _DEBUG
    // デバッグ用の当たり判定描画
    DrawCollisionDebug();

    // 体力デバッグ表示
    DebugUtil::DrawFormat(20, 100, 0x008800, "Acid HP: %.1f", m_hp);
#endif
}

void EnemyAcid::DrawCollisionDebug() const
{
#ifdef _DEBUG
    if (m_pBodyCollider)
    {
        DebugUtil::DrawCapsule(m_pBodyCollider->GetSegmentA(), m_pBodyCollider->GetSegmentB(), m_pBodyCollider->GetRadius(), 16, 0xff00ff);
    }
    if (m_pHeadCollider)
    {
        DebugUtil::DrawSphere(m_pHeadCollider->GetCenter(), m_pHeadCollider->GetRadius(), 16, 0xffff00);
    }
    if (m_pAttackRangeCollider)
    {
        DebugUtil::DrawSphere(m_pAttackRangeCollider->GetCenter(), m_pAttackRangeCollider->GetRadius(), 16, 0x00ffff);
    }
#endif
}

EnemyBase::HitPart EnemyAcid::CheckHitPart(const VECTOR& rayStart, const VECTOR& rayEnd, VECTOR& outHtPos, float& outHtDistSq) const
{
    VECTOR hitPosHead, hitPosBody;
    float hitDistSqHead = FLT_MAX;
    float hitDistSqBody = FLT_MAX;

    // 頭のフレーム位置を取得してコライダー中心に設定
    int headIndex = MV1SearchFrame(m_modelHandle, "mixamorig:Head");
    VECTOR headCenter = (headIndex != -1) ? MV1GetFramePosition(m_modelHandle, headIndex) : VAdd(m_pos, m_headPosOffset);
    m_pHeadCollider->SetCenter(headCenter);
    m_pHeadCollider->SetRadius(kHeadRadius);

    // 体のコライダーはカプセルなので、m_posの上下にオフセットした端点を設定
    // モデルのHipsフレームの位置を取得してボディコライダーの基点とする
    int hipsIndex = MV1SearchFrame(m_modelHandle, "mixamorig:Hips");
    VECTOR hipsPos = (hipsIndex != -1) ? MV1GetFramePosition(m_modelHandle, hipsIndex) : m_pos;

    VECTOR bodySegmentP1 = VAdd(hipsPos, VGet(0, ::kBodyColliderHeight / 2.0f, 0));
    VECTOR bodySegmentP2 = VAdd(hipsPos, VGet(0, -::kBodyColliderHeight / 2.0f, 0));
    m_pBodyCollider->SetSegment(bodySegmentP1, bodySegmentP2);
    m_pBodyCollider->SetRadius(::kBodyColliderRadius);

    bool headHit = m_pHeadCollider->IntersectsRay(rayStart, rayEnd, hitPosHead, hitDistSqHead);
    bool bodyHit = m_pBodyCollider->IntersectsRay(rayStart, rayEnd, hitPosBody, hitDistSqBody);

    if (headHit && bodyHit)
    {
        if (hitDistSqHead <= hitDistSqBody)
        {
            outHtPos = hitPosHead;
            outHtDistSq = hitDistSqHead;
            return HitPart::Head;
        }
        else
        {
            outHtPos = hitPosBody;
            outHtDistSq = hitDistSqBody;
            return HitPart::Body;
        }
    }
    else if (headHit)
    {
        outHtPos = hitPosHead;
        outHtDistSq = hitDistSqHead;
        return HitPart::Head;
    }
    else if (bodyHit)
    {
        outHtPos = hitPosBody;
        outHtDistSq = hitDistSqBody;
        return HitPart::Body;
    }

    outHtPos = VGet(0, 0, 0);
    outHtDistSq = FLT_MAX;
    return HitPart::None;
}

float EnemyAcid::CalcDamage(float bulletDamage, HitPart part) const
{
    float damage = bulletDamage;
    switch (part)
    {
    case HitPart::Head:
        damage *= 2.0f; // ヘッドショットは2倍ダメージ
        break;
    case HitPart::Body:
        damage *= 1.0f; // ボディは等倍ダメージ
        break;
    default:
        break;
    }
    return damage;
}

void EnemyAcid::SetOnDropItemCallback(std::function<void(const VECTOR&)> cb)
{
    m_onDropItem = cb;
}

void EnemyAcid::SetModelHandle(int handle)
{
    if (m_modelHandle != -1) MV1DeleteModel(m_modelHandle);
    m_modelHandle = MV1DuplicateModel(handle);
}

void EnemyAcid::TakeDamage(float damage)
{
    m_hp -= damage;
    if (m_hp <= 0.0f) // 死亡時一度だけ
    {
        m_hp = 0.0f;
        m_isAlive = false;
        if (m_lastHitPart == HitPart::None) m_lastHitPart = HitPart::Body;
        bool isHeadShot = (m_lastHitPart == HitPart::Head);
        int addScore = ScoreManager::Instance().AddScore(isHeadShot);
        if (SceneMain::Instance()) {
            SceneMain::Instance()->AddScorePopup(addScore, isHeadShot, ScoreManager::Instance().GetCombo());
        }
    }
}