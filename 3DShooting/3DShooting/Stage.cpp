#include "Stage.h"
#include "TransformDataLoader.h"
#include "DxLib.h"
#include <cstdio>
#include <fstream>
#include <sstream>
#include <unordered_map>

namespace
{
    constexpr float        kObjectBoundsMargin  = 200.0f;   // オブジェクト位置に加えるバウンディングマージン
    constexpr float        kDefaultBoundsExtent = 1000.0f;  // オブジェクトなし時のデフォルトバウンディング範囲
    constexpr float        kBoundsYMinOffset    = 100.0f;   // バウンディング Y 最小値への追加オフセット
    constexpr float        kBoundsYMaxOffset    = 500.0f;   // バウンディング Y 最大値への追加オフセット
    constexpr unsigned int kCollisionDebugColor = 0xff0000; // 当たり判定デバッグ描画色（赤）
}

bool Stage::s_shouldDrawCollision         = false;
bool Stage::s_shouldDrawTutorialCollision = false;

Stage::Stage()
    : m_isTutorial(false)
    , m_minBounds(VGet(0.0f, 0.0f, 0.0f))
    , m_maxBounds(VGet(0.0f, 0.0f, 0.0f))
{
}

Stage::~Stage()
{
    // キャッシュされたモデルを解放
    for (auto& pair : m_modelCache)
    {
        MV1DeleteModel(pair.second);
    }
    m_modelCache.clear();
}

void Stage::LoadStage(bool isTutorial)
{
    Clear();
    m_isTutorial = isTutorial;

    TransformDataLoader loader;
    std::vector<ObjectTransformData> objectDataList;

    if (isTutorial)
    {
        objectDataList = loader.LoadDataCSV("Data/CSV/TutorialStageTransformData.csv");
        LoadCollisionData("Data/CSV/TutorialStageCollisionData.csv");
    }
    else
    {
        objectDataList = loader.LoadDataCSV("Data/CSV/MainStageTransformData.csv");
        LoadCollisionData("Data/CSV/MainStageCollisionData.csv");
    }

    for (const auto& data : objectDataList)
    {
        std::string modelPath;

        if (isTutorial)
        {
            if      (data.name == "UNIConcrete")    modelPath = "Data/Model/UNIConcrete.mv1";
            else if (data.name == "RoadFloor")      modelPath = "Data/Model/RoadFloor.mv1";
            else if (data.name == "HangarV3")       modelPath = "Data/Model/HangarV3.mv1";
            else if (data.name == "HangarV1")       modelPath = "Data/Model/Hangar.mv1";
            else if (data.name == "CargoContainer") modelPath = "Data/Model/Container.mv1";
        }
        else
        {
            if      (data.name == "BarrierGroup1A")                     modelPath = "Data/Model/BarrierGroup1A.mv1";
            else if (data.name == "BasicStairs1B")                      modelPath = "Data/Model/BasicStairs1B.mv1";
            else if (data.name == "BlockPlatform1B")                    modelPath = "Data/Model/BlockPlatform1B.mv1";
            else if (data.name == "Block_Platform_Corner_1B(Mirrored)") modelPath = "Data/Model/Block_Platform_Corner_1B(Mirrored).mv1";
            else if (data.name == "Block_Platform_Corner_1B")           modelPath = "Data/Model/Block_Platform_Corner_1B.mv1";
            else if (data.name == "Chain")                              modelPath = "Data/Model/Chain.mv1";
            else if (data.name == "FloorA")                             modelPath = "Data/Model/FloorA.mv1";
            else if (data.name == "FloorB")                             modelPath = "Data/Model/FloorB.mv1";
            else if (data.name == "Rock3")                              modelPath = "Data/Model/Rock3.mv1";
            else if (data.name == "Rock6")                              modelPath = "Data/Model/Rock6.mv1";
        }

        if (modelPath.empty()) continue;

        // モデルキャッシュを確認し、未ロードなら新規ロード
        int originalHandle = -1;
        auto it = m_modelCache.find(modelPath);
        if (it != m_modelCache.end())
        {
            originalHandle = it->second;
        }
        else
        {
            originalHandle = MV1LoadModel(modelPath.c_str());
            if (originalHandle != -1)
            {
                m_modelCache[modelPath] = originalHandle;
            }
        }

        if (originalHandle == -1) continue;

        // キャッシュ済みモデルを複製してオブジェクトに設定
        int duplicateHandle = MV1DuplicateModel(originalHandle);
        if (duplicateHandle != -1)
        {
            StageObject obj;
            Vec3 pos   = { data.pos.x,   data.pos.y,   data.pos.z   };
            Vec3 rot   = { data.rot.x,   data.rot.y,   data.rot.z   };
            Vec3 scale = { data.scale.x, data.scale.y, data.scale.z };
            obj.Init(duplicateHandle, pos, rot, scale);
            m_objects.emplace_back(std::move(obj));
        }
    }

    CalculateBounds();
}

