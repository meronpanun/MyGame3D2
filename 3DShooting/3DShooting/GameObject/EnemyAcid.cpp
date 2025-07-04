#include "EnemyAcid.h"    
#include "Player.h"      
#include "DxLib.h"       
#include "DebugUtil.h"   
#include "SphereCollider.h" 
#include "CapsuleCollider.h" 
#include <cassert>       
#include <algorithm>     
#include <cmath> 
#include <functional>    

namespace
{
    // アニメーション関連
    constexpr char kAttackAnimName[] = "Armature|ATK"; // 攻撃アニメーション
    constexpr char kRunAnimName[] = "Armature|WALE";    // 歩くアニメーション (RunnerではRun, NormalではWalk)
    constexpr char kBackAnimName[] = "Armature|BACK";  // 後退アニメーション (新しく追加)
    constexpr char kDeadAnimName[] = "Armature|DEAD";  // 死亡アニメーション

    constexpr VECTOR kInitialPosition = { -50.0f, -30.0f, 300.0f }; // 初期位置
    constexpr VECTOR kHeadShotPositionOffset = { 0.0f, 0.0f, 0.0f }; // オフセット

    // コライダーのサイズを定義
    constexpr float kBodyColliderRadius = 18.0f;  // 体のコライダー半径
    constexpr float kBodyColliderHeight = 130.0f; // 体のコライダー高さ
    constexpr float kHeadRadius = 16.0f;  // 頭のコライダー半径

    // 体力
    constexpr float kInitialHP = 150.0f; // 初期HP

    // 攻撃関連 (遠距離攻撃に特化)
    constexpr int   kAttackCooldownMax = 90;     // 攻撃クールダウン時間
    constexpr float kAttackPower = 30.0f;  // 攻撃力 (酸のダメージ)
    constexpr float kAttackRangeRadius = 400.0f; // 攻撃範囲の半径 (広め)
    constexpr float kAcidBulletSpeed = 10.0f;  // 酸弾の速度
    constexpr float kAcidBulletRadius = 10.0f;  // 酸弾の半径
    constexpr int   kAttackEndDelay = 30;     // 攻撃後の硬直時間

    // 追跡関連 (遠距離型なので、近づきすぎたら離れる)
    constexpr float kChaseSpeed = 1.5f;   // 追跡速度
    constexpr float kOptimalAttackDistanceMin = 200.0f; // 攻撃可能最小距離
    constexpr float kOptimalAttackDistanceMax = 350.0f; // 攻撃可能最大距離
}

EnemyAcid::EnemyAcid() :
    m_headPosOffset{ kHeadShotPositionOffset },
    m_animTime(0.0f),
    m_currentAnimState(AnimState::Run),
    m_onDropItem(nullptr),
    m_hasAttacked(false),
    m_attackEndDelayTimer(0),
    m_acidBulletSpawnOffset({ 0.0f, 100.0f, 0.0f })
{
    m_modelHandle = MV1LoadModel("data/model/AcidZombie.mv1");
    assert(m_modelHandle != -1);

    // コライダーの初期化
    m_pBodyCollider = std::make_shared<CapsuleCollider>();
    m_pHeadCollider = std::make_shared<SphereCollider>();
    m_pAttackRangeCollider = std::make_shared<SphereCollider>();

    // AnimationManagerにアニメーション名を登録
    m_animationManager.SetAnimName(AnimState::Attack, kAttackAnimName);
    m_animationManager.SetAnimName(AnimState::Run, kRunAnimName);
    m_animationManager.SetAnimName(AnimState::Dead, kDeadAnimName);
    m_animationManager.SetAnimName(AnimState::Walk, kBackAnimName); // BackアニメーションをWalkとして登録
}

EnemyAcid::~EnemyAcid()
{
    MV1DeleteModel(m_modelHandle);
}

void EnemyAcid::Init()
{
    m_hp = kInitialHP;
    m_pos = kInitialPosition;
    m_attackPower = kAttackPower;
    m_attackCooldownMax = kAttackCooldownMax;
    m_attackCooldown = 0; // 最初は攻撃可能にしておく

    // ここで一度「絶対にRunでない値」にリセット
    m_currentAnimState = AnimState::Dead; // 初期アニメーションを強制的に再生させるため

    ChangeAnimation(AnimState::Run, true); // 初期化時に走行アニメーションを開始
}

void EnemyAcid::ChangeAnimation(AnimState newAnimState, bool loop)
{
    if (m_currentAnimState == newAnimState)
    {
        // 攻撃アニメーションは同じ状態でもリセットして再生し直す
        if (newAnimState == AnimState::Attack)
        {
            m_animationManager.PlayAnimation(m_modelHandle, kAttackAnimName, loop);
            m_animTime = 0.0f;
            m_hasAttacked = false;
        }
        return;
    }

    const char* animName = nullptr;

    switch (newAnimState)
    {
    case AnimState::Run:
        animName = kRunAnimName;
        break;
    case AnimState::Attack:
        animName = kAttackAnimName;
        break;
    case AnimState::Dead:
        animName = kDeadAnimName;
        break;
    case AnimState::Walk: // 後退アニメーション用
        animName = kBackAnimName;
        break;
    default:
        // 未定義のアニメーション状態
        return;
    }

    if (animName)
    {
        m_animationManager.PlayAnimation(m_modelHandle, animName, loop);
        m_animTime = 0.0f;
        if (newAnimState == AnimState::Attack)
            m_hasAttacked = false;
    }

    m_currentAnimState = newAnimState;
}

