using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using Dubito.Core;
using Dubito.Presentation;
using UnityEditor;
using UnityEditor.Animations;
using UnityEditor.SceneManagement;
using UnityEngine;
using UnityEngine.Rendering;
using UnityEngine.SceneManagement;

namespace Dubito.EditorTools
{
    // ROADMAP 2.0: builds URP materials, prefabs and the AssetReview scene from the imported art.
    // Idempotent: re-running updates the same assets (same GUIDs) instead of creating new ones.
    // Dimensions: docs/ASSETS.md §2.
    public static class DubitoAssetBuilder
    {
        const string Art = DubitoArtPostprocessor.ArtRoot;
        const string MaterialsFolder = Art + "Materials";
        const string CardMaterialsFolder = Art + "Cards/Materials";
        const string PrefabsFolder = "Assets/Dubito/Prefabs";
        const string ScenesFolder = "Assets/Dubito/Scenes";
        const string ReviewScenePath = ScenesFolder + "/AssetReview.unity";
        const string ControllerPath = DubitoArtPostprocessor.MannequinFolder + "AC_SeatedOpponent.controller";
        const string AnimationsFolder = DubitoArtPostprocessor.MannequinFolder + "Animations";

        const string CardModelPath = Art + "Cards/Model/SM_Card.fbx";
        const string TableModelPath = Art + "Props/Table/SM_Table.fbx";
        const string ChairModelPath = Art + "Props/Chair/SM_Chair.fbx";
        const string LampModelPath = Art + "Props/Lamp/SM_Lamp.fbx";

        // ASSETS §2
        const float TableHeight = 0.75f;
        const float PelvisRadius = 1.05f;
        const float EyeHeight = 1.15f;
        const float SeatHeight = 0.45f;
        // Hip joint sits about 0.1 m above the seat surface, buttocks against the backrest.
        const float HipHeight = SeatHeight + 0.10f;
        const float ChairCenterRadius = 1.12f;
        const float LampHangHeight = 2.9f; // cable top; shade bottom 1.25 m lower = 0.9 m above the table

        // Placeholder seat colours (greybox, UX §2: each seat has a colour).
        static readonly Color[] SeatColors =
        {
            Hex("#3B82F6"), Hex("#E5484D"), Hex("#30A46C"),
            Hex("#F5A524"), Hex("#8E4EC6"), Hex("#12A594"),
        };

        static readonly (string state, string clip, OpponentReaction? reaction)[] OpponentStates =
        {
            ("Idle", "Sitting Idle", null),
            ("Talking", "Sitting Talking", OpponentReaction.Talking),
            ("Disapproval", "Sitting Disapproval", OpponentReaction.Disapproval),
            ("RubbingArm", "Sitting Rubbing Arm", OpponentReaction.RubbingArm),
            ("Angry", "Sitting Angry", OpponentReaction.Angry),
            ("Disbelief", "Sitting Disbelief", OpponentReaction.Disbelief),
            ("ThumbsUp", "Sitting Thumbs Up", OpponentReaction.ThumbsUp),
            ("Victory", "Sitting Victory", OpponentReaction.Victory),
        };

        [MenuItem("Dubito/Build Phase 2 Assets")]
        public static void BuildAll()
        {
            EnsureFolder(MaterialsFolder);
            EnsureFolder(CardMaterialsFolder);
            EnsureFolder(PrefabsFolder);
            EnsureFolder(ScenesFolder);

            var mats = BuildMaterials();
            RemapModelMaterials(TableModelPath, mats);
            RemapModelMaterials(ChairModelPath, mats);
            RemapModelMaterials(LampModelPath, mats);
            RemapModelMaterials(CardModelPath, mats);
            RemapModelMaterials(DubitoArtPostprocessor.MannequinModelPath, mats);
            var library = BuildCardLibrary();

            LinkAnimationAvatars();
            var controller = BuildAnimatorController();

            var table = SimplePrefab("PF_Table", TableModelPath);
            var chair = SimplePrefab("PF_Chair", ChairModelPath);
            var lamp = BuildLampPrefab();
            var card = SimplePrefab("PF_Card", CardModelPath);
            var opponent = BuildOpponentPrefab(controller);

            AssetDatabase.SaveAssets();
            BuildReviewScene(table, chair, lamp, card, opponent, library);
            Debug.Log("[Dubito] Phase 2 assets built.");
        }

