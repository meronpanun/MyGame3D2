#include "AnimationManager.h"
#include "DxLib.h"
#include <cassert>
#include <cmath>

void AnimationManager::Attach(int modelHandle, AnimData& data, const char* animName, bool isLoop)
{
    int index = MV1GetAnimIndex(modelHandle, animName);
    assert(index != -1);
    data.attachNo = MV1AttachAnim(modelHandle, index, -1, false);
    data.count = 0.0f;
    data.isLoop = isLoop;
    data.isEnd = false;
    data.animName = animName;
    data.blendRate = 0.0f;
}

void AnimationManager::Update(int modelHandle, AnimData& data, float animSpeed)
{
    if (data.attachNo == -1) return;
    data.count += animSpeed;
    float totalTime = MV1GetAttachAnimTotalTime(modelHandle, data.attachNo);
    if (data.isLoop)
    {
        while (data.count > totalTime)
        {
            data.count -= totalTime;
        }
    }
    else
    {
        if (data.count > totalTime)
        {
            data.count = totalTime;
            data.isEnd = true;
        }
    }
    MV1SetAttachAnimTime(modelHandle, data.attachNo, data.count);
}

void AnimationManager::Change(int modelHandle, AnimData& prev, AnimData& next, const char* animName, bool isLoop)
{
    if (prev.attachNo != -1)
    {
        MV1DetachAnim(modelHandle, prev.attachNo);
    }
    prev = next;
    Attach(modelHandle, next, animName, isLoop);
    next.blendRate = 0.0f;
    // ブレンド率は外部で管理する場合はここで初期化しない
}

void AnimationManager::UpdateBlend(int modelHandle, AnimData& prev, AnimData& next, float& blendRate, float blendStep)
{
    if (next.attachNo == -1 && prev.attachNo == -1) return;
    blendRate += blendStep;
    if (blendRate > 1.0f) blendRate = 1.0f;
    if (prev.attachNo != -1)
        MV1SetAttachAnimBlendRate(modelHandle, prev.attachNo, 1.0f - blendRate);
    if (next.attachNo != -1)
        MV1SetAttachAnimBlendRate(modelHandle, next.attachNo, blendRate);
}
