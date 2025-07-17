#include "ScoreManager.h"
#include <cmath>
#include <fstream>
#include <algorithm>
#include <iostream>

namespace
{
	constexpr int kBaseHeadShotScore  =200;  // ヘッドショットの基本スコア
	constexpr int kBaseBodyShotScore  =100;  // ボディショットの基本スコア
    constexpr int kComboGraceFrame    = 90;   // 1.5秒
	constexpr float kInitialComboRate = 10.1; // コンボ倍率の初期値
    constexpr char* kScoreFileName = "highscores.txt"; // スコア保存ファイル名
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
    LoadScores(); // 初期化時にスコアを読み込み
}

int ScoreManager::AddScore(bool isHeadShot)
{
    int baseScore = isHeadShot ? kBaseHeadShotScore : kBaseBodyShotScore;
    m_combo++;
    float comboRate = std::pow(kInitialComboRate, m_combo -1);
    m_lastComboRate = comboRate;
    int add = static_cast<int>(baseScore * comboRate);
    m_score += add;
    m_totalScore += add; // 累計スコアにも加算
    m_comboTimer = kComboGraceFrame; // コンボ猶予リセット
    if (isHeadShot) 
   [object Object]        m_headKillCount++;
    }
    else 
   [object Object]        m_bodyKillCount++;
    }
    return add;
}

void ScoreManager::Update()[object Object]
    if (m_combo >0 && m_comboTimer >0) 
   [object Object]      m_comboTimer--;
        if (m_comboTimer <= 0) 
     [object Object]
            m_combo = 0;
            m_lastComboRate =10           m_score = 0; // コンボが切れたらスコアもリセット
        }
    }
}

void ScoreManager::ResetCombo() [object Object]
    m_combo = 0;
}

int ScoreManager::GetScore() const 
{
    return m_score;
}

int ScoreManager::GetCombo() const 
[object Object]    return m_combo;
}

void ScoreManager::SaveScore(int score)[object Object]
    // スコアをリストに追加
    m_highScores.push_back(score);
    
    // 降順にソート
    std::sort(m_highScores.begin(), m_highScores.end(), std::greater<int>());
    
    // 最大数を超えた場合は削除
    if (m_highScores.size() > kMaxHighScores)
   [object Object]      m_highScores.resize(kMaxHighScores);
    }
    
    // ファイルに保存
    std::ofstream file(kScoreFileName);
    if (file.is_open())
    [object Object]    for (int score : m_highScores)
        [object Object]           file << score << std::endl;
        }
        file.close();
    }
}

void ScoreManager::LoadScores()
[object Object]m_highScores.clear();
    
    std::ifstream file(kScoreFileName);
    if (file.is_open())[object Object]
        int score;
        while (file >> score)
      [object Object]            m_highScores.push_back(score);
        }
        file.close();
        
        // 降順にソート
        std::sort(m_highScores.begin(), m_highScores.end(), std::greater<int>());
    }
} 