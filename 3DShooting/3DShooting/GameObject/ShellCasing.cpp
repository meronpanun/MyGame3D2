#include "ShellCasing.h"
#include "DxLib.h"

namespace
{
	constexpr float kGravity = 0.05f; // 重力の強さ
	constexpr float kBounceDamping = 0.5f; // 跳ね返りの減衰率
	constexpr int kMaxLifeTime = 300; // 最大寿命（フレーム数）
}

ShellCasing::ShellCasing(const VECTOR& pos, const VECTOR& dir)
{
    m_pos = pos;
    m_velocity = VScale(dir, 2.0f);
    m_velocity.y += 1.0f;
    m_rotation = VGet(GetRand(360) * DX_PI_F / 180.0f, GetRand(360) * DX_PI_F / 180.0f, GetRand(360) * DX_PI_F / 180.0f);
    m_modelHandle = MV1LoadModel("data/model/shell.mv1");
    m_lifeTime = kMaxLifeTime;
}

void ShellCasing::Update()
{
    m_velocity.y -= kGravity;
    m_pos = VAdd(m_pos, m_velocity);

    if (m_pos.y < 0.0f)
    {
        m_pos.y = 0.0f;
        m_velocity.y *= -kBounceDamping;
        m_velocity.x *= 0.9f;
        m_velocity.z *= 0.9f;
    }

    m_rotation.x += 0.1f;
    m_rotation.z += 0.1f;

    m_lifeTime--;
}

void ShellCasing::Draw() const
{
    if (m_lifeTime > 0)
    {
        MV1SetPosition(m_modelHandle, m_pos);
        MV1SetRotationXYZ(m_modelHandle, m_rotation);
        MV1DrawModel(m_modelHandle);
    }
}

void ShellCasing::UpdateShellCasings(std::vector<ShellCasing>& shellCasings)
{
    for (auto it = shellCasings.begin(); it != shellCasings.end();)
    {
        it->Update();
        if (it->m_lifeTime <= 0)
        {
            MV1DeleteModel(it->m_modelHandle);
            it = shellCasings.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void ShellCasing::DrawShellCasings(const std::vector<ShellCasing>& shellCasings)
{
    for (const auto& shellCasing : shellCasings)
    {
        shellCasing.Draw();
    }
}
