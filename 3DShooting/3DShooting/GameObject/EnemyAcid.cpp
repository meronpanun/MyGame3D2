#include "EnemyAcid.h"
#include "Bullet.h"
#include "CapsuleCollider.h"
#include "Collision.h"
#include "CollisionGrid.h"
#include "DebugUtil.h"
#include "DxLib.h"
#include "Effect.h"
#include "EffekseerWarningSuppress.h"
#include "Game.h"
#include "Player.h"
#include "SceneMain.h"
#include "SphereCollider.h"
#include "Stage.h"
#include "TaskTutorialManager.h"
#include "TransformDataLoader.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>
#include "SoundManager.h"
#include "EnemyAcidState.h"

int EnemyAcid::s_modelHandle = -1;
bool EnemyAcid::s_shouldDrawCollision = false;

EnemyAcid::EnemyAcid()
    : m_headPosOffset{EnemyAcidConstants::kHeadShotPositionOffset}
    , m_animTime(0.0f)
    , m_currentAnimState(AnimState::Walk)
    , m_onDropItem(nullptr)
    , m_hasAttacked(false)
    , m_attackEndDelayTimer(0)
    , m_acidBulletSpawnOffset({0.0f, 0.0f, 0.0f})
    , m_backAnimCount(0)
    , m_hasDroppedItem(false)
    , m_isStunned(false)
    , m_stunTimer(0)
    , m_distToPlayer(0.0f)
    , m_isDeadAnimPlaying(false)
    , m_isNextAttackNormal(false)
    , m_shouldDrawParryCollider(false)
    , m_debugParryCapA(VGet(0,0,0))
    , m_debugParryCapB(VGet(0,0,0))
    , m_debugParryRadius(0.0f)

{
    // モデルの複製
    m_modelHandle = MV1DuplicateModel(s_modelHandle);

    // コライダーの初期化
    m_pBodyCollider = std::make_shared<CapsuleCollider>();
    m_pHeadCollider = std::make_shared<SphereCollider>();
    m_pAttackRangeCollider = std::make_shared<SphereCollider>();

  
    // AnimationManagerにアニメーション名を登録
    m_animationManager.SetAnimName(AnimState::Attack, EnemyAcidConstants::kAttackAnimName);
    m_animationManager.SetAnimName(AnimState::Walk, EnemyAcidConstants::kWalkAnimName);
    m_animationManager.SetAnimName(AnimState::Dead, EnemyAcidConstants::kDeadAnimName);
    m_animationManager.SetAnimName(AnimState::Back, EnemyAcidConstants::kBackAnimName);
}

EnemyAcid::~EnemyAcid()
{
    // モデルの解放
    MV1DeleteModel(m_modelHandle);

    for (auto& ball : m_acidBalls)
    {
        if (ball.effectHandle != -1)
        {
            StopEffekseer3DEffect(ball.effectHandle);
        }
    }
}

void EnemyAcid::LoadModel()
{
    s_modelHandle = MV1LoadModel("data/model/AcidZombie.mv1");
    assert(s_modelHandle != -1);
}

void EnemyAcid::DeleteModel()
{
    MV1DeleteModel(s_modelHandle);
}

void EnemyAcid::Init()
{
  m_attackCooldownMax = EnemyAcidConstants::kAttackCooldownMax;
  m_attackCooldown = 0; // 最初は攻撃可能にしておく

  m_isAlive = true;
  m_isDeadAnimPlaying = false;

  // CSVからAcidEnemyのTransform情報を取得
  LoadTransformData("AcidEnemy");

    // ChangeAnimation は同じステートを受け取ると再生をスキップするため、
    // 初期アニメーションを確実に再生させるために現在と異なる値で初期化しておく
    m_currentAnimState = AnimState::Dead;

  // 初期ステートの設定
  ChangeState(std::make_shared<EnemyAcidStateWalk>());

  m_isNextAttackNormal = false; // 最初はパリィ弾から

    // ターゲットオフセットの初期化
    float offsetX = static_cast<float>(GetRand(EnemyAcidConstants::kTargetOffsetRange * 2) - EnemyAcidConstants::kTargetOffsetRange);
    float offsetZ = static_cast<float>(GetRand(EnemyAcidConstants::kTargetOffsetRange * 2) - EnemyAcidConstants::kTargetOffsetRange);
  m_targetOffset = VGet(offsetX, 0.0f, offsetZ);
}