        // ---------- Materials ----------

        static Dictionary<string, Material> BuildMaterials()
        {
            var m = new Dictionary<string, Material>
            {
                ["M_TableFelt"] = Lit("M_TableFelt", Hex("#1F5A3A"), 0.05f),
                ["M_TableWood"] = Lit("M_TableWood", Hex("#4A2E1C"), 0.35f),
                ["M_Wood"] = ChairWood(),
                ["M_LampShade"] = Lit("M_LampShade", Hex("#2A2A2A"), 0.5f, 0.6f),
                ["M_Mannequin"] = Lit("M_Mannequin", Hex("#B8BCC4"), 0.35f),
                ["M_MannequinJoints"] = Lit("M_MannequinJoints", Hex("#2E2E33"), 0.4f),
                ["M_RoomFloor"] = Lit("M_RoomFloor", Hex("#2B2622"), 0.15f),
                ["M_RoomWall"] = Lit("M_RoomWall", Hex("#1C1C21"), 0.05f),
            };
            var back = AssetDatabase.LoadAssetAtPath<Texture2D>(DubitoArtPostprocessor.CardFacesFolder + "Back.png");
            m["M_CardBack"] = Lit("M_CardBack", Color.white, 0.25f, 0f, back, CardMaterialsFolder);
            // Default face shown by the plain PF_Card; real faces come from CardMaterialLibrary.
            m["M_CardFace"] = FaceMaterial(CardRank.Ace, CardSuit.Spades);
            m["Alpha_Body_MAT"] = m["M_Mannequin"];
            m["Alpha_Joints_MAT"] = m["M_MannequinJoints"];
            return m;
        }

        // Baked oak textures from Blender when present, plain oak colour otherwise.
        static Material ChairWood()
        {
            const string folder = Art + "Props/Chair/";
            var baseColor = AssetDatabase.LoadAssetAtPath<Texture2D>(folder + "T_Chair_BaseColor.png");
            var normal = AssetDatabase.LoadAssetAtPath<Texture2D>(folder + "T_Chair_Normal.png");
            var mat = Lit("M_Wood", baseColor != null ? Color.white : Hex("#C08A55"), 0.3f, 0f, baseColor);
            mat.SetTexture("_BumpMap", normal);
            if (normal != null) mat.EnableKeyword("_NORMALMAP");
            else mat.DisableKeyword("_NORMALMAP");
            return mat;
        }

        static Material FaceMaterial(CardRank rank, CardSuit suit)
        {
            string name = $"{rank}_{suit}";
            var tex = AssetDatabase.LoadAssetAtPath<Texture2D>(DubitoArtPostprocessor.CardFacesFolder + name + ".png");
            if (tex == null) Debug.LogError($"[Dubito] Missing card face texture {name}.png");
            return Lit("M_Card_" + name, Color.white, 0.25f, 0f, tex, CardMaterialsFolder);
        }

        static CardMaterialLibrary BuildCardLibrary()
        {
            var faces = new Material[CardMaterialLibrary.FaceCount];
            foreach (CardRank rank in Enum.GetValues(typeof(CardRank)))
            foreach (CardSuit suit in Enum.GetValues(typeof(CardSuit)))
                faces[CardMaterialLibrary.IndexOf(rank, suit)] = FaceMaterial(rank, suit);

            string path = Art + "Cards/CardMaterialLibrary.asset";
            var lib = AssetDatabase.LoadAssetAtPath<CardMaterialLibrary>(path);
            if (lib == null)
            {
                lib = ScriptableObject.CreateInstance<CardMaterialLibrary>();
                AssetDatabase.CreateAsset(lib, path);
            }
            lib.EditorAssign(AssetDatabase.LoadAssetAtPath<Material>(CardMaterialsFolder + "/M_CardBack.mat"), faces);
            EditorUtility.SetDirty(lib);
            return lib;
        }

