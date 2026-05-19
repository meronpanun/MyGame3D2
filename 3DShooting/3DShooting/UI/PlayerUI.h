#pragma once
#include "UIBase.h"
#include "PlayerWeaponManager.h"
#include "PlayerShieldSystem.h"
#include "ManagedFont.h"
#include "ManagedGraph.h"

class EnemyBase;
class Player;

/// <summary>
/// プレイヤーのHP・武器・盾・警告・ロックオンなどのUIを描画するクラス
/// </summary>
class PlayerUI : public UIBase
{
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    /// <param name="player">プレイヤーのポインタ</param>
    PlayerUI(Player* player);

    virtual ~PlayerUI() = default;

    /// <summary>
    /// 初期化処理（フォントのロードを行う）
    /// </summary>
    void Init() override;

    /// <summary>
    /// 毎フレームの更新処理（UIスケール変更の検知を行う）
    /// </summary>
    /// <param name="deltaTime">前フレームからの経過時間（秒）</param>
    void Update(float deltaTime) override;

    /// <summary>
    /// 描画処理（各UIパーツの描画を委譲する）
    /// </summary>
    void Draw() override;

private:
    /// <summary>
    /// HPバーを描画する
    /// </summary>
    /// <param name="baseAlpha">基準の不透明度（0.0〜1.0）</param>
    void DrawHPBar(float baseAlpha);

    /// <summary>
    /// 武器UIを描画する（銃画像・弾薬数テキスト）
    /// </summary>
    /// <param name="baseAlpha">基準の不透明度（0.0〜1.0）</param>
    void DrawWeaponUI(float baseAlpha);

    /// <summary>
    /// 盾UIを描画する（耐久度ゲージ）
    /// </summary>
    /// <param name="baseAlpha">基準の不透明度（0.0〜1.0）</param>
    void DrawShieldUI(float baseAlpha);

    /// <summary>
    /// 警告UIを描画する（体力低下・弾薬警告）
    /// </summary>
    /// <param name="baseAlpha">基準の不透明度（0.0〜1.0）</param>
    void DrawWarningUI(float baseAlpha);

    /// <summary>
    /// ロックオンUIを描画する（ロック中の敵上にリングを表示）
    /// </summary>
    /// <param name="baseAlpha">基準の不透明度（0.0〜1.0）</param>
    void DrawLockOnUI(float baseAlpha);

    /// <summary>
    /// ガード中テキストを描画する（ロック対象がいない時に「ターゲットなし」を表示）
    /// </summary>
    /// <param name="baseAlpha">基準の不透明度（0.0〜1.0）</param>
    void DrawGuardText(float baseAlpha);

    /// <summary>
    /// UIスケールに合わせてフォントを再生成する
    /// </summary>
    /// <param name="scale">UIスケール値</param>
    void ReloadFonts(float scale);

    /// <summary>
    /// グラデーションボックスを描画する
    /// </summary>
    void DrawGradientBox(int x1, int y1, int x2, int y2, unsigned int topColor, unsigned int bottomColor);

private:
    Player* m_pPlayer; // UIの情報元となるプレイヤーのポインタ

    // UI画像ハンドル
    ManagedGraph m_noAmmoImage;    // 弾薬切れ警告画像
    ManagedGraph m_noHealthImage;  // 体力低下警告画像
    ManagedGraph m_arImage;        // アサルトライフル画像
    ManagedGraph m_noAmmoARImage;  // アサルトライフル弾切れ画像
    ManagedGraph m_sgImage;        // ショットガン画像
    ManagedGraph m_noAmmoSGImage;  // ショットガン弾切れ画像
    ManagedGraph m_healthUiImage;  // HP UIアイコン画像
    ManagedGraph m_shieldImage;    // 盾ゲージ画像
    ManagedGraph m_lockOnUI;       // ロックオンリング画像

    // フォントハンドル
    ManagedFont m_font;        // 弾薬数テキスト用フォント
    ManagedFont m_hpFont;      // HP数値テキスト用フォント
    ManagedFont m_warningFont; // 警告テキスト用フォント

    float m_prevScale; // スケール変更検知用の前フレームUIスケール値
};