void EnemyAcid::ChangeAnimation(AnimState newAnimState, bool loop)
{
  // 後退アニメーションは同じ状態でも必ず再生し直す
    if (m_currentAnimState == newAnimState)
    {
        if (newAnimState == AnimState::Attack || newAnimState == AnimState::Back)
        {
            m_animationManager.PlayAnimation(m_modelHandle, (newAnimState == AnimState::Attack) ? EnemyAcidConstants::kAttackAnimName : EnemyAcidConstants::kBackAnimName, loop);
            m_animTime = 0.0f;
            if (newAnimState == AnimState::Attack)
            {
                m_hasAttacked = false;

                // 攻撃ボイスを距離に応じた音量で再生
                float volRatio = 1.0f - (m_distToPlayer / EnemyAcidConstants::kSoundMaxDistance);
                if (volRatio < 0.0f) volRatio = 0.0f;
                if (volRatio > 1.0f) volRatio = 1.0f;

                SoundManager::GetInstance()->Play("EnemyAcid", "Attack", (int)(EnemyAcidConstants::kSoundMaxVolume * volRatio));
            }
            if (newAnimState == AnimState::Back) m_backAnimCount = 0;
        }
        return;
    }

    const char* animName = nullptr;
    switch (newAnimState)
    {
    case AnimState::Walk:
        animName = EnemyAcidConstants::kWalkAnimName;
        break;
      case AnimState::Attack:
        animName = EnemyAcidConstants::kAttackAnimName;
        break;
      case AnimState::Dead:
        animName = EnemyAcidConstants::kDeadAnimName;
        break;
    case AnimState::Back:
        animName = EnemyAcidConstants::kBackAnimName;
        break;
    default:
        return;
    }

    if (animName)
    {
        m_animationManager.PlayAnimation(m_modelHandle, animName, loop);
        m_animTime = 0.0f;
        if (newAnimState == AnimState::Attack)
        {
            m_hasAttacked = false;

            // 攻撃ボイスを距離に応じた音量で再生
            float volRatio = 1.0f - (m_distToPlayer / EnemyAcidConstants::kSoundMaxDistance);
            if (volRatio < 0.0f) volRatio = 0.0f;
            if (volRatio > 1.0f) volRatio = 1.0f;

            SoundManager::GetInstance()->Play("EnemyAcid", "Attack", (int)(EnemyAcidConstants::kSoundMaxVolume * volRatio));
        }
        if (newAnimState == AnimState::Back) m_backAnimCount = 0;
    }

    m_currentAnimState = newAnimState;
}

void EnemyAcid::ChangeState(std::shared_ptr<EnemyState<EnemyAcid>> newState)
{
    if (m_pCurrentState)
    {
        m_pCurrentState->Exit(this);
    }
    m_pCurrentState = newState;
    if (m_pCurrentState)
    {
        m_pCurrentState->Enter(this);
    }
}

bool EnemyAcid::CanAttackPlayer(const Player &player)
{
    VECTOR playerPos = player.GetPos();
    m_pAttackRangeCollider->SetCenter(m_pos);
    m_pAttackRangeCollider->SetRadius(EnemyAcidConstants::kAttackRangeRadius);

    // プレイヤーのボディコライダーを取得
    std::shared_ptr<CapsuleCollider> playerBodyCollider = player.GetBodyCollider();

    // プレイヤーが攻撃範囲内にいるかどうかのみ判定
    return m_pAttackRangeCollider->IsIntersects(playerBodyCollider.get());
}

