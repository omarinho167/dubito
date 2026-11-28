using System.Collections.Generic;
using System.Linq;
using UnityEditor;
using UnityEngine;

namespace Dubito.EditorTools
{
    // Maps the judge's modelled finger bones (D-86) to the Humanoid fingers, keeping the body mapping
    // already stored in CHR_Juge.fbx.meta (Spine02 -> Spine, Spine01 -> Chest, Spine -> UpperChest).
    // Run again after re-exporting CHR_Juge.fbx with new or renamed finger bones.
    public static class DubitoJudgeAvatar
    {
        static readonly string[] Sides = { "Left", "Right" };
        // Blender bone name part -> Humanoid finger name ("Little" is Unity's name for the pinky).
        static readonly (string bone, string human)[] Fingers =
            { ("Thumb", "Thumb"), ("Index", "Index"), ("Middle", "Middle"), ("Ring", "Ring"), ("Pinky", "Little") };
        static readonly string[] Phalanges = { "Proximal", "Intermediate", "Distal" };

        public static IEnumerable<(string bone, string human)> FingerMap()
        {
            foreach (var side in Sides)
                foreach (var (bone, human) in Fingers)
                    for (int k = 0; k < 3; k++)
                        yield return ($"{side}Hand{bone}{k + 1}", $"{side} {human} {Phalanges[k]}");
        }

        [MenuItem("Dubito/Map Judge Fingers")]
        public static void MapFingers()
        {
            string path = DubitoArtPostprocessor.JudgeModelPath;
            var importer = (ModelImporter)AssetImporter.GetAtPath(path);
            var model = AssetDatabase.LoadAssetAtPath<GameObject>(path);
            var map = FingerMap().ToList();
            var description = importer.humanDescription;

            var human = description.human.Where(h => map.All(m => m.human != h.humanName)).ToList();
            foreach (var (bone, humanName) in map)
                human.Add(new HumanBone { boneName = bone, humanName = humanName, limit = new HumanLimit { useDefaultValues = true } });
            description.human = human.ToArray();

            // The T-pose of the new bones comes from the file's bind pose; existing entries are kept.
            var instance = Object.Instantiate(model);
            try
            {
                var skeleton = description.skeleton.ToList();
                var known = new HashSet<string>(skeleton.Select(s => s.name));
                foreach (var t in instance.GetComponentsInChildren<Transform>(true))
                {
                    if (known.Contains(t.name)) continue;
                    skeleton.Add(new SkeletonBone
                    {
                        name = t.name, position = t.localPosition, rotation = t.localRotation, scale = t.localScale
                    });
                }
                description.skeleton = skeleton.ToArray();
            }
            finally
            {
                Object.DestroyImmediate(instance);
            }

            importer.humanDescription = description;
            importer.SaveAndReimport();
            var avatar = AssetDatabase.LoadAllAssetsAtPath(path).OfType<Avatar>().FirstOrDefault();
            if (avatar == null || !avatar.isValid || !avatar.isHuman)
                Debug.LogError("[Dubito] Judge avatar invalid after mapping the fingers.");
            else
                Debug.Log($"[Dubito] Judge avatar valid, {avatar.humanDescription.human.Length} human bones mapped.");
        }
    }
}
