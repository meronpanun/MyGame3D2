#include "ItemDropManager.h"
#include "AmmoItem.h"
#include "FirstAidKitItem.h"
#include "ItemBase.h"
#include "Player.h"
#include "WaveManager.h"
#include <algorithm>

namespace
{
    constexpr float kDropInitialHeight  = 140.0f; // ドロップ時の初期上昇量（Y加算値）
    constexpr int   kWave1MaxDropCount  =   2;    // Wave1でドロップできる最大アイテム数
    constexpr int   kDropRandThreshold  =  50;    // ランダムドロップの閾値（0〜99の乱数と比較）
    constexpr float kInvalidPos         = -99999.0f; // 無効座標の初期値
}

ItemDropManager::ItemDropManager()
    : m_pWaveManager(nullptr)
    , m_hasDroppedWave1FirstAid(false)
    , m_hasDroppedWave1Ammo(false)
    , m_wave1DropCount(0)
    , m_lastDropPos(VGet(kInvalidPos, kInvalidPos, kInvalidPos))
{
}

void ItemDropManager::Init(WaveManager* pWaveManager)
{
    m_pWaveManager = pWaveManager;
    Reset();

    m_pWaveManager->SetOnEnemyDeathCallback([this](const VECTOR& pos)
    {
        OnEnemyDeath(pos);
    });
}

void ItemDropManager::Reset()
{
    m_items.clear();
    m_hasDroppedWave1FirstAid = false;
    m_hasDroppedWave1Ammo     = false;
    m_wave1DropCount          = 0;
    m_lastDropPos             = VGet(kInvalidPos, kInvalidPos, kInvalidPos);
}

void ItemDropManager::Update(Player* pPlayer, const std::vector<Stage::StageCollisionData>& collisionData)
{
    for (std::shared_ptr<ItemBase>& item : m_items)
    {
        item->Update(pPlayer, collisionData);
    }
    m_items.erase(
        std::remove_if(m_items.begin(), m_items.end(),
            [](const std::shared_ptr<ItemBase>& item) { return item->IsUsed() || item->IsExpired(); }),
        m_items.end());
}

void ItemDropManager::Draw()
{
    for (std::shared_ptr<ItemBase>& item : m_items)
    {
        item->Draw();
    }
}

void ItemDropManager::OnEnemyDeath(const VECTOR& pos)
{
    // 直前と同じ座標なら何もしない（重複ドロップ防止）
    if (pos.x == m_lastDropPos.x && pos.y == m_lastDropPos.y && pos.z == m_lastDropPos.z) return;
    m_lastDropPos = pos;

    VECTOR dropPos = pos;
    dropPos.y += kDropInitialHeight;

    if (m_pWaveManager->GetCurrentWave() == 1)
    {
        if (m_wave1DropCount >= kWave1MaxDropCount) return;

        if (!m_hasDroppedWave1FirstAid && !m_hasDroppedWave1Ammo)
        {
            // 最初のドロップ：ランダムでどちらか一方
            if (GetRand(99) < kDropRandThreshold)
            {
                auto item = std::make_shared<FirstAidKitItem>();
                item->Init();
                item->SetPos(dropPos);
                m_items.push_back(std::move(item));
                m_hasDroppedWave1FirstAid = true;
            }
            else
            {
                auto item = std::make_shared<AmmoItem>();
                item->Init();
                item->SetPos(dropPos);
                m_items.push_back(std::move(item));
                m_hasDroppedWave1Ammo = true;
            }
            m_wave1DropCount++;
        }
        else if (!m_hasDroppedWave1FirstAid)
        {
            // 2回目：まだ落としていない救急キットを確定ドロップ
            auto item = std::make_shared<FirstAidKitItem>();
            item->Init();
            item->SetPos(dropPos);
            m_items.push_back(std::move(item));
            m_hasDroppedWave1FirstAid = true;
            m_wave1DropCount++;
        }
        else if (!m_hasDroppedWave1Ammo)
        {
            // 2回目：まだ落としていない弾薬を確定ドロップ
            auto item = std::make_shared<AmmoItem>();
            item->Init();
            item->SetPos(dropPos);
            m_items.push_back(std::move(item));
            m_hasDroppedWave1Ammo = true;
            m_wave1DropCount++;
        }
        // 両方ドロップ済みの場合は何もしない
    }
    else
    {
        // Wave2以降：ランダムでどちらか一方をドロップ
        std::shared_ptr<ItemBase> item;
        if (GetRand(99) < kDropRandThreshold)
        {
            item = std::make_shared<FirstAidKitItem>();
        }
        else
        {
            item = std::make_shared<AmmoItem>();
        }
        item->Init();
        item->SetPos(dropPos);
        m_items.push_back(std::move(item));
    }
}
