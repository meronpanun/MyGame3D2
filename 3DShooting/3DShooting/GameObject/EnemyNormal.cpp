#include "Player.h"
#include "EnemyNormal.h"
#include "Bullet.h"
#include "DxLib.h"
#include "DebugUtil.h"
#include "SphereCollider.h" 
#include "CapsuleCollider.h" 
#include <cassert>
#include <algorithm>
#include <cmath>
#include <functional>
#include <float.h>

namespace
{
    // アニメーション関連
	constexpr char kAttackAnimName[] = "ATK"; // 攻撃アニメーション

	constexpr VECTOR kInitialPosition = { 0.0f, -30.0f, 300.0f };
	constexpr VECTOR kHeadShotPositionOffset = { 0.0f, 0.0f, 0.0f }; // オフセットに変更

	// カプセルコライダーのサイズを定義
	constexpr float kBodyColliderRadius = 20.0f;  // 体のコライダー半径
	constexpr float kBodyColliderHeight = 135.0f; // 体のコライダー高さ
	constexpr float kHeadRadius         = 18.0f;  // 頭のコライダー半径

	constexpr float kInitialHP = 200.0f; // 初期HP

	// 攻撃関連
	constexpr int   kAttackCooldownMax = 45;     // 攻撃クールダウン時間
	constexpr float kAttackPower       = 20.0f;  // 攻撃力
	constexpr float kAttackHitRadius   = 20.0f;  // 攻撃の当たり判定半径
    constexpr float kAttackRangeRadius = 120.0f; // 攻撃範囲の半径

}

EnemyNormal::EnemyNormal() :
    m_headPosOffset{ kHeadShotPositionOffset },
    m_isTackleHit(false),
    m_lastTackleId(-1),
	m_currentAnimLoop(false),
	m_currentAnimIndex(-1),
	m_animTime(0.0f),
	m_hasAttackHit(false),
    m_onDropItem(nullptr)
{
	// モデルの読み込み
    m_modelHandle = MV1LoadModel("data/model/NormalZombie.mv1");
    assert(m_modelHandle != -1);

    // コライダーの初期化
    m_pBodyCollider        = std::make_shared<CapsuleCollider>();
    m_pHeadCollider        = std::make_shared<SphereCollider>();
    m_pAttackRangeCollider = std::make_shared<SphereCollider>();
    m_pAttackHitCollider   = std::make_shared<CapsuleCollider>();
}

EnemyNormal::~EnemyNormal()
{

	MV1DeleteModel(m_modelHandle); 
}

void EnemyNormal::Init()
{
    m_hp                = kInitialHP;
    m_pos               = kInitialPosition;
	m_attackPower       = kAttackPower;
	m_attackCooldownMax = kAttackCooldownMax;
}

