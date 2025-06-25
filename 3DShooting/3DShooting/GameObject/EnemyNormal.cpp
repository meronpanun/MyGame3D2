#include "Player.h"
#include "EnemyNormal.h"
#include "Bullet.h"
#include "DxLib.h"
#include "DebugUtil.h"
#include "Collider.h"
#include <cassert>
#include <algorithm>
#include <cmath>
#include <functional>

namespace
{
	constexpr char kAttackAnimName[] = "ATK";

    constexpr VECTOR kInitialPosition = { 0.0f, -30.0f, 200.0f };

	constexpr VECTOR kAABBMin = { -20.0f, 0.0f, -15.0f };
	constexpr VECTOR kAABBMax = { 20.0f, 128.0f, 15.0f };

	constexpr VECTOR kHeadShotPosition = { 0.0f, 160.0f, -6.0f };

    constexpr float kHeadRadius = 13.5f;

	constexpr float kInitialHP = 200.0f;

	constexpr float kAttackHitRadius = 20.0f; 

    constexpr float kAttackRangeRadius = 120.0f; 

    float VLenSq(const VECTOR& vec)
    {
        return vec.x * vec.x + vec.y * vec.y + vec.z * vec.z;
    }


    static bool CheckCapsuleSphereHit(
        const VECTOR& capA, const VECTOR& capB, float capRadius,
        const VECTOR& sphereCenter, float sphereRadius)
    {
        VECTOR ab = VSub(capB, capA);
        VECTOR ac = VSub(sphereCenter, capA);

        float abLenSq = VDot(ab, ab); 
        float t       = 0.0f; 

        
        if (abLenSq > 0.0f)
        {
            t = VDot(ac, ab) / abLenSq; 
            t = (std::max)(0.0f, (std::min)(1.0f, t)); 
        }

        VECTOR closest = VAdd(capA, VScale(ab, t));

        float distSq = VLenSq(VSub(sphereCenter, closest));
        float radiusSum = capRadius + sphereRadius;

        return distSq <= radiusSum * radiusSum;
    }

    static bool CheckCapsuleCapsuleHit(
        const VECTOR& a1, const VECTOR& a2, float r1,
        const VECTOR& b1, const VECTOR& b2, float r2)
    {
        VECTOR d1 = VSub(a2, a1);
        VECTOR d2 = VSub(b2, b1);
        VECTOR r = VSub(a1, b1);

        float a = VDot(d1, d1);
        float e = VDot(d2, d2);
        float f = VDot(d2, r);
        float s = 0.0f, t = 0.0f;
        float c = VDot(d1, r);
        float b = VDot(d1, d2);
        float denom = a * e - b * b;

        if (denom != 0.0f)
        {
            s = std::clamp((b * f - c * e) / denom, 0.0f, 1.0f);
        }
        else
        {
            s = 0.0f;
        }
            
        t = (b * s + f) / e;
        t = std::clamp(t, 0.0f, 1.0f);

        s = (b * t - c) / a;
        s = std::clamp(s, 0.0f, 1.0f);

        VECTOR p1 = VAdd(a1, VScale(d1, s));
        VECTOR p2 = VAdd(b1, VScale(d2, t));
        float distSq = VDot(VSub(p1, p2), VSub(p1, p2));
        float radiusSum = r1 + r2;
        return distSq <= radiusSum * radiusSum;
    }

    static bool CheckSphereCapsuleHit( 
        const VECTOR& sphereCenter, float sphereRadius,
        const VECTOR& capA, const VECTOR& capB, float capRadius)
    {
        return CheckCapsuleSphereHit(capA, capB, capRadius, sphereCenter, sphereRadius);
    }
}

EnemyNormal::EnemyNormal() :
    m_aabbMin{ kAABBMin },
    m_aabbMax{ kAABBMax },
    m_headPos{ kHeadShotPosition },
    m_headRadius(kHeadRadius),
	m_isTackleHit(false),
    m_lastTackleId(-1),
	m_currentAnimLoop(false),
	m_currentAnimIndex(-1),
	m_animTime(0.0f),
	m_hasAttackHit(false),
    m_onDropItem(nullptr)
{
    m_modelHandle = MV1LoadModel("data/model/NormalZombie.mv1");
    assert(m_modelHandle != -1);
}

EnemyNormal::~EnemyNormal()
{
	DeleteGraph(m_modelHandle); 
}

