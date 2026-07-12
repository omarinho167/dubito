#include "Misc/AutomationTest.h"
#include "DubitoConstants.h"
#include "DubitoLobbyView.h"
#include "DubitoMatchState.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
	constexpr EAutomationTestFlags DubitoLobbyFlags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	FDubitoLobbySeatInput MakeSeat(int32 PlayerId, int32 SeatIndex, bool bReady, bool bIsLocal)
	{
		FDubitoLobbySeatInput Seat;
		Seat.PlayerId = PlayerId;
		Seat.SeatIndex = SeatIndex;
		Seat.bReady = bReady;
		Seat.bIsLocal = bIsLocal;
		return Seat;
	}

	// Host (seat 0, local) plus one remote player, both ready: the startable case.
	FDubitoLobbyInputs MakeStartableHostInputs()
	{
		FDubitoLobbyInputs In;
		In.Phase = EDubitoPhase::Lobby;
		In.bLocalIsHost = true;
		In.Seats.Add(MakeSeat(1, 0, true, true));
		In.Seats.Add(MakeSeat(2, 1, true, false));
		return In;
	}
}

// --- No local seat: nothing to present ------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoLobbyInvalidTest, "Dubito.Unreal.Lobby.Invalid", DubitoLobbyFlags)
bool FDubitoLobbyInvalidTest::RunTest(const FString&)
{
	FDubitoLobbyInputs In;
	In.Phase = EDubitoPhase::Lobby;
	In.Seats.Add(MakeSeat(1, 0, true, false)); // a remote player, but no local seat
	FDubitoLobbyView View = BuildDubitoLobbyView(In);
	TestFalse(TEXT("No local seat is invalid"), View.bValid);
	TestFalse(TEXT("Cannot start without a local seat"), View.bCanStart);
	return true;
}

// --- Host with everyone ready can start -----------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoLobbyHostCanStartTest, "Dubito.Unreal.Lobby.HostCanStart", DubitoLobbyFlags)
bool FDubitoLobbyHostCanStartTest::RunTest(const FString&)
{
	FDubitoLobbyView View = BuildDubitoLobbyView(MakeStartableHostInputs());
	TestTrue(TEXT("Valid with a local seat"), View.bValid);
	TestTrue(TEXT("In lobby"), View.bInLobby);
	TestTrue(TEXT("Local player is host at seat 0"), View.bIsLocalHost);
	TestEqual(TEXT("Two seated players"), View.PlayerCount, 2);
	TestEqual(TEXT("Both ready"), View.ReadyCount, 2);
	TestTrue(TEXT("Host can start when full and all ready"), View.bCanStart);
	TestEqual(TEXT("No blocked reason"), View.StartBlockedReason, EDubitoLobbyStartBlockedReason::None);
	// Seats are sorted host-first.
	TestEqual(TEXT("Seat 0 first"), View.Seats[0].SeatIndex, 0);
	TestTrue(TEXT("Seat 0 flagged host"), View.Seats[0].bIsHost);
	return true;
}

// --- Each blocked reason explains itself in precedence order ---------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoLobbyBlockedTest, "Dubito.Unreal.Lobby.BlockedReasons", DubitoLobbyFlags)
bool FDubitoLobbyBlockedTest::RunTest(const FString&)
{
	// Non-host local player may never start, even sitting at seat 0.
	{
		FDubitoLobbyInputs In;
		In.Phase = EDubitoPhase::Lobby;
		In.bLocalIsHost = false;
		In.Seats.Add(MakeSeat(1, 0, true, true));  // local client that happens to hold seat 0
		In.Seats.Add(MakeSeat(2, 1, true, false)); // the host, remote from this instance
		FDubitoLobbyView View = BuildDubitoLobbyView(In);
		TestFalse(TEXT("Non-host cannot start"), View.bCanStart);
		TestFalse(TEXT("Non-host flagged"), View.bIsLocalHost);
		TestEqual(TEXT("Not-host reason"), View.StartBlockedReason, EDubitoLobbyStartBlockedReason::NotHost);
	}

	// Host alone: not enough players.
	{
		FDubitoLobbyInputs In;
		In.Phase = EDubitoPhase::Lobby;
		In.bLocalIsHost = true;
		In.Seats.Add(MakeSeat(1, 0, true, true));
		FDubitoLobbyView View = BuildDubitoLobbyView(In);
		TestFalse(TEXT("One player cannot start"), View.bCanStart);
		TestEqual(TEXT("Waiting-for-players reason"), View.StartBlockedReason, EDubitoLobbyStartBlockedReason::WaitingForPlayers);
	}

	// Host ready but a remote player is not.
	{
		FDubitoLobbyInputs In = MakeStartableHostInputs();
		In.Seats[1].bReady = false;
		FDubitoLobbyView View = BuildDubitoLobbyView(In);
		TestFalse(TEXT("Not all ready cannot start"), View.bCanStart);
		TestEqual(TEXT("One ready of two"), View.ReadyCount, 1);
		TestEqual(TEXT("Not-all-ready reason"), View.StartBlockedReason, EDubitoLobbyStartBlockedReason::NotAllReady);
	}

	// Match already dealt: lobby is gone.
	{
		FDubitoLobbyInputs In = MakeStartableHostInputs();
		In.Phase = EDubitoPhase::PlayerTurn;
		FDubitoLobbyView View = BuildDubitoLobbyView(In);
		TestFalse(TEXT("Started match is not in lobby"), View.bInLobby);
		TestFalse(TEXT("Cannot start again"), View.bCanStart);
		TestEqual(TEXT("Already-started reason"), View.StartBlockedReason, EDubitoLobbyStartBlockedReason::AlreadyStarted);
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
