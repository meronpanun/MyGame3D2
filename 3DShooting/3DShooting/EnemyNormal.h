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
	VECTOR m_pos; // �G�̈ʒu
    VECTOR m_aabbMin; // AABB�ŏ����W
    VECTOR m_aabbMax; // AABB�ő���W
    VECTOR m_headPos; // �w�b�h�V���b�g����p���S���W

    float m_colRadius;
    float m_headRadius; // �w�b�h�V���b�g����p���a

};

