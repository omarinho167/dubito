using Dubito.Core;
using UnityEngine;

namespace Dubito.Presentation
{
    // Shared materials for the 52 faces and the back (skill dubito-card-faces: no material created
    // per card at runtime). Filled by the editor builder from Art/Cards/Faces.
    [CreateAssetMenu(menuName = "Dubito/Card Material Library", fileName = "CardMaterialLibrary")]
    public sealed class CardMaterialLibrary : ScriptableObject
    {
        public const int FaceCount = 52;
        const int SuitCount = 4;

        [SerializeField] Material back;
        [SerializeField] Material[] faces = new Material[FaceCount];

        public Material Back => back;

        public Material Face(Card card) => faces[IndexOf(card.Rank, card.Suit)];

        public static int IndexOf(CardRank rank, CardSuit suit) =>
            ((int)rank - (int)CardRank.Ace) * SuitCount + (int)suit;

#if UNITY_EDITOR
        public void EditorAssign(Material backMaterial, Material[] faceMaterials)
        {
            back = backMaterial;
            faces = faceMaterials;
        }
#endif
    }
}