void EnemyAcid::ShootAcidBullet(std::vector<Bullet> &bullets, const Player &player, Effect *pEffect, const std::vector<Stage::StageCollisionData> &stageCollision, const CollisionGrid *pGrid)
{
    // 発射位置
    int mouthIndex = MV1SearchFrame(m_modelHandle, "mixamorig:JawDowm");
    VECTOR spawnPos = m_pos;

    if (mouthIndex != -1) 
    {
        spawnPos = MV1GetFramePosition(m_modelHandle, mouthIndex);
    }
    spawnPos = VAdd(spawnPos, m_acidBulletSpawnOffset);
  
    // プレイヤーの位置
    VECTOR target = player.GetPos();

    // 直線的な攻撃処理
    VECTOR toTarget = VSub(target, spawnPos);

    AcidBall ball;
    ball.pos = spawnPos;

    // デフォルト設定
    ball.active = true;
    ball.damage = m_attackPower;
    ball.owner = this;
    ball.isReflected = false;
    ball.isParabolic = false;
    ball.gravity = 0.0f;
    ball.speed = EnemyAcidConstants::kAcidBulletSpeed;
    ball.dir = VNorm(toTarget);

    // 放物線攻撃判定
    std::string groundedObj = player.GetGroundedObjectName();

    // 発射基準位置（頭）
    int headIndex = MV1SearchFrame(m_modelHandle, "mixamorig:Head");
    VECTOR viewStartPos = (headIndex != -1) ? MV1GetFramePosition(m_modelHandle, headIndex) : VAdd(m_pos, m_headPosOffset);

    if ((groundedObj == "Rock3" || groundedObj == "Rock6") && !EnemyBase::IsTargetVisible(viewStartPos, player.GetPos(), stageCollision, pGrid)) 
    {
        ball.isParabolic = true;
        ball.gravity = EnemyAcidConstants::kParabolicGravity;
        ball.velocity = EnemyBase::CalculateParabolicVelocity(ball.pos, target, ball.gravity, EnemyAcidConstants::kAcidBulletSpeed);
    }

  
    if (m_isNextAttackNormal) 
    {
        // 通常弾 (パリィ不可)
        ball.isParryable = false;
        if (pEffect) 
        {
            ball.effectHandle = pEffect->PlayNormalBulletEffect(spawnPos.x, spawnPos.y, spawnPos.z);
        }
  
    }
    else 
    {
        // パリィ弾 (パリィ可能)
        ball.isParryable = true;
        if (pEffect) 
        {
            ball.effectHandle = pEffect->PlayAcidEffect(spawnPos.x, spawnPos.y, spawnPos.z);
        }
    }
    // 次回のためにフラグ反転
    m_isNextAttackNormal = !m_isNextAttackNormal;

    m_acidBalls.push_back(ball);
}

void EnemyAcid::Update(const EnemyUpdateContext &context)
{
    UpdateStandard(context);
}

void EnemyAcid::UpdateAI(const EnemyUpdateContext &context) 
{
#ifdef _DEBUG
    m_shouldDrawParryCollider = false;
#endif

    // 酸弾の更新・当たり判定（EnemyAcidState.cpp で実装）
    UpdateAcidBalls(context);

    if (!m_shouldUpdateAI) return;

    // 怯み状態の処理（EnemyAcidState.cpp で実装）
    if (m_isStunned) 
    {
        m_stunTimer -= m_aiUpdateInterval;
        if (m_stunTimer <= 0) 
        {
            m_isStunned = false;
            if (m_hp > 0.0f)
            {
                if (CanAttackPlayer(context.player)) ChangeState(std::make_shared<EnemyAcidStateAttack>());
                else ChangeState(std::make_shared<EnemyAcidStateWalk>());
            }
        }
        return;
    }

    // AIステートの更新
    if (m_pCurrentState)
    {
        m_pCurrentState->Update(this, context);
    }

    // 移動・攻撃AI（EnemyAcidState.cpp で実装）
    UpdateMovementAI(context);
}


