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
	constexpr char kAttackAnimName[] = "ATK";

    constexpr VECTOR kInitialPosition = { 0.0f, -30.0f, 300.0f };

	// �J�v�Z���R���C�_�[�̃T�C�Y���`
	constexpr float kBodyColliderRadius = 20.0f;
    constexpr float kBodyColliderHeight = 135.0f;

	constexpr VECTOR kHeadShotPositionOffset = { 0.0f, 0.0f, 0.0f }; // �I�t�Z�b�g�ɕύX

    constexpr float kHeadRadius = 18.0f;

	constexpr float kInitialHP = 20000.0f;

	constexpr float kAttackHitRadius = 20.0f; // �U���̓����蔻�蔼�a

    constexpr float kAttackRangeRadius = 120.0f; // �U���͈͂̔��a
}

EnemyNormal::EnemyNormal() :
    m_headPosOffset{ kHeadShotPositionOffset }, // �I�t�Z�b�g�ŏ�����
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

    // �R���C�_�[�̏�����
    m_bodyCollider = std::make_shared<CapsuleCollider>();
    m_headCollider = std::make_shared<SphereCollider>();
    m_attackRangeCollider = std::make_shared<SphereCollider>();
    m_attackHitCollider = std::make_shared<CapsuleCollider>();
}

EnemyNormal::~EnemyNormal()
{
	MV1DeleteModel(m_modelHandle); 
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
            m_onDropItem(m_pos);    // �A�C�e���h���b�v�R�[���o�b�N���Ăяo��
            m_onDropItem = nullptr; // �A�C�e���h���b�v��̓R�[���o�b�N�𖳌���
        }
        return;
    }

    MV1SetPosition(m_modelHandle, m_pos);

    // �R���C�_�[�̍X�V
    // �̂̃R���C�_�[�i�J�v�Z���j
    VECTOR bodyCapA = VAdd(m_pos, VGet(0, kBodyColliderRadius, 0));
    VECTOR bodyCapB = VAdd(m_pos, VGet(0, kBodyColliderHeight - kBodyColliderRadius, 0));
    m_bodyCollider->SetSegment(bodyCapA, bodyCapB);
    m_bodyCollider->SetRadius(kBodyColliderRadius);

    // ���̃R���C�_�[�i���j
    int headIndex = MV1SearchFrame(m_modelHandle, "Head");
    VECTOR headModelPos = (headIndex != -1) ? MV1GetFramePosition(m_modelHandle, headIndex) : VGet(0,0,0);
    VECTOR headCenter = VAdd(headModelPos, m_headPosOffset); // ���f���̓��̃t���[���ʒu�ɃI�t�Z�b�g��K�p
    m_headCollider->SetCenter(headCenter);
    m_headCollider->SetRadius(kHeadRadius);

    // �U���͈͂̃R���C�_�[�i���j
    VECTOR attackRangeCenter = m_pos;
    attackRangeCenter.y += (kBodyColliderHeight * 0.5f); // �G�̍����̔������炢
    m_attackRangeCollider->SetCenter(attackRangeCenter);
    m_attackRangeCollider->SetRadius(kAttackRangeRadius);

    // �v���C���[�̃J�v�Z���R���C�_�[�����擾
    VECTOR playerCapA, playerCapB;
    float  playerCapRadius;
    player.GetCapsuleInfo(playerCapA, playerCapB, playerCapRadius);
    CapsuleCollider playerBodyCollider(playerCapA, playerCapB, playerCapRadius);


    // �G�ƃv���C���[�̉����o������ (�J�v�Z�����m�̏Փ�)
    if (m_bodyCollider->Intersects(&playerBodyCollider))
    {
        // �ȈՓI�ȉ����o������ (��萳�m�ȕ������Z�͕ʓr�������K�v)
        VECTOR enemyCenter = VScale(VAdd(bodyCapA, bodyCapB), 0.5f);
        VECTOR playerCenter = VScale(VAdd(playerCapA, playerCapB), 0.5f);
        VECTOR diff = VSub(enemyCenter, playerCenter);
        float distSq = VDot(diff, diff);
        float minDist = kBodyColliderRadius + playerCapRadius; 

        if (distSq < minDist * minDist && distSq > 0.0001f)
        {
            float dist = std::sqrt(distSq);
            float pushBack = minDist - dist;
            if (dist > 0)
            {
                VECTOR pushDir = VScale(diff, 1.0f / dist);
                m_pos = VAdd(m_pos, VScale(pushDir, pushBack * 0.5f)); 
            }
        }
    }


    bool isPlayerInAttackRange = m_attackRangeCollider->Intersects(&playerBodyCollider);

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

                        // �U���q�b�g�p�R���C�_�[�̍X�V
                        m_attackHitCollider->SetSegment(handRPos, handLPos);
                        m_attackHitCollider->SetRadius(kAttackHitRadius);
                        
                        if (m_attackHitCollider->Intersects(&playerBodyCollider))
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

        if (m_bodyCollider->Intersects(&playerTackleCollider))
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
    // �̂̃R���C�_�[�f�o�b�O�`��
    DebugUtil::DrawCapsule(m_bodyCollider->GetSegmentA(), m_bodyCollider->GetSegmentB(), m_bodyCollider->GetRadius(), 16, 0xff0000);

    // ���̃R���C�_�[�f�o�b�O�`��
    DebugUtil::DrawSphere(m_headCollider->GetCenter(), m_headCollider->GetRadius(), 16, 0x00ff00);

    // �U���͈͂̃f�o�b�O�`��
    DebugUtil::DrawSphere(m_attackRangeCollider->GetCenter(), m_attackRangeCollider->GetRadius(), 24, 0xff8000);

    int handRIndex = MV1SearchFrame(m_modelHandle, "Hand_R");
    int handLIndex = MV1SearchFrame(m_modelHandle, "Hand_L");

    if (handRIndex != -1 && handLIndex != -1) 
    {
        VECTOR handRPos = MV1GetFramePosition(m_modelHandle, handRIndex);
        VECTOR handLPos = MV1GetFramePosition(m_modelHandle, handLIndex);
        
        // �U���q�b�g�p�R���C�_�[�̃f�o�b�O�`��
        DebugUtil::DrawCapsule(handRPos, handLPos, kAttackHitRadius, 16, 0x0000ff);
    }
}

