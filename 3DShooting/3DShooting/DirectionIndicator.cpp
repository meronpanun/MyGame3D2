#include "DirectionIndicator.h"
#include "Player.h"
#include "EnemyBase.h"
#include "Game.h"
#include "Camera.h"
#include <DxLib.h>
#include <cassert>

namespace
{
    constexpr float kIndicatorRadius  = 150.0f;       // インジケーターの描画半径（px）
    constexpr float kDisplayDuration  = 2.0f;         // インジケーターの表示時間（秒）
    constexpr float kFixedDeltaTime   = 1.0f / 60.0f; // 固定フレーム時間（秒）
    constexpr float kDirectionEpsilon = 0.001f;        // 方向ベクトルがゼロとみなす閾値
    constexpr float kIndicatorScale   = 0.1f;          // インジケーター画像の描画スケール
    constexpr int   kAlphaMax         = 255;           // アルファ値の最大値
}

int DirectionIndicator::s_indicatorImage = -1;

DirectionIndicator::DirectionIndicator()
    : m_pPlayer(nullptr)
{
}

void DirectionIndicator::LoadResources()
{
    s_indicatorImage = LoadGraph("data/image/DirectionIndicator.png");
    assert(s_indicatorImage != -1);
}

void DirectionIndicator::DeleteResources()
{
    DeleteGraph(s_indicatorImage);
}

void DirectionIndicator::Init(Player* player)
{
    m_pPlayer = player;
}

void DirectionIndicator::Update(const std::vector<std::shared_ptr<EnemyBase>>& enemies)
{
    if (!m_pPlayer) return;

    // 表示時間を減算し、期限切れのエントリを削除
    for (auto it = m_attackedEnemies.begin(); it != m_attackedEnemies.end();)
    {
        it->displayTime -= kFixedDeltaTime;

        if (it->displayTime <= 0.0f)
        {
            it = m_attackedEnemies.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void DirectionIndicator::Draw()
{
    if (!m_pPlayer || m_attackedEnemies.empty() || s_indicatorImage == -1) return;

    const auto& camera = m_pPlayer->GetCamera();
    if (!camera) return;

    int   screenW, screenH;
    GetScreenState(&screenW, &screenH, nullptr);
    float centerX = screenW * 0.5f;
    float centerY = screenH * 0.5f;

    // プレイヤーの前方ベクトル（水平成分のみ）
    Vec3 playerForward = VSub(camera->GetTarget(), camera->GetPos());
    playerForward.y = 0;
    playerForward.Normalize();

    for (const auto& attackedEnemy : m_attackedEnemies)
    {
        // プレイヤーから敵への方向ベクトル（水平成分のみ）
        Vec3 dirToEnemy = attackedEnemy.position - Vec3(m_pPlayer->GetPos());
        dirToEnemy.y = 0;
        if (dirToEnemy.Length() < kDirectionEpsilon) continue;
        dirToEnemy.Normalize();

        // 前方ベクトルと敵方向ベクトルのなす角度を -π〜π で取得
        float angle = atan2(
            dirToEnemy.x * playerForward.z - dirToEnemy.z * playerForward.x,
            dirToEnemy.x * playerForward.x + dirToEnemy.z * playerForward.z);

        // 画面中心から kIndicatorRadius の距離にインジケーターを配置
        float indicatorX = centerX + kIndicatorRadius * sin(angle);
        float indicatorY = centerY - kIndicatorRadius * cos(angle);

        // 表示時間に応じてフェードアウト
        float alpha = attackedEnemy.displayTime / attackedEnemy.maxDisplayTime;
        alpha = (std::max)(0.0f, (std::min)(1.0f, alpha));

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(alpha * kAlphaMax));
        DrawRotaGraphF(indicatorX, indicatorY, kIndicatorScale, angle, s_indicatorImage, true);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }
}

void DirectionIndicator::ShowAttackedEnemyDirection(const Vec3& enemyPos)
{
    m_attackedEnemies.emplace_back(enemyPos, kDisplayDuration);
}