void EnemyAcid::UpdateAnimation(const EnemyUpdateContext &context) 
{
    // コライダーの更新
    int hipsIndex = MV1SearchFrame(m_modelHandle, "mixamorig:Hips");
    VECTOR hipsPos = (hipsIndex != -1) ? MV1GetFramePosition(m_modelHandle, hipsIndex) : m_pos;
    VECTOR bodySegmentP1 = VAdd(hipsPos, VGet(0, EnemyAcidConstants::kBodyColliderHeight * 0.5f, 0));
    VECTOR bodySegmentP2 = VAdd(hipsPos, VGet(0, -EnemyAcidConstants::kBodyColliderHeight * 0.5f, 0));
    m_pBodyCollider->SetSegment(bodySegmentP1, bodySegmentP2);
    m_pBodyCollider->SetRadius(EnemyAcidConstants::kBodyColliderRadius);

    int headIndex = MV1SearchFrame(m_modelHandle, "mixamorig:Head");
    VECTOR headModelPos = (headIndex != -1) ? MV1GetFramePosition(m_modelHandle, headIndex) : VAdd(m_pos, m_headPosOffset);
    m_pHeadCollider->SetCenter(VAdd(headModelPos, m_headPosOffset));
    m_pHeadCollider->SetRadius(EnemyAcidConstants::kHeadRadius);

    VECTOR attackRangeCenter = m_pos;
    attackRangeCenter.y += (EnemyAcidConstants::kBodyColliderHeight * 0.5f);
    m_pAttackRangeCollider->SetCenter(attackRangeCenter);
    m_pAttackRangeCollider->SetRadius(EnemyAcidConstants::kAttackRangeRadius);

    // アニメーション時間の更新
    if (m_attackEndDelayTimer == 0 || m_isStunned) 
    {
        m_animTime += (1.0f * m_aiUpdateInterval) * Game::GetTimeScale();
        if (m_isStunned)
        {
            if (m_animTime > EnemyAcidConstants::kStunAnimFrameLimit) m_animTime = EnemyAcidConstants::kStunAnimFrameLimit;
        }
        else
        {
            const char* animName = nullptr;
            if (m_currentAnimState == AnimState::Back) animName = EnemyAcidConstants::kBackAnimName;
            else if (m_currentAnimState == AnimState::Walk) animName = EnemyAcidConstants::kWalkAnimName;
            else if (m_currentAnimState == AnimState::Attack) animName = EnemyAcidConstants::kAttackAnimName;
            else if (m_currentAnimState == AnimState::Dead) animName = EnemyAcidConstants::kDeadAnimName;

            float animTotal = m_animationManager.GetAnimationTotalTime(m_modelHandle, animName);
            if ((m_currentAnimState == AnimState::Back || m_currentAnimState == AnimState::Walk) && animTotal > 0.0f) 
            {
                if (m_animTime >= animTotal) m_animTime = fmodf(m_animTime, animTotal);
            }
            else if (m_animTime >= animTotal) m_animTime = animTotal;
        }
        m_animationManager.UpdateAnimationTime(m_modelHandle, m_animTime);
    }

    m_distToPlayer = VSize(VSub(context.player.GetPos(), m_pos));
}

void EnemyAcid::UpdateSimpleMode(const EnemyUpdateContext &context) 
{
    if (m_hp > 0.0f) 
    {
        VECTOR playerPos = context.player.GetPos();
        VECTOR targetPos = VAdd(playerPos, m_targetOffset);
        VECTOR dirTowards = VNorm(VSub(targetPos, m_pos));
        float scaledSpeed = m_chaseSpeed * Game::GetTimeScale();
        m_pos.x += dirTowards.x * scaledSpeed;
        m_pos.z += dirTowards.z * scaledSpeed;
        RotateTowards(playerPos, EnemyAcidConstants::kRotateSpeedPerFrame * Game::GetTimeScale());
    }
    UpdateStageCollision(context.collisionData, context.collisionGrid);
}

