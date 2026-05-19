#pragma once

/// <summary>
/// プレイヤーの攻撃種別を表す列挙型
/// </summary>
enum class AttackType
{
    None,
    Shoot,       // 射撃
    Shotgun,     // ショットガン
    Tackle,      // タックル
    ShieldThrow, // 盾投げ
    Parry        // パリィ
};
