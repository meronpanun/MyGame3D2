#pragma once
#include "DxLib.h"
#include <vector>

/// <summary>
/// 弾クラス
/// </summary>
class Bullet
{
public:
	Bullet(VECTOR position, VECTOR direction, float damage = 10.0f);
	virtual ~Bullet();

	void Init();
	void Update();
	void Draw() const;

	// 弾の位置を取得
	VECTOR GetPos() const { return m_pos; }

	/// <summary>
	/// 弾の半径を取得
	/// </summary>
	/// <returns>弾の半径</returns>
	float GetRadius() const { return m_radius; }

	/// <summary>
	/// 弾が有効かどうか
	/// </summary>
	/// <returns>有効ならtrue</returns>
	bool IsActive() const { return m_isActive; }

	/// <summary>
	/// 弾の更新
	/// </summary>
	/// <param name="bullets">弾の配列</param>
	static void UpdateBullets(std::vector<Bullet>& bullets);

	/// <summary>
	/// 弾の描画
	/// </summary>
	/// <param name="bullets">弾の配列</param>
	static void DrawBullets(const std::vector<Bullet>& bullets);

	/// <summary>
	/// 弾を非アクティブ化
	/// </summary>
	void Deactivate();

	float GetDamage() const { return m_damage; }

private:
	VECTOR m_pos;     // 弾の位置
	VECTOR m_dir;     // 弾の方向

	float m_speed;    // 弾の速度
	float m_radius;   // 弾の半径
	float m_damage;   // 弾のダメージ
	bool m_isActive;  // 弾の有効状態
};