EnemyBase::HitPart EnemyNormal::CheckHitPart(const VECTOR& rayStart, const VECTOR& rayEnd, VECTOR& out_hitPos, float& out_hitDistSq) const
{
    VECTOR hitPosHead, hitPosBody;
    float hitDistSqHead = FLT_MAX;
    float hitDistSqBody = FLT_MAX;

    bool headHit = m_headCollider->IntersectsRay(rayStart, rayEnd, hitPosHead, hitDistSqHead);
    bool bodyHit = m_bodyCollider->IntersectsRay(rayStart, rayEnd, hitPosBody, hitDistSqBody);

    if (headHit && bodyHit)
    {
        // �����Ƀq�b�g�����ꍇ�A���߂�����D��
        if (hitDistSqHead <= hitDistSqBody)
        {
            out_hitPos = hitPosHead;
            out_hitDistSq = hitDistSqHead;
            return HitPart::Head;
        }
        else
        {
            out_hitPos = hitPosBody;
            out_hitDistSq = hitDistSqBody;
            return HitPart::Body;
        }
    }
    else if (headHit)
    {
        out_hitPos = hitPosHead;
        out_hitDistSq = hitDistSqHead;
        return HitPart::Head;
    }
    else if (bodyHit)
    {
        out_hitPos = hitPosBody;
        out_hitDistSq = hitDistSqBody;
        return HitPart::Body;
    }

    out_hitPos = VGet(0, 0, 0); // �q�b�g���Ȃ��ꍇ�͓K���Ȓl�����Ă���
    out_hitDistSq = FLT_MAX;
    return HitPart::None;
}

float EnemyNormal::CalcDamage(float bulletDamage, HitPart part) const
{
    if (part == HitPart::Head)
    {
        return bulletDamage * 2.0f; // �w�b�h�V���b�g��2�{�_���[�W
    }
    else if (part == HitPart::Body)
    {
        return bulletDamage; // �{�f�B�V���b�g�͒ʏ�̃_���[�W
    }
    return 0.0f;
}

// �A�C�e���h���b�v���̃R�[���o�b�N�֐���ݒ肷��
void EnemyNormal::SetOnDropItemCallback(std::function<void(const VECTOR&)> cb) 
{
    m_onDropItem = cb;
}