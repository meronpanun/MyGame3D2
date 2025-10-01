#include "DirectionIndicator.h"
#include "Player.h"
#include "EnemyBase.h"
#include "Game.h"
#include "Camera.h"
#include <DxLib.h>
#include <cassert>

namespace
{
	// インジケーターの描画半径
	constexpr float kIndicatorRadius = 150.0f;
}

DirectionIndicator::DirectionIndicator() :
    m_pPlayer(nullptr),
    m_indicatorImage(-1)
{
	// 画像の読み込み
    m_indicatorImage = LoadGraph("data/image/DirectionIndicator.png");
	assert(m_indicatorImage != -1);
}

DirectionIndicator::~DirectionIndicator()
{
	// 画像の解放
    DeleteGraph(m_indicatorImage);
}

void DirectionIndicator::Init(Player* player)
{
    m_pPlayer = player;
}

void DirectionIndicator::Update(const std::vector<std::shared_ptr<EnemyBase>>& enemies)
{
    // プレイヤーが存在しない場合は処理しない
	if (!m_pPlayer) return; 

    // 敵の位置リストをクリア
	m_allEnemyPositions.clear(); 

	// カメラの取得
    const auto& camera = m_pPlayer->GetCamera();
	if (!camera) return; // カメラがない場合は処理なし

    // 敵の位置を収集
	for (const auto& enemy : enemies)
    {
        // 生存している敵のみ対象
		if (!enemy->IsAlive()) continue; 

		// 敵の位置をリストに追加
        m_allEnemyPositions.push_back(enemy->GetPos()); 
    }
}

void DirectionIndicator::Draw()
{
    // プレイヤーが存在しないか、敵がいない場合は描画しない
	if (!m_pPlayer || m_allEnemyPositions.empty() || m_indicatorImage == -1) return; 

	// カメラの取得
    const auto& camera = m_pPlayer->GetCamera();
    if (!camera) return; // カメラがない場合は処理なし

    int screenW, screenH;
    GetScreenState(&screenW, &screenH, nullptr);
    float centerX = screenW * 0.5f;
    float centerY = screenH * 0.5f;

    // プレイヤーの前方ベクトル
    Vec3 playerForward = VSub(camera->GetTarget(), camera->GetPos());
    playerForward.y = 0;
    playerForward.Normalize(); 

	// 各敵に対してインジケーターを描画
    for (const auto& enemyPos : m_allEnemyPositions)
    {        
        // プレイヤーから敵への方向ベクトル
        Vec3 dirToEnemy = enemyPos - Vec3(m_pPlayer->GetPos());
        dirToEnemy.y = 0;
        if (dirToEnemy.Length() < 0.001f) continue; // プレイヤーと敵が同じ位置の場合はスキップ
		dirToEnemy.Normalize();

        // プレイヤー前方ベクトルと敵への方向ベクトルのなす角度を計算
		// atan2を使って、右回りを正とする角度を求める
        float angle = atan2(dirToEnemy.x * playerForward.z - dirToEnemy.z * playerForward.x, 
                              dirToEnemy.x * playerForward.x + dirToEnemy.z * playerForward.z);

        // インジケーターの位置を計算
		// 画面中心からkIndicatorRadiusの距離に配置
        float indicatorX = centerX + kIndicatorRadius * sin(angle); 
        float indicatorY = centerY - kIndicatorRadius * cos(angle);

        // インジケーターを描画
        DrawRotaGraphF(indicatorX, indicatorY, 0.1, angle, m_indicatorImage, true);
    }
}