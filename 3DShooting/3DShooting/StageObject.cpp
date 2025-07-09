#include "StageObject.h"
#include "DxLib.h"
#include <cassert>

void StageObject::Init(const std::string& modelPath, Vec3 pos, Vec3 rot, Vec3 scale)
{
	m_modelHandle = MV1LoadModel(modelPath.c_str());
	assert(m_modelHandle >= 0);

	MV1SetPosition(m_modelHandle, pos.ToDxVECTOR());
	MV1SetRotationXYZ(m_modelHandle, rot.ToDxVECTOR());
	MV1SetScale(m_modelHandle, VGet(scale.x * 0.01f, scale.y * 0.01f, scale.z * 0.01f));
}

void StageObject::Draw()
{
	if (m_modelHandle >= 0)
	{
		MV1DrawModel(m_modelHandle);
	}
}