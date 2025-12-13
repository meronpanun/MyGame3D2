using UnityEngine;

public class SpawnArea : MonoBehaviour
{
    public enum SpawnAreaType
    {
        Main,
        Tutorial
    }

    [Tooltip("スポーンエリアの種類 (Main: メインステージ用, Tutorial: チュートリアル用)")]
    public SpawnAreaType Type = SpawnAreaType.Main;

    // シーンビューで範囲を可視化
    private void OnDrawGizmos()
    {
        // タイプによって色を変える
        if (Type == SpawnAreaType.Main)
            Gizmos.color = new Color(0, 0, 1, 0.3f); // Blue for Main
        else
            Gizmos.color = new Color(1, 1, 0, 0.3f); // Yellow for Tutorial

        Gizmos.matrix = transform.localToWorldMatrix;
        Gizmos.DrawCube(Vector3.zero, Vector3.one);
        Gizmos.DrawWireCube(Vector3.zero, Vector3.one);
        
        // 向きを表示
        Gizmos.color = Color.white;
        Gizmos.DrawLine(Vector3.zero, Vector3.forward * 0.5f);
    }
}
