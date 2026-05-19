#pragma once
#include "SafeHandle.h"
#include <string>

/// <summary>
/// 音声リソースを RAII で管理するクラス。
/// SafeHandle によって DxLib 音声ハンドルの解放を自動化する。
/// </summary>
class ManagedSound
{
public:
    /// <summary>
    /// 音声なしで構築する既定コンストラクタ
    /// </summary>
    ManagedSound() = default;

    /// <summary>
    /// パスを指定して音声をロードするコンストラクタ
    /// </summary>
    /// <param name="path">音声ファイルパス</param>
    explicit ManagedSound(const std::string& path);

    ~ManagedSound() = default;

    /// <summary>
    /// 音声をロードする。既存のハンドルは自動解放される。
    /// </summary>
    /// <param name="path">音声ファイルパス</param>
    void Load(const std::string& path);

    /// <summary>
    /// DxLib 音声ハンドルを返す
    /// </summary>
    /// <returns>音声ハンドル。無効な場合は -1</returns>
    int Get() const { return m_handle.Get(); }

    /// <summary>
    /// int への暗黙変換。DxLib のサウンド関数にそのまま渡せる。
    /// </summary>
    /// <returns>音声ハンドル</returns>
    operator int() const { return m_handle.Get(); }

    /// <summary>
    /// 音声ハンドルが有効かどうかを返す
    /// </summary>
    /// <returns>有効なら true</returns>
    bool IsValid() const { return m_handle.IsValid(); }

    /// <summary>
    /// 音声を再生する
    /// </summary>
    /// <param name="playType">再生タイプ（DX_PLAYTYPE_NORMAL など）</param>
    /// <param name="topPositionFlag">true で先頭から再生（デフォルト: 1）</param>
    void Play(int playType, int topPositionFlag = 1);

    /// <summary>
    /// 再生中の音声を停止する
    /// </summary>
    void Stop();

    /// <summary>
    /// 音声が再生中かどうかを返す
    /// </summary>
    /// <returns>再生中なら true</returns>
    bool IsPlaying() const;

private:
    SafeHandle<SoundDeleter> m_handle; // 音声ハンドル（RAII 管理）
    std::string              m_path;   // 音声ファイルパス
};
