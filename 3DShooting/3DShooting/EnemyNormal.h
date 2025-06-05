#pragma once
#include "EnemyBase.h"

class EnemyNormal : public EnemyBase
{
public:
    EnemyNormal(); 
    virtual ~EnemyNormal() = default;

    void Init() override;
    void Update() override;
    void Draw() override;

    virtual bool IsHit(const Bullet& bullet) const override;
    virtual void DrawCollisionDebug() const override;

protected:

private:
	VECTOR m_pos; // 敵の位置
    VECTOR m_aabbMin; // AABB最小座標
    VECTOR m_aabbMax; // AABB最大座標
    VECTOR m_headPos; // ヘッドショット判定用中心座標

    float m_colRadius;
    float m_headRadius; // ヘッドショット判定用半径

};