        static Material Lit(string name, Color color, float smoothness, float metallic = 0f,
            Texture texture = null, string folder = MaterialsFolder)
        {
            string path = $"{folder}/{name}.mat";
            var shader = Shader.Find("Universal Render Pipeline/Lit");
            var mat = AssetDatabase.LoadAssetAtPath<Material>(path);
            if (mat == null)
            {
                mat = new Material(shader);
                AssetDatabase.CreateAsset(mat, path);
            }
            else if (mat.shader != shader)
            {
                mat.shader = shader;
            }
            mat.SetColor("_BaseColor", color);
            mat.SetTexture("_BaseMap", texture);
            mat.SetFloat("_Smoothness", smoothness);
            mat.SetFloat("_Metallic", metallic);
            mat.enableInstancing = true;
            EditorUtility.SetDirty(mat);
            return mat;
        }

        static void RemapModelMaterials(string modelPath, Dictionary<string, Material> mats)
        {
            var importer = AssetImporter.GetAtPath(modelPath) as ModelImporter;
            if (importer == null)
            {
                Debug.LogWarning($"[Dubito] Model not found: {modelPath}");
                return;
            }
            bool changed = false;
            var existing = importer.GetExternalObjectMap();
            foreach (var source in AssetDatabase.LoadAllAssetsAtPath(modelPath).OfType<Material>().Select(x => x.name).ToList())
            {
                if (!mats.TryGetValue(source, out var target))
                {
                    Debug.LogWarning($"[Dubito] {Path.GetFileName(modelPath)}: no Unity material for '{source}'");
                    continue;
                }
                var id = new AssetImporter.SourceAssetIdentifier(typeof(Material), source);
                if (existing.TryGetValue(id, out var current) && current == target) continue;
                importer.AddRemap(id, target);
                changed = true;
            }
            if (changed) importer.SaveAndReimport();
        }

        // ---------- Mannequin ----------

        static void LinkAnimationAvatars()
        {
            var avatar = AssetDatabase.LoadAllAssetsAtPath(DubitoArtPostprocessor.MannequinModelPath).OfType<Avatar>().FirstOrDefault();
            if (avatar == null || !avatar.isHuman)
            {
                Debug.LogError("[Dubito] Y Bot humanoid avatar missing or invalid.");
                return;
            }
            foreach (var guid in AssetDatabase.FindAssets("t:Model", new[] { AnimationsFolder }))
            {
                var importer = (ModelImporter)AssetImporter.GetAtPath(AssetDatabase.GUIDToAssetPath(guid));
                if (importer.avatarSetup == ModelImporterAvatarSetup.CopyFromOther && importer.sourceAvatar == avatar) continue;
                importer.avatarSetup = ModelImporterAvatarSetup.CopyFromOther;
                importer.sourceAvatar = avatar;
                importer.SaveAndReimport();
            }
        }

        static AnimationClip LoadClip(string clipName)
        {
            string path = $"{AnimationsFolder}/Y Bot@{clipName}.fbx";
            var clip = AssetDatabase.LoadAllAssetsAtPath(path).OfType<AnimationClip>()
                .FirstOrDefault(c => !c.name.StartsWith("__preview__", StringComparison.Ordinal));
            if (clip == null) Debug.LogError($"[Dubito] Clip not found: {path}");
            return clip;
        }

        static AnimatorController BuildAnimatorController()
        {
            var controller = AssetDatabase.LoadAssetAtPath<AnimatorController>(ControllerPath)
                             ?? AnimatorController.CreateAnimatorControllerAtPath(ControllerPath);
            var sm = controller.layers[0].stateMachine;
            foreach (var t in sm.anyStateTransitions.ToArray()) sm.RemoveAnyStateTransition(t);
            foreach (var s in sm.states.ToArray()) sm.RemoveState(s.state);
            foreach (var p in controller.parameters.ToArray()) controller.RemoveParameter(p);
            controller.AddParameter("Reaction", AnimatorControllerParameterType.Int);
            controller.AddParameter("React", AnimatorControllerParameterType.Trigger);

            AnimatorState idle = null;
            for (int i = 0; i < OpponentStates.Length; i++)
            {
                var (stateName, clipName, reaction) = OpponentStates[i];
                var state = sm.AddState(stateName, new Vector3(300, 60 * i, 0));
                state.motion = LoadClip(clipName);
                if (reaction == null)
                {
                    idle = state;
                    sm.defaultState = state;
                    continue;
                }
                var enter = sm.AddAnyStateTransition(state);
                enter.AddCondition(AnimatorConditionMode.If, 0, "React");
                enter.AddCondition(AnimatorConditionMode.Equals, (int)reaction.Value, "Reaction");
                enter.hasExitTime = false;
                enter.duration = 0.25f;
                enter.canTransitionToSelf = false;
                var back = state.AddTransition(idle);
                back.hasExitTime = true;
                back.exitTime = 0.9f;
                back.duration = 0.3f;
            }
            EditorUtility.SetDirty(controller);
            return controller;
        }

