using System;
using UnityEngine;

namespace Dubito.Presentation
{
    // Puts a seated opponent's hands on its card fan: Humanoid IK places the wrists at the lower corners
    // of the fan (needs IK Pass on the base layer), then the finger bones close around the cards (D-86):
    // thumbs on the faces (holder side), other fingers behind. Presentation only. The hands let go
    // smoothly when the fan is empty and during reactions (one-shot clips), returning to the animation.
    [RequireComponent(typeof(Animator))]
    public sealed class SeatedOpponentHands : MonoBehaviour
    {
        // Degrees per phalanx: x = curl towards the palm, y = spread (towards the thumb; for the thumb,
        // towards the fingers), z = twist around the bone.
        [Serializable]
        public struct FingerPose
        {
            public Vector3 proximal, intermediate, distal;
            public FingerPose(Vector3 proximal, Vector3 intermediate, Vector3 distal)
            {
                this.proximal = proximal; this.intermediate = intermediate; this.distal = distal;
            }
        }

        [SerializeField] Transform fan;
        [SerializeField] Transform leftElbowHint;
        [SerializeField] Transform rightElbowHint;
        [SerializeField, Range(0, 1)] float maxWeight = 1f;
        [SerializeField, Range(0, 1)] float rotationWeight = 1f;
        [SerializeField] float fadeSpeed = 4f;
        // Each hand grips the card at this angle on the fan arc (clamped to the fan's own width).
        [SerializeField] float minGripAngle = 6f;
        [SerializeField] float maxGripAngle = 20f;
        // Wrist position in the gripped card's frame (X outwards, Y towards the holder, Z up the card).
        [SerializeField] Vector3 wristInCard = new Vector3(.06f, -.07f, -.12f);
        // Hand orientation in the same card frame (X outwards, mirrored for the right hand): where the
        // straight fingers point and where the back of the hand faces.
        [SerializeField] Vector3 fingersInCard = new Vector3(-.6f, .12f, .78f);
        [SerializeField] Vector3 backInCard = new Vector3(0f, -1f, .15f);
        [SerializeField] FingerPose thumb = new FingerPose(new Vector3(52, 25, 0), new Vector3(22, 0, 0), new Vector3(12, 0, 0));
        [SerializeField] FingerPose index = new FingerPose(new Vector3(14, 0, 0), new Vector3(22, 0, 0), new Vector3(10, 0, 0));
        [SerializeField] FingerPose middle = new FingerPose(new Vector3(16, 0, 0), new Vector3(22, 0, 0), new Vector3(10, 0, 0));
        [SerializeField] FingerPose ring = new FingerPose(new Vector3(22, 0, 0), new Vector3(24, 0, 0), new Vector3(12, 0, 0));
        [SerializeField] FingerPose little = new FingerPose(new Vector3(28, 0, 0), new Vector3(26, 0, 0), new Vector3(14, 0, 0));

        // Fan geometry (HandFanLayout) to find where the hands hold it.
        float gripAngle;
        float fanRadius = HandFanLayout.DefaultRadius;

        Animator animator;
        float weight;
        Transform[] bones;
        Quaternion[] restLocal;
        Vector3[] curlAxis, spreadAxis, twistAxis;
        // Per hand: bone rotation in the rest pose and the hand frame (fingers, back of the hand), both
        // relative to the body; then the measured bone -> Humanoid IK goal offset.
        readonly Quaternion[] restHand = new Quaternion[2], restFrame = new Quaternion[2];
        readonly Quaternion[] animatedGoal = new Quaternion[2], goalOffset = new Quaternion[2];
        bool goalSampled, offsetReady;

        public bool HoldingCards { get; set; }
        public float Weight => weight;
        public static int FingerBoneCount => 30;

#if UNITY_EDITOR
        public void EditorSetup(Transform fanAnchor, Transform leftHint, Transform rightHint)
        {
            fan = fanAnchor;
            leftElbowHint = leftHint;
            rightElbowHint = rightHint;
        }
#endif

