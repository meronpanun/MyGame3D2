#include "Game.h"
#include "SceneMain.h"
#include "SceneBase.h"
#include "Player.h"
#include "SceneManager.h"

// グローバルなカメラ感度
float Game::g_cameraSensitivity = 0.002f;
Player* Game::m_pPlayer = nullptr;
SceneManager* Game::m_pSceneManager = nullptr;

// タイムスケール関連の初期化
float Game::g_timeScale = 1.0f; // 初期値は通常速度
float Game::g_targetTimeScale = 1.0f; // 目標タイムスケール
float Game::g_timeScaleDuration = 0.0f; // 持続時間
float Game::g_timeScaleTimer = 0.0f; // タイマー
float Game::g_initialTimeScale = 1.0f; // 初期化

void Game::SetTimeScale(float scale, float duration)
{
    g_timeScale = scale;
    g_initialTimeScale = scale; // 開始時のスケールを保存
    g_targetTimeScale = 1.0f; 
    g_timeScaleDuration = duration;
    g_timeScaleTimer = duration;
}

void Game::UpdateTimeScale()
{
    if (g_timeScaleTimer > 0.0f)
    {
        // フレーム時間 (1/60) を減算。タイムスケールに関わらず実時間で管理
        g_timeScaleTimer -= 1.0f / 60.0f; 

        if (g_timeScaleTimer <= 0.0f)
        {
            g_timeScaleTimer = 0.0f;
            g_timeScale = 1.0f;
        }
        else
        {
            // 時間経過で徐々に元に戻す
            float t = 1.0f - (g_timeScaleTimer / g_timeScaleDuration);
            
            // 線形補間 (Lerp)
            g_timeScale = g_initialTimeScale + (1.0f - g_initialTimeScale) * t;
            
            // 補間曲線を変えたい場合 (例: EaseOutQuad)
            // float easeT = t * (2 - t);
            // g_timeScale = g_initialTimeScale + (1.0f - g_initialTimeScale) * easeT;
        }
    }
    else
    {
         g_timeScale = 1.0f;
    }
}

float Game::GetTimeScale()
{
    return g_timeScale;
}
