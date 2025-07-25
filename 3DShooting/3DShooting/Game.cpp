#include "Game.h"
#include "SceneMain.h"
#include "SceneBase.h"
#include "Player.h"

// グローバルなカメラ感度
float Game::g_cameraSensitivity = 0.002f;
Player* Game::m_pPlayer = nullptr;
