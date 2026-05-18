#pragma once
#include "DxLib.h"
#include <vector>

/// <summary>
/// 弾丸の薬莢クラス
/// </summary>
class ShellCasing
{
public:
    ShellCasing(const VECTOR& pos, const VECTOR& dir);

    /// <summary>
    /// 毎フレームの更新処理（位置・回転・寿命の更新）
    /// </summary>
    void Update();

    /// <summary>
    /// 3D描画処理
    /// </summary>
    void Draw() const;

    /// <summary>
    /// 薬莢コンテナの一括更新処理
    /// </summary>
    /// <param name="shellCasings">薬莢コンテナ</param>
    static void UpdateShellCasings(std::vector<ShellCasing>& shellCasings);

    /// <summary>
    /// 薬莢コンテナの一括描画処理
    /// </summary>
    /// <param name="shellCasings">薬莢コンテナ</param>
    static void DrawShellCasings(const std::vector<ShellCasing>& shellCasings);

    /// <summary>
    /// モデル・SEの読み込み
    /// </summary>
    static void LoadResources();

    /// <summary>
    /// モデル・SEの解放
    /// </summary>
    static void DeleteResources();

private:
    VECTOR m_pos;      // 現在の位置
    VECTOR m_velocity; // 移動速度ベクトル
    VECTOR m_rotation; // 現在の回転角度（ラジアン）

    int m_modelHandle; // 複製モデルハンドル
    int m_lifeTime;    // 残り寿命（フレーム数）

    static int s_modelHandle; // 共有元モデルハンドル
};
