#pragma once
#include "StageObject.h"
#include <vector>
#include <memory>

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

	~Stage();

	void LoadStage(bool isTutorial);
	void Clear();
	void Draw();

	const std::vector<StageCollisionData>& GetCollisionData() const { return m_collisionData; }

	// デバッグ用
	static void SetDrawCollision(bool isDraw) { s_isDrawCollision = isDraw; }
	static bool IsDrawCollision() { return s_isDrawCollision; }
	static void SetDrawTutorialCollision(bool isDraw) { s_isDrawTutorialCollision = isDraw; }
	static bool IsDrawTutorialCollision() { return s_isDrawTutorialCollision; }

private:
	std::vector<StageObject> m_objects;
	std::vector<StageCollisionData> m_collisionData;
	std::unordered_map<std::string, int> m_modelCache;

	static bool s_isDrawCollision;
	static bool s_isDrawTutorialCollision;
	bool m_isTutorial;

	void LoadCollisionData(const char* fileName);
};