void EnemyAcid::OnParried()
{
    // 既に怯んでいるか、死んでいる場合は何もしない
    if (m_isStunned || m_hp <= 0.0f) return;

    m_isStunned = true;
    m_stunTimer = EnemyAcidConstants::kStunDuration; // 怯み時間
    ChangeState(std::make_shared<EnemyAcidStateStunned>());

    // パリィボイスを距離に応じた音量で再生
    float volRatio = 1.0f - (m_distToPlayer / EnemyAcidConstants::kSoundMaxDistance);
    if (volRatio < 0.0f) volRatio = 0.0f;
    if (volRatio > 1.0f) volRatio = 1.0f;

    SoundManager::GetInstance()->Play("EnemyAcid", "ParryHit", (int)(EnemyAcidConstants::kSoundMaxVolume * volRatio));
}

void EnemyAcid::Draw()
{
    // 死亡時も死亡アニメーションが終わるまでは描画する
    if (!m_isAlive) 
    {
        float currentAnimTotalTime = m_animationManager.GetAnimationTotalTime(m_modelHandle, EnemyAcidConstants::kDeadAnimName);
        if (m_animTime < currentAnimTotalTime) 
        {
            MV1DrawModel(m_modelHandle);
        }
        return;
    }

    // 死亡アニメーションが完全に終了したらモデルを描画しない
    if (m_hp <= 0.0f && m_animationManager.IsAnimationFinished(m_modelHandle)) return;

    // 視錐台カリング（描画最適化）
    if (!ShouldDraw(EnemyAcidConstants::kDrawDistanceSq, EnemyAcidConstants::kDrawNearDistanceSq, EnemyAcidConstants::kDrawDotThreshold)) return;

    EnemyBase::IncrementDrawCount();
    MV1DrawModel(m_modelHandle);

    // デバッグ用の当たり判定描画
    if (s_shouldDrawCollision) 
    {
        DrawCollisionDebug();
    }

#ifdef _DEBUG
    // 体力デバッグ表示
    DebugUtil::DrawFormat(20, 100, 0xffffff, "EnemyAcid HP: %.1f", m_hp);
#endif
}

void EnemyAcid::DrawCollisionDebug() const
{

    if (!s_shouldDrawCollision) return;
  
    if (m_pBodyCollider)
    {
        DebugUtil::DrawCapsule(m_pBodyCollider->GetSegmentA(), m_pBodyCollider->GetSegmentB(), m_pBodyCollider->GetRadius(), EnemyAcidConstants::kDebugSphereDiv, EnemyAcidConstants::kDebugBodyColor);
    }
    if (m_pHeadCollider)
    {
        DebugUtil::DrawSphere(m_pHeadCollider->GetCenter(), m_pHeadCollider->GetRadius(), EnemyAcidConstants::kDebugSphereDiv, EnemyAcidConstants::kDebugHeadColor);
    }
    if (m_pAttackRangeCollider)
    {
        DebugUtil::DrawSphere(m_pAttackRangeCollider->GetCenter(), m_pAttackRangeCollider->GetRadius(), EnemyAcidConstants::kDebugSphereDiv, EnemyAcidConstants::kDebugAttackRangeColor);
    }
    if (m_shouldDrawParryCollider)
    {
        DebugUtil::DrawCapsule(m_debugParryCapA, m_debugParryCapB, m_debugParryRadius, EnemyAcidConstants::kDebugSphereDiv, EnemyAcidConstants::kDebugParryColor);
    }
}

