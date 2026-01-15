#include "Stage.h"
#include "TransformDataLoader.h"
#include "DxLib.h"
#include <unordered_map>
#include <cstdio>
#include <fstream>
#include <sstream>

bool Stage::s_isDrawCollision = false;
bool Stage::s_isDrawTutorialCollision = false;

void Stage::LoadStage(bool isTutorial)
{
	Clear();
	m_isTutorial = isTutorial;

	TransformDataLoader loader;
	std::vector<ObjectTransformData> objectDataList;

	if (isTutorial)
	{
		// 旧Stage.csvの読み込み処理 (チュートリアル用)
		objectDataList = loader.LoadDataCSV("Data/CSV/TutorialStageTransformData.csv");

		// ステージ当たり判定データの読み込み (チュートリアル用)
		LoadCollisionData("Data/CSV/TutorialStageCollisionData.csv");
	}
	else
	{
		// MainStageTransformData.csvの読み込み (メインステージ用)
		objectDataList = loader.LoadDataCSV("Data/CSV/MainStageTransformData.csv");

		// ステージ当たり判定データの読み込み (メインステージ用)
		LoadCollisionData("Data/CSV/MainStageCollisionData.csv");
	}

	int loadedCount = 0; // 読み込んだオブジェクト数
	int skippedCount = 0; // スキップされたオブジェクト数

	for (const auto& data : objectDataList)
	{
		std::string modelPath;

		if (isTutorial)
		{
			// 旧モデルの処理
			if (data.name == "UNIConcrete")
			{
				modelPath = "Data/Model/UNIConcrete.mv1";
			}
			else if (data.name == "Road_floor")
			{
				modelPath = "Data/Model/Road_floor.mv1";
			}
			else if (data.name == "Hangar_v3_basic")
			{
				modelPath = "Data/Model/Hangar_v3.mv1";
			}	
			else if (data.name == "Hangar_v1_full")
			{
				modelPath = "Data/Model/Hangar.mv1";
			}	
			else if (data.name == "Cargo_container_v1_LD1close")
			{
				modelPath = "Data/Model/container.mv1";
			}
		}
		else
		{
			// 新モデルの処理
			if (data.name == "Barrier_Group_1A")
			{
				modelPath = "Data/Model/Barrier_Group_1A.mv1";
			}
			else if (data.name == "Basic_Stairs_1B")
			{
				modelPath = "Data/Model/Basic_Stairs_1B.mv1";
			}
			else if (data.name == "Block_Platform_1B")
			{
				modelPath = "Data/Model/Block_Platform_1B.mv1";
			}
			else if (data.name == "Block_Platform_Corner_1B(Mirrored)")
			{
				modelPath = "Data/Model/Block_Platform_Corner_1B(Mirrored).mv1";
			}
			else if (data.name == "Block_Platform_Corner_1B")
			{
				modelPath = "Data/Model/Block_Platform_Corner_1B.mv1";
			}
			else if (data.name == "Chain")
			{
				modelPath = "Data/Model/Chain.mv1";
			}
			else if (data.name == "Floor_A")
			{
				modelPath = "Data/Model/Floor_A.mv1";
			}
			else if (data.name == "Floor_B")
			{
				modelPath = "Data/Model/Floor_B.mv1";
			}
			else if (data.name == "rock_3_br")
			{
				modelPath = "Data/Model/rock_3_br.mv1";
			}
			else if (data.name == "rock_6_br")
			{
				modelPath = "Data/Model/rock_6_br.mv1";
			}
		}

		if (modelPath.empty())
		{
			skippedCount++;
			continue;
		}

		// モデルキャッシュの確認
		int originalHandle = -1;
		if (m_modelCache.find(modelPath) != m_modelCache.end())
		{
			originalHandle = m_modelCache[modelPath];
		}
		else
		{
			// 新規読み込み
			originalHandle = MV1LoadModel(modelPath.c_str());
			if (originalHandle != -1)
			{
				m_modelCache[modelPath] = originalHandle;
			}
		}

		if (originalHandle != -1)
		{
			// モデルの複製
			int duplicateHandle = MV1DuplicateModel(originalHandle);
			if (duplicateHandle != -1)
			{
				StageObject obj;
				Vec3 pos = { data.pos.x, data.pos.y, data.pos.z };
				Vec3 rot = { data.rot.x, data.rot.y, data.rot.z };
				Vec3 scale = { data.scale.x, data.scale.y, data.scale.z };
				obj.Init(duplicateHandle, pos, rot, scale);
				m_objects.emplace_back(std::move(obj));
				loadedCount++;
			}
		}
	}
}

void Stage::Clear()
{
	m_objects.clear();
	m_collisionData.clear();
}

Stage::~Stage()
{
	// キャッシュされたモデルの解放
	for (auto& pair : m_modelCache)
	{
		MV1DeleteModel(pair.second);
	}
	m_modelCache.clear();
}

void Stage::Draw()
{
	for (auto& obj : m_objects)
	{
		obj.Draw();
	}

	// 当たり判定のデバッグ描画
	// 当たり判定のデバッグ描画
	bool isDraw = m_isTutorial ? s_isDrawTutorialCollision : s_isDrawCollision;
	if (isDraw)
	{
		for (const auto& col : m_collisionData)
		{
			DrawTriangle3D(col.v1, col.v2, col.v3, 0xff0000, false);
		}
	}
}

void Stage::LoadCollisionData(const char* fileName)
{
	std::ifstream file(fileName);
	if (!file.is_open())
	{
		return;
	}

	std::string line;
	bool isHeader = true;

	while (std::getline(file, line))
	{
		if (isHeader)
		{
			isHeader = false;
			continue;
		}

		std::stringstream ss(line);
		std::string element;
		StageCollisionData data;
		int index = 0;

		while (std::getline(ss, element, ','))
		{
			if (element.empty())
			{
				index++;
				continue;
			}

			try
			{
				switch (index)
				{
				case 0: data.name = element; break;
				case 1: data.v1.x = std::stof(element); break;
				case 2: data.v1.y = std::stof(element); break;
				case 3: data.v1.z = std::stof(element); break;
				case 4: data.v2.x = std::stof(element); break;
				case 5: data.v2.y = std::stof(element); break;
				case 6: data.v2.z = std::stof(element); break;
				case 7: data.v3.x = std::stof(element); break;
				case 8: data.v3.y = std::stof(element); break;
				case 9: data.v3.z = std::stof(element); break;
				}
			}
			catch (...)
			{
				// エラー処理（必要に応じてログ出力など）
			}
			index++;
		}
		m_collisionData.push_back(data);
	}
}