        static GameObject BuildOpponentPrefab(AnimatorController controller)
        {
            var model = AssetDatabase.LoadAssetAtPath<GameObject>(DubitoArtPostprocessor.MannequinModelPath);
            var root = new GameObject("PF_Opponent");
            var body = (GameObject)PrefabUtility.InstantiatePrefab(model);
            body.name = "Body";
            body.transform.SetParent(root.transform, false);
            var animator = body.GetComponent<Animator>();
            animator.runtimeAnimatorController = controller;
            animator.applyRootMotion = false;
            animator.cullingMode = AnimatorCullingMode.CullUpdateTransforms;

            // Put the hips of the idle pose on the seat: root = floor point under the pelvis, facing +Z.
            var hips = SampleIdleBone(body, animator, HumanBodyBones.Hips, out var feetY);
            body.transform.localPosition = new Vector3(-hips.x, HipHeight - hips.y, -hips.z);
            Debug.Log($"[Dubito] Idle hips at {hips:F3}, lowest foot y {feetY:F3}; body offset {body.transform.localPosition:F3}");

            var fan = new GameObject("HandFan").transform;
            fan.SetParent(root.transform, false);
            fan.localPosition = new Vector3(0f, 0.85f, 0.32f);
            fan.localRotation = Quaternion.Euler(-60f, 0f, 0f);

            var renderers = body.GetComponentsInChildren<SkinnedMeshRenderer>()
                .Where(r => r.sharedMaterial != null && r.sharedMaterial.name == "M_Mannequin")
                .Cast<Renderer>().ToArray();
            var appearance = root.AddComponent<SeatedOpponentAppearance>();
            appearance.EditorSetup(animator, renderers);

            var prefab = PrefabUtility.SaveAsPrefabAsset(root, $"{PrefabsFolder}/PF_Opponent.prefab");
            UnityEngine.Object.DestroyImmediate(root);
            return prefab;
        }

        static Vector3 SampleIdleBone(GameObject body, Animator animator, HumanBodyBones bone, out float lowestFootY)
        {
            var clip = LoadClip("Sitting Idle");
            AnimationMode.StartAnimationMode();
            try
            {
                AnimationMode.BeginSampling();
                AnimationMode.SampleAnimationClip(body, clip, 0f);
                AnimationMode.EndSampling();
                var rootInv = body.transform.worldToLocalMatrix;
                Vector3 Local(HumanBodyBones b) => rootInv.MultiplyPoint3x4(animator.GetBoneTransform(b).position);
                lowestFootY = Mathf.Min(Local(HumanBodyBones.LeftToes).y, Local(HumanBodyBones.RightToes).y);
                return Local(bone);
            }
            finally
            {
                AnimationMode.StopAnimationMode();
            }
        }

        // ---------- Props ----------

        static GameObject SimplePrefab(string name, string modelPath)
        {
            var model = AssetDatabase.LoadAssetAtPath<GameObject>(modelPath);
            var root = new GameObject(name);
            var mesh = (GameObject)PrefabUtility.InstantiatePrefab(model);
            mesh.transform.SetParent(root.transform, false);
            var prefab = PrefabUtility.SaveAsPrefabAsset(root, $"{PrefabsFolder}/{name}.prefab");
            UnityEngine.Object.DestroyImmediate(root);
            return prefab;
        }

