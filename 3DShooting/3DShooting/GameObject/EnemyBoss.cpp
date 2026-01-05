#include "EnemyBoss.h"
#include "Bullet.h"
#include "EffekseerForDXLib.h"
#include "DebugUtil.h"
#include "SceneMain.h"
#include "ScoreManager.h"
#include "SphereCollider.h"
#include "CapsuleCollider.h"
#include "TransformDataLoader.h" // Added
#include <cassert>
#include <algorithm>
#include <cmath>

namespace
{
    // アニメーション名
    constexpr char kIdleAnimName[]        = "IDLE"; 
    constexpr char kWalkAnimName[]        = "WALK"; 
    constexpr char kCloseAttackAnimName[] = "Armature|CloseRangeAttack"; // 近接範囲攻撃
    constexpr char kLongRangeAttackAnimName[] = "Armature|LongRangeAttack"; // 遠距離攻撃
    constexpr char kDeadAnimName[]        = "DEAD";

    // ...

    constexpr float kLongRangeAttackMinDist = 300.0f; // 遠距離攻撃を行う最小距離
    constexpr int   kLongRangeAttackCooldownMax = 120;
    constexpr float kHomingBulletSpeed =6.0f;
    constexpr float kHomingTurnRate = 0.05f; // 旋回性能
    constexpr float kHomingBulletMaxDist = 1500.0f; // 弾の最大飛距離
    constexpr float kHomingBulletDamage = 20.0f;
    constexpr float kHomingBulletRadius = 15.0f;
    constexpr float kDefaultInitialHP = 5000.0f; // ボスの初期体力
    constexpr float kChaseSpeed       = 1.5f;    // 追跡速度

    // コライダーサイズ
    constexpr float kBodyColliderRadius = 40.0f;
    constexpr float kBodyColliderHeight = 200.0f;
    constexpr float kHeadRadius         = 20.0f;
    constexpr float kAttackRangeRadius  = 150.0f; // 指定された近接範囲
    constexpr float kAttackHitRadius    = 60.0f;  // 攻撃自体の当たり判定

    constexpr int kAttackCooldownMax = 60;
    constexpr int kAttackEndDelay    = 30; // 攻撃後の硬直
}

EnemyBoss::EnemyBoss() :
    m_currentAnimState(AnimState::Idle),
    m_isDeadAnimPlaying(false),
    m_animTime(0.0f),
    m_chaseSpeed(kChaseSpeed),
    m_attackEndDelayTimer(0),
    m_isAttackHit(false)
{
    m_modelHandle = MV1LoadModel("data/model/Boss.mv1");
    // モデル読み込み失敗時はアサート
    // assert(m_modelHandle != -1); 
    // ※実機でアサートすると止まるので、エラーログを出して続行制御などは本来必要
    if (m_modelHandle == -1)
    {
        // 読み込み失敗時のフォールバックやエラー処理
        // ここでは一旦そのままスルー（描画されないだけ）
    }

    // コライダー初期化
    m_pBodyCollider        = std::make_shared<CapsuleCollider>();
    m_pHeadCollider        = std::make_shared<SphereCollider>();
    m_pAttackRangeCollider = std::make_shared<SphereCollider>();
    m_pAttackHitCollider   = std::make_shared<CapsuleCollider>();
}

EnemyBoss::~EnemyBoss()
{
    MV1DeleteModel(m_modelHandle);
}

void EnemyBoss::Init()
{
    m_hp = kDefaultInitialHP;
    m_chaseSpeed = kChaseSpeed;

    // CSVからデータをロード
    auto dataList = TransformDataLoader::LoadDataCSV("data/CSV/CharacterTransfromData.csv");
    for (const auto& data : dataList)
    {
        if (data.name == "Boss")
        {
            MV1SetRotationXYZ(m_modelHandle, data.rot);
            MV1SetScale(m_modelHandle, data.scale);
            m_attackPower = data.attack; // EnemyBaseにm_attackPowerがある前提
            m_hp = data.hp;
            m_chaseSpeed = data.chaseSpeed; // データ定義にあるspeedを使用
            break;
        }
    }

    m_isAlive = true;
    m_isDeadAnimPlaying = false;
    m_animTime = 0.0f;
    
    // 位置はWaveManagerでセットされるが、初期値として
    m_pos = VGet(0.0f, 0.0f, 1000.0f); 
    MV1SetPosition(m_modelHandle, m_pos);
    
    // 攻撃範囲コライダー設定
    m_pAttackRangeCollider->SetRadius(kAttackRangeRadius);

    m_longRangeAttackCooldown = 0;
    m_homingBullets.clear();
    m_hasShotLongRange = false;

    ChangeAnimation(AnimState::Walk, true); // 最初は歩いて近づく
}

