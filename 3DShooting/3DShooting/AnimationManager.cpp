#include "AnimationManager.h"
#include <cassert>

AnimationManager::AnimationManager()
{
}

AnimationManager::~AnimationManager()
{
    // マネージャーが破棄される際に全てのアニメーションをデタッチ
    for (const auto& pair : m_attachedAnimHandles)
    {
        MV1DetachAnim(pair.first, 0);
    }
}

int AnimationManager::GetAnimIndexInternal(int modelHandle, const std::string& animName)
{
    // キャッシュをチェック
    if (m_animIndexesCache.count(modelHandle) && m_animIndexesCache[modelHandle].count(animName))
    {
        return m_animIndexesCache[modelHandle][animName];
    }

    // キャッシュになければDxLibから取得し、キャッシュする
    int animIndex = MV1GetAnimIndex(modelHandle, animName.c_str());
    if (animIndex != -1)
    {
        m_animIndexesCache[modelHandle][animName] = animIndex;
    }
    return animIndex;
}

float AnimationManager::PlayAnimation(int modelHandle, const std::string& animName, bool loop)
{
    // 既に何かアタッチされている場合はデタッチ
    if (m_attachedAnimHandles.count(modelHandle) && m_attachedAnimHandles[modelHandle] != -1)
    {
        MV1DetachAnim(modelHandle, 0);
        m_attachedAnimHandles[modelHandle] = -1;
    }

    int animIndex = GetAnimIndexInternal(modelHandle, animName);

    if (animIndex != -1)
    {
        int attachedHandle = MV1AttachAnim(modelHandle, animIndex, -1, loop);
        m_attachedAnimHandles[modelHandle] = attachedHandle;
        if (attachedHandle != -1)
        {
            float totalTime = MV1GetAnimTotalTime(modelHandle, attachedHandle);
            m_currentAnimTotalTimes[modelHandle] = totalTime;
            MV1SetAttachAnimTime(modelHandle, 0, 0.0f); // アニメーション開始時間をリセット
            return totalTime;
        }
    }

    m_attachedAnimHandles[modelHandle] = -1; // アニメーションが見つからない場合は無効なハンドルを設定
    m_currentAnimTotalTimes[modelHandle] = 0.0f;
    return 0.0f;
}

void AnimationManager::UpdateAnimationTime(int modelHandle, float animTime)
{
    if (m_attachedAnimHandles.count(modelHandle) && m_attachedAnimHandles[modelHandle] != -1)
    {
        MV1SetAttachAnimTime(modelHandle, 0, animTime);
    }
}

float AnimationManager::GetAnimationTotalTime(int modelHandle, const std::string& animName)
{
    // 指定アニメーション名のインデックスを取得
    int animIndex = MV1GetAnimIndex(modelHandle, animName.c_str());
    if (animIndex != -1)
    {
        return MV1GetAnimTotalTime(modelHandle, animIndex);
    }
    // 現在アタッチされていない、またはキャッシュにない場合は0を返す（事前にPlayAnimationでロードしておくべき）
    return 0.0f;
}

int AnimationManager::GetCurrentAttachedAnimHandle(int modelHandle) const
{
    if (m_attachedAnimHandles.count(modelHandle))
    {
        return m_attachedAnimHandles.at(modelHandle);
    }
    return -1;
}

void AnimationManager::ResetAttachedAnimHandle(int modelHandle)
{
    if (m_attachedAnimHandles.count(modelHandle))
    {
        m_attachedAnimHandles[modelHandle] = -1;
    }
}
