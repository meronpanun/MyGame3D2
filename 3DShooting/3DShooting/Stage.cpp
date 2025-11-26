#include "Stage.h"
#include "TransformDataLoader.h"
#include "DxLib.h"
#include <unordered_map>
#include <cstdio>
#include <fstream>
#include <sstream>

void Stage::Init()
{
	TransformDataLoader loader;
	// 旧Stage.csvの読み込み処理
//	auto objectDataList = loader.LoadDataCSV("Data/CSV/Stage.csv");
	
	// MainStageTransformData.csvの読み込み
	auto objectDataList = loader.LoadDataCSV("Data/CSV/MainStageTransformData.csv");

	// ステージ当たり判定データの読み込み
	LoadCollisionData("Data/CSV/StageCollisionData.csv");

	int loadedCount = 0; // 読み込んだオブジェクト数
	int skippedCount = 0; // スキップされたオブジェクト数

	for (const auto& data : objectDataList)
	{
		std::string modelPath;

		// 旧モデルの処理
		//if (data.name == "UNIConcrete")
		//{
		//	modelPath = "Data/Model/UNIConcrete.mv1";
		//}
		//else if (data.name == "Road_floor")
		//{
		//	modelPath = "Data/Model/Road_floor.mv1";
		//}
		//else if (data.name == "Hangar_v3")
		//{
		//	modelPath = "Data/Model/Hangar_v3.mv1";
		//}	
		//else if (data.name == "Hangar")
		//{
		//	modelPath = "Data/Model/Hangar.mv1";
		//}	
		//else if (data.name == "container")
		//{
		//	modelPath = "Data/Model/container.mv1";
		//}

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
		else if (data.name == "Prop_Stone_Med_B")
		{
			modelPath = "Data/Model/Prop_Stone_Med_B.mv1";
		}
		else if (data.name == "Prop_Stone_Med_D")
		{
			modelPath = "Data/Model/Prop_Stone_Med_D.mv1";
		}
		else
		{
			skippedCount++;
			continue;
		}

		StageObject obj;
		Vec3 pos = { data.pos.x, data.pos.y, data.pos.z };
		Vec3 rot = { data.rot.x, data.rot.y, data.rot.z };
		Vec3 scale = { data.scale.x, data.scale.y, data.scale.z };
		obj.Init(modelPath, pos, rot, scale);
		m_objects.emplace_back(obj);
		loadedCount++;
	}
}

void Stage::Draw()
{
	for (auto& obj : m_objects)
	{
		obj.Draw();
	}

	// 当たり判定のデバッグ描画
	for (const auto& col : m_collisionData)
	{
		DrawTriangle3D(col.v1, col.v2, col.v3, GetColor(255, 0, 0), FALSE);
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
