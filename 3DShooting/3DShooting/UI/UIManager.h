#pragma once
#include <vector>
#include <memory>
#include "UIBase.h"

/// <summary>
/// UI コンポーネントを一括管理するクラス。
/// 登録された全 UI の Init / Update / Draw を順次委譲する。
/// </summary>
class UIManager
{
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    UIManager() = default;

    /// <summary>
    /// デストラクタ（登録中の UI 要素は shared_ptr により自動解放される）
    /// </summary>
    ~UIManager() = default;

    /// <summary>
    /// UI コンポーネントを登録する（nullptr は無視される）
    /// </summary>
    /// <param name="ui">追加する UI の shared_ptr</param>
    void AddUI(std::shared_ptr<UIBase> ui);

    /// <summary>
    /// 登録済みの全 UI を初期化する
    /// </summary>
    void Init();

    /// <summary>
    /// 表示中の全 UI を更新する
    /// </summary>
    /// <param name="deltaTime">前フレームからの経過時間（秒）</param>
    void Update(float deltaTime);

    /// <summary>
    /// 表示中の全 UI を描画する
    /// </summary>
    void Draw();

    /// <summary>
    /// 登録済みの全 UI をクリアする
    /// </summary>
    void Clear();

    /// <summary>
    /// 指定した型の UI を取得する（登録順で最初に見つかったもの）
    /// </summary>
    /// <returns>見つかった場合はその shared_ptr、見つからない場合は nullptr</returns>
    template<typename T>
    std::shared_ptr<T> GetUI()
    {
        for (auto& ui : m_uiElements)
        {
            auto casted = std::dynamic_pointer_cast<T>(ui);
            if (casted) return casted;
        }
        return nullptr;
    }

private:
    std::vector<std::shared_ptr<UIBase>> m_uiElements; // 登録された UI コンポーネントのリスト
};
