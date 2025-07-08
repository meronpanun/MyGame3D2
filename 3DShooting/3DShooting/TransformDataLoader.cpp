#include "TransformDataLoader.h"
#include <fstream>
#include <sstream>

namespace
{
	// 名前、位置、回転、スケール、攻撃力、体力、追跡速度、タックルクールタイム、タックル速度、タックルダメージの要素数
	constexpr int kElementNum = 22; 

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
			// 空のセルをチェック
			if (element.empty())
			{
				index++;
				continue;
			}

			switch (index)
			{
			case 0: // 名前
				data.name = element;
				break;
			case 1: // 位置X
				try 
				{
					data.pos.x = std::stof(element) * kUnityToDxPos;
				}
				catch (const std::exception&) 
				{
					data.pos.x = 0.0f;
				}
				break;
			case 2: // 位置Y
				try 
				{
					data.pos.y = std::stof(element) * kUnityToDxPos;
				}
				catch (const std::exception&) 
				{
					data.pos.y = 0.0f;
				}
				break;
			case 3: // 位置Z
				try 
				{
					data.pos.z = std::stof(element) * kUnityToDxPos;
				}
				catch (const std::exception&) 
				{
					data.pos.z = 0.0f;
				}
				break;
			case 4: // 回転X
				try 
				{
					data.rot.x = std::stof(element);
				}
				catch (const std::exception&) 
				{
					data.rot.x = 0.0f;
				}
				break;
			case 5: // 回転Y
				try 
				{
					data.rot.y = std::stof(element);
				} 
				catch (const std::exception&) 
				{
					data.rot.y = 0.0f;
				}
				break;
			case 6: // 回転Z
				try
				{
					data.rot.z = std::stof(element);
				}
				catch (const std::exception&) 
				{
					data.rot.z = 0.0f;
				}
				break;
			case 7: // スケールX
				try 
				{
					data.scale.x = std::stof(element);
				}
				catch (const std::exception&) 
				{
					data.scale.x = 1.0f;
				}
				break;
			case 8: // スケールY
				try 
				{
					data.scale.y = std::stof(element);
				}
				catch (const std::exception&) 
				{
					data.scale.y = 1.0f;
				}
				break;
			case 9: // スケールZ
				try
				{
					data.scale.z = std::stof(element);
				}
				catch (const std::exception&) 
				{
					data.scale.z = 1.0f;
				}
				break;
			case 10: // 攻撃力
				try 
				{
					data.attack = std::stof(element);
				}
				catch (const std::exception&) 
				{
					data.attack = 0.0f;
				}
				break;
			case 11: // 体力
				try 
				{
					data.hp = std::stof(element);
				} 
				catch (const std::exception&) 
				{
					data.hp = 0.0f;
				}
				break;
			case 12: // NormalChaseSpeed
				try 
				{
					if (!element.empty()) 
					{
						data.chaseSpeed = std::stof(element);
					}
				}
				catch (const std::exception&) 
				{
					data.chaseSpeed = 0.0f;
				}
				break;
			case 13: // RunnerChaseSpeed
				try
				{
					if (!element.empty()) 
					{
						data.chaseSpeed = std::stof(element);
					}
				}
				catch (const std::exception&) 
				{
					// 既に設定されている場合は変更しない
				}
				break;
			case 14: // AcidChaseSpeed
				try 
				{
					if (!element.empty()) 
					{
						data.chaseSpeed = std::stof(element);
					}
				} 
				catch (const std::exception&) 
				{
					// 既に設定されている場合は変更しない
				}
				break;
			case 15: // Speed(プレイヤー用)
				try
				{
					if (!element.empty()) 
					{
						data.speed = std::stof(element);
					}
				}
				catch (const std::exception&) 
				{
					data.speed = 0.0f;
				}
				break;
			case 16: // RunSpeed(プレイヤー用)
				try 
				{
					if (!element.empty()) 
					{
						data.runSpeed = std::stof(element);
					}
				} 
				catch (const std::exception&) 
				{
					data.runSpeed = 0.0f;
				}
				break;
			case 17: // InitialAmmo(プレイヤー用)
				try
				{
					if (!element.empty()) 
					{
						data.initialAmmo = std::stoi(element);
					}
				} 
				catch (const std::exception&) 
				{
					data.initialAmmo = 0;
				}
				break;
			case 18: // BulletPower(プレイヤー用)
				try
				{
					if (!element.empty()) 
					{
						data.bulletPower = std::stof(element);
					}
				} 
				catch (const std::exception&) 
				{
					data.bulletPower = 0.0f;
				}
				break;
			case 19: // TackleCooldown
				try 
				{
					if (!element.empty()) 
					{
						data.tackleCooldown = std::stof(element);
					}
				}
				catch (const std::exception&) 
				{
					data.tackleCooldown = 0.0f;
				}
				break;
			case 20: // TackleSpeed
				try 
				{
					if (!element.empty()) 
					{
						data.tackleSpeed = std::stof(element);
					}
				} 
				catch (const std::exception&) 
				{
					data.tackleSpeed = 0.0f;
				}
				break;
			case 21: // TackleDamage
				try 
				{
					if (!element.empty()) 
					{
						data.tackleDamage = std::stof(element);
					}
				} 
				catch (const std::exception&) 
				{
					data.tackleDamage = 0.0f;
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
