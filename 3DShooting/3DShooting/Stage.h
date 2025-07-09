#pragma once
#include "StageObject.h"
#include <vector>
#include <memory>

enum class WaveType : int
{
	Wave1,
	Wave2,
	Wave3
};

class Stage
{
public:
	void Init();
	void Draw();

private:
	std::vector<StageObject> m_objects;
};

