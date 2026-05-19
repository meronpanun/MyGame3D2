#pragma once
#include <vector>

/// <summary>
/// スコア・コンボ・キル数・ハイスコアを一元管理するシングルトンクラス。
/// カウントアップ演出用の表示値とその目標値も保持する。
/// </summary>
class ScoreManager
{
public:
    // コンボ継続の猶予フレーム数
    static constexpr int kComboGraceFrame = 240;

    /// <summary>
    /// コンストラクタ。ハイスコアをファイルから読み込む。
    /// </summary>
    ScoreManager();

    /// <summary>
    /// 毎フレームの更新処理。コンボタイマーの減算とカウントアップ演出を行う。
    /// </summary>
    void Update();

    /// <summary>
    /// シングルトンインスタンスを返す
    /// </summary>
    /// <returns>ScoreManager インスタンスへの参照</returns>
    static ScoreManager& Instance();

    /// <summary>
    /// 敵撃破時にスコアを加算する。コンボ倍率を乗算し、タイマーをリセットする。
    /// </summary>
    /// <param name="isHeadShot">ヘッドショットなら true</param>
    /// <returns>今回加算されたスコア</returns>
    int AddScore(bool isHeadShot);

    /// <summary>
    /// 現在のコンボ数をリセットする
    /// </summary>
    void ResetCombo();

    /// <summary>
    /// 現在のスコアを返す
    /// </summary>
    /// <returns>現在のスコア</returns>
    int GetScore() const;

    /// <summary>
    /// ゲーム全体の累計スコアを返す
    /// </summary>
    /// <returns>累計スコア</returns>
    int GetTotalScore() const { return m_totalScore; }

    /// <summary>
    /// 現在のコンボ数を返す
    /// </summary>
    /// <returns>現在のコンボ数</returns>
    int GetCombo() const;

    /// <summary>
    /// ボディショットキル数を返す
    /// </summary>
    /// <returns>ボディショットキル数</returns>
    int GetBodyKillCount() const { return m_bodyKillCount; }

    /// <summary>
    /// ヘッドショットキル数を返す
    /// </summary>
    /// <returns>ヘッドショットキル数</returns>
    int GetHeadKillCount() const { return m_headKillCount; }

    /// <summary>
    /// 総撃破数（ボディ＋ヘッド）を返す
    /// </summary>
    /// <returns>総撃破数</returns>
    int GetTotalDefeatedCount() const { return m_bodyKillCount + m_headKillCount; }

    /// <summary>
    /// 最後のコンボ倍率を返す
    /// </summary>
    /// <returns>最後のコンボ倍率</returns>
    float GetLastComboRate() const { return m_lastComboRate; }

    /// <summary>
    /// スコアをハイスコアリストに追加してファイルへ保存する
    /// </summary>
    /// <param name="score">保存するスコア</param>
    void SaveScore(int score);

    /// <summary>
    /// ハイスコアをファイルから読み込む
    /// </summary>
    void LoadScores();

    /// <summary>
    /// ハイスコアのリストを返す（降順ソート済み）
    /// </summary>
    /// <returns>ハイスコアのリスト</returns>
    const std::vector<int>& GetHighScores() const { return m_highScores; }

    /// <summary>
    /// 最高スコアを返す
    /// </summary>
    /// <returns>最高スコア。記録がない場合は 0</returns>
    int GetHighestScore() const { return m_highScores.empty() ? 0 : m_highScores[0]; }

    /// <summary>
    /// カウントアップ演出用の表示スコアを返す
    /// </summary>
    /// <returns>表示用スコア</returns>
    int GetDisplayScore() const { return m_displayScore; }

    /// <summary>
    /// 表示用スコアをリセットし、目標値を現在スコアに合わせる
    /// </summary>
    void ResetDisplayScore() { m_displayScore = 0; m_targetDisplayScore = m_score; }

    /// <summary>
    /// 表示用スコアの目標値を設定する
    /// </summary>
    /// <param name="score">目標スコア</param>
    void SetTargetDisplayScore(int score) { m_targetDisplayScore = score; }

    /// <summary>
    /// カウントアップ速度を設定する
    /// </summary>
    /// <param name="speed">1 フレームあたりの最低加算量</param>
    void SetScoreCountUpSpeed(int speed) { m_scoreCountUpSpeed = speed; }

    /// <summary>
    /// カウントアップ演出用の表示累計スコアを返す
    /// </summary>
    /// <returns>表示用累計スコア</returns>
    int GetDisplayTotalScore() const { return m_displayTotalScore; }

    /// <summary>
    /// 表示用スコアおよび累計スコアを 0 にリセットする
    /// </summary>
    void ResetDisplayValues();

    /// <summary>
    /// カウントアップ演出用の目標値をまとめて設定する
    /// </summary>
    /// <param name="score">目標スコア</param>
    /// <param name="totalScore">目標累計スコア</param>
    /// <param name="bodyKill">目標ボディショットキル数</param>
    /// <param name="headKill">目標ヘッドショットキル数</param>
    void SetTargetDisplayValues(int score, int totalScore, int bodyKill, int headKill);

    /// <summary>
    /// 最大コンボ数を返す
    /// </summary>
    /// <returns>最大コンボ数</returns>
    int GetMaxCombo() const { return m_maxCombo; }

    /// <summary>
    /// スコア・キル数・コンボなど全状態をリセットする
    /// </summary>
    void ResetAll();

    /// <summary>
    /// コンボタイマーの現在値を返す
    /// </summary>
    /// <returns>コンボ継続の残りフレーム数</returns>
    int GetComboTimer() const { return m_comboTimer; }

private:
    std::vector<int> m_highScores;        // ハイスコアリスト（降順ソート済み）

    // スコア関連
    int   m_score;                        // 現在のスコア（コンボが切れるとリセット）
    int   m_totalScore;                   // ゲーム全体の累計スコア

    // コンボ関連
    int   m_combo;                        // 現在のコンボ数
    int   m_maxCombo;                     // セッション中の最大コンボ数
    int   m_comboTimer;                   // コンボ継続猶予タイマー（フレーム）
    float m_lastComboRate;                // 最後のコンボ倍率

    // キル数関連
    int   m_bodyKillCount;                // ボディショットキル数
    int   m_headKillCount;                // ヘッドショットキル数

    // カウントアップ演出用
    int   m_displayScore;                 // 表示用スコア
    int   m_targetDisplayScore;           // 表示用スコアの目標値
    int   m_displayTotalScore;            // 表示用累計スコア
    int   m_targetTotalScore;             // 表示用累計スコアの目標値
    int   m_targetBodyKillCount;          // 表示用ボディショットキル数の目標値
    int   m_targetHeadKillCount;          // 表示用ヘッドショットキル数の目標値
    int   m_scoreCountUpSpeed;            // カウントアップの最低加算量（フレーム毎）
};
