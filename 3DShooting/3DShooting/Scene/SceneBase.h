#pragma once

/// <summary>
/// シーン基底クラス
/// </summary>
class SceneBase
{
public:
  SceneBase() = default;
  virtual ~SceneBase() = default;

  virtual void Init() = 0;
  virtual SceneBase *Update() = 0;
  virtual void Draw() = 0;

  /// <summary>
  /// ロード中かどうかを取得する
  /// </summary>
  /// <returns>ロード中ならtrue</returns>
  virtual bool IsLoading() const { return false; }
};
