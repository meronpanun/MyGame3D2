#pragma once
#include "UIBase.h"
#include "DxLib.h"
#include "SpawnAreaInfo.h"
#include <string>
#include <vector>

class WaveManager;

/// <summary>
/// ウェーブ関連の UI 描画を担当するクラス。
/// ウェーブ開始時のアニメーション演出と、デバッグ用スポーンエリア描画を提供する。
/// </summary>
class WaveUI : public UIBase
{
public:
    /// <summary>参照する WaveManager を受け取って初期化する</summary>
    /// <param name="waveManager">ウェーブ制御オブジェクトへのポインタ</param>
    WaveUI(WaveManager* waveManager);

    /// <summary>ウェーブ画像ハンドルを解放する</summary>
    virtual ~WaveUI();

    /// <summary>ウェーブ画像を読み込む</summary>
    void Init() override;

    /// <summary>WaveManager の状態を監視してアニメーションを制御する</summary>
    /// <param name="deltaTime">前フレームからの経過時間（秒）</param>
    void Update(float deltaTime) override;

    /// <summary>ウェーブ番号画像をアニメーションつきで描画する</summary>
    void Draw() override;

    /// <summary>スポーンエリアの AABB をワイヤーフレームでデバッグ描画する</summary>
    /// <param name="spawnAreaList">描画対象のスポーンエリアリスト</param>
    /// <param name="isTutorial">true のときチュートリアルエリアのみ描画する</param>
    void DrawDebugSpawnAreas(const std::vector<SpawnAreaInfo>& spawnAreaList, bool isTutorial);

    /// <summary>ウェーブ開始アニメーションを開始する</summary>
    void StartWaveAnimation();

    /// <summary>ウェーブ開始アニメーションが再生中かどうかを返す</summary>
    bool IsAnimating() const { return m_isWaveImageAnimating; }

private:
    WaveManager* m_pWaveManager; // 参照する WaveManager

    int  m_waveImages[5];                    // ウェーブ 1〜5 の画像ハンドル
    int  m_waveImageAnimTimer;               // アニメーション経過フレーム数
    int  m_waveImageAnimDuration;            // 中央→上部へのスライド時間（フレーム）
    int  m_waveImageAnimHoldDuration;        // スライド後のホールド時間（フレーム）
    int  m_waveImageAnimInitialHoldDuration; // スライド前の初期ホールド時間（フレーム）
    bool m_isWaveImageAnimating;             // アニメーション再生中フラグ
};
