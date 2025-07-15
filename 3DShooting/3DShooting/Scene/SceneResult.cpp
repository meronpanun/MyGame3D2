#include "SceneResult.h"
#include "DxLib.h"

namespace
{
}

SceneResult::SceneResult()
{
}

SceneResult::~SceneResult()
{
}

void SceneResult::Init()
{
}

SceneBase* SceneResult::Update()
{
	return nullptr;
}

void SceneResult::Draw()
{
	DrawString(100, 100, "Result Scene", 0xffffff);
}
