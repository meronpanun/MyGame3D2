#include "AnimationManager.h"
#include <cassert>
#include "GameObject/EnemyBase.h"

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

float AnimationManager::PlayState(int modelHandle, EnemyBase::AnimState state, bool loop)
{
    // 状態→アニメ名
    auto it = m_animStateToAnimName.find(state);
    if (it == m_animStateToAnimName.end()) return 0.0f;
    float total = PlayAnimation(modelHandle, it->second, loop);
    m_modelCurrentState[modelHandle] = state;
    m_modelAnimTime[modelHandle] = 0.0f;
    return total;
}

void AnimationManager::Update(int modelHandle, float delta)
{
    if (m_modelAnimTime.count(modelHandle)) {
        m_modelAnimTime[modelHandle] += delta;
        UpdateAnimationTime(modelHandle, m_modelAnimTime[modelHandle]);
    }
}

bool AnimationManager::IsAnimationFinished(int modelHandle) const
{
    if (!m_modelCurrentState.count(modelHandle)) return false;
    EnemyBase::AnimState state = m_modelCurrentState.at(modelHandle);
    auto it = m_animStateToAnimName.find(state);
    if (it == m_animStateToAnimName.end()) return false;
    float total = 0.0f;
    if (m_currentAnimTotalTimes.count(modelHandle)) {
        total = m_currentAnimTotalTimes.at(modelHandle);
    }
    if (!m_modelAnimTime.count(modelHandle)) return false;
    return m_modelAnimTime.at(modelHandle) >= total && total > 0.0f;
}

void AnimationManager::SetAnimName(EnemyBase::AnimState state, const std::string& animName)
{
	m_animStateToAnimName[state] = animName;
}
