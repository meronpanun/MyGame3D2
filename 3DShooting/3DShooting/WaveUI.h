#pragma once
#include "DxLib.h"
#include "SpawnAreaInfo.h"
#include <string>
#include <vector>

/// <summary>
/// WaveUIクラス
/// ウェーブ関連のUI描画およびデバッグ表示を担当
/// </summary>
class WaveUI
{
public:
    WaveUI();
    ~WaveUI();

    /// <summary>
    /// 初期化
    /// </summary>
    void Init();

    /// <summary>
    /// 更新処理（アニメーション用）
    /// </summary>
    void Update();

    /// <summary>
    /// ウェーブUIを描画
    /// </summary>
    /// <param name="currentWave">現在のウェーブ</param>
    /// <param name="isWaveActive">ウェーブが進行中か</param>
    /// <param name="isAllWavesCompleted">全ウェーブ完了したか</param>
    void DrawWaveUI(int currentWave, bool isWaveActive, bool isAllWavesCompleted);

    /// <summary>
    /// デバッグ情報を描画
    /// </summary>
    /// <param name="currentWave">現在のウェーブ</param>
    /// <param name="aliveEnemyCount">生存敵数</param>
    /// <param name="totalSpawnedCount">累計出現数</param>
    /// <param name="waveTimer">ウェーブタイマー</param>
    /// <param name="spawnTimer">スポーンタイマー</param>
    /// <param name="nextSpawnTime">次のスポーン時間</param>
    /// <param name="remainingEnemiesInWave">ウェーブ内の残り敵数</param>


    /// <summary>
    /// スポーンエリアのデバッグ表示
    /// </summary>
    /// <param name="spawnAreaList">スポーンエリアリスト</param>
    /// <param name="isTutorial">チュートリアルかどうか</param>
    void DrawDebugSpawnAreas(const std::vector<SpawnAreaInfo>& spawnAreaList, bool isTutorial);

    /// <summary>
    /// アニメーションの開始
    /// </summary>
    void StartWaveAnimation();

    /// <summary>
    /// アニメーション中かどうか
    /// </summary>
    bool IsAnimating() const { return m_isWaveImageAnimating; }

private:
    // ウェーブ画像アニメーション関連
    int m_waveImages[5];                    // 1-5ウェーブ用画像ハンドル
    int m_waveImageAnimTimer;               // ウェーブ画像アニメーションタイマー
    int m_waveImageAnimDuration;            // ウェーブ画像アニメーションの総時間
    int m_waveImageAnimHoldDuration;        // ウェーブ画像アニメーションのホールド時間
    int m_waveImageAnimInitialHoldDuration; // ウェーブ画像アニメーションの初期ホールド時間
    bool m_isWaveImageAnimating;            // ウェーブ画像アニメーション中フラグ
};
