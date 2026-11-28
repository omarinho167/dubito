using Dubito.Presentation;
using NUnit.Framework;
using UnityEditor;
using UnityEditor.Animations;
using UnityEngine;

namespace Dubito.Tests
{
    public sealed class OpponentPrefabTests
    {
        const string PrefabPath = "Assets/Dubito/Prefabs/PF_Opponent.prefab";

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
    }
}
