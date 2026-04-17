#include "ManagedFont.h"

ManagedFont::ManagedFont(const std::string& fontName, int baseSize, int thick, int fontType, float initialScale)
    : m_fontName(fontName)
    , m_baseSize(baseSize)
    , m_thick(thick)
    , m_fontType(fontType)
{
    Reload(initialScale);
}

void ManagedFont::Reload(float scale)
{
    // スケール後のサイズを計算
    int scaledSize = static_cast<int>(m_baseSize * scale);
    
    // フォントを作成し、SafeHandleにセット（古いハンドルは自動解放される）
    int newHandle = CreateFontToHandle(m_fontName.c_str(), scaledSize, m_thick, m_fontType);
    m_handle.Reset(newHandle);
}