        static GameObject BuildLampPrefab()
        {
            var model = AssetDatabase.LoadAssetAtPath<GameObject>(LampModelPath);
            var root = new GameObject("PF_Lamp");
            var mesh = (GameObject)PrefabUtility.InstantiatePrefab(model);
            mesh.transform.SetParent(root.transform, false);
            foreach (var r in mesh.GetComponentsInChildren<Renderer>()) r.shadowCastingMode = ShadowCastingMode.Off;

            // Warm key light inside the shade: the table is the brightest place (ASSETS §1, D-45).
            var lightGo = new GameObject("KeyLight");
            lightGo.transform.SetParent(root.transform, false);
            lightGo.transform.localPosition = new Vector3(0f, -1.12f, 0f);
            lightGo.transform.localRotation = Quaternion.Euler(90f, 0f, 0f);
            var light = lightGo.AddComponent<Light>();
            light.type = LightType.Spot;
            light.color = new Color(1f, 0.86f, 0.68f);
            light.intensity = 6f;
            light.range = 4f;
            light.spotAngle = 120f;
            light.innerSpotAngle = 70f;
            light.shadows = LightShadows.Soft;

            var prefab = PrefabUtility.SaveAsPrefabAsset(root, $"{PrefabsFolder}/PF_Lamp.prefab");
            UnityEngine.Object.DestroyImmediate(root);
            return prefab;
        }

        // ---------- Review scene ----------

        static void BuildReviewScene(GameObject table, GameObject chair, GameObject lamp, GameObject card,
            GameObject opponent, CardMaterialLibrary library)
        {
            for (int i = 0; i < SceneManager.sceneCount; i++)
            {
                if (SceneManager.GetSceneAt(i).isDirty)
                {
                    Debug.LogError("[Dubito] An open scene has unsaved changes; save it before building AssetReview.");
                    return;
                }
            }

            var scene = EditorSceneManager.NewScene(NewSceneSetup.EmptyScene, NewSceneMode.Single);
            RenderSettings.skybox = null;
            RenderSettings.ambientMode = AmbientMode.Flat;
            // Owner review 2026-11-27: "trop sombre". Readable room, the table stays the brightest place.
            RenderSettings.ambientLight = new Color(0.32f, 0.31f, 0.33f);
            RenderSettings.defaultReflectionMode = DefaultReflectionMode.Custom;

            var fill = new GameObject("FillLight").AddComponent<Light>();
            fill.type = LightType.Directional;
            fill.color = new Color(0.85f, 0.88f, 1f);
            fill.intensity = 0.35f;
            fill.shadows = LightShadows.None;
            fill.transform.rotation = Quaternion.Euler(50f, 30f, 0f);

            BuildRoom();
            Spawn(table, "Table", Vector3.zero, Quaternion.identity);
            Spawn(lamp, "Lamp", new Vector3(0f, LampHangHeight, 0f), Quaternion.identity);

            var seats = new GameObject("Seats").transform;
            for (int i = 0; i < 6; i++)
            {
                var facing = Quaternion.Euler(0f, i * 60f, 0f);
                var seat = new GameObject($"Seat_{i}").transform;
                seat.SetParent(seats, false);
                seat.SetPositionAndRotation(facing * new Vector3(0f, 0f, -PelvisRadius), facing);
                Spawn(chair, "Chair", facing * new Vector3(0f, 0f, -ChairCenterRadius), facing, seat);
                if (i == 0) continue; // seat 0 = the camera (the local player)
                var opp = Spawn(opponent, "Opponent", seat.position, facing, seat);
                opp.GetComponent<SeatedOpponentAppearance>().SeatColor = SeatColors[i];
            }

            // Cards: a row of faces in front of seat 0 and a small face-down pile in the centre.
            var cards = new GameObject("Cards").transform;
            var ranks = (CardRank[])Enum.GetValues(typeof(CardRank));
            for (int i = 0; i < ranks.Length; i++)
            {
                var suit = (CardSuit)(i % 4);
                float x = (i - (ranks.Length - 1) / 2f) * 0.07f;
                var c = Spawn(card, $"{ranks[i]}_{suit}", new Vector3(x, TableHeight + 0.0005f, -0.52f),
                    Quaternion.identity, cards);
                SetFace(c, library.Face(new Card(ranks[i], suit)));
            }
            for (int i = 0; i < 5; i++)
            {
                Spawn(card, $"Pile_{i}", new Vector3(0f, TableHeight + 0.0005f + i * 0.0009f, 0f),
                    Quaternion.Euler(0f, i * 23f, 180f), cards);
            }

            var camGo = new GameObject("SeatCamera");
            camGo.tag = "MainCamera";
            camGo.transform.position = new Vector3(0f, EyeHeight, -PelvisRadius);
            camGo.transform.LookAt(new Vector3(0f, TableHeight, 0f));
            var cam = camGo.AddComponent<Camera>();
            cam.clearFlags = CameraClearFlags.SolidColor;
            cam.backgroundColor = new Color(0.02f, 0.02f, 0.025f);
            cam.nearClipPlane = 0.05f;
            cam.fieldOfView = 60f;

            var overview = new GameObject("OverviewCamera");
            overview.transform.position = new Vector3(2.2f, 2.1f, -2.2f);
            overview.transform.LookAt(new Vector3(0f, 0.6f, 0f));
            var ocam = overview.AddComponent<Camera>();
            ocam.clearFlags = CameraClearFlags.SolidColor;
            ocam.backgroundColor = cam.backgroundColor;
            ocam.enabled = false;

            EditorSceneManager.SaveScene(scene, ReviewScenePath);
        }

