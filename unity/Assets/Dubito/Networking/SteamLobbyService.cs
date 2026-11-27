using System;
using System.Collections.Generic;
using System.Globalization;
using Dubito.Protocol;
using Steamworks;
using UnityEngine;

namespace Dubito.Networking
{
    [DisallowMultipleComponent]
    public sealed class SteamLobbyService : MonoBehaviour
    {
        [SerializeField] SteamBootstrap bootstrap;
        [SerializeField] DubitoNetworkManager network;
        Callback<GameLobbyJoinRequested_t> invitation;
        Callback<LobbyChatUpdate_t> membersChanged;
        readonly List<IDisposable> calls = new List<IDisposable>();
        int generation;
        double deadline;
        ulong hostId;
        bool launchHandled;
        public ulong LobbyId { get; private set; }
        public bool IsBusy { get; private set; }
        public bool IsReady => bootstrap != null && bootstrap.IsReady;
        public string LocalPlayerName => bootstrap != null ? bootstrap.LocalPlayerName : "";

        void OnEnable()
        {
            if (bootstrap == null) bootstrap = GetComponent<SteamBootstrap>();
            if (network == null) network = GetComponent<DubitoNetworkManager>();
            if (!IsReady) return;
            bootstrap.ShuttingDown += Shutdown;
            invitation = Callback<GameLobbyJoinRequested_t>.Create(value => network.JoinSteam(value.m_steamIDLobby.m_SteamID));
            membersChanged = Callback<LobbyChatUpdate_t>.Create(OnMembersChanged);
        }

        void Update()
        {
            // All Start methods (including UI subscriptions) have run before processing launch invitations.
            if (!launchHandled)
            {
                launchHandled = true;
                if (IsReady && bootstrap.LaunchLobbyId != 0) network.JoinSteam(bootstrap.LaunchLobbyId);
            }
            if (IsBusy && Time.realtimeSinceStartupAsDouble >= deadline)
                network.Fail(SteamLobbyRules.ConnectionFailed);
        }

        // A canceled Steam API request still completes natively. Keep its CallResult alive
        // to leave any late-created/joined lobby. Serialize Steam requests until it drains.
        internal bool CanBegin()
        {
            if (!IsReady) { network.Fail(SteamLobbyRules.SteamUnavailable); return false; }
            if (calls.Count != 0)
            {
                network.SetStatus("Une demande Steam est encore en cours. Réessaie dans quelques instants.");
                return false;
            }
            return true;
        }

        internal void CreateLobby(int maxPlayers)
        {
            int request = Begin();
            CallResult<LobbyCreated_t> result = null;
            result = CallResult<LobbyCreated_t>.Create((value, failed) =>
            {
                try
                {
                    if (request != generation)
                    {
                        if (!failed && value.m_eResult == EResult.k_EResultOK) Leave(value.m_ulSteamIDLobby);
                        return;
                    }
                    IsBusy = false;
                    if (failed || value.m_eResult != EResult.k_EResultOK || value.m_ulSteamIDLobby == 0)
                    { network.Fail(SteamLobbyRules.ConnectionFailed); return; }
                    LobbyId = value.m_ulSteamIDLobby;
                    hostId = bootstrap.LocalSteamId;
                    var id = new CSteamID(LobbyId);
                    bool tagged = SteamMatchmaking.SetLobbyData(id, SteamLobbyRules.GameKey, SteamLobbyRules.GameValue)
                        && SteamMatchmaking.SetLobbyData(id, SteamLobbyRules.ProtocolKey, ProtocolInfo.Version.ToString(CultureInfo.InvariantCulture))
                        && SteamMatchmaking.SetLobbyData(id, SteamLobbyRules.HostKey, hostId.ToString(CultureInfo.InvariantCulture));
                    if (!tagged) { network.Fail(SteamLobbyRules.ConnectionFailed); return; }
                    network.StartSteamHost();
                }
                catch (Exception) { network.Fail(SteamLobbyRules.ConnectionFailed); }
                finally { calls.Remove(result); result.Dispose(); }
            });
            calls.Add(result);
            try
            {
                var handle = SteamMatchmaking.CreateLobby(ELobbyType.k_ELobbyTypeFriendsOnly, maxPlayers);
                if (handle == SteamAPICall_t.Invalid) throw new InvalidOperationException();
                result.Set(handle);
            }
            catch (Exception) { calls.Remove(result); result.Dispose(); network.Fail(SteamLobbyRules.ConnectionFailed); }
        }

        public void JoinLobby(string code)
        {
            if (!SteamLobbyRules.TryParseLobbyCode(code, out ulong lobby))
            { network.SetStatus("Code du salon invalide : colle l'identifiant numérique du salon Steam."); return; }
            network.JoinSteam(lobby);
        }

