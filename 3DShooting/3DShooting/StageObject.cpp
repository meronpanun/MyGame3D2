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

	if (modelPath.find("Hangar_v3") != std::string::npos) {
		int materialNum = MV1GetMaterialNum(m_modelHandle);
		for (int i = 0; i < materialNum; ++i) {
			MV1SetMaterialDifColor(m_modelHandle, i, GetColorF(0.5f, 0.5f, 0.5f, 1.0f)); // ディフューズ色を暗く
			MV1SetMaterialAmbColor(m_modelHandle, i, GetColorF(0.5f, 0.5f, 0.5f, 1.0f)); // アンビエント色も強く
			MV1SetMaterialSpcColor(m_modelHandle, i, GetColorF(0.0f, 0.0f, 0.0f, 1.0f)); // スペキュラを消す
		}
	}
}

void StageObject::Draw()
{
	if (m_modelHandle >= 0)
	{
		MV1DrawModel(m_modelHandle);
	}
}