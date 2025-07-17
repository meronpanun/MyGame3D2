#include "Player.h"
#include "EnemyNormal.h"
#include "Bullet.h"
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
    constexpr char kAttackAnimName[] = "ATK";  // 攻撃アニメーション
    constexpr char kWalkAnimName[]   = "WALK"; // 歩行アニメーション
    constexpr char kDeadAnimName[]   = "DEAD"; // 死亡アニメーション

    constexpr VECTOR kInitialPosition        = { 0.0f, -30.0f, 300.0f };
    constexpr VECTOR kHeadShotPositionOffset = { 0.0f, 0.0f, 0.0f }; // オフセットに変更

    // カプセルコライダーのサイズを定義
    constexpr float kBodyColliderRadius = 20.0f;  // 体のコライダー半径
    constexpr float kBodyColliderHeight = 135.0f; // 体のコライダー高さ
    constexpr float kHeadRadius         = 18.0f;  // 頭のコライダー半径

	// 体力
    constexpr float kInitialHP = 200.0f; // 初期HP

    // 攻撃関連
    constexpr int   kAttackCooldownMax = 45;     // 攻撃クールダウン時間
    constexpr float kAttackPower       = 20.0f;  // 攻撃力
    constexpr float kAttackHitRadius   = 45.0f;  // 攻撃の当たり判定半径
    constexpr float kAttackRangeRadius = 120.0f; // 攻撃範囲の半径

    // 追跡関連
    constexpr float kChaseSpeed        = 2.0f; // 追跡速度
    constexpr float kChaseStopDistance = 50;   // 追跡停止距離
	constexpr int   kAttackEndDelay    = 55;   // 攻撃後の硬直時間
}

EnemyNormal::EnemyNormal() :
    m_headPosOffset{ kHeadShotPositionOffset },
    m_isTackleHit(false),
    m_lastTackleId(-1),
    m_animTime(0.0f),
    m_hasAttackHit(false),
    m_onDropItem(nullptr),
    m_currentAnimState(AnimState::Walk),
    m_attackEndDelayTimer(0),
    m_isDeadAnimPlaying(false),
	m_chaseSpeed(kChaseSpeed),
	m_isItemDropped(false)
{
    // モデルの読み込み
    m_modelHandle = MV1LoadModel("data/model/NormalZombie.mv1");
    assert(m_modelHandle != -1);

    // コライダーの初期化
    m_pBodyCollider = std::make_shared<CapsuleCollider>();
    m_pHeadCollider = std::make_shared<SphereCollider>();
    m_pAttackRangeCollider = std::make_shared<SphereCollider>();
    m_pAttackHitCollider = std::make_shared<CapsuleCollider>();
}

EnemyNormal::~EnemyNormal()
{
	// モデルの解放
    MV1DeleteModel(m_modelHandle);
}

void EnemyNormal::Init()
{
    m_hp = kInitialHP;
    m_attackPower = kAttackPower;
    m_attackCooldownMax = kAttackCooldownMax;

    m_isAlive = true;
    m_isItemDropped = false;
    m_lastHitPart = HitPart::None; // 最後のヒット部位をリセット
    m_hitDisplayTimer = 0; // ヒット表示タイマーもリセット

    // ここで一度「絶対にWalkでない値」にリセット
    m_currentAnimState = AnimState::Dead;

    ChangeAnimation(AnimState::Walk, true); // 初期化時に歩行アニメーションを開始
}

// アニメーションを変更する
void EnemyNormal::ChangeAnimation(AnimState newAnimState, bool loop)
{
    if (m_currentAnimState == newAnimState)
    {
        if (newAnimState == AnimState::Attack)
        {
            // 同じ攻撃アニメーションをリセットして再開
            m_animationManager.PlayAnimation(m_modelHandle, kAttackAnimName, loop);
        }
        else
        {
            return;
        }
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
    }

    if (animName)
    {
        // AnimationManagerにアニメーションの再生を依頼
        m_animationManager.PlayAnimation(m_modelHandle, animName, loop);
        m_animTime = 0.0f; // アニメーション切り替え時に時間をリセット
    }

    m_currentAnimState = newAnimState;
}

