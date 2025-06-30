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
    // �A�j���[�V�����֘A
	constexpr char kAttackAnimName[] = "ATK"; // �U���A�j���[�V����

	constexpr VECTOR kInitialPosition = { 0.0f, -30.0f, 300.0f };
	constexpr VECTOR kHeadShotPositionOffset = { 0.0f, 0.0f, 0.0f }; // �I�t�Z�b�g�ɕύX

	// �J�v�Z���R���C�_�[�̃T�C�Y���`
	constexpr float kBodyColliderRadius = 20.0f;  // �̂̃R���C�_�[���a
	constexpr float kBodyColliderHeight = 135.0f; // �̂̃R���C�_�[����
	constexpr float kHeadRadius         = 18.0f;  // ���̃R���C�_�[���a

	constexpr float kInitialHP = 200.0f; // ����HP

	// �U���֘A
	constexpr int   kAttackCooldownMax = 45;     // �U���N�[���_�E������
	constexpr float kAttackPower       = 20.0f;  // �U����
	constexpr float kAttackHitRadius   = 20.0f;  // �U���̓����蔻�蔼�a
    constexpr float kAttackRangeRadius = 120.0f; // �U���͈͂̔��a

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
	// ���f���̓ǂݍ���
    m_modelHandle = MV1LoadModel("data/model/NormalZombie.mv1");
    assert(m_modelHandle != -1);

    // �R���C�_�[�̏�����
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
    m_pBodyCollider->SetSegment(bodyCapA, bodyCapB);
    m_pBodyCollider->SetRadius(kBodyColliderRadius);

    // ���̃R���C�_�[�i���j
    int headIndex = MV1SearchFrame(m_modelHandle, "Head");
    VECTOR headModelPos = (headIndex != -1) ? MV1GetFramePosition(m_modelHandle, headIndex) : VGet(0,0,0);
    VECTOR headCenter = VAdd(headModelPos, m_headPosOffset); // ���f���̓��̃t���[���ʒu�ɃI�t�Z�b�g��K�p
    m_pHeadCollider->SetCenter(headCenter);
    m_pHeadCollider->SetRadius(kHeadRadius);

    // �U���͈͂̃R���C�_�[�i���j
    VECTOR attackRangeCenter = m_pos;
    attackRangeCenter.y += (kBodyColliderHeight * 0.5f); // �G�̍����̔������炢
    m_pAttackRangeCollider->SetCenter(attackRangeCenter);
    m_pAttackRangeCollider->SetRadius(kAttackRangeRadius);

    // �v���C���[�̃J�v�Z���R���C�_�[�����擾
    //VECTOR playerCapA, playerCapB;
    //float  playerCapRadius;
    //player.GetCapsuleInfo(playerCapA, playerCapB, playerCapRadius);
    //CapsuleCollider playerBodyCollider(playerCapA, playerCapB, playerCapRadius);

    std::shared_ptr<CapsuleCollider> playerBodyCollider = player.GetBodyCollider();



    // �G�ƃv���C���[�̉����o������ (�J�v�Z�����m�̏Փ�)
	if (m_pBodyCollider->Intersects(playerBodyCollider.get()))
    {
        // �ȈՓI�ȉ����o������ (��萳�m�ȕ������Z�͕ʓr�������K�v)
        VECTOR enemyCenter = VScale(VAdd(m_pBodyCollider->GetSegmentA(), m_pBodyCollider->GetSegmentB()), 0.5f);
        VECTOR playerCenter = VScale(VAdd(playerBodyCollider->GetSegmentA(), playerBodyCollider->GetSegmentB()), 0.5f);
        VECTOR diff = VSub(enemyCenter, playerCenter);
        float distSq = VDot(diff, diff);
		float minDist = kBodyColliderRadius + playerBodyCollider->GetRadius(); // �ŏ������͗����̔��a�̘a

        if (distSq < minDist * minDist && distSq > 0.0001f)
        {
            float dist = std::sqrt(distSq);
            float pushBack = minDist - dist;
            if (dist > 0)
            {
                VECTOR pushDir = VSub(enemyCenter, playerCenter);
                pushDir.y = 0.0f; // Y�����𖳎����Đ��������̉����o���ɂ���
                float horizontalDistSq = VDot(pushDir, pushDir);

                if (horizontalDistSq > 0.0001f) // ���������̐���������ꍇ�̂ݐ��K�����ēK�p
                {
                    pushDir = VNorm(pushDir); // Y������0�ɂ�����ɐ��K��
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

                        // �U���q�b�g�p�R���C�_�[�̍X�V
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
    // �̂̃R���C�_�[�f�o�b�O�`��
    DebugUtil::DrawCapsule(m_pBodyCollider->GetSegmentA(), m_pBodyCollider->GetSegmentB(), m_pBodyCollider->GetRadius(), 16, 0xff0000);

    // ���̃R���C�_�[�f�o�b�O�`��
    DebugUtil::DrawSphere(m_pHeadCollider->GetCenter(), m_pHeadCollider->GetRadius(), 16, 0x00ff00);

    // �U���͈͂̃f�o�b�O�`��
    DebugUtil::DrawSphere(m_pAttackRangeCollider->GetCenter(), m_pAttackRangeCollider->GetRadius(), 24, 0xff8000);

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

EnemyBase::HitPart EnemyNormal::CheckHitPart(const VECTOR& rayStart, const VECTOR& rayEnd, VECTOR& outHtPos, float& outHtDistSq) const
{
    VECTOR hitPosHead, hitPosBody;
    float hitDistSqHead = FLT_MAX;
    float hitDistSqBody = FLT_MAX;

    bool headHit = m_pHeadCollider->IntersectsRay(rayStart, rayEnd, hitPosHead, hitDistSqHead);
    bool bodyHit = m_pBodyCollider->IntersectsRay(rayStart, rayEnd, hitPosBody, hitDistSqBody);

    if (headHit && bodyHit)
    {
        // �����Ƀq�b�g�����ꍇ�A���߂�����D��
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

    outHtPos = VGet(0, 0, 0); // �q�b�g���Ȃ��ꍇ�͓K���Ȓl�����Ă���
    outHtDistSq = FLT_MAX;
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