void EnemyBoss::ChangeAnimation(AnimState newAnimState, bool loop)
{
    // 遠距離攻撃など、同じ状態でも再度再生したいケースがある場合は調整
    if (m_currentAnimState == newAnimState && newAnimState != AnimState::Attack && newAnimState != AnimState::LongRangeAttack)
    {
        return;
    }

    const char* animName = nullptr;
    switch (newAnimState)
    {
    case AnimState::Idle:
        animName = kIdleAnimName;
        break;
    case AnimState::Walk:
        animName = kWalkAnimName;
        break;
    case AnimState::Attack:
        animName = kCloseAttackAnimName;
        break;
    case AnimState::LongRangeAttack:
        animName = kLongRangeAttackAnimName;
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

void EnemyBoss::Update(std::vector<Bullet>& bullets, const Player::TackleInfo& tackleInfo, const Player& player, const std::vector<EnemyBase*>& enemyList, const std::vector<Stage::StageCollisionData>& collisionData, Effect* pEffect)
{
    UpdateStageCollision(collisionData);

    if (m_hp <= 0.0f) 
    {
        if (!m_isDeadAnimPlaying) 
        {
            ChangeAnimation(AnimState::Dead, false);
            m_isDeadAnimPlaying = true;
            m_animTime = 0.0f;
            m_isAlive = true; 
        }
        
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
            if (m_onDeathCallback) 
            {
                m_onDeathCallback(m_pos);
                m_onDeathCallback = nullptr;
            }
            m_isAlive = false; 
            SetActive(false);
        } 
        return;
    }

    MV1SetPosition(m_modelHandle, m_pos);

    // プレイヤーの位置・コライダー
    VECTOR playerPos = player.GetPos();
    std::shared_ptr<CapsuleCollider> playerBodyCollider = player.GetBodyCollider();

    // 攻撃範囲コライダー更新
    VECTOR attackRangeCenter = m_pos;
    attackRangeCenter.y += (kBodyColliderHeight * 0.5f);
    m_pAttackRangeCollider->SetCenter(attackRangeCenter);

    // 状態遷移ロジック
    if (m_currentAnimState == AnimState::Attack)
    {
        // ... (Existing Close Range Attack Logic) ...
        // 攻撃中
        float currentAnimTotalTime = m_animationManager.GetAnimationTotalTime(m_modelHandle, kCloseAttackAnimName);
        
        // 攻撃アニメーション終了判定
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
                m_isAttackHit = false;
                // 攻撃終了後、範囲内にいれば再度攻撃、いなければ移動へ
                if (CanAttackPlayer(player))
                {
                    ChangeAnimation(AnimState::Attack, false);
                }
                else
                {
                    ChangeAnimation(AnimState::Walk, true);
                }
            }
        }
    }
    else if (m_currentAnimState == AnimState::LongRangeAttack)
    {
        float totalTime = m_animationManager.GetAnimationTotalTime(m_modelHandle, kLongRangeAttackAnimName);
        
        // 弾生成タイミング (例えばアニメーションの30%時点)
        if (!m_hasShotLongRange && m_animTime > totalTime * 0.3f)
        {
             // 弾生成
             int headIndex = MV1SearchFrame(m_modelHandle, "mixamorig:HeadTop_End");
             VECTOR spawnPos = m_pos;
             if (headIndex != -1)
             {
                 spawnPos = MV1GetFramePosition(m_modelHandle, headIndex);
             }
             else
             {
                 // 見つからない場合は頭付近オフセット
                 spawnPos = VAdd(m_pos, VGet(0, kBodyColliderHeight, 0));
             }

             HomingBullet bullet;
             bullet.pos = spawnPos;
             bullet.active = true;
             bullet.damage = kHomingBulletDamage; // ダメージ20
             bullet.distTraveled = 0.0f;
             // プレイヤー方向へ発射
             bullet.dir = VNorm(VSub(playerPos, spawnPos));
             bullet.speed = kHomingBulletSpeed;
             
             // エフェクトがあればここで再生しハンドル保持
             if (pEffect)
             {
                 // bullet.effectHandle = pEffect->PlaySomething(...) 
             }

             m_homingBullets.push_back(bullet);
             m_hasShotLongRange = true;
        }

        if (m_animTime >= totalTime)
        {
             // アニメーション終了
             if (m_attackEndDelayTimer <= 0) m_attackEndDelayTimer = 30; // 硬直
        }

        if (m_attackEndDelayTimer > 0)
        {
            m_attackEndDelayTimer--;
            if (m_attackEndDelayTimer == 0)
            {
                 ChangeAnimation(AnimState::Walk, true);
                 m_longRangeAttackCooldown = kLongRangeAttackCooldownMax; 
            }
        }
    }
    else // Idle or Walk
    {
        // 移動処理 (Walk)
        if (m_currentAnimState == AnimState::Walk)
        {
             // クールダウン減少
             if (m_longRangeAttackCooldown > 0) m_longRangeAttackCooldown--;

             VECTOR toPlayer = VSub(playerPos, m_pos);
             toPlayer.y = 0.0f;
             float disToPlayer = VSize(toPlayer);

             // 向き変更
             if (disToPlayer > 0.1f)
             {
                 float yaw = atan2f(toPlayer.x, toPlayer.z);
                 yaw += DX_PI_F; // モデルの向き補正が必要なら調整
                 MV1SetRotationXYZ(m_modelHandle, VGet(0.0f, yaw, 0.0f));
             }

             // 攻撃判定
             if (CanAttackPlayer(player))
             {
                 m_isAttackHit = false;
                 ChangeAnimation(AnimState::Attack, false);
             }
             else if (disToPlayer > kLongRangeAttackMinDist && m_longRangeAttackCooldown <= 0)
             {
                 // 遠距離攻撃へ遷移
                 m_hasShotLongRange = false;
                 ChangeAnimation(AnimState::LongRangeAttack, false);
             }
             else
             {
                 // 範囲外なら近づく
                 VECTOR dir = VNorm(toPlayer);
                 m_pos = VAdd(m_pos, VScale(dir, m_chaseSpeed));
             }
        }
    }
    
    // ホーミング弾の更新
    for (auto& bullet : m_homingBullets)
    {
        if (!bullet.active) continue;

        // プレイヤーへの方向
        VECTOR toPlayer = VSub(player.GetPos(), bullet.pos);
        float distToPlayer = VSize(toPlayer);
        VECTOR targetDir = VNorm(toPlayer);

        // ホーミング処理 (現在の向きからターゲット向きへ徐々に補間)
        // 戻ってくる動きを応用 -> プレイヤーが動いても追従
        // シンプルにターンレートで補間
        bullet.dir = VAdd(bullet.dir, VScale(targetDir, kHomingTurnRate));
        bullet.dir = VNorm(bullet.dir);

        // 移動
        VECTOR moveVec = VScale(bullet.dir, bullet.speed);
        bullet.pos = VAdd(bullet.pos, moveVec);
        bullet.distTraveled += bullet.speed;

        // エフェクト更新(あれば)
        //if (pEffect)
        //{
        //     pEffect->PlayEnemyMuzzleEffect(bullet.pos.x, bullet.pos.y, bullet.pos.z);
        //}

        // 当たり判定 (擬似球)
        std::shared_ptr<CapsuleCollider> pCol = player.GetBodyCollider();
        SphereCollider bulletCol(bullet.pos, kHomingBulletRadius);
        if (bulletCol.IsIntersects(pCol.get()))
        {
            const_cast<Player&>(player).TakeDamage(bullet.damage, m_pos);
            bullet.active = false; // ヒットしたら消滅
        }

        // 最大飛距離チェック
        if (bullet.distTraveled > kHomingBulletMaxDist)
        {
            bullet.active = false;
        }

        // 地面接触で消滅
        if (bullet.pos.y < 0) bullet.active = false;
    }
    
    // 不要な弾を削除
    m_homingBullets.erase(std::remove_if(m_homingBullets.begin(), m_homingBullets.end(), [](const HomingBullet& b){ return !b.active; }), m_homingBullets.end());

    // アニメーション更新
    if (m_animationManager.GetCurrentAttachedAnimHandle(m_modelHandle) != -1) 
    {
        m_animTime += 1.0f;
        const char* animName = nullptr;
        if (m_currentAnimState == AnimState::Walk) animName = kWalkAnimName;
        else if (m_currentAnimState == AnimState::Attack) animName = kCloseAttackAnimName;
        else if (m_currentAnimState == AnimState::Dead) animName = kDeadAnimName;
        else animName = kIdleAnimName;

        if (animName)
        {
            float totalTime = m_animationManager.GetAnimationTotalTime(m_modelHandle, animName);
            // ループ処理
            if (m_currentAnimState == AnimState::Walk || m_currentAnimState == AnimState::Idle)
            {
                m_animTime = fmodf(m_animTime, totalTime);
            }
            m_animationManager.UpdateAnimationTime(m_modelHandle, m_animTime);
        }
    }

    // コライダー更新(Body)
    VECTOR bodyCapA = VAdd(m_pos, VGet(0, kBodyColliderRadius, 0));
    VECTOR bodyCapB = VAdd(m_pos, VGet(0, kBodyColliderHeight - kBodyColliderRadius, 0));
    m_pBodyCollider->SetSegment(bodyCapA, bodyCapB);
    m_pBodyCollider->SetRadius(kBodyColliderRadius);

    // コライダー更新(Head)
    int headIndex = MV1SearchFrame(m_modelHandle, "mixamorig:HeadTop_End");
    if (headIndex != -1)
    {
        VECTOR headPos = MV1GetFramePosition(m_modelHandle, headIndex);
        m_pHeadCollider->SetCenter(headPos);
        m_pHeadCollider->SetRadius(kHeadRadius);
    }
    else
    {
        // 見つからない場合は"Head"で再検索
        headIndex = MV1SearchFrame(m_modelHandle, "Head");
        if (headIndex != -1)
        {
             VECTOR headPos = MV1GetFramePosition(m_modelHandle, headIndex);
             m_pHeadCollider->SetCenter(headPos);
             m_pHeadCollider->SetRadius(kHeadRadius);
        }
        else
        {
             // 頭が見つからない場合のフォールバック（体の上の方）
             m_pHeadCollider->SetCenter(VAdd(m_pos, VGet(0, kBodyColliderHeight, 0)));
             m_pHeadCollider->SetRadius(kHeadRadius);
        }
    }


    // プレイヤーとの押し出しなど
    if (m_pBodyCollider->IsIntersects(playerBodyCollider.get()))
    {
         // 簡易押し出し
         VECTOR diff = VSub(m_pos, player.GetPos());
         diff.y = 0.0f;
         if (VSize(diff) > 0.001f)
         {
             VECTOR pushDir = VNorm(diff);
             float pushDist = 2.0f; // 適当な押し出し係数
             m_pos = VAdd(m_pos, VScale(pushDir, pushDist));
         }
    }


    // 攻撃判定発生
    if (m_currentAnimState == AnimState::Attack && !m_isAttackHit)
    {
        // アニメーションの特定タイミングでのみヒット判定を出す
        float totalTime = m_animationManager.GetAnimationTotalTime(m_modelHandle, kCloseAttackAnimName);
        // 判定期間を少し広げる or 調整
        // 攻撃の出始めから終わり際まで判定があっても良いならそうするが、
        // 以前のロジック(20%~70%)を維持する
        if (m_animTime > totalTime * 0.4f && m_animTime < totalTime * 0.6f)
        {
            // ユーザー要望：デバッグで表示している範囲内（m_pAttackRangeCollider）はすべてダメージが入るように
            // 範囲攻撃なので、攻撃範囲コライダーとプレイヤーが接触していればヒットとする
            if (m_pAttackRangeCollider->IsIntersects(playerBodyCollider.get()))
            {
                 // ダメージを与える (CSVから読み込んだ攻撃力を使用)
                 const_cast<Player&>(player).TakeDamage(static_cast<float>(m_attackPower), m_pos); 
                 m_isAttackHit = true;
                 
                 // デバッグ用ヒット表示も更新しておく（青色表示用）
                 // カプセルとして設定（球として扱う）
                 m_pAttackHitCollider->SetSegment(m_pAttackRangeCollider->GetCenter(), m_pAttackRangeCollider->GetCenter());
                 m_pAttackHitCollider->SetRadius(m_pAttackRangeCollider->GetRadius());
            }
        }
    }

    CheckHitAndDamage(bullets, pEffect);
    
    // タックル判定
    if (tackleInfo.isTackling && m_hp > 0.0f)
    {
        if (!m_isTackleHit) // まだヒットしてない場合のみ
        {
             CapsuleCollider tackleCol(tackleInfo.capA, tackleInfo.capB, tackleInfo.radius);
             if (m_pBodyCollider->IsIntersects(&tackleCol))
             {
                 TakeTackleDamage(tackleInfo.damage);
                 // m_isTackleHit = true; // TakeTackleDamage内でフラグ管理されるかもだが念のため
                 // EnemyNormalでは m_lastTackleId で管理していたのでそちらに合わせるのもありだが、
                 // Baseクラスの仕様に従う
             }
        }
    }
}

