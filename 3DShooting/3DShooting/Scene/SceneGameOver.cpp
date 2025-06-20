#include "SceneGameOver.h"
#include "DxLib.h"

SceneGameOver::SceneGameOver()
{
}

SceneGameOver::~SceneGameOver()
{
}

void SceneGameOver::Init()
{
}

SceneBase* SceneGameOver::Update()
{
	return nullptr;
}

void SceneGameOver::Draw()
{
	DrawString(100, 100, "Game Over", 0xff0000);
}
