#pragma once
#include "EffekseerWarningSuppress.h"
#include "EnemyBase.h"
#include <string>
#include <map>

/// <summary>
/// DxLib の MV1 モデルに対するアニメーション再生・管理クラス。
/// モデルハンドルをキーとしてアタッチ済みアニメーションと再生時間を管理し、
/// AnimState からアニメーション名へのマッピングも担う。
/// </summary>
class AnimationManager
{
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    AnimationManager();

    /// <summary>
    /// デストラクタ（管理中の全モデルからアニメーションをデタッチする）
    /// </summary>
    ~AnimationManager();

    /// <summary>
    /// 毎フレームのアニメーション時間更新処理
    /// </summary>
    /// <param name="modelHandle">更新するモデルのハンドル</param>
    /// <param name="delta">前フレームからの経過時間（秒）</param>
    void Update(int modelHandle, float delta);

    /// <summary>
    /// モデルにアニメーションをアタッチし、先頭から再生を開始する
    /// </summary>
    /// <param name="modelHandle">アニメーションを適用するモデルのハンドル</param>
    /// <param name="animName">再生するアニメーションの名前</param>
    /// <param name="loop">ループ再生するかどうか</param>
    /// <returns>アニメーションの総時間。アニメーションが見つからない場合は 0</returns>
    float PlayAnimation(int modelHandle, const std::string& animName, bool loop);

    /// <summary>
    /// アニメーションの再生時間を直接指定する
    /// </summary>
    /// <param name="modelHandle">アニメーションを更新するモデルのハンドル</param>
    /// <param name="animTime">設定するアニメーション再生時間</param>
    void UpdateAnimationTime(int modelHandle, float animTime);

    /// <summary>
    /// 指定したアニメーションの総時間を取得する
    /// </summary>
    /// <param name="modelHandle">アニメーションを取得するモデルのハンドル</param>
    /// <param name="animName">アニメーションの名前</param>
    /// <returns>アニメーションの総時間。見つからない場合は 0</returns>
    float GetAnimationTotalTime(int modelHandle, const std::string& animName) const;

    /// <summary>
    /// 現在アタッチされているアニメーションのハンドルを取得する
    /// </summary>
    /// <param name="modelHandle">アニメーションを取得するモデルのハンドル</param>
    /// <returns>現在アタッチされているアニメーションのハンドル。見つからない場合は -1</returns>
    int GetCurrentAttachedAnimHandle(int modelHandle) const;

    /// <summary>
    /// 現在アタッチされているアニメーションのハンドルを -1 にリセットする
    /// </summary>
    /// <param name="modelHandle">アニメーションをリセットするモデルのハンドル</param>
    void ResetAttachedAnimHandle(int modelHandle);

    /// <summary>
    /// 指定した AnimState に対応するアニメーション名を登録する
    /// </summary>
    /// <param name="state">アニメーションの状態</param>
    /// <param name="animName">対応するアニメーションの名前</param>
    void SetAnimName(EnemyBase::AnimState state, const std::string& animName);

    /// <summary>
    /// AnimState に対応するアニメーションを再生する
    /// </summary>
    /// <param name="modelHandle">モデルのハンドル</param>
    /// <param name="state">再生するアニメーションの状態</param>
    /// <param name="loop">ループ再生するかどうか</param>
    /// <returns>アニメーションの総時間。アニメーションが見つからない場合は 0</returns>
    float PlayState(int modelHandle, EnemyBase::AnimState state, bool loop);

    /// <summary>
    /// 指定したモデルの現在のアニメーションが終了しているかどうかを判定する
    /// </summary>
    /// <param name="modelHandle">モデルのハンドル</param>
    /// <returns>アニメーションが終了していれば true</returns>
    bool IsAnimationFinished(int modelHandle) const;

private:
    /// <summary>
    /// モデルハンドルとアニメーション名から DxLib のアニメーションインデックスを取得する（キャッシュ付き）
    /// </summary>
    /// <param name="modelHandle">モデルのハンドル</param>
    /// <param name="animName">アニメーションの名前</param>
    /// <returns>DxLib のアニメーションインデックス。見つからない場合は -1</returns>
    int GetAnimIndexInternal(int modelHandle, const std::string& animName);

private:
    std::map<int, std::map<std::string, int>> m_animIndexesCache;     // モデルハンドルとアニメーション名ごとの DxLib アニメーションインデックスのキャッシュ
    std::map<int, int>                        m_attachedAnimHandles;   // モデルハンドルと現在アタッチされている DxLib アニメーションのアタッチインデックス
    std::map<int, float>                      m_currentAnimTotalTimes; // モデルハンドルと現在アタッチされているアニメーションの総時間

    std::map<EnemyBase::AnimState, std::string> m_animStateToAnimName; // AnimState → アニメーション名のマッピング
    std::map<int, EnemyBase::AnimState>         m_modelCurrentState;   // モデルごとの現在の AnimState
    std::map<int, float>                        m_modelAnimTime;       // モデルごとの現在のアニメーション経過時間
};
