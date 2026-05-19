#pragma once
#include "SafeHandle.h"
#include <string>

/// <summary>
/// フォントリソースを RAII で管理し、UI スケール変更に応じた再作成を提供するクラス。
/// SafeHandle によって DxLib フォントハンドルの解放を自動化する。
/// </summary>
class ManagedFont
{
public:
    /// <summary>
    /// フォント情報なしで構築する既定コンストラクタ
    /// </summary>
    ManagedFont() : m_baseSize(0), m_thick(0), m_fontType(0) {}

    /// <summary>
    /// フォント情報を指定して構築し、即座にロードするコンストラクタ
    /// </summary>
    /// <param name="fontName">フォント名</param>
    /// <param name="baseSize">基準サイズ（ピクセル）</param>
    /// <param name="thick">太さ</param>
    /// <param name="fontType">フォントタイプ（DX_FONTTYPE_* 定数）</param>
    /// <param name="initialScale">初期スケール（デフォルト: 1.0f）</param>
    ManagedFont(const std::string& fontName, int baseSize, int thick, int fontType, float initialScale = 1.0f);
    ~ManagedFont() = default;

    /// <summary>
    /// フォント情報を設定してロードする。既存のハンドルは自動解放される。
    /// </summary>
    /// <param name="fontName">フォント名</param>
    /// <param name="baseSize">基準サイズ（ピクセル）</param>
    /// <param name="thick">太さ</param>
    /// <param name="fontType">フォントタイプ（DX_FONTTYPE_* 定数）</param>
    /// <param name="initialScale">初期スケール（デフォルト: 1.0f）</param>
    void Load(const std::string& fontName, int baseSize, int thick, int fontType, float initialScale = 1.0f);

    /// <summary>
    /// 指定スケールでフォントを再作成する。UI 解像度変更時に呼び出す。
    /// </summary>
    /// <param name="scale">基準サイズに掛けるスケール係数</param>
    void Reload(float scale);

    /// <summary>
    /// DxLib フォントハンドルを返す
    /// </summary>
    /// <returns>フォントハンドル。無効な場合は -1</returns>
    int Get() const { return m_handle.Get(); }

    /// <summary>
    /// int への暗黙変換。DxLib のフォント描画関数にそのまま渡せる。
    /// </summary>
    /// <returns>フォントハンドル</returns>
    operator int() const { return m_handle.Get(); }

    /// <summary>
    /// フォントハンドルが有効かどうかを返す
    /// </summary>
    /// <returns>有効なら true</returns>
    bool IsValid() const { return m_handle.IsValid(); }

private:
    SafeHandle<FontDeleter> m_handle;   // フォントハンドル（RAII 管理）
    std::string             m_fontName; // フォント名
    int                     m_baseSize; // 基準フォントサイズ（ピクセル）
    int                     m_thick;    // 太さ
    int                     m_fontType; // フォントタイプ（DX_FONTTYPE_* 定数）
};
