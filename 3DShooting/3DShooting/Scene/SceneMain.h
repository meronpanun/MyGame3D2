#pragma once
#include "SceneBase.h"
#include "EnemyBase.h"
#include "EffekseerForDXLib.h"
#include "ScoreManager.h"
#include "TutorialManager.h"
#include <memory>
#include <chrono>
#include <vector>
#include <deque>

class Player;
class Camera;
class EnemyNormal;
class EnemyRunner;
class EnemyAcid;
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
	SceneMain(bool isReturningFromOtherScene = false);
	virtual~SceneMain();

	void Init() override;
	SceneBase* Update() override;
	void Draw() override;

	/// <summary>
	/// 一時停止状態を設定
	/// </summary>
	/// <param name="paused">一時停止状態</param>
	void SetPaused(bool paused); 

	/// <summary>
	/// カメラを取得
	/// </summary>
	/// <returns>カメラのポインタ</returns>
	Camera* GetCamera() const { return m_pCamera.get(); } 

	/// <summary>
	/// カメラ感度を設定
	/// </summary>
	/// <param name="sensitivity">感度</param>
	void SetCameraSensitivity(float sensitivity);

	// プレイヤーの弾が敵にヒットした際に呼ばれる(ヒットマーク表示用)
	void OnPlayerBulletHitEnemy(EnemyBase::HitPart part);

	// スコアポップアップを追加する関数
	void AddScorePopup(int score, bool isHeadShot, int combo);

public:
	static bool s_isSkipTutorial; // チュートリアルスキップ用デバッグフラグ
	bool m_isWave1FirstAidDropped;
	bool m_isWave1AmmoDropped;
	bool m_isLoading;
	int m_wave1DropCount;

	static SceneMain* Instance();
	WaveManager* GetWaveManager() const { return m_pWaveManager.get(); }

	// ゲーム経過時間（秒）を取得
	static float GetElapsedTime() { return s_elapsedTime; }
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
	std::unique_ptr<TutorialManager> m_pTutorialManager;

	std::chrono::steady_clock::time_point m_pauseStartTime;

	EnemyBase::HitPart m_hitMarkType = EnemyBase::HitPart::Body; // ヒット部位
	bool  m_isPaused;
	bool  m_isReturningFromOption;
	bool  m_isEscapePressed;
	int   m_skyDomeHandle;
	int   m_dotHandle;
	int m_hitMarkTimer;

	float m_cameraSensitivity;

	// スコアポップアップ情報
	struct ScorePopup
	{
		int value;
		int combo;
		int timer;
		bool isHeadShot;
	};
	std::deque<ScorePopup> m_scorePopups;
	int m_totalScorePopupTimer; // 合計スコア表示用タイマー
	int m_lastTotalScorePopupValue; // 合計スコアポップアップ用の一時保存値

	static float s_elapsedTime;

	int m_bgmHandle;       // ゲームシーンBGMのハンドル
	bool m_isBGMStarted;   // BGM再生済みフラグ
	bool m_isReturningFromOtherScene; // 他のシーンから戻ってきたかどうか
};


