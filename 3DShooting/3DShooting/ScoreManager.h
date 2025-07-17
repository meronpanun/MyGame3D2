#pragma once
#include <vector>

/// <summary>
/// スコア管理クラス
/// </summary>
class ScoreManager
{
public:
    // シングルトンインスタンス取得
    static ScoreManager& Instance();

    // スコア加算（isHeadShot: ヘッドショットならtrue）。加算点を返す
    int AddScore(bool isHeadShot);

    // コンボリセット
    void ResetCombo();

    // スコア取得
    int GetScore() const;

    // ゲーム全体の累計スコア取得
    int GetTotalScore() const { return m_totalScore; }

    // 現在のコンボ数取得
    int GetCombo() const;

    // 毎フレーム呼び出すことでコンボ猶予を管理
    void Update();

    int GetBodyKillCount() const { return m_bodyKillCount; }
    int GetHeadKillCount() const { return m_headKillCount; }
    float GetLastComboRate() const { return m_lastComboRate; }

    // スコア保存・読み込み機能
    void SaveScore(int score);
    void LoadScores();
    const std::vector<int>& GetHighScores() const { return m_highScores; }
    int GetHighestScore() const { return m_highScores.empty() ? 0 : m_highScores[0]; }

private:
    ScoreManager();
    int m_score;
    int m_totalScore; // ゲーム全体の累計スコア
    int m_combo;
    int m_comboTimer; // コンボ継続猶予タイマー（フレーム単位）
    int m_bodyKillCount;
    int m_headKillCount;
    float m_lastComboRate;
    std::vector<int> m_highScores;
};

