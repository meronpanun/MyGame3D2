#pragma once

// 外部ライブラリ（Effekseer）内部で発生する警告を抑制するためのラップヘッダ
// 4100: 未使用引数
// 4244: 型変換（浮動小数点から整数への切り捨てなど）
// 4481: 非標準拡張（abstract キーワードなど）
// 26495: 未初期化メンバ変数
// 6385: 無効なデータの読み取り（静的解析警告）

#pragma warning(push)
#pragma warning(disable: 4100 4244 4481 26495 6385)

#include "EffekseerForDXLib.h"

#pragma warning(pop)
