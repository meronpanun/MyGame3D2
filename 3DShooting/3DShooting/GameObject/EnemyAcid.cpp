#include "EnemyAcid.h"    
#include "Player.h"      
#include "EffekseerForDXLib.h"       
#include "DebugUtil.h"   
#include "SphereCollider.h" 
#include "CapsuleCollider.h" 
#include "SceneMain.h"
#include "TransformDataLoader.h"
#include "Effect.h"
#include <cassert>       
#include <algorithm>     
#include <cmath> 
#include <functional>    

namespace
{
    // アニメーション関連
    constexpr char kAttackAnimName[] = "Armature|ATK";  // 攻撃アニメーション
    constexpr char kWalkAnimName[] = "Armature|WALK"; // 歩くアニメーション
    constexpr char kBackAnimName[] = "Armature|BACK"; // 後退アニメーション
    constexpr char kDeadAnimName[] = "Armature|DEAD"; // 死亡アニメーション

    constexpr VECTOR kHeadShotPositionOffset = { 0.0f, 0.0f, 0.0f }; // オフセット

    // コライダーのサイズを定義
    constexpr float kBodyColliderRadius = 40.0f;  // 体のコライダー半径
    constexpr float kBodyColliderHeight = 50.0f;  // 体のコライダー高さ
    constexpr float kHeadRadius = 18.0f;  // 頭のコライダー半径

    // 攻撃関連（遠距離攻撃に特化）
    constexpr int   kAttackCooldownMax = 160;     // 攻撃クールダウン時間
    constexpr float kAttackRangeRadius = 1000.0f; // 攻撃範囲の半径
    constexpr float kAcidBulletSpeed = 5.0f;    // 酸弾の速度

    // 追跡関連（遠距離型なので、近づきすぎたら離れる）
    constexpr float kOptimalAttackDistanceMin = 500.0f; // 攻撃可能最小距離

    // スタン関連
    constexpr int kStunDuration = 120;           // スタンの総持続時間
    constexpr float kStunAnimFrameLimit = 60.0f; // スタンアニメーションの再生上限フレーム

    // AcidBallの画面外判定距離
    constexpr float kAcidBallBoundaryDistance = 2000.0f;
}

int EnemyAcid::s_modelHandle = -1;

EnemyAcid::EnemyAcid() :
    m_headPosOffset{ kHeadShotPositionOffset },
    m_animTime(0.0f),
    m_currentAnimState(AnimState::Walk),
    m_onDropItem(nullptr),
    m_hasAttacked(false),
    m_attackEndDelayTimer(0),
    m_acidBulletSpawnOffset({ 0.0f, 0.0f, 0.0f }),
    m_backAnimCount(0),
    m_isItemDropped(false),
    m_chaseSpeed(0.0f),
    m_isStunned(false),
    m_stunTimer(0)
{
    // モデルの複製
    m_modelHandle = MV1DuplicateModel(s_modelHandle);

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
    m_attackCooldownMax = kAttackCooldownMax;
    m_attackCooldown = 0; // 最初は攻撃可能にしておく

    m_isAlive = true;
    m_isDeadAnimPlaying = false;

    // CSVからAcidEnemyのTransform情報を取得
    auto dataList = TransformDataLoader::LoadDataCSV("data/CSV/CharacterTransfromData.csv");
    for (const auto& data : dataList)
    {
        if (data.name == "AcidEnemy")
        {
            MV1SetRotationXYZ(m_modelHandle, data.rot);
            MV1SetScale(m_modelHandle, data.scale);
            m_attackPower = data.attack;
            m_hp = data.hp;
            m_chaseSpeed = data.chaseSpeed;
            break;
        }
    }

    // ここで一度「絶対にRunでない値」にリセット
    // 初期アニメーションを強制的に再生させるため
    m_currentAnimState = AnimState::Dead;

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
    return m_pAttackRangeCollider->IsIntersects(playerBodyCollider.get());
}

