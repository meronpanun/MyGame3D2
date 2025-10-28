#pragma once
#include "DxLib.h"
#include <vector>

class ShellCasing
{
public:
    ShellCasing(const VECTOR& pos, const VECTOR& dir);
    void Update();
    void Draw() const;

    static void UpdateShellCasings(std::vector<ShellCasing>& shellCasings);
    static void DrawShellCasings(const std::vector<ShellCasing>& shellCasings);

private:
    VECTOR m_pos;
    VECTOR m_velocity;
    VECTOR m_rotation;
    int m_modelHandle;
    int m_lifeTime;
};
