#include "Player.h"
#include "EnemyNormal.h"
#include "Bullet.h"
#include "DxLib.h"
#include <cassert>
#include <algorithm>
#include <cmath>

namespace
{
    // 攻撃アニメーション
	constexpr char kAttackAnimName[] = "ATK";

    // 位置
    constexpr VECTOR kInitialPosition = { 0.0f, -30.0f, 0.0f };

	// AABBの最小座標と最大座標
	constexpr VECTOR kAABBMin = { -20.0f, 0.0f, -15.0f };
	constexpr VECTOR kAABBMax = { 20.0f, 128.0f, 15.0f };

	// ヘッドショット判定用中心座標
	constexpr VECTOR kHeadShotPosition = { 0.0f, 160.0f, -6.0f };

    // ヘッドショットの判定半径
    constexpr float kHeadRadius = 13.5f;

	// 初期体力
	constexpr float kInitialHP = 200.0f;

    // VECTORの長さの二乗を計算する関数
    float VLenSq(const VECTOR& vec)
    {
        return vec.x * vec.x + vec.y * vec.y + vec.z * vec.z;
    }

    // AABBと球の当たり判定(ヘルパー関数としてnamespace内に残す)
    static bool CheckCapsuleSphereHit(
        const VECTOR& capA, const VECTOR& capB, float capRadius,
        const VECTOR& sphereCenter, float sphereRadius)
    {
        // 線分capA-capB上の最近点を求める
        VECTOR ab = VSub(capB, capA);
        VECTOR ac = VSub(sphereCenter, capA);

        float abLenSq = VDot(ab, ab); // 線分の長さの二乗
        float t       = 0.0f; // 最近点の線分上の位置

        // 線分の長さが0でない場合
        if (abLenSq > 0.0f)
        {
            t = VDot(ac, ab) / abLenSq; // 線分上の位置を計算
            t = (std::max)(0.0f, (std::min)(1.0f, t)); // tを0から1の範囲に制限
        }

        // 最近点の座標を計算
        VECTOR closest = VAdd(capA, VScale(ab, t));

        // 最近点と球の中心の距離を計算
        float distSq = VLenSq(VSub(sphereCenter, closest));
        float radiusSum = capRadius + sphereRadius;

        // 当たっているかどうかを返す
        return distSq <= radiusSum * radiusSum;
    }

