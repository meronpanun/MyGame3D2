#pragma once
#include "Vec3.h"
#include <vector>

class Player;
class EnemyBase;

/// <summary>
/// 攻撃された敵の情報を保持する構造体
/// </summary>
struct AttackedEnemyInfo
{
    Vec3  position;       // 敵の位置
    float displayTime;    // 表示残り時間
    float maxDisplayTime; // 最大表示時間

    /// <summary>
    /// コンストラクタ
    /// </summary>
    /// <param name="pos">敵の位置</param>
    /// <param name="maxTime">最大表示時間</param>
    AttackedEnemyInfo(const Vec3& pos, float maxTime)
        : position(pos), displayTime(maxTime), maxDisplayTime(maxTime) {}
};

/// <summary>
/// 攻撃を受けた方向を画面上にインジケーターで示すクラス。
/// 攻撃された敵の位置をプレイヤーの前方ベクトルとの角度差に変換し、
/// 画面中心から一定距離に回転させたアイコンとして描画する。
/// </summary>
class DirectionIndicator
{
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    DirectionIndicator();
    ~DirectionIndicator() = default;

    /// <summary>
    /// 初期化処理
    /// </summary>
    /// <param name="player">参照するプレイヤーへのポインタ</param>
    void Init(Player* player);

    /// <summary>
    /// 毎フレームの更新処理（表示時間の減算と期限切れエントリの削除）
    /// </summary>
    /// <param name="enemies">敵リスト（現在未使用）</param>
    void Update(const std::vector<std::shared_ptr<EnemyBase>>& enemies);

    /// <summary>
    /// インジケーターを描画する
    /// </summary>
    void Draw();

    /// <summary>
    /// 攻撃された敵の方向インジケーターを登録する
    /// </summary>
    /// <param name="enemyPos">攻撃した敵の位置</param>
    void ShowAttackedEnemyDirection(const Vec3& enemyPos);

    /// <summary>
    /// 共有リソース（画像）を読み込む
    /// </summary>
    static void LoadResources();

    /// <summary>
    /// 共有リソース（画像）を解放する
    /// </summary>
    static void DeleteResources();

private:
    Player*                        m_pPlayer;         // 参照するプレイヤーへのポインタ
    std::vector<AttackedEnemyInfo> m_attackedEnemies; // 表示中のインジケーター情報リスト
    static int                     s_indicatorImage;  // インジケーター画像ハンドル
};
