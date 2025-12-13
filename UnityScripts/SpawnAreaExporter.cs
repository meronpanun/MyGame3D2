#if UNITY_EDITOR
using UnityEngine;
using UnityEditor;
using System.IO;
using System.Text;

public class SpawnAreaExporter : EditorWindow
{
    [MenuItem("Tools/Export Spawn Data")]
    public static void ExportToCSV()
    {
        // シーン内の全てのSpawnAreaコンポーネントを取得
        SpawnArea[] areas = FindObjectsOfType<SpawnArea>();

        if (areas.Length == 0)
        {
            Debug.LogWarning("No SpawnArea objects found!");
            return;
        }

        StringBuilder sb = new StringBuilder();
        // ヘッダー行
        sb.AppendLine("Type,PosX,PosY,PosZ,ScaleX,ScaleY,ScaleZ");

        foreach (var area in areas)
        {
            Transform t = area.transform;
            
            // 座標とスケールを取得
            // Unityの座標系そのまま出力し、C++側で100倍する想定
            Vector3 pos = t.position;
            Vector3 scale = t.lossyScale;

            // Typeをintとして出力 (0: Main, 1: Tutorial)
            sb.Append($"{(int)area.Type},");
            sb.Append($"{pos.x},{pos.y},{pos.z},");
            sb.Append($"{scale.x},{scale.y},{scale.z}");
            sb.AppendLine();
        }

        // 保存先ダイアログを開く
        string path = EditorUtility.SaveFilePanel("Save Spawn Data CSV", "", "SpawnAreaData", "csv");
        
        if (!string.IsNullOrEmpty(path))
        {
            File.WriteAllText(path, sb.ToString(), Encoding.UTF8);
            Debug.Log($"Spawn data exported to: {path}");
            
            // ファイルを開いて確認（オプション）
            // System.Diagnostics.Process.Start(path);
        }
    }
}
#endif
