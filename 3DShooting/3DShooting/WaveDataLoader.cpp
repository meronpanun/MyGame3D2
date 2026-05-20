#include "WaveDataLoader.h"
#include "DxLib.h"
#include <fstream>
#include <sstream>

namespace
{
    // Unity 座標系（メートル）→ 本ゲーム座標系（センチメートル）への変換係数
    constexpr float kUnityToGameScale = 100.0f;
}

std::vector<WaveData> WaveDataLoader::LoadWaveData(const std::string& path)
{
    std::vector<WaveData> waveDataList;
    std::ifstream file(path);
    if (!file.is_open())
    {
        printf("Error: Cannot open %s\n", path.c_str());
        return waveDataList;
    }

    std::string line;
    std::getline(file, line); // ヘッダー行をスキップ

    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string token;
        WaveData data;

        if (!std::getline(ss, token, ',')) continue;
        data.wave = std::stoi(token);

        if (!std::getline(ss, token, ',')) continue;
        data.enemyType = token;

        if (!std::getline(ss, token, ',')) continue;
        data.count = std::stoi(token);

        if (!std::getline(ss, token, ',')) continue;
        data.spawnInterval = std::stof(token);

        if (!std::getline(ss, token, ',')) continue;
        data.startTime = std::stof(token);

        // 以下はオプション列。取得できなければ struct の既定値を使用する
        if (std::getline(ss, token, ',')) data.waveInterval      = std::stof(token);
        if (std::getline(ss, token, ',')) data.spawnLocationType  = std::stoi(token);
        if (std::getline(ss, token, ',')) data.hasShield          = (std::stoi(token) == 1);

        waveDataList.push_back(data);

        printf("Loaded: Wave %d, %s, Count %d, Interval %.1f, Start %.1f, Loc %d, Shield %d\n",
               data.wave, data.enemyType.c_str(), data.count,
               data.spawnInterval, data.startTime,
               data.spawnLocationType, data.hasShield ? 1 : 0);
    }
    return waveDataList;
}

std::vector<SpawnAreaInfo> WaveDataLoader::LoadSpawnAreaData(const std::string& path)
{
    std::vector<SpawnAreaInfo> spawnAreaList;
    std::ifstream file(path);
    if (!file.is_open())
    {
        printf("Error: Cannot open %s\n", path.c_str());
        return spawnAreaList;
    }

    std::string line;
    std::getline(file, line); // ヘッダー行をスキップ

    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string token;
        SpawnAreaInfo info;

        if (!std::getline(ss, token, ',')) continue;
        info.type = std::stoi(token);

        // 中心座標（Unity 座標系 → ゲーム座標系に変換）
        float x, y, z;
        if (!std::getline(ss, token, ',')) continue; x = std::stof(token);
        if (!std::getline(ss, token, ',')) continue; y = std::stof(token);
        if (!std::getline(ss, token, ',')) continue; z = std::stof(token);
        info.center = VGet(x * kUnityToGameScale, y * kUnityToGameScale, z * kUnityToGameScale);

        // サイズ（Unity 座標系 → ゲーム座標系に変換）
        float sx, sy, sz;
        if (!std::getline(ss, token, ',')) continue; sx = std::stof(token);
        if (!std::getline(ss, token, ',')) continue; sy = std::stof(token);
        if (!std::getline(ss, token, ',')) continue; sz = std::stof(token);
        info.size = VGet(sx * kUnityToGameScale, sy * kUnityToGameScale, sz * kUnityToGameScale);

        spawnAreaList.push_back(info);

        printf("Loaded SpawnArea: Type %d, Pos(%.1f, %.1f, %.1f), Size(%.1f, %.1f, %.1f)\n",
               info.type, info.center.x, info.center.y, info.center.z,
               info.size.x, info.size.y, info.size.z);
    }
    return spawnAreaList;
}