        // Called with the opponent's card count whenever the fan is laid out.
        public void SetFan(int count, float radius, float maxAngle, float spacing)
        {
            HoldingCards = count > 0;
            fanRadius = radius;
            float half = count <= 1 ? 0f : HandFanLayout.CardPose(count - 1, count, radius, maxAngle, spacing)
                .rotation.eulerAngles.y;
            if (half > 180f) half -= 360f;
            gripAngle = Mathf.Clamp(Mathf.Abs(half), minGripAngle, maxGripAngle);
        }

        void Awake()
        {
            animator = GetComponent<Animator>();
            if (gripAngle <= 0f) gripAngle = minGripAngle;
            CaptureRest();
        }

        // Rest (bind) pose axes of every finger bone, expressed in the bone's own local frame.
        void CaptureRest()
        {
            if (animator == null || !animator.isHuman) return;
            bones = new Transform[FingerBoneCount];
            restLocal = new Quaternion[FingerBoneCount];
            curlAxis = new Vector3[FingerBoneCount];
            spreadAxis = new Vector3[FingerBoneCount];
            twistAxis = new Vector3[FingerBoneCount];
            Vector3 back = transform.up; // T-pose: palms down
            for (int side = 0; side < 2; side++)
            {
                int first = side == 0 ? (int)HumanBodyBones.LeftThumbProximal : (int)HumanBodyBones.RightThumbProximal;
                var hand = animator.GetBoneTransform(side == 0 ? HumanBodyBones.LeftHand : HumanBodyBones.RightHand);
                var indexBase = animator.GetBoneTransform((HumanBodyBones)(first + 3));
                var littleBase = animator.GetBoneTransform((HumanBodyBones)(first + 12));
                var middleBase = animator.GetBoneTransform((HumanBodyBones)(first + 6));
                if (hand == null || indexBase == null || littleBase == null || middleBase == null)
                {
                    bones = null;
                    return;
                }
                Vector3 thumbSide = (indexBase.position - littleBase.position).normalized;
                Vector3 handDir = (middleBase.position - hand.position).normalized;
                Quaternion toBody = Quaternion.Inverse(transform.rotation);
                restHand[side] = toBody * hand.rotation;
                restFrame[side] = toBody * Quaternion.LookRotation(handDir, back);
                for (int i = 0; i < 15; i++)
                {
                    var bone = animator.GetBoneTransform((HumanBodyBones)(first + i));
                    if (bone == null) { bones = null; return; }
                    int k = i % 3;
                    Vector3 dir = k < 2
                        ? animator.GetBoneTransform((HumanBodyBones)(first + i + 1)).position - bone.position
                        : bone.position - animator.GetBoneTransform((HumanBodyBones)(first + i - 1)).position;
                    dir.Normalize();
                    Vector3 curl = Vector3.Cross(back, dir).normalized;
                    Vector3 spread = Vector3.Cross(dir, i < 3 ? handDir : thumbSide).normalized;
                    Quaternion inv = Quaternion.Inverse(bone.rotation);
                    int slot = side * 15 + i;
                    bones[slot] = bone;
                    restLocal[slot] = bone.localRotation;
                    curlAxis[slot] = inv * curl;
                    spreadAxis[slot] = inv * spread;
                    twistAxis[slot] = inv * dir;
                }
            }
        }

        void OnAnimatorIK(int layerIndex)
        {
            if (animator == null || fan == null) return;
            bool idle = !animator.IsInTransition(0) &&
                        animator.GetCurrentAnimatorStateInfo(0).IsName(SeatedOpponentAppearance.IdleState);
            animatedGoal[0] = animator.GetIKRotation(AvatarIKGoal.LeftHand);
            animatedGoal[1] = animator.GetIKRotation(AvatarIKGoal.RightHand);
            goalSampled = true;
            bool hold = idle && HoldingCards && offsetReady;
            weight = Mathf.MoveTowards(weight, hold ? maxWeight : 0f, fadeSpeed * Time.deltaTime);
            Quaternion cards = fan.rotation * Quaternion.Euler(HandFanLayout.OpponentFanTilt, 0, 0);
            Hold(true, cards);
            Hold(false, cards);
        }

