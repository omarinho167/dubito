using System;
using UnityEngine;
using UnityEngine.InputSystem;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace Dubito.Presentation
{
    public sealed class TableSeating : MonoBehaviour
    {
        [SerializeField, Range(2, 6)] int seatCount = 4;
        [SerializeField] int localSeat = 0;
        [SerializeField] GameObject chairPrefab = null;
        [SerializeField] GameObject opponentPrefab = null;
        [SerializeField] Transform seatsRoot = null;
        [SerializeField] SeatedCameraController seatCamera = null;
        [SerializeField] bool debugSeatHotkeys = true;

        public int SeatCount => seatCount;
        public int LocalSeat => localSeat;

        public void Build(int seatCount, int localSeat)
        {
            // Validate before touching the existing layout.
            SeatLayout.YawDegrees(localSeat, seatCount, localSeat);
            if (seatsRoot == null || chairPrefab == null || opponentPrefab == null || seatCamera == null)
                throw new InvalidOperationException("TableSeating requires seatsRoot, both prefabs and seatCamera.");
            if (opponentPrefab.GetComponent<SeatedOpponentAppearance>() == null)
                throw new InvalidOperationException("opponentPrefab requires SeatedOpponentAppearance on its root.");

            this.seatCount = seatCount;
            this.localSeat = localSeat;
            for (int i = seatsRoot.childCount - 1; i >= 0; i--)
            {
                GameObject child = seatsRoot.GetChild(i).gameObject;
                if (Application.isPlaying)
                {
                    child.SetActive(false);
                    child.transform.SetParent(null, true);
                    Destroy(child);
                }
                else DestroyImmediate(child);
            }

            for (int i = 0; i < seatCount; i++)
            {
                Transform anchor = new GameObject($"Seat_{i}").transform;
                anchor.SetParent(seatsRoot, false);
                Vector3 position = SeatLayout.SeatPosition(i, seatCount, localSeat, SeatLayout.PelvisRadius);
                Quaternion rotation = SeatLayout.SeatRotation(i, seatCount, localSeat);
                anchor.SetPositionAndRotation(position, rotation);
                GameObject chair = CreateInstance(chairPrefab, anchor, "Chair");
                chair.transform.SetPositionAndRotation(
                    SeatLayout.SeatPosition(i, seatCount, localSeat, SeatLayout.ChairCenterRadius), rotation);
                if (i == localSeat) continue;
                GameObject opponent = CreateInstance(opponentPrefab, anchor, "Opponent");
                opponent.transform.SetPositionAndRotation(position, rotation);
                opponent.GetComponent<SeatedOpponentAppearance>().SeatColor = SeatPalette.Get(i);
            }

            Vector3 eye = SeatLayout.SeatPosition(localSeat, seatCount, localSeat, SeatLayout.PelvisRadius);
            eye.y = SeatLayout.EyeHeight;
            seatCamera.SetSeat(eye, Quaternion.LookRotation(new Vector3(0f, SeatLayout.TableHeight, 0f) - eye));
        }

        static GameObject CreateInstance(GameObject prefab, Transform parent, string instanceName)
        {
            GameObject instance;
#if UNITY_EDITOR
            if (!Application.isPlaying)
                instance = (GameObject)PrefabUtility.InstantiatePrefab(prefab, parent);
            else
#endif
                instance = Instantiate(prefab, parent);
            instance.name = instanceName;
            return instance;
        }

        void Awake()
        {
            if (Application.isPlaying) Build(seatCount, localSeat);
        }

        void Update()
        {
            if (!Application.isPlaying || !debugSeatHotkeys || !(Application.isEditor || Debug.isDebugBuild)) return;
            Keyboard keyboard = Keyboard.current;
            if (keyboard == null) return;
            for (int n = SeatLayout.MinSeats; n <= SeatLayout.MaxSeats; n++)
            {
                if (!keyboard[(Key)((int)Key.Digit2 + n - 2)].wasPressedThisFrame &&
                    !keyboard[(Key)((int)Key.Numpad2 + n - 2)].wasPressedThisFrame) continue;
                Build(n, 0);
                break;
            }
        }
    }
}
