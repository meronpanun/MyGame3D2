#pragma once
#include <vector>

/// <summary>
/// スコア管理クラス
/// </summary>
class ScoreManager
{
public:
    ScoreManager();

    void Update();

    /// <summary>
    /// スコアマネージャーのインスタンスを取得
    /// </summary>
    /// <returns>スコアマネージャーのインスタンス</returns>
    static ScoreManager& Instance();

    /// <summary>
    /// スコアを加算する
    /// </summary>
    /// <param name="isHeadShot">ヘッドショットならtrue</param>
    /// <returns>加算されたスコア</returns>
    int AddScore(bool isHeadShot);

    /// <summary>
    /// コンボをリセットする
    /// </summary>
    void ResetCombo();

    /// <summary>
    /// 現在のスコアを取得
    /// </summary>
    /// <returns>現在のスコア</returns>
    int GetScore() const;

    /// <summary>
    /// ゲーム全体の累計スコアを取得
    /// </summary>
    /// <returns>累計スコア</returns>
    int GetTotalScore() const { return m_totalScore; }

    /// <summary>
    /// 現在のコンボ数を取得
    /// </summary>
    /// <returns>現在のコンボ数</returns>
    int GetCombo() const;

    /// <summary>
    /// ボディショットキル数を取得
    /// </summary>
    /// <returns>ボディショットキル数</returns>
    int GetBodyKillCount() const { return m_bodyKillCount; }

    /// <summary>
    /// ヘッドショットキル数を取得
    /// </summary>
    /// <returns>ヘッドショットキル数</returns>
    int GetHeadKillCount() const { return m_headKillCount; }

    /// <summary>
    /// 最後のコンボ倍率を取得
    /// </summary>
    /// <returns>最後のコンボ倍率</returns>
    float GetLastComboRate() const { return m_lastComboRate; }

    /// <summary>
    /// スコア保存・読み込み機能 
    /// </summary>
    /// <param name="score">保存するスコア</param>
    void SaveScore(int score);

    /// <summary>
    /// スコアを読み込む
    /// </summary>
    void LoadScores();

    /// <summary>
    /// ハイスコアのリストを取得
    /// </summary>
    /// <returns>ハイスコアのリスト</returns>
    const std::vector<int>& GetHighScores() const { return m_highScores; }

    /// <summary>
    /// 最高スコアを取得
    /// </summary>
    /// <returns>最高スコア</returns>
    int GetHighestScore() const { return m_highScores.empty() ? 0 : m_highScores[0]; }

    // 表示用スコアの取得
    int GetDisplayScore() const { return m_displayScore; }

    // 表示用スコアのリセット
    void ResetDisplayScore() { m_displayScore = 0; m_targetDisplayScore = m_score; }

    // 表示用スコアの目標値設定
    void SetTargetDisplayScore(int score) { m_targetDisplayScore = score; }

    // カウントアップ速度設定
    void SetScoreCountUpSpeed(int speed) { m_scoreCountUpSpeed = speed; }

    int GetDisplayTotalScore() const { return m_displayTotalScore; }

    void ResetDisplayValues();
    void SetTargetDisplayValues(int score, int totalScore, int bodyKill, int headKill);

private:
    std::vector<int> m_highScores;

    int m_score;         // 現在のスコア
    int m_totalScore;    // ゲーム全体の累計スコア
    int m_combo;         // 現在のコンボ数
    int m_comboTimer;    // コンボ継続猶予タイマー（フレーム単位）
    int m_bodyKillCount; // ボディショットキル数
    int m_headKillCount; // ヘッドショットキル数
    // カウントアップ演出用
    int m_displayScore;
    int m_targetDisplayScore;
    int m_displayTotalScore;
    int m_targetTotalScore;
    int m_targetBodyKillCount;
    int m_targetHeadKillCount;
    int m_scoreCountUpSpeed;

    float m_lastComboRate; // 最後のコンボ倍率
};

