#pragma once
#include "Collider.h"

/// <summary>
/// 球体形状のコライダークラス。
/// 球同士・球とカプセルの交差判定およびレイとの交差判定を提供する。
/// </summary>
class SphereCollider : public Collider
{
public:
    /// <summary>
    /// 中心座標と半径を指定して構築するコンストラクタ
    /// </summary>
    /// <param name="center">球の中心座標（デフォルト: 原点）</param>
    /// <param name="radius">球の半径（デフォルト: 1.0f）</param>
    SphereCollider(const VECTOR& center = VGet(0, 0, 0), float radius = 1.0f);
    virtual ~SphereCollider() = default;

    /// <summary>
    /// 他のコライダーとの交差判定を行う（球同士・球とカプセル対応）
    /// </summary>
    /// <param name="other">判定対象のコライダー</param>
    /// <returns>交差していれば true</returns>
    bool IsIntersects(const Collider* other) const override;

    /// <summary>
    /// レイとの交差判定を行う
    /// </summary>
    /// <param name="rayStart">レイの始点</param>
    /// <param name="rayEnd">レイの終点</param>
    /// <param name="outHitPos">交差位置（出力）</param>
    /// <param name="outHitDistSq">交差位置までの距離の二乗（出力）</param>
    /// <returns>交差していれば true</returns>
    bool IsIsIntersectsRay(const VECTOR& rayStart, const VECTOR& rayEnd, VECTOR& outHitPos, float& outHitDistSq) const override;

    /// <summary>
    /// 球の中心座標を返す
    /// </summary>
    /// <returns>球の中心座標</returns>
    VECTOR GetCenter() const { return m_center; }

    /// <summary>
    /// 球の半径を返す
    /// </summary>
    /// <returns>球の半径</returns>
    float GetRadius() const { return m_radius; }

    /// <summary>
    /// 球の中心座標を設定する
    /// </summary>
    /// <param name="center">新しい中心座標</param>
    void SetCenter(const VECTOR& center) { m_center = center; }

    /// <summary>
    /// 球の半径を設定する
    /// </summary>
    /// <param name="radius">新しい半径</param>
    void SetRadius(float radius) { m_radius = radius; }

private:
    VECTOR m_center; // 球の中心座標
    float  m_radius; // 球の半径
};
