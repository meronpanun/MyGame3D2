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
	struct StageCollisionData
	{
		std::string name;
		VECTOR v1;
		VECTOR v2;
		VECTOR v3;
	};

	void Init();
	void Draw();

	const std::vector<StageCollisionData>& GetCollisionData() const { return m_collisionData; }

private:
	std::vector<StageObject> m_objects;
	std::vector<StageCollisionData> m_collisionData;

	void LoadCollisionData(const char* fileName);
};

