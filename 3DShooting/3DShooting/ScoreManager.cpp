#include "ScoreManager.h"
#include "TaskTutorialManager.h"
#include <algorithm>
#include <cmath>
#include <fstream>

namespace
{
    constexpr int         kBaseHeadShotScore  = 200;             // ヘッドショットの基本スコア
    constexpr int         kBaseBodyShotScore  = 100;             // ボディショットの基本スコア
    constexpr float       kInitialComboRate   = 1.1f;            // コンボ倍率の初期値
    constexpr int         kMaxHighScores      = 10;              // 保存するハイスコアの最大件数
    constexpr const char* kScoreFileName      = "highscores.txt"; // スコア保存ファイル名
    constexpr int         kMinCountUpSpeed    = 50;              // カウントアップの最低速度（フレーム毎）
    constexpr float       kCountUpRatio       = 0.05f;           // 残り差分に対するカウントアップ割合
    constexpr int         kDefaultCountUpSpeed = 30;             // カウントアップ速度の初期値
}

ScoreManager& ScoreManager::Instance()
{
    static ScoreManager instance;
    return instance;
}

ScoreManager::ScoreManager()
    : m_score(0)
    , m_combo(0)
    , m_maxCombo(0)
    , m_comboTimer(kComboGraceFrame)
    , m_totalScore(0)
    , m_bodyKillCount(0)
    , m_headKillCount(0)
    , m_lastComboRate(1.0f)
    , m_displayScore(0)
    , m_targetDisplayScore(0)
    , m_displayTotalScore(0)
    , m_targetTotalScore(0)
    , m_targetBodyKillCount(0)
    , m_targetHeadKillCount(0)
    , m_scoreCountUpSpeed(kDefaultCountUpSpeed)
{
    LoadScores();
}

int ScoreManager::AddScore(bool isHeadShot)
{
    int baseScore = isHeadShot ? kBaseHeadShotScore : kBaseBodyShotScore;
    m_combo++;
    if (m_combo > m_maxCombo) m_maxCombo = m_combo; // 最大コンボ更新

    // コンボ倍率: pow(kInitialComboRate, combo - 1) で指数的に増える。
    // combo=1 のとき指数が 0 になって倍率 1.0 になるので、1コンボ目はそのままのスコアになる。
    // 線形より指数にすることで、高コンボほど爆発的にスコアが伸びる仕組みにしている。
    float comboRate  = static_cast<float>(std::pow(kInitialComboRate, m_combo - 1));
    m_lastComboRate  = comboRate;
    int add          = static_cast<int>(baseScore * comboRate);

    // m_score はコンボが切れるとリセットされる「コンボ中スコア」
    // m_totalScore はリセットされない「ゲーム全体の累計スコア」
    m_score         += add;
    m_totalScore    += add;
    m_comboTimer     = kComboGraceFrame; // コンボ猶予タイマーをリセット

    if (TaskTutorialManager::GetInstance() && TaskTutorialManager::GetInstance()->IsCompleted())
    {
        if (isHeadShot)
        {
            m_headKillCount++;
        }
        else
        {
            m_bodyKillCount++;
        }
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
            m_combo         = 0;
            m_lastComboRate = 1.0f;
            // コンボが切れたら「コンボ中スコア（m_score）」だけリセットする。
            // 「累計スコア（m_totalScore）」は残しておくので、
            // リザルト画面で「トータルで入れたスコア」をそのまま使える。
            m_score         = 0;
        }
    }

    // スコアカウントアップ演出
    // 差分の一定割合（最低 kMinCountUpSpeed）ずつ増やすことで、
    // スコアが大きくても短時間でアニメーションが完了する
    if (m_displayScore < m_targetDisplayScore)
    {
        int diff = m_targetDisplayScore - m_displayScore;
        int add  = (std::max)(kMinCountUpSpeed, static_cast<int>(diff * kCountUpRatio));
        add      = (std::min)(add, diff); // 目標値を超えないよう上限設定
        m_displayScore += add;
    }
    if (m_displayTotalScore < m_targetTotalScore)
    {
        int diff = m_targetTotalScore - m_displayTotalScore;
        int add  = (std::max)(kMinCountUpSpeed, static_cast<int>(diff * kCountUpRatio));
        add      = (std::min)(add, diff); // 目標値を超えないよう上限設定
        m_displayTotalScore += add;
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
    m_highScores.push_back(score);
    std::sort(m_highScores.begin(), m_highScores.end(), std::greater<int>());

    if (static_cast<int>(m_highScores.size()) > kMaxHighScores)
    {
        m_highScores.resize(kMaxHighScores);
    }

    std::ofstream file(kScoreFileName);
    if (file.is_open())
    {
        for (int s : m_highScores)
        {
            file << s << std::endl;
        }
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
        std::sort(m_highScores.begin(), m_highScores.end(), std::greater<int>());
    }
}

void ScoreManager::ResetDisplayValues()
{
    m_displayScore      = 0;
    m_displayTotalScore = 0;
}

void ScoreManager::SetTargetDisplayValues(int score, int totalScore, int bodyKill, int headKill)
{
    m_targetDisplayScore  = score;
    m_targetTotalScore    = totalScore;
    m_targetBodyKillCount = bodyKill;
    m_targetHeadKillCount = headKill;
}

void ScoreManager::ResetAll()
{
    // ゲーム開始時にスコア・コンボ・キルカウント・表示値をリセットする。
    // m_highScores はリセットしない（セッションをまたいで保持するランキングデータなので）
    m_score               = 0;
    m_totalScore          = 0;
    m_combo               = 0;
    m_maxCombo            = 0;
    m_comboTimer          = 0;
    m_lastComboRate       = 1.0f;
    m_bodyKillCount       = 0;
    m_headKillCount       = 0;
    m_displayScore        = 0;
    m_targetDisplayScore  = 0;
    m_displayTotalScore   = 0;
    m_targetTotalScore    = 0;
    m_targetBodyKillCount = 0;
    m_targetHeadKillCount = 0;
}
