#pragma once
#include "SceneBase.h"
#include "EnemyBase.h"
#include "DxLib.h"
#include <memory>
#include <chrono>
#include <vector>

class Player;
class Camera;
class EnemyNormal;
class EnemyRunner;
class EnemyAcid;
//class EnemyBase;
class FirstAidKitItem;
class ItemBase;
class Stage;
class WaveManager;

/// <summary>
/// メインシーンクラス
/// </summary>
class SceneMain : public SceneBase
{
public:
	SceneMain();
	virtual~SceneMain();

	void Init() override;
	SceneBase* Update() override;
	void Draw() override;
	void DrawShadowCasters(); // 影用描画

	void SetPaused(bool paused);

	Camera* GetCamera() const { return m_pCamera.get(); }
	
	void SetCameraSensitivity(float sensitivity);

private:
	void DrawPauseMenu();

private:
	std::unique_ptr<Player> m_pPlayer;
	std::shared_ptr<Camera> m_pCamera;
	std::shared_ptr<EnemyNormal> m_pEnemyNormal;
	std::shared_ptr<EnemyRunner> m_pEnemyRunner;
	std::shared_ptr<EnemyAcid> m_pEnemyAcid;
	std::shared_ptr<Stage> m_pStage;
	std::shared_ptr<WaveManager> m_pWaveManager;
	std::vector<EnemyBase*> m_enemyList;
	std::vector<std::shared_ptr<ItemBase>> m_items;

	std::chrono::steady_clock::time_point m_pauseStartTime;

	bool  m_isPaused;             
	bool  m_isReturningFromOption;
	bool  m_isEscapePressed;      
	int   m_skyDomeHandle;        
	int   m_dotHandle;            

	float m_cameraSensitivity;
    int m_hitMarkTimer = 0;
    static constexpr int kHitMarkDuration = 10;
    EnemyBase::HitPart m_hitMarkType = EnemyBase::HitPart::Body; // ヒット部位
public:
    // プレイヤーの弾が敵にヒットした際に呼ばれる（ヒットマーク表示用）
    void OnPlayerBulletHitEnemy(EnemyBase::HitPart part);
};