void EnemyBoss::Draw()
{
    if (!m_isAlive && m_animationManager.GetCurrentAttachedAnimHandle(m_modelHandle) == -1) return;

    MV1DrawModel(m_modelHandle);

#ifdef _DEBUG
    DrawCollisionDebug();
#endif
}

void EnemyBoss::TakeDamage(float damage, AttackType type)
{
    EnemyBase::TakeDamage(damage, type);
    
    // UIへのスコア加算などの演出はEnemyNormal同様にあれば追加
    if (m_hp <= 0.0f)
    {
        bool isHeadShot = (m_lastHitPart == HitPart::Head);
        int addScore = ScoreManager::Instance().AddScore(isHeadShot) * 10; // ボスなのでスコア高め
        if (SceneMain::Instance()) 
        {
            SceneMain::Instance()->AddScorePopup(addScore, isHeadShot, ScoreManager::Instance().GetCombo());
        }
    }
}

void EnemyBoss::TakeTackleDamage(float damage)
{
    EnemyBase::TakeTackleDamage(damage);
}

std::shared_ptr<CapsuleCollider> EnemyBoss::GetBodyCollider() const
{
    return m_pBodyCollider;
}

float EnemyBoss::CalcDamage(float bulletDamage, HitPart part) const
{
    // ボスは硬い、あるいは弱点だけ効くなどの調整が可能
    if (part == HitPart::Head)
    {
        return bulletDamage * 1.5f; 
    }
    return bulletDamage * 0.8f; // ボディは少し硬い
}