EnemyBase::HitPart EnemyAcid::CheckHitPart(const VECTOR &rayStart, const VECTOR &rayEnd, VECTOR &outHtPos, float &outHtDistSq) const
{
    if (m_isDeadAnimPlaying) return HitPart::None;

    VECTOR hitPosHead, hitPosBody; // 当たった位置
    float hitDistSqHead = FLT_MAX;
    float hitDistSqBody = FLT_MAX;

    // 頭のフレーム位置を取得してコライダー中心に設定
    int headIndex = MV1SearchFrame(m_modelHandle, "mixamorig:Head");
    VECTOR headCenter = (headIndex != -1) ? MV1GetFramePosition(m_modelHandle, headIndex) : VAdd(m_pos, m_headPosOffset);
  
    m_pHeadCollider->SetCenter(headCenter);
    m_pHeadCollider->SetRadius(EnemyAcidConstants::kHeadRadius);

    bool headHit = m_pHeadCollider->IsIsIntersectsRay(rayStart, rayEnd, hitPosHead, hitDistSqHead);
    bool bodyHit = m_pBodyCollider->IsIsIntersectsRay(rayStart, rayEnd, hitPosBody, hitDistSqBody);

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

void EnemyAcid::SetOnDropItemCallback(std::function<void(const VECTOR &)> cb)
{
    m_onDropItem = cb;
}

void EnemyAcid::TakeDamage(float damage, AttackType type)
{
    if (m_isDeadAnimPlaying) return;

    EnemyBase::TakeDamage(damage, type);
  
}
  
void EnemyAcid::OnDeath()
{
    // スコア加算
    if (m_lastHitPart == HitPart::None) m_lastHitPart = HitPart::Body;
    bool isHeadShot = (m_lastHitPart == HitPart::Head);
    int addScore = ScoreManager::Instance().AddScore(isHeadShot);

    if (SceneMain::Instance()) 
    {
        SceneMain::Instance()->AddScorePopup(addScore, isHeadShot, ScoreManager::Instance().GetCombo());
    }

    // 死亡時に残っているAcidBallを全て停止
    for (auto& ball : m_acidBalls)
    {
        if (ball.effectHandle != -1)
        {
            StopEffekseer3DEffect(ball.effectHandle);
            ball.effectHandle = -1;
        }
    }
    m_acidBalls.clear(); // 全てのAcidBallをクリア
}

void EnemyAcid::TakeTackleDamage(float damage)
{
    if (m_isDeadAnimPlaying) return;
  
    EnemyBase::TakeTackleDamage(damage);
}

std::shared_ptr<CapsuleCollider> EnemyAcid::GetBodyCollider() const 
{
    return m_pBodyCollider;
}

void EnemyAcid::AcidBall::Update(float timeScale)
{
    if (!active) return;

    if (isParabolic) 
    {
        // 放物線運動
        velocity.y -= gravity * timeScale;
        pos = VAdd(pos, VScale(velocity, timeScale));
    }
    else 
    {
        // 通常の直線運動
        // タイムスケールを適用
        pos = VAdd(pos, VScale(dir, speed * timeScale));
    }

  if (pos.y < 0.0f) active = false; // 地面で消滅
}

void EnemyAcid::UpdateDeath(const EnemyUpdateContext& context)
{
    if (!m_isDeadAnimPlaying) 
    {
        ChangeAnimation(AnimState::Dead, false);
        m_isDeadAnimPlaying = true;
        m_animTime = 0.0f; // アニメーション時間をリセット
        m_isAlive = true;  // 死亡アニメーション中はtrueのまま
    }

    // 死亡アニメーション中もアニメーション時間を更新
    if (m_animationManager.GetCurrentAttachedAnimHandle(m_modelHandle) != -1) 
    {
        m_animTime += 1.0f * Game::GetTimeScale();
        m_animationManager.UpdateAnimationTime(m_modelHandle, m_animTime);
    }

    float currentAnimTotalTime = m_animationManager.GetAnimationTotalTime(m_modelHandle, EnemyAcidConstants::kDeadAnimName);
    if (m_animTime >= currentAnimTotalTime) 
    {
        if (m_animationManager.GetCurrentAttachedAnimHandle(m_modelHandle) != -1) 
        {
            MV1DetachAnim(m_modelHandle, 0);
            m_animationManager.ResetAttachedAnimHandle(m_modelHandle);
        }

        // アイテムドロップと死亡コールバックを呼び出し
        if (!m_hasDroppedItem && m_onDropItem) 
        {
            m_onDropItem(m_pos);
            m_onDropItem = nullptr;
            m_hasDroppedItem = true;
    
        }
        if (m_onDeathCallback) 
        {
            m_onDeathCallback(m_pos);
            m_onDeathCallback = nullptr; // 一度だけ呼び出す
        }
        m_isAlive = false; // 死亡アニメーション終了時のみfalseにする
    }
}