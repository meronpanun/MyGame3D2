#include "EnemyNormal.h"
#include "Bullet.h"
#include "CapsuleCollider.h"
#include "CollisionGrid.h"
#include "DebugUtil.h"
#include "DxLib.h"
#include "EffekseerForDXLib.h"
#include "Game.h"
#include "Player.h"
#include "SceneMain.h"
#include "ScoreManager.h"
#include "SphereCollider.h"
#include "TransformDataLoader.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>
#include <string>
#include <vector>

namespace EnemyNormalConstants
{
    // アニメーション関連
    constexpr char kAttackAnimName[] = "ATK"; // 攻撃アニメーション
    constexpr char kWalkAnimName[] = "WALK";  // 歩行アニメーション
    constexpr char kDeadAnimName[] = "DEAD";  // 死亡アニメーション

    const VECTOR kHeadShotPositionOffset = { 0.0f, 0.0f, 0.0f }; // オフセット

    // カプセルコライダーのサイズを定義
    constexpr float kBodyColliderRadius = 20.0f;  // 体のコライダー半径
    constexpr float kBodyColliderHeight = 110.0f; // 体のコライダー高さ
    constexpr float kHeadRadius = 12.0f;          // 頭のコライダー半径

    // 攻撃関連
    constexpr int kAttackCooldownMax = 45;       // 攻撃クールダウン時間
    constexpr float kAttackHitRadius = 45.0f;    // 攻撃の当たり判定半径
    constexpr float kAttackRangeRadius = 120.0f; // 攻撃範囲の半径

    // 追跡関連
    constexpr float kChaseStopDistance = 50.0f; // 追跡停止距離

    // 攻撃後の硬直時間
    constexpr int kAttackEndDelay = 20;

    // ダメージ（怯み）時間
    constexpr int kDamageDuration = 30;

    // 描画距離
    constexpr float kDrawDistanceSq = 16000.0f * 16000.0f;
    constexpr float kDrawNearDistanceSq = 300.0f * 300.0f;
    constexpr float kDrawDotThreshold = 0.4f;

    // 押し出し
    constexpr float kPushBackEpsilon = 0.0001f;
}

int EnemyNormal::s_modelHandle = -1;

