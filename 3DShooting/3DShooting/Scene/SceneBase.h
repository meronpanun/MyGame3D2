#pragma once

/// <summary>
/// シーン基底クラス（純粋仮想）
/// </summary>
class SceneBase
{
public:
    SceneBase()          = default;
    virtual ~SceneBase() = default;

    /// <summary>
    /// 初期化処理
    /// </summary>
    virtual void Init() = 0;

    /// <summary>
    /// 毎フレームの更新処理
    /// </summary>
    /// <returns>遷移先シーンのポインタ（遷移しない場合は自身を返す）</returns>
    virtual SceneBase* Update() = 0;

    /// <summary>
    /// 描画処理
    /// </summary>
    virtual void Draw() = 0;

    /// <summary>
    /// ロード中かどうかを取得する
    /// </summary>
    /// <returns>ロード中ならtrue</returns>
    virtual bool IsLoading() const { return false; }
};