bool EnemyAcid::CanAttackPlayer(const Player& player)
{
    VECTOR playerPos = player.GetPos();
    m_pAttackRangeCollider->SetCenter(m_pos);
    m_pAttackRangeCollider->SetRadius(kAttackRangeRadius);

    // プレイヤーのボディコライダーを取得
    std::shared_ptr<CapsuleCollider> playerBodyCollider = player.GetBodyCollider();

    // プレイヤーとの距離が攻撃範囲内か、かつ最適攻撃距離の範囲内かをチェック
    VECTOR toPlayer = VSub(playerPos, m_pos);
    float disToPlayer = sqrtf(VSquareSize(toPlayer)); // Modified: Replaced VLen with sqrtf(VSquareSize)

    return m_pAttackRangeCollider->Intersects(playerBodyCollider.get()) &&
        disToPlayer >= kOptimalAttackDistanceMin &&
        disToPlayer <= kOptimalAttackDistanceMax;
}

void EnemyAcid::ShootAcidBullet(std::vector<Bullet>& bullets, const Player& player)
{
    // 発射位置
    int mouthIndex = MV1SearchFrame(m_modelHandle, "mixamorig:Head");
    VECTOR spawnPos = m_pos;
    if (mouthIndex != -1) {
        spawnPos = MV1GetFramePosition(m_modelHandle, mouthIndex);
    }
    spawnPos = VAdd(spawnPos, m_acidBulletSpawnOffset);

    // プレイヤーの位置
    VECTOR target = player.GetPos();

    // 放物線の初速計算
    VECTOR toTarget = VSub(target, spawnPos);
    VECTOR flat = toTarget; flat.y = 0.0f;
    float dist = VSize(flat);
    float speed = 10.0f; // kAcidBulletSpeed
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

void EnemyAcid::Update(std::vector<Bullet>& bullets, const Player::TackleInfo& tackleInfo, const Player& player)
{
    if (m_hp <= 0.0f)
    {
        // 死亡アニメーション
        if (m_currentAnimState != AnimState::Dead)
        {
            ChangeAnimation(AnimState::Dead, false); // 死亡アニメーションは非ループ
        }
        float currentAnimTotalTime = m_animationManager.GetAnimationTotalTime(m_modelHandle, kDeadAnimName);

        // 死亡アニメーションが終了したら
        if (m_animTime >= currentAnimTotalTime)
        {
            // アニメーションが完全に終了したらモデルを非表示にするためにデタッチ
            if (m_animationManager.GetCurrentAttachedAnimHandle(m_modelHandle) != -1)
            {
                MV1DetachAnim(m_modelHandle, 0); // 死亡アニメーションが終了したらデタッチ
                m_animationManager.ResetAttachedAnimHandle(m_modelHandle); // AnimationManagerの内部状態も更新
            }

            if (m_onDropItem)
            {
                m_onDropItem(m_pos);    // アイテムドロップコールバックを呼び出す
                m_onDropItem = nullptr; // アイテムドロップ後はコールバックを無効化
            }
            return; // 死亡アニメーション終了後は更新しない
        }
    }

    MV1SetPosition(m_modelHandle, m_pos); // モデルの位置は常に反映する

    // プレイヤーの方向を常に向く
    VECTOR playerPos = player.GetPos();
    VECTOR toPlayer = VSub(playerPos, m_pos);
    toPlayer.y = 0.0f; // Y成分を無視して水平距離を計算
    float disToPlayer = sqrtf(VSquareSize(toPlayer)); // Modified: Replaced VLen with sqrtf(VSquareSize)

    float yaw = 0.0f;
    if (toPlayer.x != 0.0f || toPlayer.z != 0.0f)
    {
        yaw = atan2f(toPlayer.x, toPlayer.z);
        yaw += DX_PI_F; // モデルの向きに合わせて調整
    }
    MV1SetRotationXYZ(m_modelHandle, VGet(0.0f, yaw, 0.0f));

    // 攻撃クールダウンの更新
    if (m_attackCooldown > 0)
    {
        m_attackCooldown--;
    }

    // 攻撃後の硬直タイマーの更新
    if (m_attackEndDelayTimer > 0)
    {
        m_attackEndDelayTimer--;
        if (m_attackEndDelayTimer == 0)
        {
            // 硬直が終わったらRunアニメーションに戻す
            ChangeAnimation(AnimState::Run, true);
        }
    }

    // 状態遷移ロジック
    if (m_currentAnimState != AnimState::Attack && m_attackEndDelayTimer == 0)
    {
        if (CanAttackPlayer(player) && m_attackCooldown == 0)
        {
            // 攻撃可能範囲内でクールダウンも終わっていたら攻撃
            ChangeAnimation(AnimState::Attack, false); // 攻撃アニメーションは非ループ
            m_hasAttacked = false; // 攻撃フラグをリセット
            m_attackCooldown = m_attackCooldownMax; // クールダウン開始
        }
        else if (disToPlayer < kOptimalAttackDistanceMin)
        {
            // プレイヤーが近すぎたら後退
            if (m_currentAnimState != AnimState::Walk) // Walkを後退として使用
            {
                ChangeAnimation(AnimState::Walk, true);
            }
            VECTOR dirAway = VNorm(VSub(m_pos, playerPos)); // プレイヤーから離れる方向
            m_pos.x += dirAway.x * kChaseSpeed;
            m_pos.z += dirAway.z * kChaseSpeed;
        }
        else if (disToPlayer > kOptimalAttackDistanceMax)
        {
            // プレイヤーが遠すぎたら近づく
            if (m_currentAnimState != AnimState::Run)
            {
                ChangeAnimation(AnimState::Run, true);
            }
            VECTOR dirTowards = VNorm(toPlayer); // プレイヤーに向かう方向
            m_pos.x += dirTowards.x * kChaseSpeed;
            m_pos.z += dirTowards.z * kChaseSpeed;
        }
        else
        {
            // 最適な攻撃距離にいる場合は待機 (Runアニメーションを継続)
            if (m_currentAnimState != AnimState::Run)
            {
                ChangeAnimation(AnimState::Run, true);
            }
        }
    }

    // 攻撃アニメーション中の弾の発射タイミング
    if (m_currentAnimState == AnimState::Attack)
    {
        // 攻撃アニメーションの特定のフレームで弾を発射 (例: アニメーション時間の半分くらい)
        float totalAttackAnimTime = m_animationManager.GetAnimationTotalTime(m_modelHandle, kAttackAnimName);
        if (!m_hasAttacked && m_animTime >= totalAttackAnimTime * 0.5f) // アニメーションの半分の時間で発射
        {
            ShootAcidBullet(bullets, player);
            m_hasAttacked = true;
        }

        // 攻撃アニメーションが終了したら硬直タイマーをセット
        if (m_animTime >= totalAttackAnimTime)
        {
            m_attackEndDelayTimer = kAttackEndDelay;
            // 攻撃アニメーションが終了したら、次の状態は硬直なのでアニメーションをデタッチ
            if (m_animationManager.GetCurrentAttachedAnimHandle(m_modelHandle) != -1)
            {
                MV1DetachAnim(m_modelHandle, 0);
                m_animationManager.ResetAttachedAnimHandle(m_modelHandle);
            }
            // その後、UpdateAnimationTimeが呼ばれないようにする（硬直中はアニメーション停止）
        }
    }

    // アニメーション時間の更新
    if (m_attackEndDelayTimer == 0) // 硬直中でない場合のみアニメーションを更新
    {
        m_animTime += 1.0f; // 1フレームごとに時間を進める (DxLibのアニメーション時間単位に合わせて調整)
        m_animationManager.UpdateAnimationTime(m_modelHandle, m_animTime);
    }

    // 弾との当たり判定とダメージ処理は基底クラスで共通
    CheckHitAndDamage(bullets);

    // タックルダメージ処理
    if (tackleInfo.isTackling && tackleInfo.tackleId != m_lastTackleId)
    {
        // タックルを受けた場合の処理
        TakeTackleDamage(tackleInfo.damage);
        m_lastTackleId = tackleInfo.tackleId; // 最新のタックルIDを記録
        m_isTackleHit = true;
    }
    else if (!tackleInfo.isTackling)
    {
        // タックルヒットが継続していない場合はフラグをリセット
        ResetTackleHitFlag();
        m_lastTackleId = -1;
    }

    // ヒット表示タイマーの更新
    if (m_hitDisplayTimer > 0)
    {
        m_hitDisplayTimer--;
    }

    // AcidBallの更新と当たり判定
    for (auto& ball : m_acidBalls)
    {
        if (!ball.active) continue;
        ball.Update();

        // プレイヤーとの当たり判定
        std::shared_ptr<CapsuleCollider> playerCol = player.GetBodyCollider();
        SphereCollider acidCol(ball.pos, ball.radius);
        if (acidCol.Intersects(playerCol.get())) {
            const_cast<Player&>(player).TakeDamage(ball.damage);
            ball.active = false;
        }
    }
}

void EnemyAcid::Draw()
{
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

    // デバッグ用の当たり判定描画
    DrawCollisionDebug();
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

    // 体のコライダーはカプセルなので、適当なオフセットで上下の端点を設定
    VECTOR bodySegmentP1 = VAdd(m_pos, VGet(0, kBodyColliderHeight / 2.0f, 0));
    VECTOR bodySegmentP2 = VAdd(m_pos, VGet(0, -kBodyColliderHeight / 2.0f, 0));
    m_pBodyCollider->SetSegment(bodySegmentP1, bodySegmentP2);
    m_pBodyCollider->SetRadius(kBodyColliderRadius);

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