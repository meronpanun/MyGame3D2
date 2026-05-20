#include "StageObject.h"
#include "EffekseerWarningSuppress.h"
#include <cassert>

namespace
{
    constexpr float kDegToRad        = DX_PI_F / 180.0f; // 度からラジアンへの変換係数
    constexpr float kModelScaleFactor = 0.01f;            // Unity エクスポートスケールを DxLib 単位に変換する係数
}

StageObject::StageObject()
    : m_modelHandle(-1)
    , m_pos()
    , m_rot()
    , m_scale(1.0f, 1.0f, 1.0f)
{
}

StageObject::~StageObject()
{
    if (m_modelHandle != -1)
    {
        MV1DeleteModel(m_modelHandle);
        m_modelHandle = -1;
    }
}

StageObject::StageObject(StageObject&& other) noexcept
    : m_modelHandle(other.m_modelHandle)
    , m_pos(other.m_pos)
    , m_rot(other.m_rot)
    , m_scale(other.m_scale)
{
    other.m_modelHandle = -1; // 移譲元が二重解放しないよう無効化
}

StageObject& StageObject::operator=(StageObject&& other) noexcept
{
    if (this != &other)
    {
        if (m_modelHandle != -1)
        {
            MV1DeleteModel(m_modelHandle);
        }
        m_modelHandle       = other.m_modelHandle;
        m_pos               = other.m_pos;
        m_rot               = other.m_rot;
        m_scale             = other.m_scale;
        other.m_modelHandle = -1; // 移譲元が二重解放しないよう無効化
    }
    return *this;
}

void StageObject::Init(int duplicateHandle, Vec3 pos, Vec3 rot, Vec3 scale)
{
    m_modelHandle = duplicateHandle;
    assert(m_modelHandle >= 0);
    m_pos   = pos;
    m_rot   = rot;
    m_scale = scale;

    // Unity 側で座標系変換済み（Z 軸反転）のため位置はそのまま適用する
    MV1SetPosition(m_modelHandle, pos.ToDxVECTOR());

    // Unity 側で Y 軸回転が反転済みのため、DxLib 側で再度 Y 軸を反転して正しい向きに戻す
    MV1SetRotationXYZ(m_modelHandle, VGet(rot.x * kDegToRad, -rot.y * kDegToRad, rot.z * kDegToRad));

    // Unity のスケール値は DxLib の 100 倍のため kModelScaleFactor で正規化する
    MV1SetScale(m_modelHandle, VGet(scale.x * kModelScaleFactor,
                                   scale.y * kModelScaleFactor,
                                   scale.z * kModelScaleFactor));
}

void StageObject::Draw()
{
    MV1DrawModel(m_modelHandle);
}
