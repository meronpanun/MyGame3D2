#include "EnemyRunner.h"
#include "Bullet.h"
#include "Player.h"
#include "DxLib.h"
#include "DebugUtil.h"
#include "CapsuleCollider.h"
#include "SphereCollider.h"
#include "../TransformDataLoader.h"
#include <cassert>
#include <algorithm>
#include <cmath>
#include <functional>

namespace
{
	// アニメーション関連
	constexpr char kAttackAnimName[] = "Armature|Attack"; // 攻撃アニメーション
	constexpr char kRunAnimName[]    = "Armature|Run";    // 走るアニメーション
	constexpr char kDeadAnimName[]   = "Armature|Death";  // 死亡アニメーション

	// カプセルコライダーのサイズを定義
	constexpr float kBodyColliderRadius = 15.0f;  // 体のコライダー半径(Normalより小さめ)
	constexpr float kBodyColliderHeight = 120.0f; // 体のコライダー高さ(Normalより小さめ)
	constexpr float kHeadRadius		    = 15.0f;  // 頭のコライダー半径(Normalより小さめ)

	// 攻撃関連
	constexpr int   kAttackCooldownMax = 30;     // 攻撃クールダウン時間(Normalより短め)
	constexpr float kAttackHitRadius   = 50.0f;  // 攻撃の当たり判定半径
	constexpr float kAttackRangeRadius = 100.0f; // 攻撃範囲の半径(Normalより小さめ)

	// 追跡関連
	constexpr int kAttackEndDelay = 10; 
	// 追跡速度
	constexpr float kChaseSpeed = 4.0f; // 走る敵の追跡速度

	constexpr VECTOR kHeadShotPositionOffset = { 0.0f, 0.0f, 0.0f }; // オフセットに変更
}

EnemyRunner::EnemyRunner() :
	m_headPosOffset{ kHeadShotPositionOffset },
	m_isTackleHit(false),
	m_lastTackleId(-1),
	m_animTime(0.0f),
	m_hasAttackHit(false),
	m_onDropItem(nullptr),
	m_currentAnimState(AnimState::Run),
	m_attackEndDelayTimer(0),
	m_isDeadAnimPlaying(false),
	m_chaseSpeed(kChaseSpeed)
{
	// モデルの読み込み
	m_modelHandle = MV1LoadModel("data/model/RunnerZombie.mv1");
	assert(m_modelHandle != -1);

	// コライダーの初期化
	m_pBodyCollider = std::make_shared<CapsuleCollider>();
	m_pHeadCollider = std::make_shared<SphereCollider>();
	m_pAttackRangeCollider = std::make_shared<SphereCollider>();
	m_pAttackHitCollider = std::make_shared<CapsuleCollider>();
}

EnemyRunner::~EnemyRunner()
{
	// モデルの解放
	MV1DeleteModel(m_modelHandle);
}

void EnemyRunner::Init()
{
	m_attackCooldownMax = kAttackCooldownMax;

	// CSVからRunnerEnemyのTransform情報を取得
	auto dataList = TransformDataLoader::LoadDataCSV("data/CSV/CharacterTransfromData.csv");
	for (const auto& data : dataList)
	{
		if (data.name == "RunnerEnemy") 
		{
			m_pos = data.pos;
			MV1SetRotationXYZ(m_modelHandle, data.rot);
			MV1SetScale(m_modelHandle, data.scale);
			m_attackPower = data.attack;
			m_hp = data.hp;
			m_chaseSpeed = data.chaseSpeed;
			break;
		}
	}

	m_currentAnimState = AnimState::Dead;
	ChangeAnimation(AnimState::Run, true);
}

