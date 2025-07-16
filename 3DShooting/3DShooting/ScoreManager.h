#pragma once

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

    // 現在のコンボ数取得
    int GetCombo() const;

    // 毎フレーム呼び出すことでコンボ猶予を管理
    void Update();

    int GetBodyKillCount() const { return m_bodyKillCount; }
    int GetHeadKillCount() const { return m_headKillCount; }
    float GetLastComboRate() const { return m_lastComboRate; }

private:
    ScoreManager();
    int m_score = 0;
    int m_combo = 0;
    int m_comboTimer = 0; // コンボ継続猶予タイマー（フレーム単位）
    static constexpr int kComboGraceFrame = 90; // 1.5秒（60fps換算）
    int m_bodyKillCount = 0;
    int m_headKillCount = 0;
    float m_lastComboRate = 1.0f;
};

