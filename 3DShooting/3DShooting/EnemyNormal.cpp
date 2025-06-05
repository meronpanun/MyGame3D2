#include "EnemyNormal.h"
#include "Player.h"
#include "Bullet.h"
#include "DxLib.h"
#include <cassert>

namespace
{
	// ヒット表示の持続時間
	constexpr int kHitDisplayDuration = 60; // 1秒間表示

    // 敵の位置
    constexpr VECTOR kInitialPosition = { 0.0f, -30.0f, 0.0f };

	// AABBの最小座標と最大座標
	constexpr VECTOR kAABBMin = { -20.0f, 0.0f, -15.0f };
	constexpr VECTOR kAABBMax = { 20.0f, 128.0f, 15.0f };

	// 敵のヘッドショット判定用中心座標
	constexpr VECTOR kHeadShotPosition = { 0.0f, 160.0f, -6.0f };

    // ヘッドショットの判定半径
    constexpr float kHeadRadius = 12.5f;

	// 敵の初期体力
	constexpr float kInitialHP = 200.0f;
}

EnemyNormal::EnemyNormal():
    m_colRadius(1.0f),
	m_pos{ kInitialPosition },
    m_aabbMin{ kAABBMin },
    m_aabbMax{ kAABBMax },
	m_headPos{ kHeadShotPosition },
	m_headRadius(kHeadRadius),
	m_hp(kInitialHP)
{
}


void EnemyNormal::Init()
{
    // モデルの読み込み
    m_modelHandle = MV1LoadModel("data/image/NormalZombie.mv1");
    assert(m_modelHandle != -1);
}

void EnemyNormal::Update(const std::vector<Bullet>& bullets)
{
    // モデルの位置を更新
    MV1SetPosition(m_modelHandle, m_pos);

    // 当たり判定
    CheckHitAndDamage(bullets);


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
    }
}

// AABBと球の当たり判定
static bool CheckCapsuleSphereHit(
    const VECTOR& capA, const VECTOR& capB, float capRadius,
    const VECTOR& sphereCenter, float sphereRadius)
{
    // 線分capA-capB上の最近点を求める
    VECTOR ab = { capB.x - capA.x, capB.y - capA.y, capB.z - capA.z };
    VECTOR ac = { sphereCenter.x - capA.x, sphereCenter.y - capA.y, sphereCenter.z - capA.z };

	float abLenSq = ab.x * ab.x + ab.y * ab.y + ab.z * ab.z; // 線分の長さの二乗
	float t = 0.0f; // 最近点の線分上の位置

    // 線分の長さが0でない場合
	if (abLenSq > 0.0f)  
    {
		t = (ac.x * ab.x + ac.y * ab.y + ac.z * ab.z) / abLenSq; // 線分上の位置を計算
		t = (std::max)(0.0f, (std::min)(1.0f, t));               // tを0から1の範囲に制限
    }

	// 最近点の座標を計算
    VECTOR closest = { 
        capA.x + ab.x * t,
        capA.y + ab.y * t,
        capA.z + ab.z * t
    };

	// 最近点と球の中心の距離を計算
	float dx = sphereCenter.x - closest.x;
	float dy = sphereCenter.y - closest.y;
	float dz = sphereCenter.z - closest.z;
    float distSq = dx * dx + dy * dy + dz * dz;
    float radiusSum = capRadius + sphereRadius;

    // 当たっているかどうかを返す
	return distSq <= radiusSum * radiusSum; 
}

// 敵の当たり判定を行う関数
bool EnemyNormal::IsHit(const Bullet& bullet) const
{
    // カプセルの中心軸
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

    float capRadius = (std::max)( 
        std::abs(boxMax.x - boxMin.x),
        std::abs(boxMax.z - boxMin.z)
        ) * 0.5f;

    // カプセルと球の当たり判定
    return CheckCapsuleSphereHit(capA, capB, capRadius, bullet.GetPos(), bullet.GetRadius());
}

// デバッグ用の当たり判定描画
void EnemyNormal::DrawCollisionDebug() const
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
    unsigned int color = GetColor(255, 0, 0);

    // カプセルの中心軸をAABBの上下中心に
    VECTOR centerMin = { (boxMin.x + boxMax.x) * 0.5f, boxMin.y, (boxMin.z + boxMax.z) * 0.5f };
    VECTOR centerMax = { (boxMin.x + boxMax.x) * 0.5f, boxMax.y, (boxMin.z + boxMax.z) * 0.5f };

    // 半径はX,Z方向のAABBサイズのうち大きい方の半分
    float radius = (std::max)(
        std::abs(boxMax.x - boxMin.x),
        std::abs(boxMax.z - boxMin.z)
        ) * 0.5f;

    DrawCapsule3D(centerMin, centerMax, radius, 16, color, color, false);

    // ヘッドショット判定デバッグ描画
    VECTOR headCenter = {
        m_pos.x + m_headPos.x,
        m_pos.y + m_headPos.y,
        m_pos.z + m_headPos.z
    };
    unsigned int headColor = GetColor(0, 255, 0); // 緑色で描画
    DrawSphere3D(headCenter, m_headRadius, 16, headColor, headColor, false);
}

// どこに当たったか判定する関数
EnemyBase::HitPart EnemyNormal::CheckHitPart(const Bullet& bullet) const 
{
    // ヘッドショット判定
    VECTOR headCenter = {
        m_pos.x + m_headPos.x,
        m_pos.y + m_headPos.y,
        m_pos.z + m_headPos.z
    };
    VECTOR bulletPos = bullet.GetPos();
    float dx = bulletPos.x - headCenter.x;
    float dy = bulletPos.y - headCenter.y;
    float dz = bulletPos.z - headCenter.z;
    float distSq = dx * dx + dy * dy + dz * dz;
    float radiusSum = m_headRadius + bullet.GetRadius();
    if (distSq <= radiusSum * radiusSum)
    {
        return HitPart::Head;
    }

    // 体判定
    if (IsHit(bullet)) 
    {
        return HitPart::Body;
    }
    return HitPart::None;
}

// 敵が弾に当たったかどうかをチェックし、ダメージを受ける処理
void EnemyNormal::CheckHitAndDamage(const std::vector<Bullet>& bullets)
{
    // ヒット情報はリセットしない
    for (auto& bullet : bullets)
    {
        if (!bullet.IsActive()) continue;
        HitPart part = CheckHitPart(bullet);
        if (part == HitPart::Head)
        {
            TakeDamage(50.0f);
            m_lastHitPart = HitPart::Head;
            m_hitDisplayTimer = kHitDisplayDuration;
            const_cast<Bullet&>(bullet).Deactivate();
            break;
        }
        else if (part == HitPart::Body)
        {
            TakeDamage(10.0f);
            m_lastHitPart = HitPart::Body;
            m_hitDisplayTimer = kHitDisplayDuration;
            const_cast<Bullet&>(bullet).Deactivate();
            break;
        }
    }
}

// 敵がダメージを受ける処理
void EnemyNormal::TakeDamage(float damage)
{
    m_hp -= damage;
    if (m_hp < 0) m_hp = 0;
}

