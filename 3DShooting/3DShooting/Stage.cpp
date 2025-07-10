#include "Stage.h"
#include "TransformDataLoader.h"
#include <unordered_map>

void Stage::Init()
{
	TransformDataLoader loader;
	auto objectDataList = loader.LoadDataCSV("Data/CSV/stage.csv");

	for (const auto& data : objectDataList)
	{
		std::string modelPath;

		
		if (data.name == "Road_bend_2")
		{
			modelPath = "Data/Model/Road_bend_2.mv1";
		}
		else if (data.name == "Road_floor")
		{
			modelPath = "Data/Model/Road_floor.mv1";
		}
		else if (data.name == "Road_tile_2")
		{
			modelPath = "Data/Model/Road_tile_2.mv1";
		}
		//else if (data.name == "Crossroads_1_lines_circ")
		//{
		//	modelPath = "Data/Model/Crossroads_1_lines_circ.mv1";
		//}
		else
		{
			continue;
		}

		StageObject obj;
		Vec3 pos = { data.pos.x, data.pos.y, data.pos.z };
		Vec3 rot = { data.rot.x, data.rot.y, data.rot.z };
		Vec3 scale = { data.scale.x, data.scale.y, data.scale.z };
		obj.Init(modelPath, pos, rot, scale);
		m_objects.emplace_back(obj);
	}
}

void Stage::Draw()
{
	for (auto& obj : m_objects)
	{
		obj.Draw();
	}
}
