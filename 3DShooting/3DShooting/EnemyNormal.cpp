#include "EnemyNormal.h"
#include "Player.h"
#include "Bullet.h"
#include "DxLib.h"
#include <cassert>

EnemyNormal::EnemyNormal():
    m_colRadius(1.0f),
	m_pos{ 0.0f, 0.0f, 0.0f }
{
}


void EnemyNormal::Init()
{
    // ���f���̓ǂݍ���
    m_modelHandle = MV1LoadModel("data/image/NormalZombie.mv1");
    assert(m_modelHandle != -1);

    // �C�ӂ̈ʒu�ɃX�|�[��
    VECTOR spawnPos = { 10.0f, 0.0f, 5.0f };
}

void EnemyNormal::Update()
{
    // ���f���̈ʒu���X�V
    MV1SetPosition(m_modelHandle, m_pos);
}

void EnemyNormal::Draw()
{
	// ���f���̕`��
    MV1DrawModel(m_modelHandle);
}


bool EnemyNormal::IsHit(const Bullet& bullet) const
{
    return false;
}

void EnemyNormal::DrawCollisionDebug() const
{
    VECTOR screenPos = ConvWorldPosToScreenPos(m_pos);
    if (screenPos.x >= 0 && screenPos.y >= 0)
    {
        int radius = static_cast<int>(m_colRadius * 50.0f);
        DrawCircle(static_cast<int>(screenPos.x), static_cast<int>(screenPos.y), radius, GetColor(255, 0, 0), FALSE);
    }
}