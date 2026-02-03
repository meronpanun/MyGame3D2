#include "ShellCasing.h"
#include "DxLib.h"
#include "Game.h"

namespace
{
	constexpr float kGravity       = 0.05f; // 重力の影響
	constexpr int   kMaxLifeTime   = 300;   // 最大寿命（フレーム）
}

int ShellCasing::s_modelHandle = -1;

void ShellCasing::LoadModel()
{
    s_modelHandle = MV1LoadModel("data/model/shell.mv1");
}

void ShellCasing::DeleteModel()
{
    MV1DeleteModel(s_modelHandle);
}

ShellCasing::ShellCasing(const VECTOR& pos, const VECTOR& dir)
{
    m_pos = pos;
    m_velocity = VScale(dir, 2.0f);
    m_velocity.y += 1.0f;
    m_rotation = VGet(GetRand(360) * DX_PI_F / 180.0f, GetRand(360) * DX_PI_F / 180.0f, GetRand(360) * DX_PI_F / 180.0f);
    m_modelHandle = MV1DuplicateModel(s_modelHandle);
    m_lifeTime = kMaxLifeTime;
}

void ShellCasing::Update()
{
    m_velocity.y -= kGravity;
    m_pos = VAdd(m_pos, m_velocity);

    // 3D座標を2Dスクリーン座標に変換
    VECTOR screenPos = ConvWorldPosToScreenPos(m_pos);

    // 画面外に出たら削除
    // 画面の境界に少しマージンを持たせる
    constexpr float margin = 50.0f;
    if (screenPos.x < -margin || screenPos.x > Game::GetScreenWidth() + margin ||
        screenPos.y < -margin || screenPos.y > Game::GetScreenHeight() + margin ||
        screenPos.z < 0.0f) // Zが0未満はカメラの後ろ、または遠すぎる場合
    {
        m_lifeTime = 0; // 寿命を0にして削除対象にする
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