        // Pose of the card held by one hand, in the fan anchor's tilted frame (see HandFanLayout.CardPose).
        public Pose GripCard(bool left)
        {
            float a = (left ? -gripAngle : gripAngle) * Mathf.Deg2Rad;
            return new Pose(new Vector3(fanRadius * Mathf.Sin(a), 0f, fanRadius * (Mathf.Cos(a) - 1f)),
                Quaternion.Euler(0f, a * Mathf.Rad2Deg, 0f));
        }

        void Hold(bool left, Quaternion cards)
        {
            var goal = left ? AvatarIKGoal.LeftHand : AvatarIKGoal.RightHand;
            float sx = left ? -1f : 1f;
            Pose card = GripCard(left);
            Quaternion cardRot = cards * card.rotation;
            Vector3 cardPos = fan.position + cards * card.position;
            Vector3 local = new Vector3(sx * wristInCard.x, wristInCard.y, wristInCard.z);
            animator.SetIKPositionWeight(goal, weight);
            animator.SetIKPosition(goal, cardPos + cardRot * local);

            // Wanted hand frame, mapped from the rest pose to a bone rotation, then to the Humanoid IK goal frame.
            int s = left ? 0 : 1;
            Quaternion body = transform.rotation;
            Vector3 fingers = new Vector3(sx * fingersInCard.x, fingersInCard.y, fingersInCard.z);
            Vector3 backOfHand = new Vector3(sx * backInCard.x, backInCard.y, backInCard.z);
            Quaternion frame = Quaternion.LookRotation(cardRot * fingers, cardRot * backOfHand);
            Quaternion bone = frame * Quaternion.Inverse(body * restFrame[s]) * (body * restHand[s]);
            animator.SetIKRotation(goal, bone * goalOffset[s]);
            animator.SetIKRotationWeight(goal, weight * rotationWeight);

            var hint = left ? AvatarIKHint.LeftElbow : AvatarIKHint.RightElbow;
            var elbow = left ? leftElbowHint : rightElbowHint;
            if (elbow == null) return;
            animator.SetIKHintPositionWeight(hint, weight);
            animator.SetIKHintPosition(hint, elbow.position);
        }

        // After the Animator has written the pose: close the fingers around the cards, blended by weight.
        void LateUpdate()
        {
            if (bones == null) return;
            if (weight <= 0f)
            {
                // Pure animation this frame: measure the constant bone -> IK goal offset of each hand.
                var l = animator.GetBoneTransform(HumanBodyBones.LeftHand);
                var r = animator.GetBoneTransform(HumanBodyBones.RightHand);
                if (goalSampled)
                {
                    goalOffset[0] = Quaternion.Inverse(l.rotation) * animatedGoal[0];
                    goalOffset[1] = Quaternion.Inverse(r.rotation) * animatedGoal[1];
                    offsetReady = true;
                }
                return;
            }
            for (int slot = 0; slot < bones.Length; slot++)
            {
                int i = slot % 15;
                Vector3 e = PhalanxEuler(i / 3, i % 3);
                Quaternion grip = restLocal[slot] * Quaternion.AngleAxis(e.x, curlAxis[slot]) *
                                  Quaternion.AngleAxis(e.y, spreadAxis[slot]) * Quaternion.AngleAxis(e.z, twistAxis[slot]);
                bones[slot].localRotation = Quaternion.Slerp(bones[slot].localRotation, grip, weight);
            }
        }

        Vector3 PhalanxEuler(int finger, int phalanx)
        {
            FingerPose p = finger switch { 0 => thumb, 1 => index, 2 => middle, 3 => ring, _ => little };
            return phalanx switch { 0 => p.proximal, 1 => p.intermediate, _ => p.distal };
        }
    }
}
