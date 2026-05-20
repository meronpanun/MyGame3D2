#pragma once
#include <memory>

class SceneBase;

/// <summary>
/// シーン管理クラス。フェードイン・アウトを伴うシーン遷移を制御する。
/// 具体的なシーンクラスへの依存を持たないため、新しいシーンを追加しても
/// 本クラスの変更が不要な設計となっている（OCP 準拠）。
/// シーンの所有権は unique_ptr で管理し、生ポインタのリークを防ぐ。
/// </summary>
class SceneManager
{
public:
    /// <summary>
    /// フェード状態を表す列挙体
    /// </summary>
    enum class FadeState
    {
        Idle,      // 待機中（フェードなし）
        FadingOut, // フェードアウト中
        FadingIn,  // フェードイン中
        Loading,   // ロード中
    };

    SceneManager();

    /// <summary>
    /// デストラクタ（サウンドリソースを解放する。シーンは unique_ptr が自動解放）
    /// </summary>
    ~SceneManager();

    /// <summary>
    /// 初期化処理（サウンドロード・初期シーン生成など）
    /// </summary>
    void Init();

    /// <summary>
    /// 毎フレームの更新処理（フェード・シーン遷移・入力を処理する）
    /// </summary>
    void Update();

    /// <summary>
    /// 描画処理（現在シーンの描画・フェード膜の合成を行う）
    /// </summary>
    void Draw();

    /// <summary>
    /// シーンを変更するリクエストを行う
    /// </summary>
    /// <param name="newScene">新しいシーンのポインタ（SceneManager が所有権を引き受ける）</param>
    void RequestChangeScene(SceneBase* newScene);

    /// <summary>
    /// 現在のシーンを取得する
    /// </summary>
    /// <returns>現在のシーンのポインタ（非所有）</returns>
    SceneBase* GetCurrentScene() const { return m_pCurrentScene.get(); }

private:
    // シーン遷移管理
    std::unique_ptr<SceneBase> m_pCurrentScene;  // 現在実行中のシーン（所有）
    SceneBase*                 m_pNextScene;      // Update() が返す次シーン候補（非所有・観察のみ）
    std::unique_ptr<SceneBase> m_pSceneToChange;  // フェードアウト完了後に切り替えるシーン（所有）

    // ローディング演出
    int m_loadingDotCount;  // ローディングドットのアニメーションカウンタ
    int m_loadingAnimTimer; // ローディングアニメーションのタイマー

    // フェード処理
    FadeState m_fadeState; // 現在のフェード状態
    int       m_fadeAlpha; // フェードのアルファ値（0〜255）
    int       m_fadeSpeed; // フェードの速度（フレームあたりのアルファ変化量）
};