// プレイヤーに攻撃可能かどうかを判定
bool EnemyNormal::CanAttackPlayer(const Player& player)
{
    int handRIndex = MV1SearchFrame(m_modelHandle, "Hand_R");
    int handLIndex = MV1SearchFrame(m_modelHandle, "Hand_L");
    if (handRIndex == -1 || handLIndex == -1) return false;

    VECTOR handRPos = MV1GetFramePosition(m_modelHandle, handRIndex);
    VECTOR handLPos = MV1GetFramePosition(m_modelHandle, handLIndex);

    m_pAttackHitCollider->SetSegment(handRPos, handLPos);
    m_pAttackHitCollider->SetRadius(kAttackHitRadius);

    std::shared_ptr<CapsuleCollider> playerBodyCollider = player.GetBodyCollider();
    return m_pAttackHitCollider->Intersects(playerBodyCollider.get());
}

// モデルハンドルを設定する
void EnemyNormal::SetModelHandle(int handle)
{
    if (m_modelHandle != -1) MV1DeleteModel(m_modelHandle);
    m_modelHandle = MV1DuplicateModel(handle);
}

void EnemyNormal::Update(std::vector<Bullet>& bullets, const Player::TackleInfo& tackleInfo, const Player& player, const std::vector<EnemyBase*>& enemyList)
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
            SetActive(false);  // プールに戻す
        } 
        else 
        {
            m_isAlive = true; // 死亡アニメーション中はtrueのまま
        }
        return;
    }

    MV1SetPosition(m_modelHandle, m_pos); // モデルの位置は常に反映する

    // 攻撃アニメーション中は追尾しない
    if (m_currentAnimState == AnimState::Walk) // 追尾はWalk状態でのみ行う
    {
        VECTOR playerPos = player.GetPos();
        VECTOR toPlayer = VSub(playerPos, m_pos);
        toPlayer.y = 0.0f; // Y成分を無視して水平距離を計算

        // プレイヤーとの距離を計算
        float disToPlayer = sqrtf(toPlayer.x * toPlayer.x + toPlayer.z * toPlayer.z);

        // プレイヤーの方向を常に向く
        float yaw = 0.0f;
        if (toPlayer.x != 0.0f || toPlayer.z != 0.0f)
        {
            yaw = atan2f(toPlayer.x, toPlayer.z);
            yaw += DX_PI_F; // モデルの向きに合わせて調整
        }
        MV1SetRotationXYZ(m_modelHandle, VGet(0.0f, yaw, 0.0f));

        // 移動処理
        if (disToPlayer > kChaseStopDistance)
        {
            VECTOR dir = VNorm(toPlayer);
            float moveDist = disToPlayer - kChaseStopDistance;
            float step = (std::min)(moveDist, kChaseSpeed); // 1フレームで進みすぎない
            m_pos.x += dir.x * step;
            m_pos.z += dir.z * step;
        }
    }

    // プレイヤーのカプセルコライダー情報を取得
    std::shared_ptr<CapsuleCollider> playerBodyCollider = player.GetBodyCollider();
    bool isPlayerInAttackRange = m_pAttackRangeCollider->Intersects(playerBodyCollider.get());

    // アニメーションの状態管理
    if (m_currentAnimState == AnimState::Attack)
    {
        // 攻撃アニメーションはループしないので、終了したらディレイタイマーをセット
        float currentAnimTotalTime = m_animationManager.GetAnimationTotalTime(m_modelHandle, kAttackAnimName);
        if (m_animTime > currentAnimTotalTime)
        {
            if (m_attackEndDelayTimer <= 0)
            {
                m_attackEndDelayTimer = kAttackEndDelay; // ディレイ開始
            }
        }
        // ディレイタイマーが動作中ならカウントダウン
        if (m_attackEndDelayTimer > 0)
        {
            --m_attackEndDelayTimer;
            if (m_attackEndDelayTimer == 0)
            {
                m_hasAttackHit = false; // 攻撃ヒットフラグをリセット
                if (isPlayerInAttackRange)
                {
                    ChangeAnimation(AnimState::Attack, false); // 攻撃範囲内なら再度攻撃
                }
                else
                {
                    ChangeAnimation(AnimState::Walk, true);    // 範囲外なら歩行
                }
            }
        }
    }
    else if (m_currentAnimState == AnimState::Dead)
    {
        // 死亡アニメーション中は移動や攻撃を行わない
    }
    else // Walk 状態(常に歩行アニメーションが基本)
    {
        // 攻撃が届くまでWalkを維持し、届いたらAttackに遷移
        if (CanAttackPlayer(player))
        {
            m_hasAttackHit = false;
            ChangeAnimation(AnimState::Attack, false);
        }
    }

    // アニメーション時間の更新
    if (m_animationManager.GetCurrentAttachedAnimHandle(m_modelHandle) != -1) // アニメーションがアタッチされている場合のみ時間を更新
    {
        m_animTime += 1.0f;

        float currentAnimTotalTime = m_animationManager.GetAnimationTotalTime(m_modelHandle,
            (m_currentAnimState == AnimState::Walk ? kWalkAnimName :
                (m_currentAnimState == AnimState::Attack ? kAttackAnimName : kDeadAnimName)));

        if (m_currentAnimState == AnimState::Attack)
        {
            // ここは何もしない（歩行アニメーションへの遷移はディレイタイマーでのみ行う）
        }
        else if (m_currentAnimState == AnimState::Dead)
        {
            if (m_animTime >= currentAnimTotalTime)
            {
                m_animTime = currentAnimTotalTime;
            }
        }
        else if (m_currentAnimState == AnimState::Walk)
        {
            if (m_animTime >= currentAnimTotalTime)
            {
                m_animTime = fmodf(m_animTime, currentAnimTotalTime);
            }
        }
        // AnimationManagerにアニメーション時間の更新を依頼
        m_animationManager.UpdateAnimationTime(m_modelHandle, m_animTime);
    }

    // コライダーの更新
    // 体のコライダー（カプセル）
    VECTOR bodyCapA = VAdd(m_pos, VGet(0, kBodyColliderRadius, 0));
    VECTOR bodyCapB = VAdd(m_pos, VGet(0, kBodyColliderHeight - kBodyColliderRadius, 0));
    m_pBodyCollider->SetSegment(bodyCapA, bodyCapB);
    m_pBodyCollider->SetRadius(kBodyColliderRadius);

    // 頭のコライダー（球）
    int headIndex = MV1SearchFrame(m_modelHandle, "Head");
    VECTOR headModelPos = (headIndex != -1) ? MV1GetFramePosition(m_modelHandle, headIndex) : VGet(0, 0, 0);
    VECTOR headCenter = VAdd(headModelPos, m_headPosOffset); // モデルの頭のフレーム位置にオフセットを適用
    m_pHeadCollider->SetCenter(headCenter);
    m_pHeadCollider->SetRadius(kHeadRadius);

    // 攻撃範囲のコライダー（球）
    VECTOR attackRangeCenter = m_pos;
    attackRangeCenter.y += (kBodyColliderHeight * 0.5f); // 敵の高さの半分くらい
    m_pAttackRangeCollider->SetCenter(attackRangeCenter);
    m_pAttackRangeCollider->SetRadius(kAttackRangeRadius);

    // 敵とプレイヤーの押し出し処理（カプセル同士の衝突）
    if (m_pBodyCollider->Intersects(playerBodyCollider.get()))
    {
        // 押し出し処理
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

    // 敵同士の押し出し処理（横方向への広がり）
    for (EnemyBase* other : enemyList)
    {
        if (!other) continue;
        // 自分自身は除外
        if (other == this) continue;

        // 位置取得
        VECTOR otherPos = other->GetPos();
        VECTOR diff = VSub(m_pos, otherPos);

        diff.y = 0.0f;
        float distSq = VDot(diff, diff);
        float minDist = kBodyColliderRadius * 2.0f; // 体の半径×2
        if (distSq < minDist * minDist && distSq > 0.0001f) 
        {
            float dist = std::sqrt(distSq);
            float pushBack = minDist - dist;
            if (dist > 0) 
            {
                VECTOR pushDir = VNorm(diff);
                // 横方向に広がるように、プレイヤー方向ベクトルと直交する方向に少し加算
                VECTOR playerDir = VNorm(VSub(player.GetPos(), m_pos));
                VECTOR up        = VGet(0, 1, 0);
                VECTOR side      = VNorm(VCross(playerDir, up));
                // 直交方向にランダム性を加える（左右どちらか）
                float sign = (reinterpret_cast<size_t>(this) % 2 == 0) ? 1.0f : -1.0f;
                side = VScale(side, sign * 0.5f); // 横成分を少し加える
                pushDir = VNorm(VAdd(pushDir, side));
                m_pos = VAdd(m_pos, VScale(pushDir, pushBack * 0.5f));
            }
        }
    }

    if (m_currentAnimState == AnimState::Attack) // 攻撃アニメーションが再生中の場合のみ攻撃判定を行う
    {
        // m_currentAnimTotalTime を AnimationManager から取得
        float currentAnimTotalTime = m_animationManager.GetAnimationTotalTime(m_modelHandle, kAttackAnimName);
        float attackStart = currentAnimTotalTime * 0.5f; // 攻撃開始時間
        float attackEnd = currentAnimTotalTime * 0.7f;   // 攻撃終了時間

		// 攻撃アニメーションの範囲内でのみ攻撃判定を行う
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
                    const_cast<Player&>(player).TakeDamage(m_attackPower); // プレイヤーにダメージ
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

void EnemyNormal::Draw()
{
    // m_currentAnimHandle は AnimationManager 内部で管理される
    if (m_hp <= 0.0f && m_animationManager.GetCurrentAttachedAnimHandle(m_modelHandle) == -1) return; // 死亡アニメーション終了後も描画しない

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
#endif

}

// デバック用の当たり判定を描画する
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

// どこに当たったのか判定する
EnemyBase::HitPart EnemyNormal::CheckHitPart(const VECTOR& rayStart, const VECTOR& rayEnd, VECTOR& outHtPos, float& outHtDistSq) const
{
	// ヘッドとボディのコライダーをそれぞれチェック
    VECTOR hitPosHead, hitPosBody;
    float hitDistSqHead = FLT_MAX;
    float hitDistSqBody = FLT_MAX;

	// ヘッドとボディのコライダーに対してRayをチェック
    bool headHit = m_pHeadCollider->IntersectsRay(rayStart, rayEnd, hitPosHead, hitDistSqHead);
    bool bodyHit = m_pBodyCollider->IntersectsRay(rayStart, rayEnd, hitPosBody, hitDistSqBody);

	// ヒットした部位を判定
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

// ダメージ計算処理
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

void EnemyNormal::TakeDamage(float damage)
{
    m_hp -= damage;
    if (m_hp <= 0.0f) // 死亡時一度だけ
    {
        m_hp = 0.0f;
        m_isAlive = false;
        if (m_lastHitPart == HitPart::None) m_lastHitPart = HitPart::Body;
        bool isHeadShot = (m_lastHitPart == HitPart::Head);
        int addScore = ScoreManager::Instance().AddScore(isHeadShot);
        if (SceneMain::Instance()) 
        {
            SceneMain::Instance()->AddScorePopup(addScore, isHeadShot, ScoreManager::Instance().GetCombo());
        }
    }
}