    // 2つのカプセルの当たり判定
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
	m_animTime(0.0f)
{
    // モデルの読み込み
    m_modelHandle = MV1LoadModel("data/model/NormalZombie.mv1");
    assert(m_modelHandle != -1);
}

EnemyNormal::~EnemyNormal()
{
    // モデルの解放
	DeleteGraph(m_modelHandle); 
}


void EnemyNormal::Init()
{
    // 初期化
    m_hp                = kInitialHP;
    m_pos               = kInitialPosition;
	m_attackRange       = 150.0f; // 攻撃範囲
	m_attackPower       = 20.0f;  // 攻撃力
	m_attackCooldownMax = 45;     // 攻撃クールダウンの最大値
}

void EnemyNormal::Update(std::vector<Bullet>& bullets, const Player::TackleInfo& tackleInfo)
{
    // モデルの位置を更新
    MV1SetPosition(m_modelHandle, m_pos);

    // 攻撃アニメーションの再生
    if (m_currentAnimIndex == -1)
    {
        m_currentAnimIndex = MV1GetAnimIndex(m_modelHandle, kAttackAnimName);
        if (m_currentAnimIndex != -1) 
        {
            MV1DetachAnim(m_modelHandle, 0);
            MV1AttachAnim(m_modelHandle, m_currentAnimIndex, -1, m_currentAnimLoop);
            m_currentAnimLoop = true;
            m_animTime = 0.0f;
        }
    }

    // アニメーションの進行
    if (m_currentAnimIndex != -1) 
    {
        // アニメーションの長さを取得
        float animTotalTime = MV1GetAnimTotalTime(m_modelHandle, m_currentAnimIndex);
        m_animTime += 1.0f; // 1フレーム進める（必要に応じてフレームレートで調整）
        if (m_currentAnimLoop && m_animTime > animTotalTime) 
        {
            m_animTime = 0.0f; // ループ
        }
        MV1SetAttachAnimTime(m_modelHandle, 0, m_animTime);
    }

	// 弾の当たり判定をチェック
	CheckHitAndDamage(const_cast<std::vector<Bullet>&>(bullets));

    // タックル当たり判定の追加
    if (tackleInfo.isTackling && m_hp > 0.0f && tackleInfo.tackleId != m_lastTackleId)
    {
        // 敵のカプセル情報
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

        // カプセル同士の当たり判定
        if (CheckCapsuleCapsuleHit(
            tackleInfo.capA, tackleInfo.capB, tackleInfo.radius,
            capA, capB, capRadius))
        {
            TakeTackleDamage(tackleInfo.damage);
            m_lastTackleId = tackleInfo.tackleId; // このタックルIDでダメージを受けたことを記録
        }
    }

    // デバッグ表示タイマー減少
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
    // HPが0より大きい場合のみ描画
    if (m_hp > 0.0f)
    {
        // モデルの描画
        MV1DrawModel(m_modelHandle);

#ifdef _DEBUG
        // デバッグ用の当たり判定描画
        DrawCollisionDebug();

        // デバッグ表示
        const char* hitMsg = "";

		// ヒット部位に応じてメッセージを設定
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

        // 体力のデバッグ表示
        DrawFormatString(20, 80, 0x000000, "Enemy HP: %.1f", m_hp);
#endif
    }
}

// 敵の当たり判定を行う関数
bool EnemyNormal::IsHit(const Bullet& bullet) const
{
    // ボーンのワールド座標を取得
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

    // カプセルの上下中心を計算
    VECTOR capA = { (boxMin.x + boxMax.x) * 0.5f, boxMin.y, (boxMin.z + boxMax.z) * 0.5f };
    VECTOR capB = { (boxMin.x + boxMax.x) * 0.5f, boxMax.y, (boxMin.z + boxMax.z) * 0.5f };

    float capRadius = (std::max)(
        std::abs(boxMax.x - boxMin.x),
        std::abs(boxMax.z - boxMin.z)
        ) * 0.5f;

    // カプセルと球の当たり判定
    return CheckCapsuleSphereHit(capA, capB, capRadius, bullet.GetPos(), bullet.GetRadius());
}

void EnemyNormal::DrawCollisionDebug() const
{
    // ボーンのワールド座標を取得
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

    DrawCapsule3D(centerMin, centerMax, radius, 16, 0xff0000, 0xff0000, false);

    // Headボーンのワールド座標を取得
    int headIndex = MV1SearchFrame(m_modelHandle, "Head");
    VECTOR headPos = MV1GetFramePosition(m_modelHandle, headIndex);

    DrawSphere3D(headPos, m_headRadius, 16, 0x00ff00, 0x00ff00, false);

    // 攻撃範囲のデバッグ表示
    float centerY = (boxMin.y + boxMax.y) * 0.5f;
    VECTOR attackCenter = { spinePos.x, centerY, spinePos.z };
    DrawSphere3D(attackCenter, m_attackRange, 16, 0xff8000, 0xff8000, false);

    // Hand_RからHand_Lまでの攻撃カプセル描画
    int handRIndex = MV1SearchFrame(m_modelHandle, "Hand_R");
    int handLIndex = MV1SearchFrame(m_modelHandle, "Hand_L");

    if (handRIndex != -1 && handLIndex != -1)
    {
        VECTOR handRPos = MV1GetFramePosition(m_modelHandle, handRIndex);
        VECTOR handLPos = MV1GetFramePosition(m_modelHandle, handLIndex);
        float attackCapsuleRadius = 20.0f;
        DrawCapsule3D(handRPos, handLPos, attackCapsuleRadius, 16, 0x0000ff, 0x0000ff, false);
    }

#ifdef _DEBUG
    // プレイヤー座標を取得
    extern Player* g_pPlayer; // グローバルでプレイヤーへのポインタがある場合
    if (g_pPlayer)
    {
        VECTOR playerPos = g_pPlayer->GetPos();
        float dx = playerPos.x - attackCenter.x;
        float dy = playerPos.y - attackCenter.y;
        float dz = playerPos.z - attackCenter.z;
        float distSq = dx * dx + dy * dy + dz * dz;

        if (distSq <= m_attackRange * m_attackRange)
        {
            DrawFormatString(20, 100, 0xff0000, "攻撃範囲内に入った");
        }
        else
        {
            DrawFormatString(20, 100, 0x00ff00, "攻撃範囲外");
        }
    }
#endif
}

// どこに当たったか判定する関数
EnemyBase::HitPart EnemyNormal::CheckHitPart(const Bullet& bullet) const 
{
    // Headボーンのワールド座標を取得
    int headIndex  = MV1SearchFrame(m_modelHandle, "Head"); 
    VECTOR headPos = MV1GetFramePosition(m_modelHandle, headIndex);

    VECTOR headCenter = {
        headPos.x,
        headPos.y,
        headPos.z
    };

    VECTOR bulletPos = bullet.GetPos();

	// ヘッドショット判定のための距離計算
	float dx        = bulletPos.x - headCenter.x;        // X座標の差
	float dy        = bulletPos.y - headCenter.y;        // Y座標の差
	float dz        = bulletPos.z - headCenter.z;        // Z座標の差
	float distSq    = dx * dx + dy * dy + dz * dz;       // 距離の二乗を計算
	float radiusSum = m_headRadius + bullet.GetRadius(); // ヘッドショット半径と弾の半径の和を計算

	// ヘッドショット判定
    if (distSq <= radiusSum * radiusSum)
    {
        return HitPart::Head;
    }

    // ボディヒット判定
    if (IsHit(bullet)) 
    {
        return HitPart::Body;
    }
    return HitPart::None;
}

// ダメージ計算
float EnemyNormal::CalcDamage(const Bullet& bullet, HitPart part) const
{
    if (part == HitPart::Head)
    {
        return bullet.GetDamage() * 2.0f; // ヘッドショットはダメージ2倍
    }
    else if (part == HitPart::Body)
    {
        return bullet.GetDamage();
    }
    return 0.0f;
}

// 攻撃用当たり判定
// TODO:まだ使えていない
bool EnemyNormal::IsAttackHit(const VECTOR& targetPos, float targetRadius) const
{
    int handRIndex = MV1SearchFrame(m_modelHandle, "Hand_R");
    int handLIndex = MV1SearchFrame(m_modelHandle, "Hand_L");
    if (handRIndex == -1 || handLIndex == -1) return false;

    VECTOR handRPos = MV1GetFramePosition(m_modelHandle, handRIndex);
    VECTOR handLPos = MV1GetFramePosition(m_modelHandle, handLIndex);

    float attackCapsuleRadius = 20.0f; 

    // カプセルと球の当たり判定
    return CheckCapsuleSphereHit(handRPos, handLPos, attackCapsuleRadius, targetPos, targetRadius);
}

// TODO:攻撃範囲の中心取得修正
VECTOR EnemyNormal::GetAttackRangeCenter() const
{
    // ボーンのワールド座標を取得
    int spineIndex = MV1SearchFrame(m_modelHandle, "Root");
    VECTOR spinePos = MV1GetFramePosition(m_modelHandle, spineIndex);

    VECTOR boxMin = {
        spinePos.x + m_aabbMin.x,
        spinePos.y + m_aabbMin.y,
        spinePos.z + m_aabbMin.z
    };
    VECTOR boxMax = {
        spinePos.x + m_aabbMax.x,
        spinePos.y + m_aabbMax.y,
        spinePos.z + m_aabbMax.z
    };

    float centerY = (boxMin.y + boxMax.y) * 0.5f;
    return { spinePos.x, centerY, spinePos.z };
}

float EnemyNormal::GetAttackRange() const
{
	// 攻撃範囲の半径を返す
	return m_attackRange;
}


