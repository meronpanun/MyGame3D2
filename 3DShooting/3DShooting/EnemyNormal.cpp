#include "EnemyNormal.h"
#include "Player.h"
#include "Bullet.h"
#include "DxLib.h"
#include <cassert>

EnemyNormal::EnemyNormal():
    m_colRadius(1.0f),
	m_pos{ 0.0f, -30.0f, 0.0f },
	m_aabbMin{ 25.0f, 150.0f, -18.0f },
	m_aabbMax{ -25.0f, 0.0f, 13.0f }
{
}


void EnemyNormal::Init()
{
    // ���f���̓ǂݍ���
    m_modelHandle = MV1LoadModel("data/image/NormalZombie.mv1");
    assert(m_modelHandle != -1);
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

	// �f�o�b�O�p�̓����蔻��`��
	DrawCollisionDebug();
}

// AABB�Ƌ��̓����蔻��
static bool CheckAABBSphereHit(
    const VECTOR& boxMin, const VECTOR& boxMax,
    const VECTOR& sphereCenter, float sphereRadius)
{
    float distSq = 0.0f;
    // X
    if (sphereCenter.x < boxMin.x) distSq += (boxMin.x - sphereCenter.x) * (boxMin.x - sphereCenter.x);
    else if (sphereCenter.x > boxMax.x) distSq += (sphereCenter.x - boxMax.x) * (sphereCenter.x - boxMax.x);
    // Y
    if (sphereCenter.y < boxMin.y) distSq += (boxMin.y - sphereCenter.y) * (boxMin.y - sphereCenter.y);
    else if (sphereCenter.y > boxMax.y) distSq += (sphereCenter.y - boxMax.y) * (sphereCenter.y - boxMax.y);
    // Z
    if (sphereCenter.z < boxMin.z) distSq += (boxMin.z - sphereCenter.z) * (boxMin.z - sphereCenter.z);
    else if (sphereCenter.z > boxMax.z) distSq += (sphereCenter.z - boxMax.z) * (sphereCenter.z - boxMax.z);

    return distSq <= sphereRadius * sphereRadius;
}

bool EnemyNormal::IsHit(const Bullet& bullet) const
{
    // ���f����AABB�����[���h���W�ɕϊ�
    VECTOR boxMin = {
        m_pos.x + m_aabbMin.x,
        m_pos.y + m_aabbMin.y,
        m_pos.z + m_aabbMin.z
    };
    VECTOR boxMax = {
        m_pos.x + m_aabbMax.x,
        m_pos.y + m_aabbMax.y,
        m_pos.z + m_aabbMax.z
    };
    return CheckAABBSphereHit(boxMin, boxMax, bullet.GetPos(), bullet.GetRadius());
}

//void EnemyNormal::DrawCollisionDebug() const
//{
//    DrawSphere3D(
//        m_pos,
//        m_colRadius,
//        16,
//        0xff0000, 
//        0xff0000, 
//        false     
//    );
//}

void EnemyNormal::DrawCollisionDebug() const
{
    VECTOR boxMin = {
        m_pos.x + m_aabbMin.x,
        m_pos.y + m_aabbMin.y,
        m_pos.z + m_aabbMin.z
    };
    VECTOR boxMax = {
        m_pos.x + m_aabbMax.x,
        m_pos.y + m_aabbMax.y,
        m_pos.z + m_aabbMax.z
    };
    unsigned int color = GetColor(255, 0, 0);

    // �J�v�Z���̒��S����AABB�̏㉺���S��
    VECTOR centerMin = { (boxMin.x + boxMax.x) * 0.5f, boxMin.y, (boxMin.z + boxMax.z) * 0.5f };
    VECTOR centerMax = { (boxMin.x + boxMax.x) * 0.5f, boxMax.y, (boxMin.z + boxMax.z) * 0.5f };

    // ���a��X,Z������AABB�T�C�Y�̂����傫�����̔���
    float radius = (std::max)(
        std::abs(boxMax.x - boxMin.x),
        std::abs(boxMax.z - boxMin.z)
        ) * 0.5f;

    DrawCapsule3D(centerMin, centerMax, radius, 16, color, color, FALSE);
}

