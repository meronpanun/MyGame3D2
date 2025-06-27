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

    // 弾の現在の位置を取得
    VECTOR GetPos() const { return m_pos; }
    // 弾の前フレームの位置を取得 (Rayの始点として使用)
    VECTOR GetPrevPos() const { return m_prevPos; }

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
    VECTOR m_pos;     // 現在の位置
    VECTOR m_prevPos; // 前フレームの位置 (Rayの始点)
    VECTOR m_dir;     // 進行方向
    float  m_speed;   // 速度
    bool   m_isActive;
    float  m_damage;
};