void EnemyNormal::Update(std::vector<Bullet>& bullets, const Player::TackleInfo& tackleInfo, const Player& player)
{
    if (m_hp <= 0.0f)
    {
        if (m_onDropItem)
        {
            m_onDropItem(m_pos);    // アイテムドロップコールバックを呼び出す
            m_onDropItem = nullptr; // アイテムドロップ後はコールバックを無効化
        }
        return;
    }

    MV1SetPosition(m_modelHandle, m_pos);

    // コライダーの更新
    // 体のコライダー（カプセル）
    VECTOR bodyCapA = VAdd(m_pos, VGet(0, kBodyColliderRadius, 0));
    VECTOR bodyCapB = VAdd(m_pos, VGet(0, kBodyColliderHeight - kBodyColliderRadius, 0));
    m_pBodyCollider->SetSegment(bodyCapA, bodyCapB);
    m_pBodyCollider->SetRadius(kBodyColliderRadius);

    // 頭のコライダー（球）
    int headIndex = MV1SearchFrame(m_modelHandle, "Head");
    VECTOR headModelPos = (headIndex != -1) ? MV1GetFramePosition(m_modelHandle, headIndex) : VGet(0,0,0);
    VECTOR headCenter = VAdd(headModelPos, m_headPosOffset); // モデルの頭のフレーム位置にオフセットを適用
    m_pHeadCollider->SetCenter(headCenter);
    m_pHeadCollider->SetRadius(kHeadRadius);

    // 攻撃範囲のコライダー（球）
    VECTOR attackRangeCenter = m_pos;
    attackRangeCenter.y += (kBodyColliderHeight * 0.5f); // 敵の高さの半分くらい
    m_pAttackRangeCollider->SetCenter(attackRangeCenter);
    m_pAttackRangeCollider->SetRadius(kAttackRangeRadius);

    // プレイヤーのカプセルコライダー情報を取得
    //VECTOR playerCapA, playerCapB;
    //float  playerCapRadius;
    //player.GetCapsuleInfo(playerCapA, playerCapB, playerCapRadius);
    //CapsuleCollider playerBodyCollider(playerCapA, playerCapB, playerCapRadius);

    std::shared_ptr<CapsuleCollider> playerBodyCollider = player.GetBodyCollider();



    // 敵とプレイヤーの押し出し処理 (カプセル同士の衝突)
	if (m_pBodyCollider->Intersects(playerBodyCollider.get()))
    {
        // 簡易的な押し出し処理 (より正確な物理演算は別途実装が必要)
        VECTOR enemyCenter = VScale(VAdd(m_pBodyCollider->GetSegmentA(), m_pBodyCollider->GetSegmentB()), 0.5f);
        VECTOR playerCenter = VScale(VAdd(playerBodyCollider->GetSegmentA(), playerBodyCollider->GetSegmentB()), 0.5f);
        VECTOR diff = VSub(enemyCenter, playerCenter);
        float distSq = VDot(diff, diff);
		float minDist = kBodyColliderRadius + playerBodyCollider->GetRadius(); // 最小距離は両方の半径の和

        if (distSq < minDist * minDist && distSq > 0.0001f)
        {
            float dist = std::sqrt(distSq);
            float pushBack = minDist - dist;
            if (dist > 0)
            {
                VECTOR pushDir = VSub(enemyCenter, playerCenter);
                pushDir.y = 0.0f; // Y成分を無視して水平方向の押し出しにする
                float horizontalDistSq = VDot(pushDir, pushDir);

                if (horizontalDistSq > 0.0001f) // 水平方向の成分がある場合のみ正規化して適用
                {
                    pushDir = VNorm(pushDir); // Y成分を0にした後に正規化
                    m_pos = VAdd(m_pos, VScale(pushDir, pushBack * 0.5f));
                }
            }
        }
    }

	bool isPlayerInAttackRange = m_pAttackRangeCollider->Intersects(playerBodyCollider.get());

    int attackAnimIndex = MV1GetAnimIndex(m_modelHandle, kAttackAnimName);
    if (attackAnimIndex != -1)
    {
        double animTotalTime = MV1GetAnimTotalTime(m_modelHandle, attackAnimIndex);

        if (m_currentAnimIndex != -1)
        {
            m_animTime += 1.0f;
            if (m_animTime >= animTotalTime)
            {
                m_animTime = animTotalTime;
                m_currentAnimIndex = -1;
                m_hasAttackHit = false;
            }
            else
            {
                MV1SetAttachAnimTime(m_modelHandle, 0, m_animTime);

                float attackStart = animTotalTime * 0.3f;
                float attackEnd = animTotalTime * 0.7f;
                if (!m_hasAttackHit && m_animTime >= attackStart && m_animTime <= attackEnd)
                {
                    int handRIndex = MV1SearchFrame(m_modelHandle, "Hand_R");
                    int handLIndex = MV1SearchFrame(m_modelHandle, "Hand_L");
                    if (handRIndex != -1 && handLIndex != -1)
                    {
                        VECTOR handRPos = MV1GetFramePosition(m_modelHandle, handRIndex);
                        VECTOR handLPos = MV1GetFramePosition(m_modelHandle, handLIndex);

                        // 攻撃ヒット用コライダーの更新
                        m_pAttackHitCollider->SetSegment(handRPos, handLPos);
                        m_pAttackHitCollider->SetRadius(kAttackHitRadius);
                        
                        if (m_pAttackHitCollider->Intersects(playerBodyCollider.get()))
                        {
                            const_cast<Player&>(player).TakeDamage(m_attackPower);
                            m_hasAttackHit = true;
                        }
                    }
                }
            }
        }
        else if (isPlayerInAttackRange)
        {
            MV1DetachAnim(m_modelHandle, 0);
            m_currentAnimIndex = MV1AttachAnim(m_modelHandle, attackAnimIndex, -1, false);
            m_currentAnimLoop = false;
            m_animTime = 0.0f;
            MV1SetAttachAnimTime(m_modelHandle, 0, m_animTime);
        }
    }
    
	CheckHitAndDamage(const_cast<std::vector<Bullet>&>(bullets));

    if (tackleInfo.isTackling && m_hp > 0.0f && tackleInfo.tackleId != m_lastTackleId)
    {
        CapsuleCollider playerTackleCollider(tackleInfo.capA, tackleInfo.capB, tackleInfo.radius);

        if (m_pBodyCollider->Intersects(&playerTackleCollider))
        {
            TakeTackleDamage(tackleInfo.damage);
            m_lastTackleId = tackleInfo.tackleId;
        }
    }

    if (m_hitDisplayTimer > 0) 
    {
        --m_hitDisplayTimer;
        if (m_hitDisplayTimer == 0) 
        {
            m_lastHitPart = HitPart::None;
        }
    }
}

