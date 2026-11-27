using System;
using Steamworks;
using UnityEngine;

namespace Dubito.Networking
{
    [DefaultExecutionOrder(-1000)]
    [DisallowMultipleComponent]
    public sealed class SteamBootstrap : MonoBehaviour
    {
        static SteamBootstrap instance;
        static bool attempted;
        bool initialized;
        public bool IsReady => initialized && instance == this;
        public string Error { get; private set; } = SteamLobbyRules.SteamUnavailable;
        public string LocalPlayerName => IsReady ? SteamLobbyRules.TruncatePlayerName(SteamFriends.GetPersonaName()) : "";
        public ulong LocalSteamId => IsReady ? SteamUser.GetSteamID().m_SteamID : 0;
        public ulong LaunchLobbyId { get; private set; }
        public event Action ShuttingDown;

        [RuntimeInitializeOnLoadMethod(RuntimeInitializeLoadType.SubsystemRegistration)]
        static void ResetStatics() { instance = null; attempted = false; }

        void Awake()
        {
            if (instance != null || attempted) return;
            instance = this;
            attempted = true;
            if (SteamLobbyRules.TryParseConnectLobby(Environment.GetCommandLineArgs(), out ulong lobby)) LaunchLobbyId = lobby;
            try
            {
                // Development App ID comes from steam_appid.txt (480); never restart the editor/game.
                initialized = SteamAPI.InitEx(out string detail) == ESteamAPIInitResult.k_ESteamAPIInitResult_OK;
                if (!initialized) Debug.LogWarning("[Dubito] " + Error + ". " + detail);
                else Error = null;
            }
            catch (Exception ex)
            {
                Debug.LogWarning("[Dubito] " + Error + ". " + ex.Message);
            }
        }

        void Update()
        {
            if (IsReady) SteamAPI.RunCallbacks();
        }

        // Stop lobby/transport users BEFORE shutting down the native API, irrespective of
        // the order Unity destroys components in. Also handles Play mode without domain reload.
        void OnDestroy()
        {
            if (instance != this) return;
            try { if (initialized) ShuttingDown?.Invoke(); }
            finally
            {
                if (initialized) SteamAPI.Shutdown();
                initialized = false;
                instance = null;
            }
        }
    }
}
