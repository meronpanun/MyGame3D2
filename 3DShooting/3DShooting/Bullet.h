#pragma once
#include "DxLib.h"
#include <vector>

/// <summary>
/// ’eƒNƒ‰ƒX
/// </summary>
class Bullet
{
public:
	Bullet(VECTOR position, VECTOR direction, float damage = 10.0f);
	virtual ~Bullet();

	void Init();
	void Update();
	void Draw() const;

	// ’e‚ÌˆÊ’u‚ğæ“¾
	VECTOR GetPos() const { return m_pos; }

	/// <summary>
	/// ’e‚Ì”¼Œa‚ğæ“¾
	/// </summary>
	/// <returns>’e‚Ì”¼Œa</returns>
	float GetRadius() const { return m_radius; }

	/// <summary>
	/// ’e‚ª—LŒø‚©‚Ç‚¤‚©
	/// </summary>
	/// <returns>—LŒø‚È‚çtrue</returns>
	bool IsActive() const { return m_isActive; }

	/// <summary>
	/// ’e‚ÌXV
	/// </summary>
	/// <param name="bullets">’e‚Ì”z—ñ</param>
	static void UpdateBullets(std::vector<Bullet>& bullets);

	/// <summary>
	/// ’e‚Ì•`‰æ
	/// </summary>
	/// <param name="bullets">’e‚Ì”z—ñ</param>
	static void DrawBullets(const std::vector<Bullet>& bullets);

	/// <summary>
	/// ’e‚ğ”ñƒAƒNƒeƒBƒu‰»
	/// </summary>
	void Deactivate();

	float GetDamage() const { return m_damage; }

private:
	VECTOR m_pos; // ’e‚ÌˆÊ’u
	VECTOR m_dir; // ’e‚Ì•ûŒü

	float m_speed;    // ’e‚Ì‘¬“x
	float m_radius;   // ’e‚Ì”¼Œa
	float m_damage;   // ’e‚Ìƒ_ƒ[ƒW
	bool  m_isActive; // ’e‚Ì—LŒø«
};

