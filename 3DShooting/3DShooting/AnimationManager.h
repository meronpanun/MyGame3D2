#pragma once
#include "DxLib.h"
#include <string>
#include <map> // To store animation handles by name/enum

/// <summary>
/// アニメーション管理クラス
/// </summary>
class AnimationManager
{
public:
    AnimationManager();
    ~AnimationManager();

    // モデルにアニメーションをアタッチし、再生を開始する
    // modelHandle: アニメーションを適用するモデルのハンドル
    // animName: 再生するアニメーションの名前 (例: "WALK", "ATK", "DEAD")
    // loop: ループ再生するかどうか
    // Returns: アニメーションの総時間 (秒)。アニメーションが見つからない場合は0。
    float PlayAnimation(int modelHandle, const std::string& animName, bool loop);

    // アニメーションの時間を更新する
    // modelHandle: モデルのハンドル
    // animTime: 現在のアニメーション再生時間
    void UpdateAnimationTime(int modelHandle, float animTime);

    // 指定したアニメーションの総時間を取得する
    // modelHandle: モデルのハンドル
    // animName: アニメーションの名前
    // Returns: アニメーションの総時間 (秒)。見つからない場合は0。
    float GetAnimationTotalTime(int modelHandle, const std::string& animName);

    // 現在アタッチされているアニメーションのハンドルを取得（デバッグ用など）
    int GetCurrentAttachedAnimHandle(int modelHandle) const;

    // 現在アタッチされているアニメーションのハンドルをリセットする（外部からデタッチされた場合などに使用）
    void ResetAttachedAnimHandle(int modelHandle);

private:
    // モデルハンドルとアニメーション名ごとのDxLibアニメーションインデックスをキャッシュ
    std::map<int, std::map<std::string, int>> m_animIndexesCache;
    // モデルハンドルと現在アタッチされているDxLibアニメーションハンドル
    std::map<int, int> m_attachedAnimHandles;
    // モデルハンドルと現在アタッチされているアニメーションの総時間
    std::map<int, float> m_currentAnimTotalTimes;

    // 内部ヘルパー関数：アニメーションインデックスを取得し、キャッシュする
    int GetAnimIndexInternal(int modelHandle, const std::string& animName);
};