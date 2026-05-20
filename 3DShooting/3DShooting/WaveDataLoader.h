#pragma once
#include "SpawnAreaInfo.h"
#include "WaveData.h"
#include <string>
#include <vector>

/// <summary>
/// CSV ファイルからウェーブデータとスポーンエリアデータを読み込む静的ユーティリティクラス。
/// ゲーム起動時に WaveManager が呼び出し、ステージ構成を初期化する。
/// </summary>
class WaveDataLoader
{
public:
    /// <summary>
    /// ウェーブデータを CSV から読み込む。
    /// ヘッダー行を 1 行スキップし、以降の各行を WaveData に変換して返す。
    /// </summary>
    /// <param name="path">CSV ファイルのパス</param>
    /// <returns>読み込んだ WaveData のリスト（ファイルを開けない場合は空リスト）</returns>
    static std::vector<WaveData> LoadWaveData(const std::string& path);

    /// <summary>
    /// スポーンエリアデータを CSV から読み込む。
    /// 座標・スケールは Unity 座標系（メートル）から本ゲーム座標系（センチメートル）に変換して格納する。
    /// </summary>
    /// <param name="path">CSV ファイルのパス</param>
    /// <returns>読み込んだ SpawnAreaInfo のリスト（ファイルを開けない場合は空リスト）</returns>
    static std::vector<SpawnAreaInfo> LoadSpawnAreaData(const std::string& path);
};
