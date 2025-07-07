#pragma once
#include "DxLib.h" 
#include <string>
#include <vector>

struct ObjectTransformData
{
	std::string name;    // オブジェクト名
	VECTOR pos;		     // 位置
	VECTOR rot;		     // 回転
	VECTOR scale;	     // スケール
	float attack = 0.0f; // 攻撃力
	float hp = 0.0f;     // 体力
	float speed = 0.0f;  // プレイヤー用の移動速度
	float chaseSpeed = 0.0f; // 追跡速度
	float tackleCooldown = 0.0f; // タックルクールタイム
	float tackleSpeed = 0.0f;    // タックル時の速度
	float tackleDamage = 0.0f;   // タックルダメージ
	float runSpeed = 0.0f;      // プレイヤー用の走る速度
	int initialAmmo = 0;        // プレイヤー用の初期弾薬数
	float bulletPower = 0.0f;    // プレイヤー用の弾の威力
};

/// <summary>
/// オブジェクト変換データローダー
/// </summary>
class TransformDataLoader
{
public:

	/// <summary>
	/// CSVデータローダー
	/// </summary>
	/// <param name="fileName">ファイル名</param>
	/// <returns>読み込んだオブジェクト変換データのベクトル</returns>
	static std::vector< ObjectTransformData> LoadDataCSV(const char* fileName);
};

