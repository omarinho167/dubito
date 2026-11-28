using UnityEngine;

namespace Dubito.Presentation
{
    // Moves a seated opponent's hands onto its card fan with Humanoid IK (needs IK Pass on the base layer).
    // Presentation only. The hands let go when the fan is empty and during reactions (one-shot clips).
    [RequireComponent(typeof(Animator))]
    public sealed class SeatedOpponentHands : MonoBehaviour
    {
        [SerializeField] Transform fan;
        [SerializeField] Transform leftElbowHint;
        [SerializeField] Transform rightElbowHint;
        [SerializeField, Range(0, 1)] float maxWeight = 1f;
        [SerializeField, Range(0, 1)] float rotationWeight = 1f;
        [SerializeField] float fadeSpeed = 4f;
        // Wrist position in the card frame (X right, Y towards the holder, Z up the cards).
        [SerializeField] float wristSpread = .06f;
        [SerializeField] float wristBelowEdge = 0f;
        [SerializeField] float wristTowardsHolder = .07f;
        // Extra twist of each hand around the fingers, to rest the palm on the cards.
        [SerializeField] float palmTwist = 0f;

        Animator animator;
        float weight;

        public bool HoldingCards { get; set; }
        public float Weight => weight;

#if UNITY_EDITOR
        public void EditorSetup(Transform fanAnchor, Transform leftHint, Transform rightHint)
        {
            fan = fanAnchor;
            leftElbowHint = leftHint;
            rightElbowHint = rightHint;
        }
#endif

        void Awake() => animator = GetComponent<Animator>();

        void OnAnimatorIK(int layerIndex)
        {
            if (animator == null || fan == null) return;
            bool idle = !animator.IsInTransition(0) &&
                        animator.GetCurrentAnimatorStateInfo(0).IsName(SeatedOpponentAppearance.IdleState);
            weight = Mathf.MoveTowards(weight, idle && HoldingCards ? maxWeight : 0f, fadeSpeed * Time.deltaTime);
            Quaternion cards = fan.rotation * Quaternion.Euler(HandFanLayout.OpponentFanTilt, 0, 0);
            Hold(true, cards);
            Hold(false, cards);
        }

        void Hold(bool left, Quaternion cards)
        {
            var goal = left ? AvatarIKGoal.LeftHand : AvatarIKGoal.RightHand;
            Vector3 local = new Vector3(left ? -wristSpread : wristSpread, wristTowardsHolder,
                -(HandFanLayout.CardHalfHeight + wristBelowEdge));
            animator.SetIKPositionWeight(goal, weight);
            animator.SetIKPosition(goal, fan.position + cards * local);

            // Humanoid IK rotations are relative to the T-pose: map the T-pose hand (fingers out to the side,
            // back of the hand up) to fingers up the cards, back of the hand towards the holder.
            Quaternion root = transform.rotation;
            Quaternion tPose = Quaternion.LookRotation(root * (left ? Vector3.left : Vector3.right), root * Vector3.up);
            Quaternion target = Quaternion.LookRotation(cards * Vector3.forward, cards * Vector3.up) *
                                Quaternion.AngleAxis(left ? palmTwist : -palmTwist, Vector3.forward);
            animator.SetIKRotationWeight(goal, weight * rotationWeight);
            animator.SetIKRotation(goal, target * Quaternion.Inverse(tPose) * root);

            var hint = left ? AvatarIKHint.LeftElbow : AvatarIKHint.RightElbow;
            var elbow = left ? leftElbowHint : rightElbowHint;
            if (elbow == null) return;
            animator.SetIKHintPositionWeight(hint, weight);
            animator.SetIKHintPosition(hint, elbow.position);
        }
    }
}
