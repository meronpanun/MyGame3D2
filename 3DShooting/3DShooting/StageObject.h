#include "Vec3.h"
#include <string>

/// <summary>
/// ステージオブジェクトクラス
/// </summary>
class StageObject
{
public:
	StageObject() : m_modelHandle(-1), m_pos(), m_rot(), m_scale(1.0f, 1.0f, 1.0f) {}

	void Init(int duplicateHandle, Vec3 pos, Vec3 rot, Vec3 scale);
	void Draw();

	Vec3 GetPos() const { return m_pos; }
	Vec3 GetScale() const { return m_scale; }

private:
	int m_modelHandle;
	Vec3 m_pos;
	Vec3 m_rot;
	Vec3 m_scale;
};