void EnemyNormal::Init()
{
    m_hp                = kInitialHP;
    m_pos               = kInitialPosition;
	m_attackPower       = 20.0f;  
	m_attackCooldownMax = 45;     
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

    VECTOR playerCapA, playerCapB;
    float  playerCapRadius;
    player.GetCapsuleInfo(playerCapA, playerCapB, playerCapRadius);

    VECTOR boxMin = {
        m_pos.x + m_aabbMin.x,
        m_pos.y + m_aabbMin.y,
        m_pos.z + m_aabbMin.z
    };
    VECTOR boxMax = {
        m_pos.x + m_aabbMax.x,
        m_pos.y + m_aabbMax.y,
        m_pos.z + m_aabbMax.z
    };
    VECTOR enemyCapA = { (boxMin.x + boxMax.x) * 0.5f, boxMin.y, (boxMin.z + boxMax.z) * 0.5f };
    VECTOR enemyCapB = { (boxMin.x + boxMax.x) * 0.5f, boxMax.y, (boxMin.z + boxMax.z) * 0.5f };
    float enemyCapRadius = (std::max)(std::abs(boxMax.x - boxMin.x), std::abs(boxMax.z - boxMin.z)) * 0.5f;

    auto ClosestPtSegmentSegment = [](const VECTOR& p1, const VECTOR& q1, const VECTOR& p2, const VECTOR& q2, VECTOR& c1, VECTOR& c2) {
        VECTOR d1 = VSub(q1, p1);
        VECTOR d2 = VSub(q2, p2);
        VECTOR r = VSub(p1, p2);
        float a = VDot(d1, d1);
        float e = VDot(d2, d2);
        float f = VDot(d2, r);

        float s, t;
        float c = VDot(d1, r);
        float b = VDot(d1, d2);
        float denom = a * e - b * b;

        if (denom != 0.0f) 
        {
            s = std::clamp((b * f - c * e) / denom, 0.0f, 1.0f);
        }
        else 
        {
            s = 0.0f;
        }

        t = (b * s + f) / e;
        t = std::clamp(t, 0.0f, 1.0f);

        s = (b * t - c) / a;
        s = std::clamp(s, 0.0f, 1.0f);

        c1 = VAdd(p1, VScale(d1, s));
        c2 = VAdd(p2, VScale(d2, t));
        };

    VECTOR closestPlayer, closestEnemy;
    ClosestPtSegmentSegment(
        playerCapA, playerCapB,
        enemyCapA, enemyCapB,
        closestPlayer, closestEnemy
    );

    VECTOR diff = VSub(closestPlayer, closestEnemy);
    float distSq = VDot(diff, diff);
    float minDist = playerCapRadius + enemyCapRadius;

    if (distSq < minDist * minDist && distSq > 0.0001f)
    {
        float dist = std::sqrt(distSq);
        float pushBack = minDist - dist;
        VECTOR pushDir = VScale(diff, 1.0f / dist);

        m_pos = VSub(m_pos, VScale(pushDir, pushBack * 0.5f));
    }


    MV1SetPosition(m_modelHandle, m_pos);

    VECTOR attackCenter = m_pos;
    attackCenter.y += (m_aabbMax.y - m_aabbMin.y) * 0.5f; 
    float attackRadius = kAttackRangeRadius;


    bool isPlayerInAttackRange = CheckSphereCapsuleHit(
        attackCenter, attackRadius,
        playerCapA, playerCapB, playerCapRadius
    );

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

                        VECTOR playerCapA, playerCapB;
                        float  playerCapRadius;
                        player.GetCapsuleInfo(playerCapA, playerCapB, playerCapRadius);

                        if (CheckCapsuleCapsuleHit(
                            handRPos, handLPos, kAttackHitRadius,
                            playerCapA, playerCapB, playerCapRadius))
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
        VECTOR boxMin = {
            m_pos.x + m_aabbMin.x,
            m_pos.y + m_aabbMin.y,
            m_pos.z + m_aabbMin.z
        };
        VECTOR boxMax = {
            m_pos.x + m_aabbMax.x,
            m_pos.y + m_aabbMax.y,
            m_pos.z + m_aabbMax.z
        };
        VECTOR capA = { (boxMin.x + boxMax.x) * 0.5f, boxMin.y, (boxMin.z + boxMax.z) * 0.5f };
        VECTOR capB = { (boxMin.x + boxMax.x) * 0.5f, boxMax.y, (boxMin.z + boxMax.z) * 0.5f };
        float capRadius = (std::max)(std::abs(boxMax.x - boxMin.x), std::abs(boxMax.z - boxMin.z)) * 0.5f;

        if (CheckCapsuleCapsuleHit(
            tackleInfo.capA, tackleInfo.capB, tackleInfo.radius,
            capA, capB, capRadius))
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

    DebugUtil::DrawMessage(20, 100, 0xff0000, hitMsg);
    DebugUtil::DrawFormat(20, 80, 0x000000, "Enemy HP: %.1f", m_hp);
#endif
    
}

