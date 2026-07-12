#include "DubitoLobbyView.h"

FDubitoLobbyView BuildDubitoLobbyView(const FDubitoLobbyInputs& Inputs)
{
	FDubitoLobbyView View;
	View.bInLobby = Inputs.Phase == EDubitoPhase::Lobby;

	// Copy seats and sort by seat index (host first), breaking ties by player id for determinism.
	for (const FDubitoLobbySeatInput& SeatInput : Inputs.Seats)
	{
		if (SeatInput.PlayerId == DubitoConstants::NoPlayerId || SeatInput.SeatIndex < 0)
		{
			continue;
		}

		FDubitoLobbySeatView SeatView;
		SeatView.PlayerId = SeatInput.PlayerId;
		SeatView.SeatIndex = SeatInput.SeatIndex;
		SeatView.bReady = SeatInput.bReady;
		SeatView.bIsLocal = SeatInput.bIsLocal;
		SeatView.bIsHost = SeatInput.SeatIndex == 0;
		View.Seats.Add(SeatView);
	}

	View.Seats.Sort([](const FDubitoLobbySeatView& Left, const FDubitoLobbySeatView& Right)
	{
		if (Left.SeatIndex == Right.SeatIndex)
		{
			return Left.PlayerId < Right.PlayerId;
		}
		return Left.SeatIndex < Right.SeatIndex;
	});

	View.PlayerCount = View.Seats.Num();
	for (const FDubitoLobbySeatView& SeatView : View.Seats)
	{
		if (SeatView.bReady)
		{
			++View.ReadyCount;
		}

		if (SeatView.bIsLocal)
		{
			View.bValid = true;
			View.LocalSeatIndex = SeatView.SeatIndex;
			View.bLocalReady = SeatView.bReady;
		}
	}

	// The host is the listen-server instance, identified by network authority rather than by a seat
	// index, so a client that happens to hold seat 0 is never mistaken for the host.
	View.bIsLocalHost = View.bValid && Inputs.bLocalIsHost;

	// Resolve the most fundamental blocker first so the explanation is stable.
	if (!View.bInLobby)
	{
		View.StartBlockedReason = EDubitoLobbyStartBlockedReason::AlreadyStarted;
	}
	else if (!View.bIsLocalHost)
	{
		View.StartBlockedReason = EDubitoLobbyStartBlockedReason::NotHost;
	}
	else if (View.PlayerCount < DubitoConstants::MinPlayers || View.PlayerCount > DubitoConstants::MaxPlayers)
	{
		View.StartBlockedReason = EDubitoLobbyStartBlockedReason::WaitingForPlayers;
	}
	else if (View.ReadyCount < View.PlayerCount)
	{
		View.StartBlockedReason = EDubitoLobbyStartBlockedReason::NotAllReady;
	}
	else
	{
		View.StartBlockedReason = EDubitoLobbyStartBlockedReason::None;
	}

	View.bCanStart = View.StartBlockedReason == EDubitoLobbyStartBlockedReason::None;
	return View;
}
