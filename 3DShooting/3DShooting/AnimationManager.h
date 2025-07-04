#pragma once
#include "DxLib.h"
#include <string>
#include <map>
#include "GameObject/EnemyBase.h"

/// <summary>
/// アニメーション管理クラス
/// </summary>
class AnimationManager
{
public:
    AnimationManager();
    ~AnimationManager();

    /// <summary>
	/// モデルにアニメーションをアタッチし、再生を開始する
    /// </summary>
	/// <param name="modelHandle">アニメーションを適用するモデルのハンドル</param>
	/// <param name="animName">再生するアニメーションの名前</param>
	/// <param name="loop">ループ再生するかどうか</param>
	/// <returns>アニメーションの総時間。アニメーションが見つからない場合は0</returns>
    float PlayAnimation(int modelHandle, const std::string& animName, bool loop);

    /// <summary>
	/// アニメーションの時間を更新する
    /// </summary>
	/// <param name="modelHandle">アニメーションを更新するモデルのハンドル</param>
	/// <param name="animTime">現在のアニメーション再生時間</param>
    void UpdateAnimationTime(int modelHandle, float animTime);

    /// <summary>
	/// 指定したアニメーションの総時間を取得する
    /// </summary>
	/// <param name="modelHandle">アニメーションを取得するモデルのハンドル</param>
	/// <param name="animName">アニメーションの名前</param>
	/// <returns>アニメーションの総時間。見つからない場合は0</returns>
    float GetAnimationTotalTime(int modelHandle, const std::string& animName);

    // 現在アタッチされているアニメーションのハンドルを取得
    int GetCurrentAttachedAnimHandle(int modelHandle) const;

    // 現在アタッチされているアニメーションのハンドルをリセットする
    void ResetAttachedAnimHandle(int modelHandle);

    // 状態→アニメ名の登録
    void SetAnimName(EnemyBase::AnimState state, const std::string& animName);

    // 状態指定で再生
    float PlayState(int modelHandle, EnemyBase::AnimState state, bool loop);

    // アニメーション進行
    void Update(int modelHandle, float delta);

    // アニメーション終了判定
    bool IsAnimationFinished(int modelHandle) const;

private:
    // モデルハンドルとアニメーション名ごとのDxLibアニメーションインデックスをキャッシュ
    std::map<int, std::map<std::string, int>> m_animIndexesCache;
    // モデルハンドルと現在アタッチされているDxLibアニメーションハンドル
    std::map<int, int> m_attachedAnimHandles;
    // モデルハンドルと現在アタッチされているアニメーションの総時間
    std::map<int, float> m_currentAnimTotalTimes;

    // 状態→アニメ名マッピング
    std::map<EnemyBase::AnimState, std::string> m_animStateToAnimName;
    // モデルごとの現在のAnimState
    std::map<int, EnemyBase::AnimState> m_modelCurrentState;
    // モデルごとの現在のアニメーション経過時間
    std::map<int, float> m_modelAnimTime;

    // 内部ヘルパー関数：アニメーションインデックスを取得し、キャッシュする
    int GetAnimIndexInternal(int modelHandle, const std::string& animName);
};