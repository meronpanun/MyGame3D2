#include "TransformDataLoader.h"
#include <fstream>
#include <sstream>

namespace
{
	// 名前、位置、回転、スケールの要素数
	constexpr int kElementNum = 10; 

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