void EnemyNormal::Draw()
{
    if (m_hp <= 0.0f) return;

    MV1DrawModel(m_modelHandle);

#ifdef _DEBUG
    DrawCollisionDebug();

    const char* hitMsg = "";

    switch (m_lastHitPart)
    {
    case HitPart::Head: hitMsg = "HeadShot!"; break;
    case HitPart::Body: hitMsg = "BodyHit!"; break;
    default: break;
    }
    if (*hitMsg)
    {
        DrawFormatString(20, 100, 0xff0000, "%s", hitMsg); 
    }

    DebugUtil::DrawFormat(20, 80, 0x000000, "Enemy HP: %.1f", m_hp);
#endif
    
}

void EnemyNormal::DrawCollisionDebug() const
{
    // 体のコライダーデバッグ描画
    DebugUtil::DrawCapsule(m_pBodyCollider->GetSegmentA(), m_pBodyCollider->GetSegmentB(), m_pBodyCollider->GetRadius(), 16, 0xff0000);

    // 頭のコライダーデバッグ描画
    DebugUtil::DrawSphere(m_pHeadCollider->GetCenter(), m_pHeadCollider->GetRadius(), 16, 0x00ff00);

    // 攻撃範囲のデバッグ描画
    DebugUtil::DrawSphere(m_pAttackRangeCollider->GetCenter(), m_pAttackRangeCollider->GetRadius(), 24, 0xff8000);

    int handRIndex = MV1SearchFrame(m_modelHandle, "Hand_R");
    int handLIndex = MV1SearchFrame(m_modelHandle, "Hand_L");

    if (handRIndex != -1 && handLIndex != -1) 
    {
        VECTOR handRPos = MV1GetFramePosition(m_modelHandle, handRIndex);
        VECTOR handLPos = MV1GetFramePosition(m_modelHandle, handLIndex);
        
        // 攻撃ヒット用コライダーのデバッグ描画
        DebugUtil::DrawCapsule(handRPos, handLPos, kAttackHitRadius, 16, 0x0000ff);
    }
}

EnemyBase::HitPart EnemyNormal::CheckHitPart(const VECTOR& rayStart, const VECTOR& rayEnd, VECTOR& outHtPos, float& outHtDistSq) const
{
    VECTOR hitPosHead, hitPosBody;
    float hitDistSqHead = FLT_MAX;
    float hitDistSqBody = FLT_MAX;

    bool headHit = m_pHeadCollider->IntersectsRay(rayStart, rayEnd, hitPosHead, hitDistSqHead);
    bool bodyHit = m_pBodyCollider->IntersectsRay(rayStart, rayEnd, hitPosBody, hitDistSqBody);

    if (headHit && bodyHit)
    {
        // 両方にヒットした場合、より近い方を優先
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

    outHtPos = VGet(0, 0, 0); // ヒットしない場合は適当な値を入れておく
    outHtDistSq = FLT_MAX;
    return HitPart::None;
}

float EnemyNormal::CalcDamage(float bulletDamage, HitPart part) const
{
    if (part == HitPart::Head)
    {
        return bulletDamage * 2.0f; // ヘッドショットは2倍ダメージ
    }
    else if (part == HitPart::Body)
    {
        return bulletDamage; // ボディショットは通常のダメージ
    }
    return 0.0f;
}

// アイテムドロップ時のコールバック関数を設定する
void EnemyNormal::SetOnDropItemCallback(std::function<void(const VECTOR&)> cb) 
{
    m_onDropItem = cb;
}