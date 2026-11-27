using System.Globalization;
using System.Text;
using Dubito.Protocol;

namespace Dubito.Networking
{
    // Pure boundary validation, shared with dotnet tests. No Steam/Unity types.
    public static class SteamLobbyRules
    {
        public const string GameKey = "dubito";
        public const string GameValue = "1";
        public const string ProtocolKey = "dubito_protocol";
        public const string HostKey = "dubito_host";
        public const string SteamUnavailable = "Steam n'est pas lancé";
        public const string ConnectionFailed = "Connexion Steam impossible. Réessaie ou demande une nouvelle invitation.";

        public static bool TryParseLobbyCode(string code, out ulong lobbyId) =>
            ulong.TryParse(code?.Trim(), NumberStyles.None, CultureInfo.InvariantCulture, out lobbyId) && lobbyId != 0;

        public static bool TryParseConnectLobby(string[] arguments, out ulong lobbyId)
        {
            lobbyId = 0;
            if (arguments == null) return false;
            for (int i = 0; i < arguments.Length; i++)
                if (arguments[i] == "+connect_lobby")
                    return i + 1 < arguments.Length && TryParseLobbyCode(arguments[i + 1], out lobbyId);
            return false;
        }

        // null means valid. Check the game marker before the version on shared App ID 480.
        public static string ValidateLobbyData(string game, string protocol, string host, out ulong hostId)
        {
            hostId = 0;
            if (game != GameValue) return "Ce salon n'est pas une partie Dubito";
            if (protocol != ProtocolInfo.Version.ToString(CultureInfo.InvariantCulture))
                return "Version différente : l'hôte et le client doivent utiliser la même version.";
            if (!TryParseLobbyCode(host, out hostId)) return ConnectionFailed;
            return null;
        }

        // Protocol counts UTF-16 characters. Never split a surrogate pair at its limit,
        // and replace malformed UTF-16 so the strict protocol UTF-8 encoder can accept it.
        public static string TruncatePlayerName(string name)
        {
            if (string.IsNullOrEmpty(name)) return "";
            var result = new StringBuilder();
            for (int i = 0; i < name.Length && result.Length < ProtocolInfo.MaxPlayerNameCharacters; i++)
            {
                char ch = name[i];
                if (char.IsHighSurrogate(ch) && i + 1 < name.Length && char.IsLowSurrogate(name[i + 1]))
                {
                    if (result.Length + 2 > ProtocolInfo.MaxPlayerNameCharacters) break;
                    result.Append(ch).Append(name[++i]);
                }
                else result.Append(char.IsSurrogate(ch) ? '\uFFFD' : ch);
            }
            return result.ToString();
        }
    }
}
