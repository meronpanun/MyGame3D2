#pragma once
#include "SceneBase.h"
#include "EnemyBase.h"
#include "EffekseerForDXLib.h"
#include "ScoreManager.h"
#include "TutorialManager.h"
#include "TaskTutorialManager.h"
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
class Effect;
class DirectionIndicator;

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

	/// <summary>
	/// プレイヤーの弾が敵にヒットした際に呼ばれる(ヒットマーク表示用)
	/// </summary>
	/// <param name="part">ヒットした部位</param>
	void OnPlayerBulletHitEnemy(EnemyBase::HitPart part);

	/// <summary>
	/// スコアポップアップを追加
	/// </summary>
	/// <param name="score">加算スコア</param>
	/// <param name="isHeadShot">ヘッドショットならtrue</param>
	/// <param name="combo">コンボ数</param>
	void AddScorePopup(int score, bool isHeadShot, int combo);

public:
	static bool  s_isSkipTutorial; // チュートリアルスキップフラグ

	/// <summary>
	/// シングルトンインスタンスを取得
	/// </summary>
	/// <returns>シングルトンインスタンス</returns>
	static SceneMain* Instance();

	/// <summary>
	/// WaveManagerを取得
	/// </summary>
	/// <returns>WaveManagerのポインタ</returns>
	WaveManager* GetWaveManager() const { return m_pWaveManager.get(); }

	/// <summary>
	/// ゲーム経過時間（秒）を取得
	/// </summary>
	/// <returns>経過時間（秒）</returns>
	static float GetElapsedTime() { return s_elapsedTime; }

	/// <summary>
	/// すべてのエフェクトを停止する
	/// </summary>
	void StopAllEffects();

private:
	void DrawPauseMenu();

private:
    // ゲームオブジェクト管理
    std::unique_ptr<Player> m_pPlayer;
    std::shared_ptr<Camera> m_pCamera;
    std::shared_ptr<EnemyNormal> m_pEnemyNormal;
    std::shared_ptr<EnemyRunner> m_pEnemyRunner;
    std::shared_ptr<EnemyAcid> m_pEnemyAcid;
    std::shared_ptr<Stage> m_pStage;
    std::shared_ptr<WaveManager> m_pWaveManager;
	std::unique_ptr<Effect> m_pEffect;
    std::vector<EnemyBase*> m_enemyList;
    std::vector<std::shared_ptr<ItemBase>> m_items;

    std::unique_ptr<DirectionIndicator> m_pDirectionIndicator;

    // 状態管理
    bool  m_isPaused;                  // 一時停止中か
    bool  m_isReturningFromOption;     // オプションから戻ったか
    bool  m_isEscapePressed;           // Escapeキー押下状態
    bool  m_isReturningFromOtherScene; // 他シーンから戻ったか
    bool  m_isLoading;                 // ロード中か
    bool  m_isWave1FirstAidDropped;    // Wave1救急キットドロップ済み
    bool  m_isWave1AmmoDropped;        // Wave1弾薬ドロップ済み
    int   m_wave1DropCount;            // Wave1ドロップ回数
    EnemyBase::HitPart m_hitMarkType = EnemyBase::HitPart::Body; // ヒット部位

	// リソース管理
    int   m_skyDomeHandle;     // スカイドーム画像ハンドル
    int   m_dotDefaultHandle;          
    int   m_dotOnTargetHandle;         
    int   m_scoreFontHandle;   // スコアポップアップ用フォントハンドル
    int   m_bgmHandle;         // BGMハンドル
    bool  m_isBGMStarted;      // BGM再生済みフラグ

    // スコアポップアップ管理
    struct ScorePopup
    {
        int value;
        int combo;
        int timer;
        bool isHeadShot;
    };
    std::deque<ScorePopup> m_scorePopups;
    int m_totalScorePopupTimer;        // 合計スコア表示用タイマー
    int m_lastTotalScorePopupValue;    // 合計スコアポップアップ用一時保存値
    std::unique_ptr<TutorialManager> m_pTutorialManager;

    // 経過時間管理
    std::chrono::steady_clock::time_point m_pauseStartTime;
    int   m_hitMarkTimer;              // ヒットマーク表示タイマー
    int   m_clearSceneDelayTimer;      // ゲームクリア遷移遅延タイマー
    float m_cameraSensitivity;         // カメラ感度
    static float s_elapsedTime;        // ゲーム経過時間（秒）

    bool m_isPlayerInit;
	int m_gameOverDelayTimer; // ゲームオーバー遅延タイマー
	bool m_isTaskTutorialInit;
    static bool s_isLowHealthTutorialShown; // 低体力チュートリアル表示済みフラグ
};
