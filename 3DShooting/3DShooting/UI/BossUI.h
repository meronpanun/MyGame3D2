#pragma once
#include "UIBase.h"
#include "DxLib.h"
#include "ManagedFont.h"
#include <vector>
#include <memory>

class EnemyBase;
class WaveManager;

/// <summary>
/// ボスのHPバーと名前を画面上部に表示するUIクラス
/// </summary>
class BossUI : public UIBase
{
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    /// <param name="waveManager">WaveManagerのポインタ（ボス情報の取得に使用）</param>
    BossUI(WaveManager* waveManager);

    virtual ~BossUI() = default;

    /// <summary>
    /// 初期化処理（フォントのロードを行う）
    /// </summary>
    void Init() override;

    /// <summary>
    /// 毎フレームの更新処理（スケール変更の検知を行う）
    /// </summary>
    /// <param name="deltaTime">前フレームからの経過時間（秒）</param>
    void Update(float deltaTime) override;

    /// <summary>
    /// 描画処理（ボスが存在する場合にHPバーと名前を描画する）
    /// </summary>
    void Draw() override;

private:
    /// <summary>
    /// ボスHPバーを描画する
    /// </summary>
    /// <param name="hp">現在の体力</param>
    /// <param name="maxHp">最大体力</param>
    void DrawBossHPBar(float hp, float maxHp);

    /// <summary>
    /// UIスケールに合わせてフォントを再生成する
    /// </summary>
    /// <param name="scale">UIスケール値</param>
    void ReloadFonts(float scale);

private:
    WaveManager* m_pWaveManager; // ボス情報を取得するWaveManagerのポインタ
    float m_healthBarAnim;       // HPバーの滑らかな追従アニメーション用体力値
    ManagedFont m_font;          // ボス名・HPテキスト用フォント
    float m_prevScale;           // スケール変更検知用の前フレームUIスケール値
};
