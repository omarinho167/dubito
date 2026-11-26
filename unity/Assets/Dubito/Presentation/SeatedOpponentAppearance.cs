using UnityEngine;

namespace Dubito.Presentation
{
    // Reactions available in AC_SeatedOpponent (value of the "Reaction" parameter).
    public enum OpponentReaction
    {
        Talking = 1,
        Disapproval = 2,
        RubbingArm = 3,
        Angry = 4,
        Disbelief = 5,
        ThumbsUp = 6,
        Victory = 7,
    }

    // Seat colour tint and idle offset for a seated mannequin (skill dubito-character-import).
    // Presentation only: driven by presentation events, never by hidden game state.
    public sealed class SeatedOpponentAppearance : MonoBehaviour
    {
        public const string IdleState = "Idle";
        static readonly int BaseColorId = Shader.PropertyToID("_BaseColor");
        static readonly int ReactionId = Animator.StringToHash("Reaction");
        static readonly int ReactId = Animator.StringToHash("React");

        [SerializeField] Color seatColor = Color.gray;
        [SerializeField] Renderer[] tintedRenderers = new Renderer[0];
        [SerializeField] Animator animator;
        [SerializeField] bool randomizeIdleStart = true;

        MaterialPropertyBlock block;

        public Color SeatColor
        {
            get => seatColor;
            set { seatColor = value; ApplyTint(); }
        }

        public void PlayReaction(OpponentReaction reaction)
        {
            if (animator == null) return;
            animator.SetInteger(ReactionId, (int)reaction);
            animator.SetTrigger(ReactId);
        }

#if UNITY_EDITOR
        public void EditorSetup(Animator bodyAnimator, Renderer[] tinted)
        {
            animator = bodyAnimator;
            tintedRenderers = tinted;
        }
#endif

        void OnEnable() => ApplyTint();

        void OnValidate() => ApplyTint();

        void Start()
        {
            // Six mannequins must not breathe in sync.
            if (randomizeIdleStart && animator != null)
                animator.Play(IdleState, 0, Random.value);
        }

        void ApplyTint()
        {
            if (tintedRenderers == null) return;
            block ??= new MaterialPropertyBlock();
            foreach (var r in tintedRenderers)
            {
                if (r == null) continue;
                r.GetPropertyBlock(block);
                block.SetColor(BaseColorId, seatColor);
                r.SetPropertyBlock(block);
            }
        }
    }
}
