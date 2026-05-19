#pragma once
#include "DxLib.h"
#include <utility>

// DxLib リソースの解放処理を定義するデリータ構造体
struct FontDeleter  { void operator()(int h) { DeleteFontToHandle(h); } };
struct GraphDeleter { void operator()(int h) { DeleteGraph(h);        } };
struct SoundDeleter { void operator()(int h) { DeleteSoundMem(h);     } };
struct ModelDeleter { void operator()(int h) { MV1DeleteModel(h);     } };

/// <summary>
/// DxLib リソースハンドルを RAII で安全に管理するクラステンプレート。
/// スコープを抜けると Deleter を介してリソースを自動解放する。
/// コピー不可・ムーブ可。
/// </summary>
template <typename Deleter>
class SafeHandle
{
public:
    /// <summary>
    /// 無効なハンドル（-1）で初期化する既定コンストラクタ
    /// </summary>
    SafeHandle() : m_handle(-1) {}

    /// <summary>
    /// 既存のハンドルを受け取るコンストラクタ
    /// </summary>
    /// <param name="handle">DxLib リソースハンドル</param>
    explicit SafeHandle(int handle) : m_handle(handle) {}

    SafeHandle(const SafeHandle&)            = delete; // コピー禁止
    SafeHandle& operator=(const SafeHandle&) = delete; // コピー禁止

    /// <summary>
    /// ムーブコンストラクタ。所有権を移譲し、移譲元を無効化する。
    /// </summary>
    SafeHandle(SafeHandle&& other) noexcept : m_handle(other.m_handle)
    {
        other.m_handle = -1;
    }

    /// <summary>
    /// ムーブ代入演算子。既存ハンドルを解放してから所有権を移譲する。
    /// </summary>
    SafeHandle& operator=(SafeHandle&& other) noexcept
    {
        if (this != &other)
        {
            Reset();
            m_handle       = other.m_handle;
            other.m_handle = -1;
        }
        return *this;
    }

    /// <summary>
    /// デストラクタ。保持しているハンドルを Deleter で解放する。
    /// </summary>
    ~SafeHandle()
    {
        Reset();
    }

    /// <summary>
    /// 現在のハンドルを解放し、新しいハンドルを設定する。
    /// 同一ハンドルを渡した場合は何もしない（自己代入防止）。
    /// </summary>
    /// <param name="newHandle">新しいハンドル（デフォルト: -1 で解放のみ）</param>
    void Reset(int newHandle = -1)
    {
        if (m_handle != -1 && m_handle != newHandle)
        {
            Deleter()(m_handle);
        }
        m_handle = newHandle;
    }

    /// <summary>
    /// ハンドル値を返す
    /// </summary>
    /// <returns>DxLib リソースハンドル。無効な場合は -1</returns>
    int Get() const { return m_handle; }

    /// <summary>
    /// int への暗黙変換。DxLib の各種関数にそのまま渡せる。
    /// </summary>
    /// <returns>DxLib リソースハンドル</returns>
    operator int() const { return m_handle; }

    /// <summary>
    /// ハンドルが有効（-1 でない）かどうかを返す
    /// </summary>
    /// <returns>有効なら true</returns>
    bool IsValid() const { return m_handle != -1; }

private:
    int m_handle; // 管理対象の DxLib リソースハンドル
};
