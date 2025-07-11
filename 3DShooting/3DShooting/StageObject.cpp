#include "StageObject.h"
#include "DxLib.h"
#include <cassert>
#include <cmath> // For M_PI or equivalent constant

// Define M_PI if not already defined (common in C++)
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void StageObject::Init(const std::string& modelPath, Vec3 pos, Vec3 rot, Vec3 scale)
{
	m_modelHandle = MV1LoadModel(modelPath.c_str());
	assert(m_modelHandle >= 0);

	MV1SetPosition(m_modelHandle, pos.ToDxVECTOR());

	float rotXRad = rot.x * (M_PI / 180.0f);
	float rotYRad = rot.y * (M_PI / 180.0f);
	float rotZRad = rot.z * (M_PI / 180.0f);

	MV1SetRotationXYZ(m_modelHandle, VGet(rotXRad, rotYRad, rotZRad)); 
	MV1SetScale(m_modelHandle, VGet(scale.x * 0.01f, scale.y * 0.01f, scale.z * 0.01f));
}

void StageObject::Draw()
{
	if (m_modelHandle >= 0)
	{
		MV1DrawModel(m_modelHandle);
	}
}