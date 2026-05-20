#pragma once

class SceneBase;

/// <summary>
/// シーン管理クラス。フェードイン・アウトを伴うシーン遷移を制御する。
/// 具体的なシーンクラスへの依存を持たないため、新しいシーンを追加しても
/// 本クラスの変更が不要な設計となっている（OCP 準拠）。
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

    /// <summary>
    /// コンストラクタ
    /// </summary>
    SceneManager();

    /// <summary>
    /// デストラクタ（現在シーン・遷移先シーン・サウンドリソースを解放する）
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
    /// <returns>現在のシーンのポインタ</returns>
    SceneBase* GetCurrentScene() const { return m_pCurrentScene; }

private:
    // シーン遷移管理
    SceneBase* m_pCurrentScene;  // 現在実行中のシーン（所有）
    SceneBase* m_pNextScene;     // 次フレームに遷移するシーン（Update の戻り値、非所有）
    SceneBase* m_pSceneToChange; // フェードアウト完了後に切り替えるシーン（所有）

    // ローディング演出
    int m_loadingDotCount;  // ローディングドットのアニメーションカウンタ
    int m_loadingAnimTimer; // ローディングアニメーションのタイマー

    bool m_isExternalSceneChange; // 外部からのシーン変更要求フラグ

    // フェード処理
    FadeState m_fadeState; // 現在のフェード状態
    int m_fadeAlpha;       // フェードのアルファ値（0〜255）
    int m_fadeSpeed;       // フェードの速度（フレームあたりのアルファ変化量）
};
