#include "Bullet.h"
#include "Collision.h"
#include "EffekseerWarningSuppress.h"
#include <algorithm>

namespace
{
	constexpr float kBulletSpeed            = 60.0f;    // 弾の速度
	constexpr float kPlayerBoundaryDistance = 2000.0f;  // 弾を非アクティブにするプレイヤーからの距離
	constexpr float kMinDistSqThreshold     = 0.0001f;  // 最近傍衝突点の比較に使う距離二乗の最小値

	// デバッグ描画用パラメータ
	constexpr float kDebugSphereRadius = 2.0f;     // デバッグ球の半径
	constexpr int   kDebugSphereDiv    = 16;       // 球の分割数
	constexpr int   kDebugColor        = 0xffff00; // デバッグ表示色（黄色）
}

Bullet::Bullet(VECTOR position, VECTOR direction, AttackType attackType, float damage, float attenuationStartDist, float attenuationEndDist, float minDamageRatio)
	: m_pos(position)
	, m_prevPos(position)
	, m_spawnPos(position)
	, m_dir(direction)
	, m_speed(kBulletSpeed)
	, m_isActive(true)
	, m_damage(damage)
	, m_attackType(attackType)
	, m_attenuationStartDist(attenuationStartDist)
	, m_attenuationEndDist(attenuationEndDist)
	, m_minDamageRatio(minDamageRatio)
{
}

Bullet::~Bullet()
{
}

void Bullet::Init()
{
}

void Bullet::Update(const VECTOR &playerPos, const std::vector<Stage::StageCollisionData> &collisionData)
{
	if (!m_isActive) return;

	m_prevPos = m_pos; // 現在の位置を前フレームの位置として保存
	m_pos = VAdd(m_pos, VScale(VNorm(m_dir), m_speed)); // 新しい位置を計算

	// ステージとの衝突判定
	// 前フレームの位置と現在の位置を線分として判定を行う
	float minT = 1.0f;
	bool hit = false;
	VECTOR hitPos = m_pos;

	for (const auto &col : collisionData)
	{
		HITRESULT_LINE result = HitCheck_Line_Triangle(m_prevPos, m_pos, col.v1, col.v2, col.v3);
		if (result.HitFlag)
		{
			// できるだけ前で衝突したものを使用するため、最近傍比較を行う
			float distSq = VSquareSize(VSub(result.Position, m_prevPos));
			float totalDistSq = VSquareSize(VSub(m_pos, m_prevPos));

			if (totalDistSq > kMinDistSqThreshold)
			{
				float t = distSq / totalDistSq;
				if (t >= 0.0f && t < minT)
				{
					minT = t;
					hit = true;
					hitPos = result.Position;
				}
			}
		}
	}

	if (hit)
	{
		// 衝突した場合は位置を合わせ、非アクティブ化（壁に埋まらないようにする）
		m_pos = hitPos;
		m_isActive = false;
	}

	// プレイヤーからの距離を計算
	float distanceToPlayer = VSize(VSub(m_pos, playerPos));

	// プレイヤーより遠くに離れたら非アクティブにする
	if (distanceToPlayer > kPlayerBoundaryDistance)
	{
		m_isActive = false;
	}
}

void Bullet::Draw() const
{
#ifdef _DEBUG
	if (!m_isActive) return;

	// Rayのデバッグ描画
	DrawLine3D(m_prevPos, m_pos, kDebugColor);
	DrawSphere3D(m_pos, kDebugSphereRadius, kDebugSphereDiv, kDebugColor, kDebugColor, false);
#endif
}

void Bullet::UpdateBullets(std::vector<Bullet> &bullets, const VECTOR &playerPos, const std::vector<Stage::StageCollisionData> &collisionData)
{
	for (auto &bullet : bullets)
	{
		bullet.Update(playerPos, collisionData);
	}
	// 非アクティブな弾を削除
	bullets.erase(std::remove_if(bullets.begin(), bullets.end(), [](const Bullet &b) { return !b.IsActive(); }), bullets.end());
}

void Bullet::DrawBullets(const std::vector<Bullet> &bullets)
{
	for (const auto &bullet : bullets)
	{
		bullet.Draw();
	}
}

void Bullet::Deactivate()
{
	m_isActive = false;
}

float Bullet::GetDamage() const
{
	// 減衰なしの設定ならそのまま返す
	if (m_attenuationEndDist <= 0.0f || m_attenuationStartDist >= m_attenuationEndDist) return m_damage;

	// 発射位置からの距離を計算
	float distance = VSize(VSub(m_pos, m_spawnPos));

	// 減衰開始以前ならダメージ変わらず
	if (distance <= m_attenuationStartDist) return m_damage;

	// 減衰終了以降なら最低ダメージ
	if (distance >= m_attenuationEndDist) return m_damage * m_minDamageRatio;

	// 距離に応じて線形補間
	float t = (distance - m_attenuationStartDist) / (m_attenuationEndDist - m_attenuationStartDist);
	float currentRatio = 1.0f - t * (1.0f - m_minDamageRatio);

	return m_damage * currentRatio;
}
