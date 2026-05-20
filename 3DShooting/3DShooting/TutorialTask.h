#pragma once
#include "DxLib.h"
#include <string>

class Player;
class WaveManager;
class TaskTutorialManager;

/// <summary>
/// チュートリアル個別タスクの純粋仮想基底インターフェース。
/// 各タスクは Start → Update → DrawTaskUI のライフサイクルで動作し、
/// IsCompleted() が true を返すと TaskTutorialManager が次ステップへ進む。
/// </summary>
class ITutorialTask
{
public:
    virtual ~ITutorialTask() = default;

    /// <summary>
    /// タスク開始時に一度だけ呼ばれる初期化処理
    /// </summary>
    /// <param name="pWaveManager">ウェーブ制御オブジェクト（敵のスポーン指示に使用）</param>
    /// <param name="pPlayer">プレイヤーオブジェクト（攻撃制限の設定に使用）</param>
    virtual void Start(WaveManager* pWaveManager, Player* pPlayer) = 0;

    /// <summary>
    /// 毎フレームの更新処理
    /// </summary>
    virtual void Update() = 0;

    /// <summary>
    /// タスク固有の UI を描画する
    /// </summary>
    /// <param name="x">描画開始 X 座標</param>
    /// <param name="y">描画開始 Y 座標</param>
    /// <param name="scale">UI スケール係数</param>
    /// <param name="alpha">描画アルファ値（0〜255）</param>
    /// <param name="pManager">画像・フォントハンドルの取得元</param>
    virtual void DrawTaskUI(int x, int y, float scale, int alpha,
                            const TaskTutorialManager* pManager) = 0;

    /// <summary>
    /// タスクが完了しているかを返す
    /// </summary>
    /// <returns>完了なら true</returns>
    virtual bool IsCompleted() const = 0;

    /// <summary>
    /// UI に表示するタスクタイトル文字列を返す
    /// </summary>
    /// <returns>タイトル文字列</returns>
    virtual std::string GetTitle() const = 0;

    /// <summary>
    /// タスクの進捗率を返す（0.0 = 未達成、1.0 = 完了）
    /// </summary>
    /// <returns>進捗率（0.0〜1.0）</returns>
    virtual float GetProgress() const = 0;

    /// <summary>
    /// 進捗バーの隣に表示する進捗テキストを返す（例: "3/5"）
    /// </summary>
    /// <returns>進捗テキスト</returns>
    virtual std::string GetProgressText() const = 0;

    /// <summary>
    /// 敵が倒されたことを通知する（デフォルト実装は何もしない）
    /// </summary>
    /// <param name="attackType">倒した攻撃種別（AttackType を int にキャストした値）</param>
    virtual void NotifyEnemyKilled(int attackType) {}

    /// <summary>
    /// 盾投げで敵が倒されたことを通知する（デフォルト実装は何もしない）
    /// </summary>
    virtual void NotifyShieldThrowKill() {}

    /// <summary>
    /// パリィ成功を通知する（デフォルト実装は何もしない）
    /// </summary>
    virtual void NotifyParrySuccess() {}
};
