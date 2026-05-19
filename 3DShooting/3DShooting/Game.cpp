#include "Game.h"
#include "Player.h"
#include "SceneBase.h"
#include "SceneMain.h"
#include "SceneManager.h"
#include "DxLib.h"

namespace
{
    constexpr float kDefaultCameraSensitivity = 0.002f; // カメラ感度のデフォルト値
    constexpr int   kDefaultScreenWidth       = 1280;   // デフォルト画面幅
    constexpr int   kDefaultScreenHeight      = 720;    // デフォルト画面高さ
    constexpr int   kDefaultColorBitDepth     = 32;     // カラービット数
    constexpr float kFixedDeltaTime           = 1.0f / 60.0f; // 固定フレーム時間（秒）
    constexpr float kUIScaleReferenceHeight   = 720.0f; // UI スケール計算の基準画面高さ
    constexpr float kUIScaleFactor            = 0.7f;   // UI 全体サイズの補正係数
}

// グローバルなカメラ感度
float        Game::g_cameraSensitivity = kDefaultCameraSensitivity;
Player*      Game::m_pPlayer           = nullptr;
WaveManager* Game::m_pWaveManager      = nullptr;
SceneManager* Game::m_pSceneManager   = nullptr;

// 画面解像度とモードの初期化
int  Game::m_screenWidth  = kDefaultScreenWidth;
int  Game::m_screenHeight = kDefaultScreenHeight;
int  Game::m_colorBitNum  = kDefaultColorBitDepth;
bool Game::s_isWindowMode = true;

// タイムスケール関連の初期化
float Game::g_timeScale         = 1.0f;
float Game::g_targetTimeScale   = 1.0f;
float Game::g_timeScaleDuration = 0.0f;
float Game::g_timeScaleTimer    = 0.0f;
float Game::g_initialTimeScale  = 1.0f;

bool Game::s_isPaused = false;

void Game::SetResolution(int w, int h)
{
    m_screenWidth  = w;
    m_screenHeight = h;
    SetGraphMode(m_screenWidth, m_screenHeight, m_colorBitNum);
    if (s_isWindowMode)
    {
        SetWindowSize(m_screenWidth, m_screenHeight);
    }
    SetDrawScreen(DX_SCREEN_BACK); // 描画先を裏画面に設定し直す
}

void Game::SetWindowMode(bool windowed)
{
    s_isWindowMode = windowed;
    ChangeWindowMode(s_isWindowMode);
    // 画面モード切替時はマウスカーソルを表示する（デバッグ操作用）
    SetMouseDispFlag(true);
}

void Game::SetTimeScale(float scale, float duration)
{
    g_timeScale         = scale;
    g_initialTimeScale  = scale; // 補間開始時のスケールを保存
    g_targetTimeScale   = 1.0f;
    g_timeScaleDuration = duration;
    g_timeScaleTimer    = duration;
}

void Game::UpdateTimeScale()
{
    if (g_timeScaleTimer > 0.0f)
    {
        // 実時間で管理するため、タイムスケールに関わらず固定フレーム時間を減算
        g_timeScaleTimer -= kFixedDeltaTime;

        if (g_timeScaleTimer <= 0.0f)
        {
            g_timeScaleTimer = 0.0f;
            g_timeScale      = 1.0f;
        }
        else
        {
            // 線形補間（Lerp）で徐々に通常速度へ復帰
            float t     = 1.0f - (g_timeScaleTimer / g_timeScaleDuration);
            g_timeScale = g_initialTimeScale + (1.0f - g_initialTimeScale) * t;
        }
    }
    else
    {
        g_timeScale = 1.0f;
    }
}

float Game::GetTimeScale()
{
    if (s_isPaused) return 0.0f;
    return g_timeScale;
}

float Game::GetUIScale()
{
    // 基準解像度（720p）に対する比率に補正係数を掛けてスケールを返す
    return (static_cast<float>(m_screenHeight) / kUIScaleReferenceHeight) * kUIScaleFactor;
}

void Game::SetPaused(bool paused)
{
    s_isPaused = paused;
}

bool Game::IsPaused()
{
    return s_isPaused;
}