void EnemyBoss::DrawCollisionDebug() const
{
    // 体
    if (m_pBodyCollider)
        DebugUtil::DrawCapsule(m_pBodyCollider->GetSegmentA(), m_pBodyCollider->GetSegmentB(), m_pBodyCollider->GetRadius(), 16, 0xff0000); // 赤
    
    // 頭
    if (m_pHeadCollider)
        DebugUtil::DrawSphere(m_pHeadCollider->GetCenter(), m_pHeadCollider->GetRadius(), 16, 0x00ff00); // 緑

    // 攻撃範囲（指定されたデバッグ表示）
    // 黄色やオレンジでわかりやすく
    if (m_pAttackRangeCollider)
        DebugUtil::DrawSphere(m_pAttackRangeCollider->GetCenter(), m_pAttackRangeCollider->GetRadius(), 32, 0xffaa00); 

    // 攻撃判定（攻撃中のみ青などで）
    if (m_currentAnimState == AnimState::Attack && m_pAttackHitCollider)
    {
         DebugUtil::DrawCapsule(m_pAttackHitCollider->GetSegmentA(), m_pAttackHitCollider->GetSegmentB(), m_pAttackHitCollider->GetRadius(), 16, 0x0000ff);
    }

    // 遠距離攻撃有効範囲（紫色）
    // プレイヤーとの距離がこの範囲外なら遠距離攻撃を行う
    DebugUtil::DrawSphere(m_pos, kLongRangeAttackMinDist, 32, 0xff00ff);

    // ホーミング弾のデバッグ（黄色）
    for (const auto& bullet : m_homingBullets)
    {
        if (bullet.active)
        {
             DebugUtil::DrawSphere(bullet.pos, kHomingBulletRadius, 8, 0xffff00);
        }
    }
}

