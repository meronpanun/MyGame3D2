#pragma once
#include "UIBase.h"
#include "DxLib.h"
#include "EnemyBase.h"

/// <summary>
/// 敵弾着弾時にクロスのヒットマークを表示するUIクラス。
/// ヘッドショット時は金色の二重線、ボディショット時はオレンジ色の単線で描画する
/// </summary>
class HitMarkUI : public UIBase
{
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    HitMarkUI();

    virtual ~HitMarkUI() = default;

    /// <summary>
    /// 初期化処理
    /// </summary>
    void Init() override;

    /// <summary>
    /// 毎フレームの更新処理（表示タイマーを減算する）
    /// </summary>
    /// <param name="deltaTime">前フレームからの経過時間（秒）</param>
    void Update(float deltaTime) override;

    /// <summary>
    /// 描画処理（タイマーが残っている間、画面中央にヒットマークを描画する）
    /// </summary>
    void Draw() override;

    /// <summary>
    /// ヒットマークの表示をトリガーする
    /// </summary>
    /// <param name="type">ヒットした部位（Head/Body）</param>
    /// <param name="distance">敵との距離（遠いほど透明度が上がる）</param>
    void Trigger(EnemyBase::HitPart type, float distance);

private:
    float m_timer;              // 残り表示時間（0以下で非表示）
    float m_distance;           // 最後にヒットした敵との距離
    EnemyBase::HitPart m_type; // 最後にヒットした部位
};
