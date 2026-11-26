using UnityEngine;
using UnityEngine.InputSystem;

namespace Dubito.Presentation
{
    public sealed class SeatedCameraController : MonoBehaviour
    {
        [SerializeField, Min(0f)] float sensitivity = 0.12f;
        [SerializeField, Min(0f)] float smoothing = 18f;

        InputAction lookHeld;
        InputAction lookDelta;
        Quaternion baseRotation;
        Vector2 targetLook;
        Vector2 displayedLook;
        bool holding;
        bool focused;

        public void SetSeat(Vector3 eyePosition, Quaternion baseRotation)
        {
            this.baseRotation = baseRotation;
            targetLook = displayedLook = Vector2.zero;
            transform.SetPositionAndRotation(eyePosition, baseRotation);
        }

        void Awake() => baseRotation = transform.rotation;

        void OnEnable()
        {
            lookHeld ??= new InputAction("LookHeld", InputActionType.Button, "<Mouse>/rightButton");
            lookDelta ??= new InputAction("LookDelta", InputActionType.Value, "<Mouse>/delta");
            lookHeld.canceled += OnLookReleased;
            lookHeld.Enable();
            lookDelta.Enable();
            focused = Application.isFocused;
        }

        void LateUpdate()
        {
            bool wantsLook = focused && lookHeld.IsPressed();
            if (wantsLook && !holding && lookHeld.WasPressedThisFrame())
            {
                holding = true;
                Cursor.lockState = CursorLockMode.Locked;
                Cursor.visible = false;
                // Skip the lock frame's delta to avoid a cursor-warp jump.
            }
            else if (holding && wantsLook)
            {
                targetLook = LookClamp.Apply(targetLook, lookDelta.ReadValue<Vector2>() * sensitivity,
                    60f, 15f, 30f);
            }
            else if (holding) ReleaseLook();

            if (!holding) return;
            float blend = smoothing <= 0f ? 1f : 1f - Mathf.Exp(-smoothing * Time.unscaledDeltaTime);
            displayedLook = Vector2.Lerp(displayedLook, targetLook, blend);
            // Yaw around the world vertical, pitch around the camera's own horizontal axis: the horizon never rolls.
            Vector3 baseEuler = baseRotation.eulerAngles;
            transform.rotation = Quaternion.Euler(baseEuler.x - displayedLook.y, baseEuler.y + displayedLook.x, 0f);
        }

        void OnLookReleased(InputAction.CallbackContext context) => ReleaseLook();

        void ReleaseLook()
        {
            if (!holding) return;
            holding = false;
            targetLook = displayedLook;
            Cursor.lockState = CursorLockMode.None;
            Cursor.visible = true;
        }

        void OnApplicationFocus(bool hasFocus)
        {
            focused = hasFocus;
            if (!hasFocus) ReleaseLook();
        }

        void OnDisable()
        {
            ReleaseLook();
            if (lookHeld == null) return;
            lookHeld.canceled -= OnLookReleased;
            lookHeld.Disable();
            lookDelta.Disable();
        }

        void OnDestroy()
        {
            lookHeld?.Dispose();
            lookDelta?.Dispose();
        }
    }
}
