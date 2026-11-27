using System;
using System.Globalization;
using System.Linq;
using Dubito.Networking;
using Dubito.Protocol;
using NUnit.Framework;

namespace Dubito.Tests.Networking
{
    public class SteamLobbyRulesTests
    {
        const string Lobby = "109775241234567890";
        const string Host = "76561198012345678";
        static string Version => ProtocolInfo.Version.ToString(CultureInfo.InvariantCulture);

        [TestCase(null)]
        [TestCase("")]
        [TestCase("0")]
        [TestCase("-1")]
        [TestCase("+123")]
        [TestCase("1.5")]
        [TestCase("1e5")]
        [TestCase("123 456")]
        [TestCase("18446744073709551616")]
        [TestCase("steam://joinlobby/480/123")]
        public void InvalidCodeIsRejected(string code) => Assert.That(SteamLobbyRules.TryParseLobbyCode(code, out _), Is.False);

        [TestCase(Lobby)]
        [TestCase(" 109775241234567890\r\n")]
        public void DecimalCodeCanBePastedWithSurroundingWhitespace(string code)
        {
            Assert.That(SteamLobbyRules.TryParseLobbyCode(code, out ulong id), Is.True);
            Assert.That(id, Is.EqualTo(109775241234567890UL));
        }

        [Test]
        public void LaunchInvitationIsReadFromArgumentTokens()
        {
            Assert.That(SteamLobbyRules.TryParseConnectLobby(new[] { "Dubito.exe", "-windowed", "+connect_lobby", Lobby, "-other" }, out ulong id), Is.True);
            Assert.That(id, Is.EqualTo(109775241234567890UL));
        }

        [Test]
        public void InvalidOrMissingLaunchInvitationIsIgnored()
        {
            foreach (var args in new[] { null, Array.Empty<string>(), new[] { "-connect_lobby", Lobby },
                new[] { "+connect_lobby" }, new[] { "+connect_lobby", "-windowed" }, new[] { "+connect_lobby", "0" } })
            {
                Assert.That(SteamLobbyRules.TryParseConnectLobby(args, out ulong id), Is.False);
                Assert.That(id, Is.Zero);
            }
        }

        [TestCase(null)]
        [TestCase("")]
        [TestCase("0")]
        [TestCase("true")]
        [TestCase("Dubito")]
        public void SharedSpacewarLobbyMustHaveExactDubitoMarker(string marker)
        {
            Assert.That(SteamLobbyRules.ValidateLobbyData(marker, Version, Host, out ulong id), Is.EqualTo("Ce salon n'est pas une partie Dubito"));
            Assert.That(id, Is.Zero);
        }

        [Test]
        public void GameMarkerIsCheckedBeforeVersion()
        {
            Assert.That(SteamLobbyRules.ValidateLobbyData(null, null, null, out _), Is.EqualTo("Ce salon n'est pas une partie Dubito"));
        }

        [TestCase(null)]
        [TestCase("")]
        [TestCase("-1")]
        [TestCase("garbage")]
        [TestCase("999999")]
        public void MissingOrDifferentProtocolIsRejectedBeforeConnecting(string version)
        {
            Assert.That(SteamLobbyRules.ValidateLobbyData("1", version, Host, out ulong id), Does.StartWith("Version différente"));
            Assert.That(id, Is.Zero);
        }

        [TestCase(null)]
        [TestCase("")]
        [TestCase("0")]
        [TestCase("-1")]
        [TestCase("localhost")]
        public void LobbyRequiresNumericNonzeroHost(string host)
        {
            Assert.That(SteamLobbyRules.ValidateLobbyData("1", Version, host, out _), Is.EqualTo(SteamLobbyRules.ConnectionFailed));
        }

        [Test]
        public void MatchingLobbyReturnsHostSteamIdentity()
        {
            Assert.That(SteamLobbyRules.ValidateLobbyData("1", Version, Host, out ulong id), Is.Null);
            Assert.That(id, Is.EqualTo(76561198012345678UL));
        }

        [TestCase(null, "")]
        [TestCase("", "")]
        [TestCase("Léa", "Léa")]
        [TestCase("Anna 🎴", "Anna 🎴")]
        public void ShortPersonaNamesArePreserved(string input, string expected) =>
            Assert.That(SteamLobbyRules.TruncatePlayerName(input), Is.EqualTo(expected));

        [Test]
        public void LongNameIsLimitedToProtocolCharacters()
        {
            Assert.That(SteamLobbyRules.TruncatePlayerName(new string('a', 80)), Is.EqualTo(new string('a', 32)));
        }

        [Test]
        public void TruncationDoesNotSplitEmojiAtBoundary()
        {
            Assert.That(SteamLobbyRules.TruncatePlayerName(new string('a', 31) + "🎴"), Is.EqualTo(new string('a', 31)));
            Assert.That(SteamLobbyRules.TruncatePlayerName(new string('a', 30) + "🎴x"), Is.EqualTo(new string('a', 30) + "🎴"));
        }

        [Test]
        public void SanitizedPersonasAlwaysSurviveTheRealHelloCodec()
        {
            foreach (string input in new[] { new string('a', 31) + "🎴", string.Concat(Enumerable.Repeat("🎴", 40)),
                "bad\uD800name\uDC00", new string('é', 90), null })
            {
                string name = SteamLobbyRules.TruncatePlayerName(input);
                var decoded = ProtocolCodec.Decode(ProtocolCodec.Encode(new HelloMessage(ProtocolInfo.Version, name)));
                Assert.That(decoded.Success, Is.True);
                Assert.That(((HelloMessage)decoded.Message).PlayerName, Is.EqualTo(name));
                Assert.That(name.Length, Is.LessThanOrEqualTo(32));
            }
        }

        [Test]
        public void SteamNamesReachLobbyAndWelcomeThroughExistingAuthority()
        {
            var authority = new HostAuthority();
            string name = SteamLobbyRules.TruncatePlayerName("Léa 🎴");
            var packets = authority.Receive(0, ProtocolCodec.Encode(new HelloMessage(ProtocolInfo.Version, name)));
            var lobby = (LobbyStateMessage)ProtocolCodec.Decode(packets.Single().Payload).Message;
            Assert.That(lobby.Players[0].PlayerName, Is.EqualTo(name));
            authority.Receive(1, ProtocolCodec.Encode(new HelloMessage(ProtocolInfo.Version, "Paul")));
            var welcome = authority.StartGame(new Dubito.Core.SystemRandomSource())
                .Select(p => ProtocolCodec.Decode(p.Payload).Message).OfType<WelcomeMessage>().First();
            Assert.That(welcome.PlayerNames[0], Is.EqualTo(name));
        }
    }
}
