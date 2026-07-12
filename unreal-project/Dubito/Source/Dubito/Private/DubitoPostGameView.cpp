#include "DubitoPostGameView.h"

FDubitoPostGameView BuildDubitoPostGameView(const FDubitoGameOverInfo& GameOver, int32 LocalPlayerId)
{
	FDubitoPostGameView View;

	// A real game-over names a reason; the default-constructed payload does not.
	View.bValid = GameOver.Reason != EDubitoGameOverReason::Unknown;
	if (!View.bValid)
	{
		return View;
	}

	View.Reason = GameOver.Reason;
	View.WinnerId = GameOver.WinnerId;
	View.bHasWinner = GameOver.WinnerId != DubitoConstants::NoPlayerId;

	const bool bHasLocal = LocalPlayerId != DubitoConstants::NoPlayerId;
	View.bLocalIsWinner = bHasLocal && View.bHasWinner && LocalPlayerId == GameOver.WinnerId;

	if (!View.bHasWinner)
	{
		View.Result = EDubitoPostGameResult::NoWinner;
	}
	else if (!bHasLocal)
	{
		View.Result = EDubitoPostGameResult::Spectator;
	}
	else if (View.bLocalIsWinner)
	{
		View.Result = EDubitoPostGameResult::LocalWin;
	}
	else
	{
		View.Result = EDubitoPostGameResult::LocalLoss;
	}

	return View;
}
