#pragma once
// 外部ライブラリ（Effekseer）内部で発生する警告を、このインクルードの間だけ抑制する
// 4100: 未使用引数, 4244: 型変換, 4481: 非標準拡張(abstract等), 26495: 未初期化メンバー
#pragma warning(push)
#pragma warning(disable: 4100 4244 4481 26495)
#include "EffekseerWarningSuppress.h"
#pragma warning(pop)
#include "EnemyBase.h"
#include "SceneBase.h"
#include "ScoreManager.h"
#include "TaskTutorialManager.h"
#include "TutorialManager.h"
#include "ManagedFont.h"
#include "ManagedGraph.h"
#include "ManagedSound.h"
#include "SafeHandle.h"
#include <chrono>
#include <deque>
#include <memory>
#include <vector>

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
class AnimationManager;
class UIManager;

/// <summary>
/// メインシーンクラス
/// </summary>
class SceneMain : public SceneBase
{
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    /// <param name="isReturningFromOtherScene">他シーンから戻った場合はtrue</param>
    SceneMain(bool isReturningFromOtherScene = false);
    virtual ~SceneMain();

    /// <summary>
    /// 初期化処理（リソースロード・各システム生成など）
    /// </summary>
    void Init() override;

    /// <summary>
    /// 毎フレームの更新処理
    /// </summary>
    /// <returns>遷移先シーンのポインタ（遷移しない場合はthisを返す）</returns>
    SceneBase* Update() override;

    /// <summary>
    /// 描画処理
    /// </summary>
    void Draw() override;

    /// <summary>
    /// ロード中かどうかを取得する
    /// </summary>
    /// <returns>ロード中ならtrue</returns>
    bool IsLoading() const override;

    /// <summary>
    /// 一時停止状態を設定する
    /// </summary>
    /// <param name="paused">一時停止するならtrue</param>
    void SetPaused(bool paused);

    /// <summary>
    /// デバッグHUDの表示を設定する
    /// </summary>
    /// <param name="show">表示するならtrue</param>
    void SetShowDebugHUD(bool show);

    /// <summary>
    /// デバッグHUDを表示中かどうかを取得する
    /// </summary>
    /// <returns>表示中ならtrue</returns>
    bool IsShowDebugHUD() const { return m_isShowDebugHUD; }

    /// <summary>
    /// カメラを取得する
    /// </summary>
    /// <returns>カメラのポインタ</returns>
    Camera* GetCamera() const { return m_pCamera.get(); }

    /// <summary>
    /// プレイヤーを取得する
    /// </summary>
    /// <returns>プレイヤーの参照</returns>
    Player& GetPlayer() const { return *m_pPlayer; }

    /// <summary>
    /// カメラ感度を設定する
    /// </summary>
    /// <param name="sensitivity">カメラ感度の値</param>
    void SetCameraSensitivity(float sensitivity);

    /// <summary>
    /// プレイヤーの弾が敵にヒットした際に呼ばれる（ヒットマーク表示用）
    /// </summary>
    /// <param name="part">ヒットした部位</param>
    /// <param name="distance">ヒットした距離</param>
    void OnPlayerBulletHitEnemy(EnemyBase::HitPart part, float distance);

    /// <summary>
    /// スコアポップアップを追加する
    /// </summary>
    /// <param name="score">加算スコア</param>
    /// <param name="isHeadShot">ヘッドショットならtrue</param>
    /// <param name="combo">コンボ数</param>
    void AddScorePopup(int score, bool isHeadShot, int combo);

public:
    /// <summary>
    /// チュートリアルをスキップするかどうかのフラグ
    /// </summary>
    static bool s_isSkipTutorial;

    /// <summary>
    /// シングルトンインスタンスを取得する
    /// </summary>
    /// <returns>シングルトンインスタンス</returns>
    static SceneMain* Instance();

    /// <summary>
    /// WaveManagerを取得する
    /// </summary>
    /// <returns>WaveManagerのポインタ</returns>
    WaveManager* GetWaveManager() const { return m_pWaveManager.get(); }

    /// <summary>
    /// TutorialManagerを取得する
    /// </summary>
    /// <returns>TutorialManagerのポインタ</returns>
    TutorialManager* GetTutorialManager() const { return m_pTutorialManager.get(); }

    /// <summary>
    /// Playerのポインタを取得する
    /// </summary>
    /// <returns>Playerのポインタ</returns>
    Player* GetPlayerPtr() const { return m_pPlayer.get(); }

