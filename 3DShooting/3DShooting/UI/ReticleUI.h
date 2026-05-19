#pragma once
#include "UIBase.h"
#include "DxLib.h"
#include "ManagedGraph.h"

class Player;

/// <summary>
/// 画面中央に照準（レティクル・ドット）を描画するUIクラス。
/// ショットガン使用時はレティクル線、全武器共通でドットを描画する。
/// 狙い中は赤みがかった加算ブレンドで描画される。
/// </summary>
class ReticleUI : public UIBase
{
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    /// <param name="player">プレイヤーのポインタ（武器状態・狙い状態の取得に使用）</param>
    ReticleUI(Player* player);

    virtual ~ReticleUI() = default;

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
    /// 描画処理（レティクルとドットを画面中央に描画する）
    /// </summary>
    void Draw() override;

private:
    Player*      m_pPlayer;         // 武器・狙い状態を取得するプレイヤーのポインタ
    ManagedGraph m_reticleDefault;  // ショットガン通常時レティクル画像
    ManagedGraph m_reticleOnTarget; // ショットガン狙い時レティクル画像
    ManagedGraph m_dotDefault;      // 通常時ドット画像
    ManagedGraph m_dotOnTarget;     // 狙い時ドット画像
};
