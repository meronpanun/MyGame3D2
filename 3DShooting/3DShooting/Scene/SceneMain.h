#pragma once
#include "SceneBase.h"
#include "DxLib.h"
#include <memory>
#include <chrono>
#include <vector>

class Player;
class Camera;
class EnemyNormal;
class EnemyBase;
class FirstAidKitItem;
class ItemBase;

class SceneMain : public SceneBase
{
public:
	SceneMain();
	virtual~SceneMain();

	void Init() override;
	SceneBase* Update() override;
	void Draw() override;

	void SetPaused(bool paused);

	Camera* GetCamera() const { return m_pCamera.get(); }
	
	void SetCameraSensitivity(float sensitivity);

private:
	void DrawPauseMenu();

private:
	std::unique_ptr<Player> m_pPlayer;
	std::shared_ptr<Camera> m_pCamera;
	std::shared_ptr<EnemyNormal> m_pEnemyNormal;
	std::vector<EnemyBase*> m_enemyList;
	std::vector<std::shared_ptr<ItemBase>> m_items;

	std::chrono::steady_clock::time_point m_pauseStartTime;

	bool  m_isPaused;             
	bool  m_isReturningFromOption;
	bool  m_isEscapePressed;      
	int   m_skyDomeHandle;        
	int   m_dotHandle;            

	float m_cameraSensitivity;
};

