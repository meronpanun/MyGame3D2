#pragma once

// 攻撃の種類を定義するenum
enum class AttackType
{
	None,
	Shoot, // 射撃
	Tackle // タックル
};

/// <summary>
/// 武器の種類列挙型
/// </summary>
enum class WeaponType
{
	AssaultRifle, // アサルトライフル
	Shotgun       // ショットガン
};
