#pragma once
#include "DxLib.h"
#include "ManagedFont.h"
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

	/// <summary>
	/// フォントのリロード（スケール変更時に呼び出す）
	/// </summary>
	/// <param name="scale">新しいスケール</param>
	void ReloadFonts(float scale);

	/// <summary>
	/// グラデーションボックスの描画（上部と下部で色が変わる）
	/// </summary>
	/// <param name="x1">x座標1</param>
	/// <param name="y1">y座標1</param>
	/// <param name="x2">x座標2</param>
	/// <param name="y2">y座標2</param>
	/// <param name="topColor">上部の色</param>
	/// <param name="bottomColor">下部の色</param>
	void DrawGradientBox(int x1, int y1, int x2, int y2, unsigned int topColor, unsigned int bottomColor);

private:
	float m_healthBarAnim; // HPバーアニメーション用体力値
	ManagedFont m_font;    // フォントハンドル

	// スケール管理
	float m_prevScale;

};
