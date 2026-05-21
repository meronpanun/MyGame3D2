#pragma once
#include "Stage.h"
#include <memory>
#include <vector>

class Player;
class WaveManager;
class ItemBase;

/// <summary>
/// 敵死亡時のアイテムドロップを管理するクラス
/// </summary>
class ItemDropManager
{
public:
    ItemDropManager();

    /// <summary>
    /// 初期化処理。WaveManagerへのコールバックを登録する
    /// </summary>
    /// <param name="pWaveManager">WaveManagerへのポインタ</param>
    void Init(WaveManager* pWaveManager);

    /// <summary>
    /// ドロップ状態をリセットする（ステージ切り替え時などに呼ぶ）
    /// </summary>
    void Reset();

    /// <summary>
    /// 毎フレームの更新処理
    /// </summary>
    /// <param name="pPlayer">プレイヤーへのポインタ</param>
    /// <param name="collisionData">ステージコリジョンデータ</param>
    void Update(Player* pPlayer, const std::vector<Stage::StageCollisionData>& collisionData);

    /// <summary>
    /// 描画処理
    /// </summary>
    void Draw();

    /// <summary>
    /// 敵死亡時に呼ばれる。アイテムをドロップする
    /// </summary>
    /// <param name="pos">死亡した敵の座標</param>
    void OnEnemyDeath(const VECTOR& pos);

private:
    WaveManager* m_pWaveManager;
    std::vector<std::shared_ptr<ItemBase>> m_items;
    bool   m_hasDroppedWave1FirstAid; // Wave1救急キットドロップ済み
    bool   m_hasDroppedWave1Ammo;     // Wave1弾薬ドロップ済み
    int    m_wave1DropCount;          // Wave1ドロップ回数
    VECTOR m_lastDropPos;             // 直前のドロップ座標（重複ドロップ防止）
};
