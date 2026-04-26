#pragma once

class EnemyNormal;
struct EnemyUpdateContext;

/// <summary>
/// EnemyNormalの状態を管理するステートパターンの基底クラス
/// </summary>
class EnemyState
{
public:
    virtual ~EnemyState() = default;

    /// <summary>
    /// 状態に遷移した時に一度だけ呼ばれる処理
    /// </summary>
    virtual void Enter(EnemyNormal* enemy) {}

    /// <summary>
    /// 毎フレーム呼ばれる更新処理
    /// </summary>
    virtual void Update(EnemyNormal* enemy, const EnemyUpdateContext& context) = 0;

    /// <summary>
    /// 他の状態へ遷移する直前に呼ばれる処理
    /// </summary>
    virtual void Exit(EnemyNormal* enemy) {}
};
