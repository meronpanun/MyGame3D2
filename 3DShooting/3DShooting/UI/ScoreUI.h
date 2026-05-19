#pragma once
#include "UIBase.h"
#include <vector>
#include <string>
#include "DxLib.h"

/// <summary>
/// スコアポップアップ1件分の情報を保持する構造体
/// </summary>
struct ScorePopup
{
    int   value;      // 表示するスコア値
    bool  isHeadShot; // ヘッドショットかどうか
    float timer;      // 残り表示時間（0 以下で削除）
    float maxTimer;   // 表示時間の最大値
};

/// <summary>
/// スコアのポップアップとコンボUIを描画するクラス。
/// 個別スコアポップアップ・合計スコア・コンボ数・コンボゲージを表示する。
/// </summary>
class ScoreUI : public UIBase
{
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    ScoreUI();

    virtual ~ScoreUI();

    /// <summary>
    /// 初期化処理（フォント生成・スコアマネージャーリセット）
    /// </summary>
    void Init() override;

    /// <summary>
    /// 毎フレームの更新処理（ポップアップタイマー・コンボアニメーション更新）
    /// </summary>
    /// <param name="deltaTime">前フレームからの経過時間（秒）</param>
    void Update(float deltaTime) override;

    /// <summary>
    /// 描画処理（スコアポップアップとコンボUIを描画する）
    /// </summary>
    void Draw() override;

    /// <summary>
    /// スコアポップアップを追加する
    /// </summary>
    /// <param name="value">表示するスコア値</param>
    /// <param name="isHeadShot">ヘッドショットかどうか</param>
    void AddScorePopup(int value, bool isHeadShot);

private:
    std::vector<ScorePopup> m_popups;            // 個別スコアポップアップのリスト
    int   m_totalScorePopupValue;                // 合計スコアポップアップの累計値
    float m_totalScorePopupTimer;                // 合計スコアポップアップの残り表示時間
    int   m_scoreFont;                           // スコアテキスト用フォントハンドル

    int   m_prevCombo;                           // 前フレームのコンボ数（変化検知用）
    float m_comboPulseScale;                     // コンボテキストのパルスアニメーションスケール
    float m_comboAnimTimer;                      // コンボ増加演出の残りタイマー
};
