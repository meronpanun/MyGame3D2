#pragma once
#include "UIBase.h"

class Player;

/// <summary>
/// プレイヤーのエフェクト（画面エフェクト等）を描画するUIクラス
/// </summary>
class PlayerEffectUI : public UIBase
{
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    /// <param name="player">プレイヤーのポインタ</param>
    PlayerEffectUI(Player* player);

    virtual ~PlayerEffectUI() = default;

    /// <summary>
    /// 初期化処理
    /// </summary>
    void Init() override;

    /// <summary>
    /// 毎フレームの更新処理
    /// </summary>
    /// <param name="deltaTime">前フレームからの経過時間（秒）</param>
    void Update(float deltaTime) override;

    /// <summary>
    /// 描画処理（PlayerEffectManagerの描画を委譲する）
    /// </summary>
    void Draw() override;

private:
    Player* m_pPlayer; // エフェクト情報を取得するプレイヤーのポインタ
};