    /// <summary>
    /// ゲーム経過時間（秒）を取得する
    /// </summary>
    /// <returns>経過時間（秒）</returns>
    static float GetElapsedTime() { return s_elapsedTime; }

    /// <summary>
    /// エフェクト管理クラスを取得する
    /// </summary>
    /// <returns>エフェクト管理クラスのポインタ</returns>
    Effect* GetEffect() const { return m_pEffect.get(); }

    /// <summary>
    /// すべてのエフェクトを停止する
    /// </summary>
    void StopAllEffects();

    /// <summary>
    /// TaskTutorialInitフラグを設定する
    /// </summary>
    /// <param name="isInit">初期化済みならtrue</param>
    void SetTaskTutorialInit(bool isInit) { m_isTaskTutorialInit = isInit; }

private:
    /// <summary>
    /// ポーズメニューを描画する
    /// </summary>
    void DrawPauseMenu();

    /// <summary>
    /// デバッグHUDを描画する
    /// </summary>
    void DrawDebugHUD();

    /// <summary>
    /// チュートリアルステージからメインステージへ切り替える
    /// </summary>
    void SwitchToMainStage();

private:
    bool  m_isShowDebugHUD;    // デバッグHUDを表示するか
    float m_lastDeltaTime;     // デバッグ用デルタタイム
    long long m_prevTimeCount; // デルタタイム計測用カウンタ

    // ゲームオブジェクト管理
    std::unique_ptr<Player>       m_pPlayer;
    std::shared_ptr<Camera>       m_pCamera;
    std::shared_ptr<Stage>        m_pStage;
    std::shared_ptr<WaveManager>  m_pWaveManager;
    std::unique_ptr<Effect>       m_pEffect;
    std::unique_ptr<AnimationManager> m_pAnimManager;
    std::vector<std::shared_ptr<ItemBase>> m_items;

    std::unique_ptr<DirectionIndicator> m_pDirectionIndicator;
    std::unique_ptr<UIManager>          m_pUIManager;

    // 状態管理
    bool m_isPaused;                   // 一時停止中か
    bool m_isEscapePressed;            // Escapeキー押下状態
    bool m_isReturningFromOtherScene;  // 他シーンから戻ったか
    bool m_isLoading;                  // ロード中か
    bool m_hasDroppedWave1FirstAid;    // Wave1救急キットドロップ済み
    bool m_hasDroppedWave1Ammo;        // Wave1弾薬ドロップ済み
    int  m_wave1DropCount;             // Wave1ドロップ回数

    // リソース管理
    SafeHandle<ModelDeleter> m_skyDome; // スカイドームモデルハンドル
    int m_headShotSECooldownTimer;      // ヘッドショットSEのクールタイムタイマー
    std::unique_ptr<TutorialManager> m_pTutorialManager;

    // 経過時間管理
    std::chrono::steady_clock::time_point m_pauseStartTime;

    float m_hitDistance;         // ヒットした距離
    int   m_clearSceneDelayTimer;// ゲームクリア遷移遅延タイマー
    float m_cameraSensitivity;   // カメラ感度
    static float s_elapsedTime;  // ゲーム経過時間（秒）

    bool m_isPlayerInit;      // プレイヤー初期化済みフラグ
    int  m_gameOverDelayTimer;// ゲームオーバー遅延タイマー
    bool m_isTaskTutorialInit;// タスクチュートリアル初期化済みフラグ
    static bool s_hasShownLowHealthTutorial; // 低体力チュートリアル表示済みフラグ

    bool m_isTutorialStage; // チュートリアルステージかどうか

    // ローディング演出
    int   m_loadingFrameCount;    // ロード完了後の待機フレームカウンタ
    int   m_loadingDotCount;      // ローディングアニメーションのドット数
    int   m_loadingAnimTimer;     // ローディングアニメーションタイマー

    // デバッグ表示用キャッシュ
    float m_cachedFPS;            // FPS（キャッシュ）
    float m_cachedDeltaTime;      // デルタタイム（キャッシュ）
    int   m_cachedTotalEnemies;   // 総敵数（キャッシュ）
    int   m_cachedUpdatedEnemies; // AI更新済み敵数（キャッシュ）
    int   m_cachedDrawnEnemies;   // 描画済み敵数（キャッシュ）
    int   m_debugDisplayTimer;    // デバッグ情報の更新間隔タイマー

    // ローディング画面用
    SafeHandle<ModelDeleter> m_loadingModel;   // ローディング用モデルハンドル
    VECTOR m_loadingModelPos;                  // ローディング用モデル位置
    float  m_loadingModelAnimTime;             // ローディング用モデルアニメーション時間
};
