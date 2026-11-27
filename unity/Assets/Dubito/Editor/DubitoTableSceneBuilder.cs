using System.Linq;
using Dubito.Presentation;
using UnityEditor;
using UnityEditor.SceneManagement;
using UnityEngine;
using UnityEngine.Rendering;
using UnityEngine.Rendering.Universal;

namespace Dubito.EditorTools
{
    // ROADMAP 2.1: builds the Table scene (room, light, table, seats placed by TableSeating, seated camera).
    // Idempotent: re-running rewrites Table.unity and the volume profile in place.
    public static class DubitoTableSceneBuilder
    {
        const string ScenePath = DubitoAssetBuilder.ScenesFolder + "/Table.unity";
        const string VolumeProfilePath = DubitoAssetBuilder.ScenesFolder + "/TableVolumeProfile.asset";
        const int PreviewSeatCount = 4;

        [MenuItem("Dubito/Build Table Scene")]
        public static void Build()
        {
            if (!DubitoAssetBuilder.NoDirtyScenes()) return;

            var tablePrefab = LoadPrefab("PF_Table");
            var chairPrefab = LoadPrefab("PF_Chair");
            var lampPrefab = LoadPrefab("PF_Lamp");
            var opponentPrefab = LoadPrefab("PF_Opponent");
            if (tablePrefab == null || chairPrefab == null || lampPrefab == null || opponentPrefab == null)
            {
                Debug.LogError("[Dubito] Prefabs missing: run Dubito/Build Phase 2 Assets first.");
                return;
            }

            var scene = EditorSceneManager.NewScene(NewSceneSetup.EmptyScene, NewSceneMode.Single);

            DubitoAssetBuilder.BuildRoom();

            var lighting = new GameObject("Lighting").transform;
            DubitoAssetBuilder.ApplyRoomLighting(lighting);
            DubitoAssetBuilder.Spawn(lampPrefab, "Lamp", new Vector3(0f, DubitoAssetBuilder.LampHangHeight, 0f),
                Quaternion.identity, lighting);
            BuildVolume(lighting);

            DubitoAssetBuilder.Spawn(tablePrefab, "Table", Vector3.zero, Quaternion.identity);
            var pile = new GameObject("Pile").transform;
            pile.position = new Vector3(0f, SeatLayout.TableHeight, 0f);

            var seats = new GameObject("Seats").transform;

            var camGo = new GameObject("SeatCamera");
            camGo.tag = "MainCamera";
            var cam = camGo.AddComponent<Camera>();
            cam.clearFlags = CameraClearFlags.SolidColor;
            cam.backgroundColor = new Color(0.02f, 0.02f, 0.025f);
            cam.nearClipPlane = 0.05f;
            cam.fieldOfView = 60f;
            camGo.AddComponent<AudioListener>();
            var camData = camGo.GetComponent<UniversalAdditionalCameraData>() ?? camGo.AddComponent<UniversalAdditionalCameraData>();
            camData.renderPostProcessing = true;
            var look = camGo.AddComponent<SeatedCameraController>();

            var seatingGo = new GameObject("TableSeating");
            var seating = seatingGo.AddComponent<TableSeating>();
            var so = new SerializedObject(seating);
            so.FindProperty("seatCount").intValue = PreviewSeatCount;
            so.FindProperty("localSeat").intValue = 0;
            so.FindProperty("chairPrefab").objectReferenceValue = chairPrefab;
            so.FindProperty("opponentPrefab").objectReferenceValue = opponentPrefab;
            so.FindProperty("seatsRoot").objectReferenceValue = seats;
            so.FindProperty("seatCamera").objectReferenceValue = look;
            so.FindProperty("debugSeatHotkeys").boolValue = false; // LocalTableController owns keys 2-6 (new game)
            so.ApplyModifiedPropertiesWithoutUndo();
            seating.Build(PreviewSeatCount, 0); // edit-time preview; rebuilt in Awake when playing

            // Step 2.2: local session -> PlayerView -> 3D cards + basic HUD.
            var game = new GameObject("LocalGame");
            var cardsView = game.AddComponent<TableCardsView>();
            Assign(cardsView, ("cardPrefab", LoadPrefab("PF_Card")),
                ("materials", AssetDatabase.LoadAssetAtPath<CardMaterialLibrary>("Assets/Dubito/Art/Cards/CardMaterialLibrary.asset")),
                ("seating", seating), ("pileAnchor", pile));
            var hud = game.AddComponent<TableHud>();
            Assign(hud, ("seating", seating), ("viewCamera", cam),
                ("font", AssetDatabase.LoadAssetAtPath<TMPro.TMP_FontAsset>("Assets/TextMesh Pro/Resources/Fonts & Materials/LiberationSans SDF.asset")));
            var controller = game.AddComponent<LocalTableController>();
            Assign(controller, ("seating", seating), ("cards", cardsView), ("hud", hud));

            EditorSceneManager.SaveScene(scene, ScenePath);
            AddToBuildSettings(ScenePath);
            Debug.Log("[Dubito] Table scene built.");
        }

        // Light post-processing (skill dubito-unity-scene): neutral tonemapping and a soft vignette.
        static void BuildVolume(Transform parent)
        {
            var profile = AssetDatabase.LoadAssetAtPath<VolumeProfile>(VolumeProfilePath);
            if (profile == null)
            {
                profile = ScriptableObject.CreateInstance<VolumeProfile>();
                AssetDatabase.CreateAsset(profile, VolumeProfilePath);
            }
            if (!profile.TryGet<Tonemapping>(out var tonemapping)) tonemapping = profile.Add<Tonemapping>(true);
            tonemapping.mode.Override(TonemappingMode.Neutral);
            if (!profile.TryGet<Vignette>(out var vignette)) vignette = profile.Add<Vignette>(true);
            vignette.intensity.Override(0.25f);
            vignette.smoothness.Override(0.5f);
            foreach (var c in profile.components)
            {
                if (!AssetDatabase.Contains(c)) AssetDatabase.AddObjectToAsset(c, profile);
                EditorUtility.SetDirty(c);
            }
            EditorUtility.SetDirty(profile);
            AssetDatabase.SaveAssets();

            var go = new GameObject("GlobalVolume");
            go.transform.SetParent(parent, false);
            var volume = go.AddComponent<Volume>();
            volume.isGlobal = true;
            volume.sharedProfile = profile;
        }

        static void AddToBuildSettings(string path)
        {
            var scenes = EditorBuildSettings.scenes.ToList();
            if (scenes.Any(s => s.path == path)) return;
            scenes.Insert(0, new EditorBuildSettingsScene(path, true));
            EditorBuildSettings.scenes = scenes.ToArray();
        }

        static void Assign(Object target, params (string field, Object value)[] values)
        {
            var so = new SerializedObject(target);
            foreach (var (field, value) in values)
            {
                if (value == null) Debug.LogError($"[Dubito] {target.GetType().Name}.{field}: missing asset");
                so.FindProperty(field).objectReferenceValue = value;
            }
            so.ApplyModifiedPropertiesWithoutUndo();
        }

        static GameObject LoadPrefab(string name) =>
            AssetDatabase.LoadAssetAtPath<GameObject>($"{DubitoAssetBuilder.PrefabsFolder}/{name}.prefab");
    }
}
