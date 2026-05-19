#pragma once
#include "SafeHandle.h"
#include <string>

/// <summary>
/// 画像リソースを RAII で管理するクラス。
/// SafeHandle によって DxLib 画像ハンドルの解放を自動化する。
/// </summary>
class ManagedGraph
{
public:
    /// <summary>
    /// 画像なしで構築する既定コンストラクタ
    /// </summary>
    ManagedGraph() = default;

    /// <summary>
    /// パスを指定して画像をロードするコンストラクタ
    /// </summary>
    /// <param name="path">画像ファイルパス</param>
    explicit ManagedGraph(const std::string& path);

    ~ManagedGraph() = default;

    /// <summary>
    /// 画像をロードする。既存のハンドルは自動解放される。
    /// </summary>
    /// <param name="path">画像ファイルパス</param>
    void Load(const std::string& path);

    /// <summary>
    /// DxLib 画像ハンドルを返す
    /// </summary>
    /// <returns>画像ハンドル。無効な場合は -1</returns>
    int Get() const { return m_handle.Get(); }

    /// <summary>
    /// int への暗黙変換。DxLib の描画関数にそのまま渡せる。
    /// </summary>
    /// <returns>画像ハンドル</returns>
    operator int() const { return m_handle.Get(); }

    /// <summary>
    /// 画像ハンドルが有効かどうかを返す
    /// </summary>
    /// <returns>有効なら true</returns>
    bool IsValid() const { return m_handle.IsValid(); }

private:
    SafeHandle<GraphDeleter> m_handle; // 画像ハンドル（RAII 管理）
    std::string              m_path;   // 画像ファイルパス
};