void EnemyRunner::ChangeAnimation(AnimState newAnimState, bool loop)
{
    // Attackだけはリセット再生
    if (m_currentAnimState == newAnimState)
    {
        // どの状態でも必ず再生し直す
        switch (newAnimState)
        {
        case AnimState::Attack:
            m_animationManager.PlayAnimation(m_modelHandle, kAttackAnimName, loop);
            break;
        case AnimState::Run:
            m_animationManager.PlayAnimation(m_modelHandle, kRunAnimName, loop);
            break;
        case AnimState::Dead:
            m_animationManager.PlayAnimation(m_modelHandle, kDeadAnimName, loop);
            break;
        }
        m_animTime = 0.0f;
        m_currentAnimState = newAnimState;
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
    }

    if (animName)
    {
        m_animationManager.PlayAnimation(m_modelHandle, animName, loop);
        m_animTime = 0.0f;
    }

    m_currentAnimState = newAnimState;
}

bool EnemyRunner::CanAttackPlayer(const Player& player)
{
	// 手の位置を取得
	int handRIndex = MV1SearchFrame(m_modelHandle, "mixamorig:RightHand");
	int handLIndex = MV1SearchFrame(m_modelHandle, "mixamorig:LeftHand");
	if (handRIndex == -1 || handLIndex == -1) return false;

	VECTOR handRPos = MV1GetFramePosition(m_modelHandle, handRIndex);
	VECTOR handLPos = MV1GetFramePosition(m_modelHandle, handLIndex);

	m_pAttackHitCollider->SetSegment(handRPos, handLPos);
	m_pAttackHitCollider->SetRadius(kAttackHitRadius);

	std::shared_ptr<CapsuleCollider> playerBodyCollider = player.GetBodyCollider();
	return m_pAttackHitCollider->Intersects(playerBodyCollider.get());
}

