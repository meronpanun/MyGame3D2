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
    static int  m_screenWidth;    // 現在の画面幅（ピクセル）
    static int  m_screenHeight;   // 現在の画面高さ（ピクセル）
    static int  m_colorBitNum;    // カラービット数
    static bool s_isWindowMode;   // ウィンドウモードフラグ

    /// <summary>
    /// 解像度を変更し、DxLib のグラフィックモードとウィンドウサイズを更新する
    /// </summary>
    /// <param name="w">画面幅（ピクセル）</param>
    /// <param name="h">画面高さ（ピクセル）</param>
    static void SetResolution(int w, int h);

    /// <summary>
    /// 現在の画面幅を返す
    /// </summary>
    /// <returns>画面幅（ピクセル）</returns>
    static int GetScreenWidth() { return m_screenWidth; }

    /// <summary>
    /// 現在の画面高さを返す
    /// </summary>
    /// <returns>画面高さ（ピクセル）</returns>
    static int GetScreenHeight() { return m_screenHeight; }

    /// <summary>
    /// ウィンドウモード／フルスクリーンモードを切り替える
    /// </summary>
    /// <param name="windowed">true ならウィンドウモード</param>
    static void SetWindowMode(bool windowed);

    /// <summary>
    /// 現在ウィンドウモードかどうかを返す
    /// </summary>
    /// <returns>ウィンドウモードなら true</returns>
    static bool IsWindowMode() { return s_isWindowMode; }

    // ウィンドウタイトル
    static constexpr const char* kWindowTitle = "WAVEBREAKER";

    /// <summary>
    /// タイムスケールを設定し、指定時間かけて通常速度（1.0）へ復帰させる
    /// </summary>
    /// <param name="scale">初期タイムスケール</param>
    /// <param name="duration">復帰までの時間（秒）</param>
    static void SetTimeScale(float scale, float duration);

    /// <summary>
    /// タイムスケールを毎フレーム更新し、目標値へ線形補間する
    /// </summary>
    static void UpdateTimeScale();

    /// <summary>
    /// 現在のタイムスケールを返す（ポーズ中は 0.0 を返す）
    /// </summary>
    /// <returns>タイムスケール</returns>
    static float GetTimeScale();

    /// <summary>
    /// 画面高さを基準にした UI スケール値を返す
    /// </summary>
    /// <returns>UI スケール係数</returns>
    static float GetUIScale();

    /// <summary>
    /// ゲームの一時停止状態を設定する
    /// </summary>
    /// <param name="paused">true で一時停止、false で再開</param>
    static void SetPaused(bool paused);

    /// <summary>
    /// ゲームが一時停止中かどうかを返す
    /// </summary>
    /// <returns>一時停止中なら true</returns>
    static bool IsPaused();

    static Player*       m_pPlayer;       // プレイヤーへのポインタ
    static WaveManager*  m_pWaveManager;  // ウェーブマネージャへのポインタ
    static SceneManager* m_pSceneManager; // シーンマネージャへのポインタ

    static float g_cameraSensitivity; // カメラ感度（グローバル設定）

private:
    static float g_timeScale;         // 現在のタイムスケール
    static float g_targetTimeScale;   // 目標タイムスケール（常に 1.0）
    static float g_timeScaleDuration; // タイムスケール補間の総持続時間（秒）
    static float g_timeScaleTimer;    // 補間残り時間（秒）
    static float g_initialTimeScale;  // 補間開始時のタイムスケール

    static bool s_isPaused;           // 一時停止フラグ
};
