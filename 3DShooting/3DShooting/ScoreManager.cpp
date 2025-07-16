#include "ScoreManager.h"
#include <cmath>

namespace
{
	constexpr int kBaseHeadShotScore  = 200;  // ヘッドショットの基本スコア
	constexpr int kBaseBodyShotScore  = 100;  // ボディショットの基本スコア
    constexpr int kComboGraceFrame    = 90;   // 1.5秒
	constexpr float kInitialComboRate = 1.1f; // コンボ倍率の初期値
}

ScoreManager& ScoreManager::Instance()
{
    static ScoreManager instance;
    return instance;
}

ScoreManager::ScoreManager() : 
    m_score(0),
    m_combo(0),
	m_comboTimer(kComboGraceFrame), // 初期は猶予フレーム数
	m_totalScore(0), // ゲーム全体の累計スコア
	m_bodyKillCount(0),
	m_headKillCount(0),
	m_lastComboRate(1.0f) // 初期コンボ倍率は1.0
{
}

int ScoreManager::AddScore(bool isHeadShot)
{
    int baseScore = isHeadShot ? kBaseHeadShotScore : kBaseBodyShotScore;
    m_combo++;
    float comboRate = std::pow(kInitialComboRate, m_combo - 1);
    m_lastComboRate = comboRate;
    int add = static_cast<int>(baseScore * comboRate);
    m_score += add;
    m_totalScore += add; // 累計スコアにも加算
    m_comboTimer = kComboGraceFrame; // コンボ猶予リセット
    if (isHeadShot) 
    {
        m_headKillCount++;
    }
    else 
    {
        m_bodyKillCount++;
    }
    return add;
}

void ScoreManager::Update()
{
    if (m_combo > 0 && m_comboTimer > 0) 
    {
        m_comboTimer--;
        if (m_comboTimer <= 0) 
        {
            m_combo = 0;
            m_lastComboRate = 1.0f;
            m_score = 0; // コンボが切れたらスコアもリセット
        }
    }
}

void ScoreManager::ResetCombo() 
{
    m_combo = 0;
}

int ScoreManager::GetScore() const 
{
    return m_score;
}

int ScoreManager::GetCombo() const 
{
    return m_combo;
}
