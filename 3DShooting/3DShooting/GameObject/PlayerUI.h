#pragma once
#include "PlayerWeaponManager.h"
#include "PlayerShieldSystem.h"

class EnemyBase;

/// <summary>
/// プレイヤーのUI描画クラス
/// </summary>
class PlayerUI
{
public:
    PlayerUI();
    ~PlayerUI();

    /// <summary>
    /// UIの描画
    /// </summary>
    /// <param name="isDead">死亡しているかどうか</param>
    /// <param name="isGuarding">ガード中かどうか</param>
    /// <param name="lockedOnEnemy">ロックオンしている敵</param>
    /// <param name="isTargetAvailable">ロックオン可能な敵がいるか</param>
    /// <param name="health">現在の体力</param>
    /// <param name="healthBarAnim">HPバーアニメーション用体力値</param>
    /// <param name="maxHealth">最大体力</param>
    /// <param name="isLowHealth">体力が低いかどうか</param>
    /// <param name="lowHealthBlinkTimer">体力低下の点滅タイマー</param>
    /// <param name="ammoTextFlashTimer">弾薬テキストのフラッシュタイマー</param>
    /// <param name="weaponManager">武器マネージャーへの参照</param>
    /// <param name="shieldSystem">盾システムへの参照</param>
    void Draw(bool isDead, bool isGuarding, EnemyBase* lockedOnEnemy, bool isTargetAvailable,
        float health, float healthBarAnim, float maxHealth, bool isLowHealth, float lowHealthBlinkTimer, float ammoTextFlashTimer,
        const PlayerWeaponManager& weaponManager, const PlayerShieldSystem& shieldSystem);

private:
    /// <summary>
    /// HPバーの描画
    /// </summary>
    /// <param name="health">現在の体力</param>
    /// <param name="healthBarAnim">HPバーアニメーション用体力値</param>
    /// <param name="maxHealth">最大体力</param>
    void DrawHPBar(float health, float healthBarAnim, float maxHealth);

    /// <summary>
    /// 武器UIの描画
    /// </summary>
    /// <param name="weaponManager">武器マネージャーへの参照</param>
    /// <param name="ammoTextFlashTimer">弾薬テキストのフラッシュタイマー</param>
    void DrawWeaponUI(const PlayerWeaponManager& weaponManager, float ammoTextFlashTimer);

    /// <summary>
    /// 盾UIの描画
    /// </summary>
    /// <param name="shieldSystem">盾システムへの参照</param>
    void DrawShieldUI(const PlayerShieldSystem& shieldSystem);

    /// <summary>
    /// 警告UIの描画
    /// </summary>
    /// <param name="isLowHealth">体力が低いかどうか</param>
    /// <param name="lowHealthBlinkTimer">体力低下の点滅タイマー</param>
    /// <param name="weaponManager">武器マネージャーへの参照</param>
    void DrawWarningUI(bool isLowHealth, float lowHealthBlinkTimer, const PlayerWeaponManager& weaponManager);

    /// <summary>
    /// ロックオンUIの描画
    /// </summary>
    /// <param name="lockedOnEnemy">ロックオンしている敵</param>
    void DrawLockOnUI(EnemyBase* lockedOnEnemy);

    /// <summary>
    /// ガード中のテキスト表示
    /// </summary>
    /// <param name="isGuarding">ガード中かどうか</param>
    /// <param name="lockedOnEnemy">ロックオンしている敵</param>
    /// <param name="isTargetAvailable">ロックオン可能な敵がいるか</param>
    void DrawGuardText(bool isGuarding, EnemyBase* lockedOnEnemy, bool isTargetAvailable);

    /// <summary>
    /// フォントのリロード（スケール変更時）
    /// </summary>
    /// <param name="scale">UIスケール</param>
    void ReloadFonts(float scale);

private:
    // UI画像ハンドル
    int m_noAmmoImageHandle;
    int m_noHealthImageHandle;
    int m_arImageHandle;
    int m_noAmmoARImageHandle;
    int m_sgImageHandle;
    int m_noAmmoSGImageHandle;
    int m_healthUiImageHandle;
    int m_shieldImageHandle;
    int m_lockOnUIHandle;

    // フォントハンドル
    int m_fontHandle;
    int m_hpFontHandle;
    int m_warningFontHandle;

    // スケール管理
    float m_prevScale;

    /// <summary>
    /// グラデーション矩形を描画する
    /// </summary>
    /// <param name="x1">左上X</param>
    /// <param name="y1">左上Y</param>
    /// <param name="x2">右下X</param>
    /// <param name="y2">右下Y</param>
    /// <param name="topColor">上部の色</param>
    /// <param name="bottomColor">下部の色</param>
    void DrawGradientBox(int x1, int y1, int x2, int y2, unsigned int topColor, unsigned int bottomColor);
};