        internal void JoinLobby(ulong lobbyId)
        {
            var id = new CSteamID(lobbyId);
            if (!id.IsValid() || !id.IsLobby()) { network.Fail("Code du salon invalide."); return; }
            int request = Begin();
            CallResult<LobbyEnter_t> result = null;
            result = CallResult<LobbyEnter_t>.Create((value, failed) =>
            {
                bool entered = !failed && value.m_EChatRoomEnterResponse == (uint)EChatRoomEnterResponse.k_EChatRoomEnterResponseSuccess;
                try
                {
                    if (request != generation)
                    {
                        if (entered) Leave(value.m_ulSteamIDLobby);
                        return;
                    }
                    IsBusy = false;
                    if (!entered)
                    {
                        network.Fail(!failed && value.m_EChatRoomEnterResponse == (uint)EChatRoomEnterResponse.k_EChatRoomEnterResponseFull
                            ? "Salon plein : toutes les places sont prises." : SteamLobbyRules.ConnectionFailed);
                        return;
                    }
                    LobbyId = value.m_ulSteamIDLobby;
                    if (LobbyId != lobbyId) { network.Fail(SteamLobbyRules.ConnectionFailed); return; }
                    string error = SteamLobbyRules.ValidateLobbyData(
                        SteamMatchmaking.GetLobbyData(id, SteamLobbyRules.GameKey),
                        SteamMatchmaking.GetLobbyData(id, SteamLobbyRules.ProtocolKey),
                        SteamMatchmaking.GetLobbyData(id, SteamLobbyRules.HostKey), out ulong host);
                    if (error != null) { network.Fail(error); return; }
                    var hostSteamId = new CSteamID(host);
                    // No migration: Steam may transfer lobby ownership when the actual host leaves.
                    if (!hostSteamId.IsValid() || !hostSteamId.BIndividualAccount() || SteamMatchmaking.GetLobbyOwner(id).m_SteamID != host)
                    { network.Fail("L'hôte a quitté la partie."); return; }
                    hostId = host;
                    network.StartSteamClient(host);
                }
                catch (Exception) { network.Fail(SteamLobbyRules.ConnectionFailed); }
                finally { calls.Remove(result); result.Dispose(); }
            });
            calls.Add(result);
            try
            {
                var handle = SteamMatchmaking.JoinLobby(id);
                if (handle == SteamAPICall_t.Invalid) throw new InvalidOperationException();
                result.Set(handle);
            }
            catch (Exception) { calls.Remove(result); result.Dispose(); network.Fail(SteamLobbyRules.ConnectionFailed); }
        }

        int Begin()
        {
            IsBusy = true;
            deadline = Time.realtimeSinceStartupAsDouble + 35;
            return ++generation;
        }

        public void InviteFriends()
        {
            if (IsReady && LobbyId != 0 && network.IsHosting)
                SteamFriends.ActivateGameOverlayInviteDialog(new CSteamID(LobbyId));
        }

        internal bool SetMemberLimit(int count)
        {
            return IsReady && LobbyId != 0 && network.IsHosting &&
                SteamMatchmaking.SetLobbyMemberLimit(new CSteamID(LobbyId), count);
        }

        void OnMembersChanged(LobbyChatUpdate_t value)
        {
            if (value.m_ulSteamIDLobby != LobbyId || LobbyId == 0) return;
            const uint departed = (uint)(EChatMemberStateChange.k_EChatMemberStateChangeLeft |
                EChatMemberStateChange.k_EChatMemberStateChangeDisconnected |
                EChatMemberStateChange.k_EChatMemberStateChangeKicked |
                EChatMemberStateChange.k_EChatMemberStateChangeBanned);
            if ((value.m_rgfChatMemberStateChange & departed) != 0 &&
                (value.m_ulSteamIDUserChanged == hostId || value.m_ulSteamIDUserChanged == bootstrap.LocalSteamId))
                network.Fail(value.m_ulSteamIDUserChanged == hostId ? "L'hôte a quitté la partie." : SteamLobbyRules.ConnectionFailed);
        }

        // Called by the manager for local switches, cancel, leave, errors and disconnects.
        internal void LeaveLobby()
        {
            ++generation;
            IsBusy = false;
            ulong previous = LobbyId;
            LobbyId = hostId = 0;
            Leave(previous);
        }

        void Leave(ulong lobby)
        {
            if (IsReady && lobby != 0) SteamMatchmaking.LeaveLobby(new CSteamID(lobby));
        }

        void Shutdown()
        {
            if (network != null) network.StopSession();
            invitation?.Dispose(); invitation = null;
            membersChanged?.Dispose(); membersChanged = null;
            foreach (var call in calls) call.Dispose();
            calls.Clear();
        }

        void OnDisable()
        {
            if (bootstrap != null) bootstrap.ShuttingDown -= Shutdown;
            Shutdown();
        }
    }
}
