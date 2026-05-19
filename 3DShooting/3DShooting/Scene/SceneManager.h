#pragma once

class SceneTitle;
class SceneMain;
class SceneResult;
class SceneGameOver;
class SceneBase;

/// <summary>
/// シーン管理クラス。フェードイン・アウトを伴うシーン遷移を制御する
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
    /// デストラクタ（保持シーンとサウンドリソースを解放する）
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
    /// <param name="newScene">新しいシーンのポインタ</param>
    void RequestChangeScene(SceneBase* newScene);

    /// <summary>
    /// 現在のシーンを取得する
    /// </summary>
    /// <returns>現在のシーンのポインタ</returns>
    SceneBase* GetCurrentScene() const { return m_pCurrentScene; }

private:
    // シーン遷移管理
    SceneBase* m_pCurrentScene;  // 現在実行中のシーン
    SceneBase* m_pNextScene;     // 次フレームに遷移するシーン（Updateの戻り値）
    SceneBase* m_pSceneToChange; // フェードアウト完了後に切り替えるシーン

    // 各シーンの所有ポインタ（二重解放防止用）
    SceneTitle*   m_pTitle;     // タイトルシーン
    SceneMain*    m_pSceneMain; // メインシーン
    SceneResult*  m_pResult;    // リザルトシーン
    SceneGameOver* m_pGameOver; // ゲームオーバーシーン

    // ローディング演出
    int m_loadingDotCount;  // ローディングドットのアニメーションカウンタ
    int m_loadingAnimTimer; // ローディングアニメーションのタイマー

    bool m_isExternalSceneChange; // 外部からのシーン変更要求フラグ

    // フェード処理
    FadeState m_fadeState; // 現在のフェード状態
    int m_fadeAlpha;       // フェードのアルファ値（0〜255）
    int m_fadeSpeed;       // フェードの速度（フレームあたりのアルファ変化量）
};
