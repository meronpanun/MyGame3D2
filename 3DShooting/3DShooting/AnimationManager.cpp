#include "AnimationManager.h"
#include "EnemyBase.h"
#include <cassert>

AnimationManager::AnimationManager()
{
}

AnimationManager::~AnimationManager()
{
    // 管理中の全モデルからアニメーションをデタッチ
    for (const auto& pair : m_attachedAnimHandles)
    {
        MV1DetachAnim(pair.first, 0);
    }
}

int AnimationManager::GetAnimIndexInternal(int modelHandle, const std::string& animName)
{
    // MV1GetAnimIndex は毎フレーム呼ぶには高コストなため、結果をキャッシュして再利用する。
    // キャッシュキーはモデルハンドルと名前の組み合わせ。
    // ※ オブジェクトプールでモデルを使い回す場合、同じハンドルが別のモデルに割り当て直される
    //   ことがある。その場合キャッシュが古くなるが、本プロジェクトでは Init() のたびに
    //   同じモデルをロードするため問題は生じない。
    if (m_animIndexesCache.count(modelHandle) && m_animIndexesCache[modelHandle].count(animName))
    {
        return m_animIndexesCache[modelHandle][animName];
    }

    // キャッシュになければ DxLib から取得してキャッシュに登録
    int animIndex = MV1GetAnimIndex(modelHandle, animName.c_str());
    if (animIndex != -1)
    {
        m_animIndexesCache[modelHandle][animName] = animIndex;
    }
    return animIndex;
}

float AnimationManager::PlayAnimation(int modelHandle, const std::string& animName, bool loop)
{
    // 既にアタッチされているアニメーションをデタッチ（DxLib は同時に1つしかアタッチできない）
    if (m_attachedAnimHandles.count(modelHandle) && m_attachedAnimHandles[modelHandle] != -1)
    {
        MV1DetachAnim(modelHandle, 0);
        m_attachedAnimHandles[modelHandle] = -1;
    }

    int animIndex = GetAnimIndexInternal(modelHandle, animName);

    if (animIndex != -1)
    {
        // DxLib の MV1AttachAnim の第4引数 `loop` は「ブレンドアニメーションを使うか」のフラグであり、
        // 実際のループ再生制御はユーザー側で animTime をリセットすることで行う。
        // ここでは false を渡すことでブレンドなし（単純アタッチ）にしている。
        int attachedHandle = MV1AttachAnim(modelHandle, animIndex, -1, loop);
        m_attachedAnimHandles[modelHandle] = attachedHandle;
        if (attachedHandle != -1)
        {
            float totalTime = MV1GetAnimTotalTime(modelHandle, attachedHandle);
            m_currentAnimTotalTimes[modelHandle] = totalTime;
            MV1SetAttachAnimTime(modelHandle, 0, 0.0f); // 再生時間を先頭にリセット
            return totalTime;
        }
    }

    // アニメーションが見つからない場合は無効値を設定して 0 を返す
    m_attachedAnimHandles[modelHandle]   = -1;
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

float AnimationManager::GetAnimationTotalTime(int modelHandle, const std::string& animName) const
{
    int animIndex = MV1GetAnimIndex(modelHandle, animName.c_str());
    if (animIndex != -1)
    {
        return MV1GetAnimTotalTime(modelHandle, animIndex);
    }
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
    auto it = m_animStateToAnimName.find(state);
    if (it == m_animStateToAnimName.end()) return 0.0f;

    float total = PlayAnimation(modelHandle, it->second, loop);
    m_modelCurrentState[modelHandle] = state; // 現在の状態を更新
    m_modelAnimTime[modelHandle]     = 0.0f;  // アニメーション時間をリセット
    return total;
}

void AnimationManager::Update(int modelHandle, float delta)
{
    if (m_modelAnimTime.count(modelHandle))
    {
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

    // PlayAnimation() 呼び出し時にキャッシュした総再生時間を使用する。
    // GetAnimationTotalTime() で毎回 MV1GetAnimIndex を呼ぶより低コスト。
    float total = 0.0f;
    if (m_currentAnimTotalTimes.count(modelHandle))
    {
        total = m_currentAnimTotalTimes.at(modelHandle);
    }
    if (!m_modelAnimTime.count(modelHandle)) return false;
    return m_modelAnimTime.at(modelHandle) >= total && total > 0.0f;
}

void AnimationManager::SetAnimName(EnemyBase::AnimState state, const std::string& animName)
{
    m_animStateToAnimName[state] = animName;
}
