#include "ScoreManager.h"
#include <cmath>
#include <fstream>
#include <algorithm>

namespace
{
    constexpr int kBaseHeadShotScore  = 200;  // ヘッドショットの基本スコア
    constexpr int kBaseBodyShotScore  = 100;  // ボディショットの基本スコア
    constexpr int kComboGraceFrame    = 90;   // 1.5秒
    constexpr float kInitialComboRate = 1.1f; // コンボ倍率の初期値
    const char* kScoreFileName = "highscores.txt"; // スコア保存ファイル名
    constexpr int kMaxHighScores = 10;
}

ScoreManager& ScoreManager::Instance()
{
    static ScoreManager instance;
    return instance;
}

ScoreManager::ScoreManager() : 
    m_score(0),
    m_combo(0),
    m_comboTimer(kComboGraceFrame),
    m_totalScore(0),
    m_bodyKillCount(0),
    m_headKillCount(0),
    m_lastComboRate(1.0f)
{
    LoadScores(); // 初期化時にスコアを読み込み
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

void ScoreManager::SaveScore(int score)
{
    // スコアをリストに追加
    m_highScores.push_back(score);
    // 降順にソート
    std::sort(m_highScores.begin(), m_highScores.end(), std::greater<int>());
    // 最大数を超えた場合は削除
    if (m_highScores.size() > kMaxHighScores)
    {
        m_highScores.resize(kMaxHighScores);
    }
        
    // ファイルに保存
    std::ofstream file(kScoreFileName);
    if (file.is_open())
    {
        for (int s : m_highScores)
        {
            file << s << std::endl;
        }
        file.close();
    }
}

void ScoreManager::LoadScores()
{
    m_highScores.clear();
    std::ifstream file(kScoreFileName);
    if (file.is_open())
    {
        int score;
        while (file >> score)
        {
            m_highScores.push_back(score);
        }
        file.close();
        // 降順にソート
        std::sort(m_highScores.begin(), m_highScores.end(), std::greater<int>());
    }
}