bool EnemyBoss::CanAttackPlayer(const Player& player)
{
    // 攻撃範囲コライダー内にプレイヤーがいるか
    auto playerCol = player.GetBodyCollider();
    return m_pAttackRangeCollider->IsIntersects(playerCol.get());
}

EnemyBase::HitPart EnemyBoss::CheckHitPart(const VECTOR& rayStart, const VECTOR& rayEnd, VECTOR& outHtPos, float& outHtDistSq) const
{
    HitPart part = HitPart::None;
    float minDistSq = FLT_MAX;
    VECTOR hitPos = VGet(0, 0, 0);

    // 頭との判定
    if (m_pHeadCollider)
    {
        VECTOR tmpHitPos;
        float tmpDistSq;
        if (m_pHeadCollider->IsIsIntersectsRay(rayStart, rayEnd, tmpHitPos, tmpDistSq))
        {
            if (tmpDistSq < minDistSq)
            {
                minDistSq = tmpDistSq;
                hitPos = tmpHitPos;
                part = HitPart::Head;
            }
        }
    }

    // 体との判定
    if (m_pBodyCollider)
    {
        VECTOR tmpHitPos;
        float tmpDistSq;
        if (m_pBodyCollider->IsIsIntersectsRay(rayStart, rayEnd, tmpHitPos, tmpDistSq))
        {
            if (tmpDistSq < minDistSq)
            {
                minDistSq = tmpDistSq;
                hitPos = tmpHitPos;
                part = HitPart::Body;
            }
        }
    }

    if (part != HitPart::None)
    {
        outHtPos = hitPos;
        outHtDistSq = minDistSq;
    }

    return part;
}