void EnemyRunner::Update(std::vector<Bullet>& bullets, const Player::TackleInfo& tackleInfo, const Player& player)
{
    if (m_hp <= 0.0f) 
    {
        if (!m_isDeadAnimPlaying) 
        {
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
            if (m_onDropItem) 
			{
                m_onDropItem(m_pos);
                m_onDropItem = nullptr;
            }
            if (m_onDeathCallback) 
			{
                m_onDeathCallback(m_pos);
                m_onDeathCallback = nullptr; // 一度だけ呼び出す
            }
            m_isAlive = false; // 死亡アニメーション終了時のみfalseにする
            SetActive(false); // プールに戻す
        } else {
            m_isAlive = true; // 死亡アニメーション中はtrueのまま
        }
        return;
    }

	MV1SetPosition(m_modelHandle, m_pos);

	if (m_currentAnimState == AnimState::Run)
	{
		VECTOR playerPos = player.GetPos();
		VECTOR toPlayer = VSub(playerPos, m_pos);
		toPlayer.y = 0.0f;

		float disToPlayer = sqrtf(toPlayer.x * toPlayer.x + toPlayer.z * toPlayer.z);

		float yaw = 0.0f;
		if (toPlayer.x != 0.0f || toPlayer.z != 0.0f)
		{
			yaw = atan2f(toPlayer.x, toPlayer.z);
			yaw += DX_PI_F; // モデルの向きに合わせて調整
		}
		MV1SetRotationXYZ(m_modelHandle, VGet(0.0f, yaw, 0.0f));

		if (!CanAttackPlayer(player))
		{
			VECTOR dir = VNorm(toPlayer);
			float step = (std::min)(disToPlayer, m_chaseSpeed);
			m_pos.x += dir.x * step;
			m_pos.z += dir.z * step;
		}

		// Run状態でアニメーションが外れていたら必ず再生し直す
		int attachedAnim = m_animationManager.GetCurrentAttachedAnimHandle(m_modelHandle);
		if (attachedAnim == -1)
		{
			m_animationManager.PlayAnimation(m_modelHandle, kRunAnimName, true);
			m_animTime = 0.0f;
		}
	}

	std::shared_ptr<CapsuleCollider> playerBodyCollider = player.GetBodyCollider();
	bool isPlayerInAttackRange = m_pAttackRangeCollider->Intersects(playerBodyCollider.get());

	if (m_currentAnimState == AnimState::Attack)
	{
		float currentAnimTotalTime = m_animationManager.GetAnimationTotalTime(m_modelHandle, kAttackAnimName);
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
				m_hasAttackHit = false;
				if (isPlayerInAttackRange)
				{
					ChangeAnimation(AnimState::Attack, false);
				}
				else
				{
					ChangeAnimation(AnimState::Run, true);
				}
			}
		}
	}
	else if (m_currentAnimState == AnimState::Dead)
	{
		// 死亡アニメーション中は移動や攻撃を行わない
	}
	else // Run 状態
	{
		// 攻撃が届くまでRunを維持し、届いたらAttackに遷移
		if (CanAttackPlayer(player))
		{
			m_hasAttackHit = false;
			ChangeAnimation(AnimState::Attack, false);
		}
	}

	if (m_animationManager.GetCurrentAttachedAnimHandle(m_modelHandle) != -1)
	{
		m_animTime += 1.0f;

		float currentAnimTotalTime = m_animationManager.GetAnimationTotalTime(
			m_modelHandle,
			(m_currentAnimState == AnimState::Run ? kRunAnimName :
				(m_currentAnimState == AnimState::Attack ? kAttackAnimName : kDeadAnimName)));

		if (m_currentAnimState == AnimState::Attack)
		{
			// 何もしない
		}
		else if (m_currentAnimState == AnimState::Dead)
		{
			if (m_animTime >= currentAnimTotalTime)
			{
				m_animTime = currentAnimTotalTime;
			}
		}
		else if (m_currentAnimState == AnimState::Run)
		{
			// ループ切れ目で必ず0に戻す
			if (m_animTime >= currentAnimTotalTime)
			{
				m_animTime = 0.0f;
			}
		}

		m_animationManager.UpdateAnimationTime(m_modelHandle, m_animTime);
	}

	// コライダーの更新
	// 体のコライダーの位置を設定
	VECTOR bodyCapA = VAdd(m_pos, VGet(0, kBodyColliderRadius, 0));
	VECTOR bodyCapB = VAdd(m_pos, VGet(0, kBodyColliderHeight - kBodyColliderRadius, 0));
	m_pBodyCollider->SetSegment(bodyCapA, bodyCapB);
	m_pBodyCollider->SetRadius(kBodyColliderRadius);

	// 頭のコライダーの位置を取得
	int headIndex = MV1SearchFrame(m_modelHandle, "mixamorig:Head");
	VECTOR headModelPos = (headIndex != -1) ? MV1GetFramePosition(m_modelHandle, headIndex) : VGet(0, 0, 0);
	VECTOR headCenter = VAdd(headModelPos, m_headPosOffset);
	m_pHeadCollider->SetCenter(headCenter);
	m_pHeadCollider->SetRadius(kHeadRadius);

	// 攻撃範囲のコライダーの位置と半径を設定
	VECTOR attackRangeCenter = m_pos;
	attackRangeCenter.y += (kBodyColliderHeight * 0.5f);
	m_pAttackRangeCollider->SetCenter(attackRangeCenter);
	m_pAttackRangeCollider->SetRadius(kAttackRangeRadius);

	// 敵とプレイヤーの押し出し処理
	if (m_pBodyCollider->Intersects(playerBodyCollider.get()))
	{
		VECTOR enemyCenter = VScale(VAdd(m_pBodyCollider->GetSegmentA(), m_pBodyCollider->GetSegmentB()), 0.5f);
		VECTOR playerCenter = VScale(VAdd(playerBodyCollider->GetSegmentA(), playerBodyCollider->GetSegmentB()), 0.5f);
		VECTOR diff = VSub(enemyCenter, playerCenter);
		float distSq = VDot(diff, diff);
		float minDist = kBodyColliderRadius + playerBodyCollider->GetRadius();

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

	if (m_currentAnimState == AnimState::Attack)
	{
		float currentAnimTotalTime = m_animationManager.GetAnimationTotalTime(m_modelHandle, kAttackAnimName);
		float attackStart = currentAnimTotalTime * 0.5f;
		float attackEnd = currentAnimTotalTime * 0.7f;
		if (!m_hasAttackHit && m_animTime >= attackStart && m_animTime <= attackEnd)
		{
			// ここをLeftHandとRightHandに変更
			int handRIndex = MV1SearchFrame(m_modelHandle, "mixamorig:RightHand");
			int handLIndex = MV1SearchFrame(m_modelHandle, "mixamorig:LeftHand");
			if (handRIndex != -1 && handLIndex != -1)
			{
				VECTOR handRPos = MV1GetFramePosition(m_modelHandle, handRIndex);
				VECTOR handLPos = MV1GetFramePosition(m_modelHandle, handLIndex);

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

void EnemyRunner::Draw()
{
	if (m_hp <= 0.0f && m_animationManager.GetCurrentAttachedAnimHandle(m_modelHandle) == -1) return;

	MV1DrawModel(m_modelHandle);

#ifdef _DEBUG
	DrawCollisionDebug();

	const char* hitMsg = "";

	switch (m_lastHitPart)
	{
	case HitPart::Head:
		hitMsg = "HeadShot!";
		break;
	case HitPart::Body:
		hitMsg = "BodyHit!";
		break;
	default:
		break;
	}

	if (*hitMsg)
	{
		DrawFormatString(20, 100, 0xff0000, "%s", hitMsg);
	}

	DebugUtil::DrawFormat(20, 80, 0x000000, "Runner HP: %.1f", m_hp);
#endif
}

void EnemyRunner::DrawCollisionDebug() const
{
	// 体のコライダーデバッグ描画
	DebugUtil::DrawCapsule(m_pBodyCollider->GetSegmentA(), m_pBodyCollider->GetSegmentB(), m_pBodyCollider->GetRadius(), 16, 0xff0000);

	// 頭のコライダーデバッグ描画
	DebugUtil::DrawSphere(m_pHeadCollider->GetCenter(), m_pHeadCollider->GetRadius(), 16, 0x00ff00);

	// 攻撃範囲のデバッグ描画
	DebugUtil::DrawSphere(m_pAttackRangeCollider->GetCenter(), m_pAttackRangeCollider->GetRadius(), 24, 0xff8000);

	// ここをLeftHandとRightHandに変更
	int handRIndex = MV1SearchFrame(m_modelHandle, "mixamorig:RightHand");
	int handLIndex = MV1SearchFrame(m_modelHandle, "mixamorig:LeftHand");

	if (handRIndex != -1 && handLIndex != -1)
	{
		VECTOR handRPos = MV1GetFramePosition(m_modelHandle, handRIndex);
		VECTOR handLPos = MV1GetFramePosition(m_modelHandle, handLIndex);

		// 攻撃ヒット用コライダーのデバッグ描画
		DebugUtil::DrawCapsule(handRPos, handLPos, kAttackHitRadius, 16, 0x0000ff);
	}
}

// どこに当たったのか判定する
EnemyBase::HitPart EnemyRunner::CheckHitPart(const VECTOR& rayStart, const VECTOR& rayEnd, VECTOR& outHtPos, float& outHtDistSq) const
{
	VECTOR hitPosHead, hitPosBody;
	float hitDistSqHead = FLT_MAX;
	float hitDistSqBody = FLT_MAX;

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

float EnemyRunner::CalcDamage(float bulletDamage, HitPart part) const
{
	if (part == HitPart::Head)
	{
		return bulletDamage * 2.5f; // Runnerはヘッドショット倍率を少し高めに
	}
	else if (part == HitPart::Body)
	{
		return bulletDamage;
	}
	return 0.0f;
}

void EnemyRunner::SetOnDropItemCallback(std::function<void(const VECTOR&)> cb)
{
	m_onDropItem = cb;
}

void EnemyRunner::SetModelHandle(int handle)
{
    if (m_modelHandle != -1) MV1DeleteModel(m_modelHandle);
    m_modelHandle = MV1DuplicateModel(handle);
}