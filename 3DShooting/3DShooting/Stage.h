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
	std::vector<StageObject> m_objects;
};

