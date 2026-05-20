#pragma once
#include <vector>

class Player;
class SceneManager;
class WaveManager;

/// <summary>
/// ゲーム全体で共有するグローバル状態（解像度・タイムスケール・一時停止など）を
/// static メンバとして一元管理するクラス。インスタンス化は不要。
/// </summary>
class Game
{
public:
    // ウィンドウタイトル
    static constexpr const char* kWindowTitle = "WAVEBREAKER";

    /// <summary>
    /// 解像度を変更し、DxLib のグラフィックモードとウィンドウサイズを更新する
    /// </summary>
    /// <param name="w">画面幅（ピクセル）</param>
    /// <param name="h">画面高さ（ピクセル）</param>
    static void SetResolution(int w, int h);

    /// <summary>現在の画面幅を返す</summary>
    /// <returns>画面幅（ピクセル）</returns>
    static int  GetScreenWidth()  { return s_screenWidth; }

    /// <summary>現在の画面高さを返す</summary>
    /// <returns>画面高さ（ピクセル）</returns>
    static int  GetScreenHeight() { return s_screenHeight; }

    /// <summary>カラービット数を返す</summary>
    /// <returns>カラービット数</returns>
    static int  GetColorBitNum()  { return s_colorBitNum; }

    /// <summary>
    /// ウィンドウモード／フルスクリーンモードを切り替える
    /// </summary>
    /// <param name="windowed">true ならウィンドウモード</param>
    static void SetWindowMode(bool windowed);

    /// <summary>現在ウィンドウモードかどうかを返す</summary>
    /// <returns>ウィンドウモードなら true</returns>
    static bool IsWindowMode() { return s_isWindowMode; }

    /// <summary>
    /// タイムスケールを設定し、指定時間かけて通常速度（1.0）へ復帰させる
    /// </summary>
    /// <param name="scale">初期タイムスケール</param>
    /// <param name="duration">復帰までの時間（秒）</param>
    static void  SetTimeScale(float scale, float duration);

    /// <summary>タイムスケールを毎フレーム更新し、目標値へ線形補間する</summary>
    static void  UpdateTimeScale();

    /// <summary>現在のタイムスケールを返す（ポーズ中は 0.0 を返す）</summary>
    /// <returns>タイムスケール</returns>
    static float GetTimeScale();

    /// <summary>画面高さを基準にした UI スケール値を返す</summary>
    /// <returns>UI スケール係数</returns>
    static float GetUIScale();

    /// <summary>ゲームの一時停止状態を設定する</summary>
    /// <param name="paused">true で一時停止、false で再開</param>
    static void SetPaused(bool paused);

    /// <summary>ゲームが一時停止中かどうかを返す</summary>
    /// <returns>一時停止中なら true</returns>
    static bool IsPaused();

    /// <summary>
    /// プレイヤーへの非所有ポインタを返す。
    /// SceneMain が存在しない間は nullptr を返す。呼び出し元は必ず null チェックを行うこと。
    /// </summary>
    static Player*       GetPlayer()          { return s_pPlayer; }

    /// <summary>
    /// プレイヤーへの非所有ポインタを登録する。
    /// SceneMain::Init() で所有オブジェクトの .get() を渡し、
    /// SceneMain::~SceneMain() で nullptr を渡してダングリングを防ぐ。
    /// </summary>
    static void          SetPlayer(Player* p) { s_pPlayer = p; }

    /// <summary>
    /// ウェーブマネージャへの非所有ポインタを返す。
    /// SceneMain が存在しない間は nullptr を返す。呼び出し元は必ず null チェックを行うこと。
    /// </summary>
    static WaveManager*  GetWaveManager()            { return s_pWaveManager; }

    /// <summary>
    /// ウェーブマネージャへの非所有ポインタを登録する。
    /// SceneMain::Init() で所有オブジェクトの .get() を渡し、
    /// SceneMain::~SceneMain() で nullptr を渡してダングリングを防ぐ。
    /// </summary>
    static void          SetWaveManager(WaveManager* wm) { s_pWaveManager = wm; }

    /// <summary>
    /// シーンマネージャへの非所有ポインタを返す。
    /// main.cpp が所有し、プログラム終了まで有効。
    /// </summary>
    static SceneManager* GetSceneManager()             { return s_pSceneManager; }

    /// <summary>シーンマネージャへの非所有ポインタを登録する（main.cpp から一度だけ呼ぶ）</summary>
    static void          SetSceneManager(SceneManager* sm) { s_pSceneManager = sm; }

    /// <summary>カメラ感度を返す</summary>
    static float GetCameraSensitivity()        { return s_cameraSensitivity; }

    /// <summary>カメラ感度を設定する</summary>
    static void  SetCameraSensitivity(float v) { s_cameraSensitivity = v; }

private:
    static int   s_screenWidth;    // 現在の画面幅（ピクセル）
    static int   s_screenHeight;   // 現在の画面高さ（ピクセル）
    static int   s_colorBitNum;    // カラービット数
    static bool  s_isWindowMode;   // ウィンドウモードフラグ

    static Player*       s_pPlayer;       // 非所有。SceneMain の生存期間中のみ有効（破棄時に nullptr）
    static WaveManager*  s_pWaveManager;  // 非所有。SceneMain の生存期間中のみ有効（破棄時に nullptr）
    static SceneManager* s_pSceneManager; // 非所有。main.cpp が所有しプログラム終了まで有効

    static float s_cameraSensitivity; // カメラ感度

    static float s_timeScale;         // 現在のタイムスケール
    static float s_targetTimeScale;   // 目標タイムスケール（常に 1.0）
    static float s_timeScaleDuration; // タイムスケール補間の総持続時間（秒）
    static float s_timeScaleTimer;    // 補間残り時間（秒）
    static float s_initialTimeScale;  // 補間開始時のタイムスケール

    static bool  s_isPaused;          // 一時停止フラグ
};