bool EnemyNormal::IsHit(const Bullet& bullet) const
{
    int spineIndex  = MV1SearchFrame(m_modelHandle, "Root");
    VECTOR spinePos = MV1GetFramePosition(m_modelHandle, spineIndex);

    VECTOR boxMin = 
    {
        spinePos.x + m_aabbMin.x,
        spinePos.y + m_aabbMin.y,
        spinePos.z + m_aabbMin.z
    };
    VECTOR boxMax = 
    {
        spinePos.x + m_aabbMax.x,
        spinePos.y + m_aabbMax.y,
        spinePos.z + m_aabbMax.z
    };

    VECTOR capA = { (boxMin.x + boxMax.x) * 0.5f, boxMin.y, (boxMin.z + boxMax.z) * 0.5f };
    VECTOR capB = { (boxMin.x + boxMax.x) * 0.5f, boxMax.y, (boxMin.z + boxMax.z) * 0.5f };

    float capRadius = (std::max)(
        std::abs(boxMax.x - boxMin.x),
        std::abs(boxMax.z - boxMin.z)
        ) * 0.5f;

    return CheckCapsuleSphereHit(capA, capB, capRadius, bullet.GetPos(), bullet.GetRadius());
}

void EnemyNormal::DrawCollisionDebug() const
{
    int spineIndex = MV1SearchFrame(m_modelHandle, "Root");
    VECTOR spinePos = MV1GetFramePosition(m_modelHandle, spineIndex);

    VECTOR boxMin = 
    {
        spinePos.x + m_aabbMin.x,
        spinePos.y + m_aabbMin.y,
        spinePos.z + m_aabbMin.z
    };
    VECTOR boxMax = 
    {
        spinePos.x + m_aabbMax.x,
        spinePos.y + m_aabbMax.y,
        spinePos.z + m_aabbMax.z
    };

    VECTOR centerMin = { (boxMin.x + boxMax.x) * 0.5f, boxMin.y, (boxMin.z + boxMax.z) * 0.5f };
    VECTOR centerMax = { (boxMin.x + boxMax.x) * 0.5f, boxMax.y, (boxMin.z + boxMax.z) * 0.5f };

    float radius = (std::max)(
        std::abs(boxMax.x - boxMin.x),
        std::abs(boxMax.z - boxMin.z)
        ) * 0.5f;

    DebugUtil::DrawCapsule(centerMin, centerMax, radius, 16, 0xff0000);

    int headIndex = MV1SearchFrame(m_modelHandle, "Head");
    VECTOR headPos = MV1GetFramePosition(m_modelHandle, headIndex);

    DebugUtil::DrawSphere(headPos, m_headRadius, 16, 0x00ff00);

    float attackCenterY = m_pos.y + (m_aabbMax.y - m_aabbMin.y) * 0.5f;
    VECTOR attackCenter = m_pos;
    attackCenter.y = attackCenterY;

    DebugUtil::DrawSphere(attackCenter, kAttackRangeRadius, 24, 0xff8000);

    int handRIndex = MV1SearchFrame(m_modelHandle, "Hand_R");
    int handLIndex = MV1SearchFrame(m_modelHandle, "Hand_L");

    if (handRIndex != -1 && handLIndex != -1) 
    {
        VECTOR handRPos = MV1GetFramePosition(m_modelHandle, handRIndex);
        VECTOR handLPos = MV1GetFramePosition(m_modelHandle, handLIndex);
        
        DebugUtil::DrawCapsule(handRPos, handLPos, kAttackHitRadius, 16, 0x0000ff);
    }
}

EnemyBase::HitPart EnemyNormal::CheckHitPart(const Bullet& bullet) const 
{
    int headIndex  = MV1SearchFrame(m_modelHandle, "Head"); 
    VECTOR headPos = MV1GetFramePosition(m_modelHandle, headIndex);

    VECTOR headCenter = {
        headPos.x,
        headPos.y,
        headPos.z
    };

    VECTOR bulletPos = bullet.GetPos();

	float dx        = bulletPos.x - headCenter.x;        
	float dy        = bulletPos.y - headCenter.y;        
	float dz        = bulletPos.z - headCenter.z;        
	float distSq    = dx * dx + dy * dy + dz * dz;       
	float radiusSum = m_headRadius + bullet.GetRadius(); 

    if (distSq <= radiusSum * radiusSum)
    {
        return HitPart::Head;
    }

    if (IsHit(bullet)) 
    {
        return HitPart::Body;
    }
    return HitPart::None;
}

float EnemyNormal::CalcDamage(const Bullet& bullet, HitPart part) const
{
    if (part == HitPart::Head)
    {
        return bullet.GetDamage() * 2.0f; 
    }
    else if (part == HitPart::Body)
    {
        return bullet.GetDamage();
    }
    return 0.0f;
}

// アイテムドロップ時のコールバック関数を設定する
void EnemyNormal::SetOnDropItemCallback(std::function<void(const VECTOR&)> cb) 
{
    m_onDropItem = cb;
}

