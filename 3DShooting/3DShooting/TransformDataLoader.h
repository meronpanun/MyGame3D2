#pragma once
#include "DxLib.h" 
#include <string>
#include <vector>

struct ObjectTransformData
{
	std::string name;  // オブジェクト名
	VECTOR pos;		   // 位置
	VECTOR rot;		   // 回転
	VECTOR scale;	   // スケール
};

/// <summary>
/// 外部ファイルから変換データを読み込むクラス
/// </summary>
class TransformDataLoader
{
public:

	/// <summary>
	/// CSVファイルから変換データを読み込む
	/// </summary>
	/// <param name="fileName">ファイル名</param>
	/// <returns>読み込んだ変換データのリスト</returns>
	static std::vector< ObjectTransformData> LoadDataCSV(const char* fileName);
};