        static void SetFace(GameObject cardInstance, Material face)
        {
            foreach (var r in cardInstance.GetComponentsInChildren<MeshRenderer>())
            {
                var shared = r.sharedMaterials;
                for (int i = 0; i < shared.Length; i++)
                    if (shared[i] != null && shared[i].name.StartsWith("M_Card_", StringComparison.Ordinal))
                        shared[i] = face;
                r.sharedMaterials = shared;
            }
        }

        static void BuildRoom()
        {
            var room = new GameObject("Room").transform;
            var floorMat = AssetDatabase.LoadAssetAtPath<Material>(MaterialsFolder + "/M_RoomFloor.mat");
            var wallMat = AssetDatabase.LoadAssetAtPath<Material>(MaterialsFolder + "/M_RoomWall.mat");
            Box(room, "Floor", new Vector3(0f, -0.05f, 0f), new Vector3(6f, 0.1f, 6f), floorMat);
            Box(room, "Ceiling", new Vector3(0f, 3.05f, 0f), new Vector3(6f, 0.1f, 6f), wallMat);
            Box(room, "Wall_N", new Vector3(0f, 1.5f, 3.05f), new Vector3(6.2f, 3f, 0.1f), wallMat);
            Box(room, "Wall_S", new Vector3(0f, 1.5f, -3.05f), new Vector3(6.2f, 3f, 0.1f), wallMat);
            Box(room, "Wall_E", new Vector3(3.05f, 1.5f, 0f), new Vector3(0.1f, 3f, 6f), wallMat);
            Box(room, "Wall_W", new Vector3(-3.05f, 1.5f, 0f), new Vector3(0.1f, 3f, 6f), wallMat);
        }

        static void Box(Transform parent, string name, Vector3 pos, Vector3 size, Material mat)
        {
            var go = GameObject.CreatePrimitive(PrimitiveType.Cube);
            go.name = name;
            go.transform.SetParent(parent, false);
            go.transform.localPosition = pos;
            go.transform.localScale = size;
            go.GetComponent<MeshRenderer>().sharedMaterial = mat;
            go.isStatic = true;
        }

        static GameObject Spawn(GameObject prefab, string name, Vector3 pos, Quaternion rot, Transform parent = null)
        {
            var go = (GameObject)PrefabUtility.InstantiatePrefab(prefab);
            go.name = name;
            if (parent != null) go.transform.SetParent(parent, false);
            go.transform.SetPositionAndRotation(pos, rot);
            return go;
        }

        // ---------- Helpers ----------

        static void EnsureFolder(string path)
        {
            if (AssetDatabase.IsValidFolder(path)) return;
            string parent = Path.GetDirectoryName(path).Replace('\\', '/');
            EnsureFolder(parent);
            AssetDatabase.CreateFolder(parent, Path.GetFileName(path));
        }

        static Color Hex(string hex) => ColorUtility.TryParseHtmlString(hex, out var c) ? c : Color.magenta;
    }
}
