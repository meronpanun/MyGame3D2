#pragma once
#include "StageObject.h"
#include <vector>
#include <memory>

/// <summary>
/// ウェーブの種類を表す列挙型
/// </summary>
enum class WaveType : int
{
	Wave1,
	Wave2,
	Wave3
};

/// <summary>
/// ステージクラス
/// </summary>
class Stage
{
public:
	void Init();
	void Draw();

private:
	struct StageCollisionData
	{
		std::string name;
		VECTOR v1;
		VECTOR v2;
		VECTOR v3;
	};

private:
	std::vector<StageObject> m_objects;
	std::vector<StageCollisionData> m_collisionData;

	void LoadCollisionData(const char* fileName);
};

