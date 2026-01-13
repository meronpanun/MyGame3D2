#pragma once
#include "DxLib.h"
#include <vector>
#include <memory>

class EnemyBase;

/// <summary>
/// ボスUI描画クラス
/// </summary>
class BossUI
{
public:
	BossUI();
	~BossUI();

	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="enemyList">敵のリスト</param>
	void Draw(const std::vector<std::shared_ptr<EnemyBase>>& enemyList);

private:
	/// <summary>
	/// ボスHPバーの描画
	/// </summary>
	/// <param name="hp">現在の体力</param>
	/// <param name="maxHp">最大体力</param>
	void DrawBossHPBar(float hp, float maxHp);

private:
	float m_healthBarAnim; // HPバーアニメーション用体力値
	int m_fontHandle;      // フォントハンドル
};
