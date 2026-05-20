#include "TransformDataLoader.h"
#include <fstream>
#include <sstream>

namespace
{
    // CSV の列数（Name, Pos*3, Rot*3, Scale*3, Attack, HP, ChaseSpeed, WalkSpeed, RunSpeed,
    //              ARInitAmmo, SGInitAmmo, ARPower, SGPower, TackleCD, TackleSPD, TackleDMG,
    //              MaxShield, ShieldRegen）
    constexpr int   kElementNum    = 24;
    constexpr float kUnityToDxPos  = 100.0f; // Unity 座標（m）を DxLib 座標（cm）に変換する係数
}

std::vector<ObjectTransformData> TransformDataLoader::LoadDataCSV(const char* fileName)
{
    std::vector<ObjectTransformData> objects;

    std::ifstream file(fileName);
    if (!file.is_open()) return objects;

    // パース補助ラムダ（変換失敗時はデフォルト値を返す）
    auto parseFloat = [](const std::string& s, float def = 0.0f) -> float
    {
        try   { return std::stof(s); }
        catch (...) { return def;    }
    };
    auto parseInt = [](const std::string& s, int def = 0) -> int
    {
        try   { return std::stoi(s); }
        catch (...) { return def;    }
    };

    bool        isHeader = true;
    std::string line;

    while (std::getline(file, line))
    {
        if (isHeader) { isHeader = false; continue; } // ヘッダー行をスキップ

        std::stringstream   ss(line);
        ObjectTransformData data;
        std::string         element;
        int                 index = 0;

        while (std::getline(ss, element, ',') && index < kElementNum)
        {
            if (element.empty()) { index++; continue; }

            switch (index)
            {
            case  0: data.name              = element;                              break;
            case  1: data.pos.x             = parseFloat(element) * kUnityToDxPos; break;
            case  2: data.pos.y             = parseFloat(element) * kUnityToDxPos; break;
            case  3: data.pos.z             = parseFloat(element) * kUnityToDxPos; break;
            case  4: data.rot.x             = parseFloat(element);                  break;
            case  5: data.rot.y             = parseFloat(element);                  break;
            case  6: data.rot.z             = parseFloat(element);                  break;
            case  7: data.scale.x           = parseFloat(element, 1.0f);            break;
            case  8: data.scale.y           = parseFloat(element, 1.0f);            break;
            case  9: data.scale.z           = parseFloat(element, 1.0f);            break;
            case 10: data.attack            = parseFloat(element);                  break;
            case 11: data.hp                = parseFloat(element);                  break;
            case 12: data.chaseSpeed        = parseFloat(element);                  break;
            case 13: data.speed             = parseFloat(element);                  break;
            case 14: data.runSpeed          = parseFloat(element);                  break;
            case 15: data.arInitAmmo        = parseInt(element);                    break;
            case 16: data.sgInitAmmo        = parseInt(element);                    break;
            case 17: data.bulletPower       = parseFloat(element);                  break;
            case 18: data.sgBulletPower     = parseFloat(element);                  break;
            case 19: data.tackleCooldown    = parseFloat(element);                  break;
            case 20: data.tackleSpeed       = parseFloat(element);                  break;
            case 21: data.tackleDamage      = parseFloat(element);                  break;
            case 22: data.maxShieldDurability = parseFloat(element);                break;
            case 23: data.shieldRegenRate   = parseFloat(element);                  break;
            default: break;
            }
            index++;
        }
        objects.push_back(data);
    }

    return objects;
}
