#pragma once
#include "DxLib.h"
#include <vector>

/// <summary>
/// 
/// </summary>
class ShellCasing
{
public:
    ShellCasing(const VECTOR& pos, const VECTOR& dir);

    void Update();
    void Draw() const;

    /// <summary>
	/// 
    /// </summary>
	/// <param name="shellCasings">
    static void UpdateShellCasings(std::vector<ShellCasing>& shellCasings);

    /// <summary>
	/// 
    /// </summary>
	/// <param name="shellCasings">
    static void DrawShellCasings(const std::vector<ShellCasing>& shellCasings);

public:
    /// <summary>
    /// モデルの読み込み
    /// </summary>
    static void LoadModel();

    /// <summary>
	/// モデルの解放
    /// </summary>
    static void DeleteModel();

private:
    VECTOR m_pos;
    VECTOR m_velocity;
    VECTOR m_rotation;

    int m_modelHandle;
    int m_lifeTime;

    static int s_modelHandle;
};
