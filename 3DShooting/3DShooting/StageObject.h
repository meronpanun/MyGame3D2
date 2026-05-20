#pragma once
#include "Vec3.h"

/// <summary>
/// ステージオブジェクト 1 つ分（モデルハンドル・位置・回転・スケール）を保持するクラス。
/// MV1DuplicateModel で生成した複製ハンドルを所有し、デストラクタで自動解放する。
/// コピーは禁止し、ムーブのみ許可する（ハンドルの二重解放を防ぐため）。
/// </summary>
class StageObject
{
public:
    /// <summary>
    /// コンストラクタ。モデルハンドルを無効値（-1）で初期化する。
    /// </summary>
    StageObject();

    /// <summary>
    /// デストラクタ。保持するモデルハンドルを解放する。
    /// </summary>
    ~StageObject();

    // モデルハンドルの二重解放を防ぐためコピーを禁止する
    StageObject(const StageObject&)            = delete;
    StageObject& operator=(const StageObject&) = delete;

    // ムーブ時はハンドルの所有権を移譲する
    StageObject(StageObject&& other) noexcept;
    StageObject& operator=(StageObject&& other) noexcept;

    /// <summary>
    /// 複製済みモデルハンドルとトランスフォームを受け取り、DxLib に座標・回転・スケールを設定する
    /// </summary>
    /// <param name="duplicateHandle">MV1DuplicateModel で生成した複製ハンドル</param>
    /// <param name="pos">ワールド座標（Unity 座標系変換済み）</param>
    /// <param name="rot">回転角度・度単位（Unity 座標系変換済み）</param>
    /// <param name="scale">スケール（Unity エクスポート値）</param>
    void Init(int duplicateHandle, Vec3 pos, Vec3 rot, Vec3 scale);

    /// <summary>
    /// モデルを描画する
    /// </summary>
    void Draw();

    /// <summary>
    /// ワールド座標を返す
    /// </summary>
    /// <returns>ワールド座標</returns>
    Vec3 GetPos() const { return m_pos; }

    /// <summary>
    /// スケールを返す
    /// </summary>
    /// <returns>スケール値</returns>
    Vec3 GetScale() const { return m_scale; }

private:
    int  m_modelHandle; // MV1 モデルハンドル（-1 = 無効）
    Vec3 m_pos;         // ワールド座標
    Vec3 m_rot;         // 回転角度（度）
    Vec3 m_scale;       // スケール
};
