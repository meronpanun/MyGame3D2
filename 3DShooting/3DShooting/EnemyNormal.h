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
	VECTOR m_pos; // ìGÇÃà íu

    float m_colRadius = 1.0f;

    VECTOR m_aabbMin; // AABBç≈è¨ç¿ïW
    VECTOR m_aabbMax; // AABBç≈ëÂç¿ïW
};

