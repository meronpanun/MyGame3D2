#include "Vec3.h"
#include <string>

/// <summary>
/// ステージオブジェクトクラス
/// </summary>
class StageObject
{
public:
	StageObject();

	void Init(const std::string& modelPath, Vec3 pos, Vec3 rot, Vec3 scale);
	void Draw();

private:
	int m_modelHandle;
};