EnemyNormal::EnemyNormal()
    : m_headPosOffset(EnemyNormalConstants::kHeadShotPositionOffset)
    , m_isTackleHit(false)
    , m_animTime(0.0f)
    , m_isAttackHit(false)
    , m_onDropItem(nullptr)
    , m_currentAnimState(AnimState::Walk)
    , m_attackEndDelayTimer(0)
    , m_isDeadAnimPlaying(false)
    , m_chaseSpeed(0.0f)
    , m_isItemDropped(false)
{
    // モデルの複製
    m_modelHandle = MV1DuplicateModel(s_modelHandle);

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

void EnemyNormal::LoadModel()
{
    s_modelHandle = MV1LoadModel("data/model/NormalZombie.mv1");
    assert(s_modelHandle != -1);
}

void EnemyNormal::DeleteModel()
{
    MV1DeleteModel(s_modelHandle);
}

void EnemyNormal::Init()
{
    m_attackCooldownMax = EnemyNormalConstants::kAttackCooldownMax;

    m_isAlive = true;
    m_isItemDropped = false;
    m_lastHitPart = HitPart::None; // 最後のヒット部位をリセット
    m_hitDisplayTimer = 0;         // ヒット表示タイマーもリセット
    m_damageTimer = 0;             // ダメージタイマー初期化
    m_isBlownAway = false;         // 吹き飛びフラグリセット
    m_deathKnockbackSpeed = 0.0f;  // 速度リセット

    // CSVからNormalEnemyのTransform情報を取得
    auto dataList = TransformDataLoader::LoadDataCSV("data/CSV/CharacterTransfromData.csv");
    for (const auto& data : dataList)
    {
        if (data.name == "NormalEnemy")
        {
            MV1SetRotationXYZ(m_modelHandle, data.rot);
            MV1SetScale(m_modelHandle, data.scale);
            m_attackPower = data.attack;
            m_hp = data.hp;
            m_chaseSpeed = data.chaseSpeed;
            break;
        }
    }

    // ここで一度「絶対にWalkでない値」にリセット
    // 初期アニメーションを強制的に再生させるため
    m_currentAnimState = AnimState::Dead;

    // 初期化時に歩行アニメーションを開始
    ChangeAnimation(AnimState::Walk, true);

    // ターゲットオフセットの初期化 (±100.0f)
    float offsetX = static_cast<float>(GetRand(200) - 100);
    float offsetZ = static_cast<float>(GetRand(200) - 100);
    m_targetOffset = VGet(offsetX, 0.0f, offsetZ);

    // 徘徊用パラメータ初期化
    m_wanderTimer = 0;
    m_wanderOffset = VGet(0.0f, 0.0f, 0.0f);
}

// アニメーションを変更する
void EnemyNormal::ChangeAnimation(AnimState newAnimState, bool loop)
{
    if (m_currentAnimState == newAnimState)
    {
        if (newAnimState == AnimState::Attack)
        {
            // 同じ攻撃アニメーションをリセットして再開
            m_animationManager.PlayAnimation(m_modelHandle, EnemyNormalConstants::kAttackAnimName, loop);
        }
        else return;
    }

    const char* animName = nullptr;

    switch (newAnimState)
    {
    case AnimState::Walk:
        animName = EnemyNormalConstants::kWalkAnimName;
        break;
    case AnimState::Attack:
        animName = EnemyNormalConstants::kAttackAnimName;
        break;
    case AnimState::Damage: // ダメージ（怯み）時は歩行モーションなどを流用（あるいは専用）
        animName = EnemyNormalConstants::kWalkAnimName;
        break;
    case AnimState::Dead:
        animName = EnemyNormalConstants::kDeadAnimName;
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
    if (handRIndex == -1 || handLIndex == -1)
        return false;

    VECTOR handRPos = MV1GetFramePosition(m_modelHandle, handRIndex);
    VECTOR handLPos = MV1GetFramePosition(m_modelHandle, handLIndex);

    m_pAttackHitCollider->SetSegment(handRPos, handLPos);
    m_pAttackHitCollider->SetRadius(EnemyNormalConstants::kAttackHitRadius);

    std::shared_ptr<CapsuleCollider> playerBodyCollider = player.GetBodyCollider();
    return m_pAttackHitCollider->IsIntersects(playerBodyCollider.get());
}

void EnemyNormal::Update(const EnemyUpdateContext& context)
{
    // コンテキストから必要な変数を展開
    std::vector<Bullet>& bullets = context.bullets;
    const Player::TackleInfo& tackleInfo = context.tackleInfo;
    const Player& player = context.player;
    const std::vector<EnemyBase*>& enemyList = context.enemyList;
    const std::vector<Stage::StageCollisionData>& collisionData = context.collisionData;
    Effect* pEffect = context.pEffect;

    // AI間引き処理の更新
    UpdateThrottling(player.GetPos());

    // 視界外の単純動作モード
    if (m_isSimpleMode)
    {
        // ステージとの当たり判定のみ簡易に行う（重力適用など）
        UpdateStageCollision(collisionData);
        return;
    }

    // ステージとの当たり判定
    UpdateStageCollision(collisionData);

    // コライダーの更新 (死亡中も判定を残すため、死亡チェックの前に移動)
    // 体のコライダー（カプセル）
    VECTOR bodyCapA = VAdd(m_pos, VGet(0, EnemyNormalConstants::kBodyColliderRadius, 0));
    VECTOR bodyCapB = VAdd(m_pos, VGet(0, EnemyNormalConstants::kBodyColliderHeight - EnemyNormalConstants::kBodyColliderRadius, 0));
    m_pBodyCollider->SetSegment(bodyCapA, bodyCapB);
    m_pBodyCollider->SetRadius(EnemyNormalConstants::kBodyColliderRadius);

    // 頭のコライダー（球）
    int headIndex = MV1SearchFrame(m_modelHandle, "Head");
    VECTOR headModelPos = (headIndex != -1)
                              ? MV1GetFramePosition(m_modelHandle, headIndex)
                              : VGet(0, 0, 0);
    VECTOR headCenter = VAdd(headModelPos, m_headPosOffset); // モデルの頭のフレーム位置にオフセットを適用
    m_pHeadCollider->SetCenter(headCenter);
    m_pHeadCollider->SetRadius(EnemyNormalConstants::kHeadRadius);

    // 攻撃範囲のコライダー（球）
    VECTOR attackRangeCenter = m_pos;
    attackRangeCenter.y += (EnemyNormalConstants::kBodyColliderHeight * 0.5f);
    m_pAttackRangeCollider->SetCenter(attackRangeCenter);
    m_pAttackRangeCollider->SetRadius(EnemyNormalConstants::kAttackRangeRadius);

    if (m_hp <= 0.0f)
    {
        if (!m_isDeadAnimPlaying)
        {
            // スコア加算処理はTakeDamageで行うのでここでは不要
            ChangeAnimation(AnimState::Dead, false);
            m_isDeadAnimPlaying = true;
            m_animTime = 0.0f; // アニメーション時間をリセット
            m_isAlive = true;  // 死亡アニメーション中はtrueのまま
        }

        // 死亡アニメーション中もアニメーション時間を更新
        if (m_animationManager.GetCurrentAttachedAnimHandle(m_modelHandle) != -1)
        {
            if (m_shouldUpdateAI)
            {
                m_animTime += (1.0f * m_aiUpdateInterval) * Game::GetTimeScale();
                m_animationManager.UpdateAnimationTime(m_modelHandle, m_animTime);
            }
        }

        // 死亡吹き飛び処理
        if (m_isBlownAway && m_deathKnockbackSpeed > 0.0f)
        {
            // 移動
            m_pos = VAdd(m_pos, VScale(m_deathKnockbackDir, m_deathKnockbackSpeed * Game::GetTimeScale()));
            // 減速 (摩擦)
            m_deathKnockbackSpeed -= 0.5f * Game::GetTimeScale();
            if (m_deathKnockbackSpeed < 0.0f)
                m_deathKnockbackSpeed = 0.0f;

            // ステージ衝突判定 (壁抜け防止)
            UpdateStageCollision(collisionData);
        }

        // モデル位置更新 (死亡中も移動するため必要)
        MV1SetPosition(m_modelHandle, m_pos);

        float currentAnimTotalTime = m_animationManager.GetAnimationTotalTime(m_modelHandle, EnemyNormalConstants::kDeadAnimName);
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
        return;
    }

    MV1SetPosition(m_modelHandle, m_pos); // モデルの位置は常に反映する

    // 攻撃アニメーション中は追尾しない
    if (m_currentAnimState == AnimState::Walk) // 追尾はWalk状態でのみ行う
    {
        VECTOR playerPos = player.GetPos();
        VECTOR targetPos;

        // プレイヤーが岩の上にいる場合は、プレイヤーの周囲をうろうろする
        std::string groundObj = player.GetGroundedObjectName();
        if (groundObj == "rock_3_br" || groundObj == "rock_6_br")
        {
            m_wanderTimer--;
            if (m_wanderTimer <= 0)
            {
                // 2秒ごとに新しい目標位置を設定 (距離300~700) - 範囲拡大
                m_wanderTimer = 120;
                float angle = static_cast<float>(GetRand(360)) * DX_PI_F / 180.0f;
                float dist = static_cast<float>(300 + GetRand(400));
                m_wanderOffset = VGet(cosf(angle) * dist, 0.0f, sinf(angle) * dist);
            }
            targetPos = VAdd(playerPos, m_wanderOffset);
        }
        else
        {
            // 通常時はプレイヤーに向かう（オフセット付き）
            targetPos = VAdd(playerPos, m_targetOffset);
        }

        VECTOR toPlayer = VSub(targetPos, m_pos);
        toPlayer.y = 0.0f; // Y成分を無視して水平距離を計算

        // プレイヤー(ターゲット)との距離を計算
        float disToPlayer = VSize(toPlayer);

        // プレイヤーの方向を常に向く
        float yaw = 0.0f;
        if (toPlayer.x != 0.0f || toPlayer.z != 0.0f)
        {
            yaw = atan2f(toPlayer.x, toPlayer.z);
            yaw += DX_PI_F; // モデルの向きに合わせて調整
        }

        // 補間速度(0.05f で滑らかにする)
        float rotSpeed = 0.05f * Game::GetTimeScale();
        float currentYaw = MV1GetRotationXYZ(m_modelHandle).y;

        // 角度差を計算して滑らかに回転
        float diffYaw = yaw - currentYaw;
        while (diffYaw <= -DX_PI_F) diffYaw += DX_TWO_PI_F;
        while (diffYaw > DX_PI_F) diffYaw -= DX_TWO_PI_F;

        if (fabs(diffYaw) > rotSpeed)
        {
            currentYaw += (diffYaw > 0 ? rotSpeed : -rotSpeed);
        }
        else
        {
            currentYaw = yaw;
        }

        MV1SetRotationXYZ(m_modelHandle, VGet(0.0f, currentYaw, 0.0f));

        // 移動処理
        if (disToPlayer > EnemyNormalConstants::kChaseStopDistance)
        {
            VECTOR dir = VNorm(toPlayer);
            float moveDist = disToPlayer - EnemyNormalConstants::kChaseStopDistance;
            // タイムスケールを移動速度に適用
            float scaledSpeed = m_chaseSpeed * Game::GetTimeScale();
            float step = (std::min)(moveDist, scaledSpeed); // 1フレームで進みすぎない
            m_pos.x += dir.x * step;
            m_pos.z += dir.z * step;
        }
    }

    // プレイヤーのカプセルコライダー情報を取得
    std::shared_ptr<CapsuleCollider> playerBodyCollider = player.GetBodyCollider();
    bool isPlayerInAttackRange = m_pAttackRangeCollider->IsIntersects(playerBodyCollider.get());

    // アニメーションの状態管理 (AI間引き対象)
    if (m_shouldUpdateAI)
    {
        if (m_currentAnimState == AnimState::Attack)
        {
            // 攻撃アニメーションはループしないので、終了したらディレイタイマーをセット
            float currentAnimTotalTime = m_animationManager.GetAnimationTotalTime(m_modelHandle, EnemyNormalConstants::kAttackAnimName);
            if (m_animTime > currentAnimTotalTime)
            {
                if (m_attackEndDelayTimer <= 0) m_attackEndDelayTimer = EnemyNormalConstants::kAttackEndDelay; // ディレイ開始
            }
            // ディレイタイマーが動作中ならカウントダウン
            if (m_attackEndDelayTimer > 0)
            {
                m_attackEndDelayTimer -= m_aiUpdateInterval; // 間引き分減算
                if (m_attackEndDelayTimer <= 0)
                {
                    m_isAttackHit = false; // 攻撃ヒットフラグをリセット
                    if (isPlayerInAttackRange)
                    {
                        ChangeAnimation(AnimState::Attack, false); // 攻撃範囲内なら再度攻撃
                    }
                    else
                    {
                        ChangeAnimation(AnimState::Walk, true); // 範囲外なら歩行
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
                m_isAttackHit = false;
                ChangeAnimation(AnimState::Attack, false);
            }
        }
    }

    // アニメーションがアタッチされている場合のみ時間を更新 (AI間引き対象)
    if (m_shouldUpdateAI && m_animationManager.GetCurrentAttachedAnimHandle(m_modelHandle) != -1)
    {
        m_animTime += (1.0f * m_aiUpdateInterval) * Game::GetTimeScale();

        const char* animName = nullptr;
        if (m_currentAnimState == AnimState::Walk || m_currentAnimState == AnimState::Damage)
        {
            animName = EnemyNormalConstants::kWalkAnimName;
        }
        else if (m_currentAnimState == AnimState::Attack)
        {
            animName = EnemyNormalConstants::kAttackAnimName;
        }
        else
        {
            animName = EnemyNormalConstants::kDeadAnimName;
        }

        float currentAnimTotalTime = m_animationManager.GetAnimationTotalTime(m_modelHandle, animName);

        if (m_currentAnimState == AnimState::Attack)
        {
            // ここは何もしない
        }
        else if (m_currentAnimState == AnimState::Dead)
        {
            if (m_animTime >= currentAnimTotalTime)
            {
                m_animTime = currentAnimTotalTime;
            }
        }
        else if (m_currentAnimState == AnimState::Walk || m_currentAnimState == AnimState::Damage)
        {
            if (m_animTime >= currentAnimTotalTime)
            {
                m_animTime = fmodf(m_animTime, currentAnimTotalTime);
            }
        }
        // AnimationManagerにアニメーション時間の更新を依頼
        m_animationManager.UpdateAnimationTime(m_modelHandle, m_animTime);
    }

    // コライダーの更新は上部に移動済み

    // 敵とプレイヤーの押し出し処理（カプセル同士の衝突）
    if (m_pBodyCollider->IsIntersects(playerBodyCollider.get()))
    {
        // 押し出し処理
        VECTOR enemyCenter = VScale(VAdd(m_pBodyCollider->GetSegmentA(), m_pBodyCollider->GetSegmentB()), 0.5f);
        VECTOR playerCenter = VScale(VAdd(playerBodyCollider->GetSegmentA(), playerBodyCollider->GetSegmentB()), 0.5f);
        VECTOR diff = VSub(enemyCenter, playerCenter);
        float distSq = VDot(diff, diff);
        float minDist = EnemyNormalConstants::kBodyColliderRadius + playerBodyCollider->GetRadius(); // 最小距離は両方の半径の和

        if (distSq < minDist * minDist && distSq > EnemyNormalConstants::kPushBackEpsilon)
        {
            float dist = std::sqrt(distSq);
            float pushBack = minDist - dist;
            if (dist > 0)
            {
                VECTOR pushDir = VSub(enemyCenter, playerCenter);
                pushDir.y = 0.0f; // Y成分を無視して水平方向の押し出しにする
                float horizontalDistSq = VDot(pushDir, pushDir);

                if (horizontalDistSq > EnemyNormalConstants::kPushBackEpsilon) // 水平方向の成分がある場合のみ正規化して適用
                {
                    pushDir = VNorm(pushDir); // Y成分を0にした後に正規化
                    m_pos = VAdd(m_pos, VScale(pushDir, pushBack * 0.5f));
                }
            }
        }
    }

    // 敵同士の押し出し処理（横方向への広がり）
    std::vector<EnemyBase*> neighbors;
    if (context.collisionGrid)
    {
        context.collisionGrid->GetNeighbors(m_pos, neighbors);
    }
    const std::vector<EnemyBase*>& targets = (context.collisionGrid) ? neighbors : enemyList;

    for (EnemyBase* other : targets)
    {
        if (!other) continue;
        // 自分自身は除外
        if (other == this) continue;

        // 位置取得
        VECTOR otherPos = other->GetPos();
        VECTOR diff = VSub(m_pos, otherPos);

        diff.y = 0.0f;
        float distSq = VDot(diff, diff);
        float minDist = EnemyNormalConstants::kBodyColliderRadius * 2.0f; // 体の半径×2
        if (distSq < minDist * minDist && distSq > EnemyNormalConstants::kPushBackEpsilon)
        {
            float dist = std::sqrt(distSq);
            float pushBack = minDist - dist;
            if (dist > 0)
            {
                VECTOR pushDir = VNorm(diff);
                // 横方向に広がるように、プレイヤー方向ベクトルと直交する方向に少し加算
                VECTOR playerDir = VNorm(VSub(player.GetPos(), m_pos));
                VECTOR up = VGet(0, 1, 0);
                VECTOR side = VNorm(VCross(playerDir, up));
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
        // m_shouldUpdateAI
        // で判定して、間引き時は前回の結果を維持（あるいはスキップ）
        // ここでは攻撃の当たり判定は重いので間引き対象にする
        if (m_shouldUpdateAI)
        {
            // m_currentAnimTotalTime を AnimationManager から取得
            float currentAnimTotalTime = m_animationManager.GetAnimationTotalTime(m_modelHandle, EnemyNormalConstants::kAttackAnimName);
            float attackStart = currentAnimTotalTime * 0.5f; // 攻撃開始時間
            float attackEnd = currentAnimTotalTime * 0.7f;   // 攻撃終了時間

            // 攻撃アニメーションの範囲内でのみ攻撃判定を行う
            if (!m_isAttackHit && m_animTime >= attackStart && m_animTime <= attackEnd)
            {
                int handRIndex = MV1SearchFrame(m_modelHandle, "Hand_R");
                int handLIndex = MV1SearchFrame(m_modelHandle, "Hand_L");
                if (handRIndex != -1 && handLIndex != -1)
                {
                    VECTOR handRPos = MV1GetFramePosition(m_modelHandle, handRIndex);
                    VECTOR handLPos = MV1GetFramePosition(m_modelHandle, handLIndex);

                    // 攻撃ヒット用コライダーの更新
                    m_pAttackHitCollider->SetSegment(handRPos, handLPos);
                    m_pAttackHitCollider->SetRadius(EnemyNormalConstants::kAttackHitRadius);

                    if (m_pAttackHitCollider->IsIntersects(playerBodyCollider.get()))
                    {
                        const_cast<Player&>(player).TakeDamage(m_attackPower, m_pos); // プレイヤーにダメージ（攻撃者の位置を渡す）
                        m_isAttackHit = true;
                    }
                }
            }
        }
    }
    else if (m_currentAnimState == AnimState::Damage)
    {
        // ダメージ（怯み）状態の更新
        if (m_damageTimer > 0)
        {
            m_damageTimer--;
            if (m_damageTimer <= 0)
            {
                ChangeAnimation(AnimState::Walk, true); // 復帰
            }
        }
        // ノックバック処理（少し後ろに下がる）
        // プレイヤーと逆方向に少し移動
        VECTOR toPlayer = VSub(player.GetPos(), m_pos);
        toPlayer.y = 0.0f;
        if (VSquareSize(toPlayer) > EnemyNormalConstants::kPushBackEpsilon)
        {
            VECTOR knockbackDir = VNorm(VScale(toPlayer, -1.0f));
            // 減衰させつつ移動
            if (m_damageTimer > 10) // 最初だけ下がる
            {
                float knockbackSpeed = 2.0f * Game::GetTimeScale();
                m_pos = VAdd(m_pos, VScale(knockbackDir, knockbackSpeed));
            }
        }
    }

    CheckHitAndDamage(const_cast<std::vector<Bullet>&>(bullets), pEffect);

    if (tackleInfo.isTackling && m_hp > 0.0f && tackleInfo.tackleId != m_lastTackleId)
    {
        CapsuleCollider playerTackleCollider(tackleInfo.capA, tackleInfo.capB, tackleInfo.radius);

        if (m_pBodyCollider->IsIntersects(&playerTackleCollider))
        {
            TakeTackleDamage(tackleInfo.damage);
            m_lastTackleId = tackleInfo.tackleId;
        }
    }
    else if (!tackleInfo.isTackling)
    {
        m_lastTackleId = -1;
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
    // 死亡アニメーション終了後も描画しない
    if (m_hp <= 0.0f && m_animationManager.GetCurrentAttachedAnimHandle(m_modelHandle) == -1)
        return;

    // 視錐台カリング (描画最適化)
    // CheckCameraViewClip系の関数が環境によって不安定なため、
    // 手動で「カメラ前方への内積チェック(簡易コーン判定)」を行う
    VECTOR camPos = GetCameraPosition();
    VECTOR camTarget = GetCameraTarget();
    VECTOR camDir = VNorm(VSub(camTarget, camPos));
    VECTOR toEnemy = VSub(m_pos, camPos);
    float distSq = VSquareSize(toEnemy);

    // 1. 距離チェック (Farクリップ + マージン)
    if (distSq > EnemyNormalConstants::kDrawDistanceSq)
        return;

    // 2. 画角チェック (内積)
    // 近距離(300.0f以内)なら無条件で描画 (すり抜け防止)
    if (distSq > EnemyNormalConstants::kDrawNearDistanceSq)
    {
        VECTOR dirToEnemy = VNorm(toEnemy);
        float dot = VDot(camDir, dirToEnemy);
        // 視野角90度(cos45=0.7)に対し、余裕を持ってcos66=0.4程度以上なら描画
        // これにより視野外でも少し描画されるが、消えるよりは安全
        if (dot < EnemyNormalConstants::kDrawDotThreshold) return;
    }

    EnemyBase::IncrementDrawCount();
    MV1DrawModel(m_modelHandle);

#ifdef _DEBUG
    DrawCollisionDebug();

    const char* hitMsg = "";

    switch (m_lastHitPart)
    {
    case HitPart::Head:
        hitMsg = "ヘッドショット！";
        break;
    case HitPart::Body:
        hitMsg = "ボディヒット！";
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
        DebugUtil::DrawCapsule(handRPos, handLPos, EnemyNormalConstants::kAttackHitRadius, 16, 0x0000ff);
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
    bool headHit = m_pHeadCollider->IsIsIntersectsRay(rayStart, rayEnd, hitPosHead, hitDistSqHead);
    bool bodyHit = m_pBodyCollider->IsIsIntersectsRay(rayStart, rayEnd, hitPosBody, hitDistSqBody);

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

// 部位ごとのダメージ計算処理
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

// アイテムドロップ時のコールバック関数
void EnemyNormal::SetOnDropItemCallback(std::function<void(const VECTOR&)> cb)
{
    m_onDropItem = cb;
}

// ダメージ処理
void EnemyNormal::TakeDamage(float damage, AttackType type)
{
    EnemyBase::TakeDamage(damage, type);

    // ショットガンによる怯み処理
    if (type == AttackType::Shotgun && m_hp > 0.0f)
    {
        // 既に怯み中でなければ、あるいは上書きありなら
        ChangeAnimation(AnimState::Damage, false);                // ダメージモーションへ
        m_damageTimer = EnemyNormalConstants::kDamageDuration; // 30フレーム
    }

    // HP減算・死亡判定は基底クラスで行う
    if (m_hp <= 0.0f) // 死亡時一度だけ
    {
        if (type == AttackType::Shotgun)
        {
            m_isBlownAway = true;
            // プレイヤーと逆方向に吹き飛ぶ
            if (SceneMain::Instance())
            {
                VECTOR playerPos = SceneMain::Instance()->GetPlayer().GetPos();
                VECTOR toEnemy = VSub(m_pos, playerPos);
                toEnemy.y = 0.0f; // 水平方向のみ
                if (VSquareSize(toEnemy) > EnemyNormalConstants::kPushBackEpsilon)
                {
                    m_deathKnockbackDir = VNorm(toEnemy);
                    m_deathKnockbackSpeed = 15.0f; // 初速を強化
                }
            }

            // フォールバック: もし速度が設定されなかった場合（プレイヤー位置取得失敗 or 重なり）
            if (m_deathKnockbackSpeed <= 0.0f)
            {
                // 敵の向いている方向の逆（後ろ）へ飛ばす
                MATRIX worldMat = MV1GetLocalWorldMatrix(m_modelHandle);
                VECTOR forward = VGet(worldMat.m[2][0], worldMat.m[2][1], worldMat.m[2][2]);
                forward.y = 0.0f;
                if (VSquareSize(forward) > EnemyNormalConstants::kPushBackEpsilon)
                {
                    m_deathKnockbackDir = VScale(VNorm(forward), -1.0f);
                }
                else
                {
                    m_deathKnockbackDir = VGet(0, 0, 1); // 完全なフォールバック
                }
                m_deathKnockbackSpeed = 15.0f;
            }
        }

        if (m_lastHitPart == HitPart::None)
            m_lastHitPart = HitPart::Body;
        bool isHeadShot = (m_lastHitPart == HitPart::Head);
        int addScore = ScoreManager::Instance().AddScore(isHeadShot);
        if (SceneMain::Instance())
        {
            SceneMain::Instance()->AddScorePopup(addScore, isHeadShot, ScoreManager::Instance().GetCombo());
        }
    }
}

// タックル攻撃のダメージ処理
void EnemyNormal::TakeTackleDamage(float damage)
{
    EnemyBase::TakeTackleDamage(damage);
}

std::shared_ptr<CapsuleCollider> EnemyNormal::GetBodyCollider() const
{
    return m_pBodyCollider;
}