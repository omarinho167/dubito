using Dubito.Presentation;
using NUnit.Framework;
using UnityEditor;
using UnityEditor.Animations;
using System.Linq;
using UnityEngine;

namespace Dubito.Tests
{
    public sealed class OpponentPrefabTests
    {
        const string PrefabPath = "Assets/Dubito/Prefabs/PF_Opponent.prefab";
        const string JudgeModelPath = "Assets/Dubito/Art/Characters/Juge/CHR_Juge.fbx";

        [Test]
        public void OpponentHoldsItsFanWithIk()
        {
            var prefab = AssetDatabase.LoadAssetAtPath<GameObject>(PrefabPath);
            Assert.That(prefab, Is.Not.Null);
            var hands = prefab.GetComponentInChildren<SeatedOpponentHands>();
            Assert.That(hands, Is.Not.Null, "SeatedOpponentHands missing on the opponent body");
            Assert.That(hands.HoldingCards, Is.False, "hands must not grab before cards are dealt");
            var animator = hands.GetComponent<Animator>();
            Assert.That(animator.isHuman, Is.True);
            var controller = (AnimatorController)animator.runtimeAnimatorController;
            Assert.That(controller.layers[0].iKPass, Is.True, "IK Pass must be on for OnAnimatorIK");

            var so = new SerializedObject(hands);
            Assert.That(so.FindProperty("fan").objectReferenceValue,
                Is.EqualTo(prefab.transform.Find("HandFan")));
        }
    

        // D-86: the judge's modelled finger bones drive the Humanoid fingers (3 per finger, both hands).
        [Test]
        public void JudgeAvatarMapsAllFingerBones()
        {
            var avatar = AssetDatabase.LoadAllAssetsAtPath(JudgeModelPath).OfType<Avatar>().FirstOrDefault();
            Assert.That(avatar, Is.Not.Null);
            Assert.That(avatar.isValid && avatar.isHuman, Is.True);
            var mapped = avatar.humanDescription.human.ToDictionary(h => h.humanName, h => h.boneName);
            Assert.That(mapped["Spine"], Is.EqualTo("Spine02"), "body mapping must be kept");
            foreach (var side in new[] { "Left", "Right" })
                foreach (var (bone, human) in new[] { ("Thumb", "Thumb"), ("Index", "Index"), ("Middle", "Middle"), ("Ring", "Ring"), ("Pinky", "Little") })
                    for (int k = 0; k < 3; k++)
                    {
                        string humanName = $"{side} {human} {new[] { "Proximal", "Intermediate", "Distal" }[k]}";
                        Assert.That(mapped.TryGetValue(humanName, out var boneName), Is.True, humanName + " not mapped");
                        Assert.That(boneName, Is.EqualTo($"{side}Hand{bone}{k + 1}"));
                    }

            var prefab = AssetDatabase.LoadAssetAtPath<GameObject>(PrefabPath);
            var animator = prefab.GetComponentInChildren<Animator>();
            for (var b = HumanBodyBones.LeftThumbProximal; b <= HumanBodyBones.RightLittleDistal; b++)
                Assert.That(animator.GetBoneTransform(b), Is.Not.Null, b + " missing on the opponent");
        }

        [Test]
        public void HandsGripTheFanSymmetricallyAndLetGoWhenEmpty()
        {
            var go = new GameObject("Hands", typeof(Animator));
            try
            {
                var hands = go.AddComponent<SeatedOpponentHands>();
                hands.SetFan(13, HandFanLayout.DefaultRadius, HandFanLayout.DefaultMaxAngle, HandFanLayout.DefaultSpacing);
                Assert.That(hands.HoldingCards, Is.True);
                Pose left = hands.GripCard(true), right = hands.GripCard(false);
                Assert.That(left.position.x, Is.LessThan(0f));
                Assert.That(right.position.x, Is.EqualTo(-left.position.x).Within(1e-5f));
                // Never wider than the outermost card of the fan.
                Pose outer = HandFanLayout.CardPose(12, 13);
                Assert.That(right.position.x, Is.LessThanOrEqualTo(outer.position.x + 1e-5f));

                hands.SetFan(1, HandFanLayout.DefaultRadius, HandFanLayout.DefaultMaxAngle, HandFanLayout.DefaultSpacing);
                Assert.That(hands.GripCard(false).position.x, Is.GreaterThan(0.02f), "one card: hands still apart");
                hands.SetFan(0, HandFanLayout.DefaultRadius, HandFanLayout.DefaultMaxAngle, HandFanLayout.DefaultSpacing);
                Assert.That(hands.HoldingCards, Is.False);
            }
            finally
            {
                Object.DestroyImmediate(go);
            }
        }
    }
}