void Stage::Clear()
{
    m_objects.clear();
    m_collisionData.clear();
}

void Stage::Draw()
{
    for (auto& obj : m_objects)
    {
        obj.Draw();
    }

    // 当たり判定のデバッグ描画
    bool isDraw = m_isTutorial ? s_shouldDrawTutorialCollision : s_shouldDrawCollision;
    if (isDraw)
    {
        for (const auto& col : m_collisionData)
        {
            DrawTriangle3D(col.v1, col.v2, col.v3, kCollisionDebugColor, false);
        }
    }
}

void Stage::LoadCollisionData(const char* fileName)
{
    std::ifstream file(fileName);
    if (!file.is_open()) return;

    std::string line;
    bool isHeader = true;

    while (std::getline(file, line))
    {
        if (isHeader) { isHeader = false; continue; }

        std::stringstream ss(line);
        std::string element;
        StageCollisionData data;
        int index = 0;

        while (std::getline(ss, element, ','))
        {
            if (element.empty()) { index++; continue; }

            try
            {
                switch (index)
                {
                case 0: data.name  = element;            break;
                case 1: data.v1.x  = std::stof(element); break;
                case 2: data.v1.y  = std::stof(element); break;
                case 3: data.v1.z  = std::stof(element); break;
                case 4: data.v2.x  = std::stof(element); break;
                case 5: data.v2.y  = std::stof(element); break;
                case 6: data.v2.z  = std::stof(element); break;
                case 7: data.v3.x  = std::stof(element); break;
                case 8: data.v3.y  = std::stof(element); break;
                case 9: data.v3.z  = std::stof(element); break;
                }
            }
            catch (...) {}
            index++;
        }
        m_collisionData.push_back(data);
    }
}

void Stage::CalculateBounds()
{
    if (m_objects.empty())
    {
        m_minBounds = VGet(-kDefaultBoundsExtent, 0.0f, -kDefaultBoundsExtent);
        m_maxBounds = VGet( kDefaultBoundsExtent, 0.0f,  kDefaultBoundsExtent);
        return;
    }

    m_minBounds = VGet( FLT_MAX,  FLT_MAX,  FLT_MAX);
    m_maxBounds = VGet(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    for (const auto& obj : m_objects)
    {
        Vec3 pos = obj.GetPos();
        // 位置に固定マージンを加えてバウンディングボックスを拡張する
        // （モデルのバウンディングボックスは取得せず位置＋マージンで簡易計算）
        if (pos.x - kObjectBoundsMargin < m_minBounds.x) m_minBounds.x = pos.x - kObjectBoundsMargin;
        if (pos.y - kObjectBoundsMargin < m_minBounds.y) m_minBounds.y = pos.y - kObjectBoundsMargin;
        if (pos.z - kObjectBoundsMargin < m_minBounds.z) m_minBounds.z = pos.z - kObjectBoundsMargin;
        if (pos.x + kObjectBoundsMargin > m_maxBounds.x) m_maxBounds.x = pos.x + kObjectBoundsMargin;
        if (pos.y + kObjectBoundsMargin > m_maxBounds.y) m_maxBounds.y = pos.y + kObjectBoundsMargin;
        if (pos.z + kObjectBoundsMargin > m_maxBounds.z) m_maxBounds.z = pos.z + kObjectBoundsMargin;
    }

    // Y 方向に追加オフセットを付与（地面下・天井上の余裕）
    m_minBounds.y -= kBoundsYMinOffset;
    m_maxBounds.y += kBoundsYMaxOffset;
}
