#pragma once
#include "AttackType.h"

class WaveManager;
class Player;

/// <summary>
/// タスク型チュートリアルマネージャークラス
/// </summary>
class TaskTutorialManager {
public:
  TaskTutorialManager();
  ~TaskTutorialManager();

  void Init(WaveManager *pWaveManager, Player *pPlayer);
  void Update();
  void Draw();

  // シングルトンインスタンスを取得
  static TaskTutorialManager *GetInstance();

  // 敵が倒されたことを通知する
  void NotifyEnemyKilled(AttackType attackType);

  // チュートリアルが完了したか
  bool IsCompleted() const;

  // 状態をリセットする
  void Reset();

  // チュートリアルをスキップする
  void Skip(WaveManager *pWaveManager);

private:
  // チュートリアルの進行ステップ
  enum class TaskStep {
    None,
    Shoot,               // 射撃タスク
    ShootCompleteDelay,  // 射撃タスク完了後の待機
    Tackle,              // タックルタスク
    TackleCompleteDelay, // タックルタスク完了後の待機
    Completed            // 全て完了
  };
  TaskTutorialManager(const TaskTutorialManager &) = delete;
  TaskTutorialManager &operator=(const TaskTutorialManager &) = delete;

  static TaskTutorialManager *m_instance;

  WaveManager *m_pWaveManager; // WaveManagerへのポインタ
  Player *m_pPlayer;           // Playerへのポインタ

  TaskStep m_step;
  int m_shootKills;
  int m_tackleKills;

  int m_titleFontHandle; // タイトル用のフォントハンドル
  int m_taskFontHandle;  // タスク内容用のフォントハンドル
  int m_diamondImg;
  int m_mouseLeftImg;
  int m_mouseRightImg;

  // 武器切り替えヒント用画像
  int m_alpha1Img;     // キーボード1キーの画像
  int m_alpha2Img;     // キーボード2キーの画像
  int m_mouseWheelImg; // マウスホイールの画像

  // タイトルアニメーション用
  float m_titlePosX;
  float m_titleAnimSpeed;
  bool m_isTitleAnimFinished;

  // タスク内容フェードイン用
  int m_taskAlpha;
  float m_taskFadeSpeed;

  // アニメーション後の待機タイマー
  int m_animationWaitTimer;

  // プログレスバーアニメーション用
  float m_displayedShootProgress;  // 射撃タスクの表示用進捗
  float m_displayedTackleProgress; // タックルタスクの表示用進捗
  float m_progressAnimSpeed;       // 進捗バーのアニメーション速度

  // ステップ移行の遅延タイマー
  int m_transitionDelayTimer;
};