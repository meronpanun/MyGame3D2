#include "TransformDataLoader.h"
#include <fstream>
#include <sstream>

namespace
{
	// 名前、位置、回転、スケール、攻撃力、体力、追跡速度、タックルクールタイム、タックル速度、タックルダメージの要素数
	constexpr int kElementNum = 16; 

	// Unityの座標系からDxLibの座標系への変換係数
	constexpr float kUnityToDxPos = 100.0f; 
}

std::vector<ObjectTransformData> TransformDataLoader::LoadDataCSV(const char* fileName)
{
	// データを格納する配列
	std::vector<ObjectTransformData> Object;

	// ファイルを開く
	std::ifstream file(fileName);

	// もしもファイルを開けなかったら
	if (!file.is_open())
	{
		return Object; // 空の配列を返す
	}

	// 1行ずつ読み込む
	std::string line;
	//最初のヘッダーはスキップ
	bool isHeader = true;

	// CSVファイルの各行を読み込む
	while (std::getline(file, line))
	{
		if (isHeader)
		{
			isHeader = false; // ヘッダー行をスキップ
			continue;
		}

		std::stringstream ss(line);
		ObjectTransformData data;

		// カンマで分割して各要素を読み込む
		std::string element;
		int index = 0;

		while (std::getline(ss, element, ',') && index < kElementNum)
		{
			switch (index)
			{
			case 0: // 名前
				data.name = element;
				break;
			case 1: // 位置X
				data.pos.x = std::stof(element) * kUnityToDxPos;
				break;
			case 2: // 位置Y
				data.pos.y = std::stof(element) * kUnityToDxPos;
				break;
			case 3: // 位置Z
				data.pos.z = std::stof(element) * kUnityToDxPos;
				break;
			case 4: // 回転X
				data.rot.x = std::stof(element);
				break;
			case 5: // 回転Y
				data.rot.y = std::stof(element);
				break;
			case 6: // 回転Z
				data.rot.z = std::stof(element);
				break;
			case 7: // スケールX
				data.scale.x = std::stof(element);
				break;
			case 8: // スケールY
				data.scale.y = std::stof(element);
				break;
			case 9: // スケールZ
				data.scale.z = std::stof(element);
				break;
			case 10: // 攻撃力
				data.attack = std::stof(element);
				break;
			case 11: // 体力
				data.hp = std::stof(element);
				break;
			case 12: // speed（プレイヤー用）またはchaseSpeed（敵用）
				if (data.name == "Player") 
				{
					data.speed = std::stof(element);
				}
				else 
				{
					data.chaseSpeed = std::stof(element);
				}
				break;
			case 13: // タックルクールタイム
				data.tackleCooldown = element.empty() ? 0.0f : std::stof(element);
				break;
			case 14: // タックル速度
				data.tackleSpeed = element.empty() ? 0.0f : std::stof(element);
				break;
			case 15: // タックルダメージ
				data.tackleDamage = element.empty() ? 0.0f : std::stof(element);
				break;
			case 16: // RunSpeed（プレイヤー用）
				if (data.name == "Player") 
				{
					data.runSpeed = std::stof(element);
				}
				break;
			case 17: // InitialAmmo（プレイヤー用）
				if (data.name == "Player") 
				{
					data.initialAmmo = std::stoi(element);
				}
				break;
			case 18: // BulletPower（プレイヤー用）
				if (data.name == "Player") {
					data.bulletPower = std::stof(element);
				}
				break;
			default:
				break; // 不明な要素は無視
			}
			index++;
		}
		Object.push_back(data); // 読み込んだデータを配列に追加
	}

	// ファイルを閉じる
	file.close();
	return Object; // 読み込んだデータの配列を返す
}
