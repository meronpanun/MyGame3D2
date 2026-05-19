#pragma once

/// <summary>
/// 全 UI コンポーネントの基底クラス。
/// Init / Update / Draw の純粋仮想インターフェースと
/// 表示・非表示の共通機能を提供する。
/// </summary>
class UIBase
{
public:
    /// <summary>
    /// コンストラクタ（デフォルトで表示状態に設定する）
    /// </summary>
    UIBase() : m_isVisible(true) {}

    virtual ~UIBase() = default;

    /// <summary>
    /// 初期化処理
    /// </summary>
    virtual void Init() = 0;

    /// <summary>
    /// 毎フレームの更新処理
    /// </summary>
    /// <param name="deltaTime">前フレームからの経過時間（秒）</param>
    virtual void Update(float deltaTime) = 0;

    /// <summary>
    /// 描画処理
    /// </summary>
    virtual void Draw() = 0;

    /// <summary>
    /// 表示・非表示を設定する
    /// </summary>
    /// <param name="visible">true で表示、false で非表示</param>
    void SetVisible(bool visible) { m_isVisible = visible; }

    /// <summary>
    /// 現在の表示状態を取得する
    /// </summary>
    /// <returns>表示中なら true</returns>
    bool IsVisible() const { return m_isVisible; }

protected:
    bool m_isVisible; // 表示状態フラグ（true: 表示、false: 非表示）
};