// 酸を吐く攻撃を行う
void EnemyAcid::ShootAcidBullet(std::vector<Bullet>& bullets, const Player& player, Effect* pEffect)
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
    ball.dir = VNorm(toTarget);
    ball.speed = kAcidBulletSpeed;
    ball.active = true;
    ball.damage = m_attackPower;
    ball.owner = this;
    ball.isReflected = false;
    if (pEffect)
    {
        ball.effectHandle = pEffect->PlayAcidEffect(spawnPos.x, spawnPos.y, spawnPos.z);
    }
    m_acidBalls.push_back(ball);
}

void EnemyAcid::Update(std::vector<Bullet>& bullets, const Player::TackleInfo& tackleInfo, const Player& player, const std::vector<EnemyBase*>& enemyList, const std::vector<Stage::StageCollisionData>& collisionData, Effect* pEffect)
{
    // ステージとの当たり判定
    UpdateStageCollision(collisionData);

#ifdef _DEBUG
    m_shouldDrawParryCollider = false;
#endif

    // AcidBallの更新と当たり判定をスタン状態に関わらず行う
    for (auto& ball : m_acidBalls)
    {
        if (!ball.active) continue;
        ball.Update();

        if (ball.effectHandle != -1)
        {
            SetPosPlayingEffekseer3DEffect(ball.effectHandle, ball.pos.x, ball.pos.y, ball.pos.z);
        }

        // プレイヤーからの距離を計算
        VECTOR toPlayer = VSub(ball.pos, player.GetPos());
        float distanceToPlayer = sqrtf(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y + toPlayer.z * toPlayer.z);

        // プレイヤーから一定距離以上離れたら非アクティブにする
        if (distanceToPlayer > kAcidBallBoundaryDistance)
        {
            ball.active = false;
            if (ball.effectHandle != -1)
            {
                StopEffekseer3DEffect(ball.effectHandle);
                ball.effectHandle = -1;
            }
            continue; // 以降の処理をスキップ
        }

        // まだ反射されていない弾の処理
        if (!ball.isReflected)
        {
            SphereCollider acidCol(ball.pos, ball.radius);
            bool hitDetected = false;

            // パリィ判定を先に行う
            if (player.IsJustGuarded())
            {
                // パリィ用の拡大コライダーを作成
                VECTOR playerCapA, playerCapB;
                float playerRadius;
                player.GetCapsuleInfo(playerCapA, playerCapB, playerRadius);

                // 半径を1.5倍にしてパリィ判定を甘くする
                float parryRadius = playerRadius * 1.5f;
                CapsuleCollider parryCollider(playerCapA, playerCapB, parryRadius);

#ifdef _DEBUG
                // デバッグ描画用に情報を保存
                m_shouldDrawParryCollider = true;
                m_debugParryCapA = playerCapA;
                m_debugParryCapB = playerCapB;
                m_debugParryRadius = parryRadius;
#endif

                if (acidCol.IsIntersects(&parryCollider))
                {
                    // パリィ成功
                    hitDetected = true;
                    ball.isReflected = true;

                    // プレイヤーSからカメラを取得
                    const auto& playerCam = player.GetCamera();
                    if (playerCam)
                    {
                        // カメラの前方ベクトルを計算
                        VECTOR camForward = VNorm(VSub(playerCam->GetTarget(), playerCam->GetPos()));
                        // プレイヤーの足元から、盾があるであろう前方少し上の位置を計算
                        VECTOR shieldPos = player.GetPos();
                        shieldPos.y += 50.0f; // 少し上に
                        shieldPos = VAdd(shieldPos, VScale(camForward, 60.0f)); // 前方に60

                        // 反射方向を「盾の位置」から「敵の位置」へ
                        VECTOR reflectDir = VNorm(VSub(this->GetPos(), shieldPos));
                        ball.dir = reflectDir;

                        // 反射した弾の速度を上げる
                        ball.speed *= 1.5f;
                    }
                }
            }

            // パリィが成功しなかった場合、通常の当たり判定を行う
            if (!hitDetected)
            {
                std::shared_ptr<CapsuleCollider> playerCol = player.GetBodyCollider();
                if (acidCol.IsIntersects(playerCol.get()))
                {
                    hitDetected = true;
                    // 通常ガードか、被弾か
                    if (player.IsGuarding())
                    {
                        // 通常ガード時の処理（ダメージを受ける）
                        const_cast<Player&>(player).TakeDamage(ball.damage, m_pos);
                    }
                    else
                    {
                        // ガードしていない場合
                        const_cast<Player&>(player).TakeDamage(ball.damage, m_pos);
                    }

                    // 弾を非アクティブ化
                    ball.active = false;
                    if (ball.effectHandle != -1)
                    {
                        StopEffekseer3DEffect(ball.effectHandle);
                        ball.effectHandle = -1;
                    }
                }
            }
        }
        // 反射された弾の処理
        else
        {
            // この弾の所有者(ball.owner)と当たり判定を行う
            if (ball.owner) // ownerがnullptrでないことを確認
            {
                SphereCollider reflectedAcidCol(ball.pos, ball.radius);
                // 敵の体のコライダーと判定
                if (reflectedAcidCol.IsIntersects(this->GetBodyCollider().get()))
                {
                    // 自分自身にダメージ
                    this->TakeDamage(ball.damage, AttackType::Shoot); // AttackTypeは適切なものを選ぶ
                    // 敵を怯ませる
                    this->OnParried();
                    ball.active = false;
                    if (ball.effectHandle != -1)
                    {
                        StopEffekseer3DEffect(ball.effectHandle);
                        ball.effectHandle = -1;
                    }
                }
            }
        }

        // 地面に着弾
        if (ball.pos.y < 0.0f)
        {
            ball.active = false;
            if (ball.effectHandle != -1)
            {
                StopEffekseer3DEffect(ball.effectHandle);
                ball.effectHandle = -1;
            }
        }
    }

    // 非アクティブなAcidBallを削除
    m_acidBalls.erase(std::remove_if(m_acidBalls.begin(), m_acidBalls.end(),
        [](const AcidBall& b) { return !b.active; }), m_acidBalls.end());

    // 弾との当たり判定・ダメージ処理
    CheckHitAndDamage(bullets, pEffect);

    // タックルダメージ処理
    if (tackleInfo.isTackling && tackleInfo.tackleId != m_lastTackleId)
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
        ResetTackleHitFlag();
        m_lastTackleId = -1;
    }

    // コライダーの更新
    int hipsIndex = MV1SearchFrame(m_modelHandle, "mixamorig:Hips");
    VECTOR hipsPos = (hipsIndex != -1) ? MV1GetFramePosition(m_modelHandle, hipsIndex) : m_pos;

    VECTOR bodySegmentP1 = VAdd(hipsPos, VGet(0, ::kBodyColliderHeight * 0.5f, 0));
    VECTOR bodySegmentP2 = VAdd(hipsPos, VGet(0, -::kBodyColliderHeight * 0.5f, 0));
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

    // 怯み状態の処理
    if (m_isStunned)
    {
        m_stunTimer--;
        if (m_stunTimer <= 0)
        {
            m_isStunned = false;
            // スタンからの復帰時の行動を決定
            if (CanAttackPlayer(player))
            {
                ChangeAnimation(AnimState::Attack, false);
                m_hasAttacked = false;
                m_attackCooldown = m_attackCooldownMax;
            }
            else
            {
                ChangeAnimation(AnimState::Walk, true); // 歩行状態に戻す
            }
        }
        else
        {
            // アニメーションをkStunAnimFrameLimitで停止
            if (m_animTime < kStunAnimFrameLimit)
            {
                m_animTime += 1.0f;
            }
            m_animationManager.UpdateAnimationTime(m_modelHandle, m_animTime);
        }
        MV1SetPosition(m_modelHandle, m_pos); // 怯み中もモデルの位置は更新
        return; // 他のAIロジックをスキップ
    }

    if (m_hp <= 0.0f)
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
        // 死亡アニメーション中は、敵のモデル更新や行動ロジックはスキップ
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

    // プレイヤーとの物理衝突判定
    std::shared_ptr<CapsuleCollider> playerBodyCollider = player.GetBodyCollider();
    if (m_pBodyCollider->IsIntersects(playerBodyCollider.get()))
    {
        VECTOR enemyCenter = VScale(VAdd(m_pBodyCollider->GetSegmentA(), m_pBodyCollider->GetSegmentB()), 0.5f);
        VECTOR playerCenter = VScale(VAdd(playerBodyCollider->GetSegmentA(), playerBodyCollider->GetSegmentB()), 0.5f);
        VECTOR diff = VSub(enemyCenter, playerCenter);

        float distSq = VDot(diff, diff);
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
    bool inAttackRange = m_pAttackRangeCollider->IsIntersects(playerBodyCollider.get());

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
            m_pos.x += dirAway.x * m_chaseSpeed;
            m_pos.z += dirAway.z * m_chaseSpeed;
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
        m_pos.x += dirTowards.x * m_chaseSpeed;
        m_pos.z += dirTowards.z * m_chaseSpeed;
    }

    // 攻撃アニメーション中の酸弾発射タイミング
    if (m_currentAnimState == AnimState::Attack)
    {
        float totalAttackAnimTime = m_animationManager.GetAnimationTotalTime(m_modelHandle, kAttackAnimName);
        if (!m_hasAttacked && m_animTime >= totalAttackAnimTime * 0.3f)
        {
            ShootAcidBullet(bullets, player, pEffect);
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

    if (m_hitDisplayTimer > 0)
    {
        m_hitDisplayTimer--;
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

void EnemyAcid::OnParried()
{
    // 既に怯んでいるか、死んでいる場合は何もしない
    if (m_isStunned || m_hp <= 0.0f)
    {
        return;
    }

    m_isStunned = true;
    m_stunTimer = kStunDuration; // 怯み時間（アニメーション50F + 硬直30F）
    ChangeAnimation(AnimState::Dead, false); // 死亡アニメーションを怯みモーションとして再生
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
    MV1DrawModel(m_modelHandle);

#ifdef _DEBUG
    // デバッグ用の当たり判定描画
    DrawCollisionDebug();

    // 体力デバッグ表示
    DebugUtil::DrawFormat(20, 100, 0xffffff, "EnemyAcid HP: %.1f", m_hp);
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
    if (m_shouldDrawParryCollider)
    {
        DebugUtil::DrawCapsule(m_debugParryCapA, m_debugParryCapB, m_debugParryRadius, 16, 0x0000ff);
    }
#endif
}

// どこに当たったのか判定する
EnemyBase::HitPart EnemyAcid::CheckHitPart(const VECTOR& rayStart, const VECTOR& rayEnd, VECTOR& outHtPos, float& outHtDistSq) const
{

    VECTOR hitPosHead, hitPosBody; // 当たった位置
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

    VECTOR bodySegmentP1 = VAdd(hipsPos, VGet(0, ::kBodyColliderHeight * 0.5f, 0));
    VECTOR bodySegmentP2 = VAdd(hipsPos, VGet(0, -::kBodyColliderHeight * 0.5f, 0));
    m_pBodyCollider->SetSegment(bodySegmentP1, bodySegmentP2);
    m_pBodyCollider->SetRadius(::kBodyColliderRadius);

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

// 部位ごとのダメージ計算
float EnemyAcid::CalcDamage(float bulletDamage, HitPart part) const
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
void EnemyAcid::SetOnDropItemCallback(std::function<void(const VECTOR&)> cb)
{
    m_onDropItem = cb;
}


// ダメージ処理
void EnemyAcid::TakeDamage(float damage, AttackType type)
{
    EnemyBase::TakeDamage(damage, type);
    // HP減算・死亡判定は基底クラスで行う
    if (m_hp <= 0.0f) // 死亡時一度だけ
    {
        if (m_lastHitPart == HitPart::None) m_lastHitPart = HitPart::Body;
        bool isHeadShot = (m_lastHitPart == HitPart::Head);
        int addScore = ScoreManager::Instance().AddScore(isHeadShot);
        if (SceneMain::Instance())
        {
            SceneMain::Instance()->AddScorePopup(addScore, isHeadShot, ScoreManager::Instance().GetCombo());
        }
    }
}

// タックル攻撃のダメージ処理
void EnemyAcid::TakeTackleDamage(float damage)
{
    EnemyBase::TakeTackleDamage(damage);
}

std::shared_ptr<CapsuleCollider> EnemyAcid::GetBodyCollider() const
{
    return m_pBodyCollider;
}

void EnemyAcid::OnDeath()
{
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