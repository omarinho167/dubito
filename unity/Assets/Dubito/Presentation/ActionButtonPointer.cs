using System;
using UnityEngine;
using UnityEngine.EventSystems;

namespace Dubito.Presentation
{
    // Receives hover even when the Button itself is disabled.
    public sealed class ActionButtonPointer : MonoBehaviour, IPointerEnterHandler, IPointerExitHandler, IPointerDownHandler, IPointerUpHandler
    {
        public Action<bool> Hover;
        public Action<bool> Hold;
        public void OnPointerEnter(PointerEventData e) => Hover?.Invoke(true);
        public void OnPointerExit(PointerEventData e) { Hover?.Invoke(false); Hold?.Invoke(false); }
        public void OnPointerDown(PointerEventData e) { if (e.button == PointerEventData.InputButton.Left) Hold?.Invoke(true); }
        public void OnPointerUp(PointerEventData e) { if (e.button == PointerEventData.InputButton.Left) Hold?.Invoke(false); }
        void OnDisable() { Hover?.Invoke(false); Hold?.Invoke(false); }
    }
}
