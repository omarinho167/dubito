#include "Misc/AutomationTest.h"
#include "DubitoConstants.h"
#include "DubitoGameOver.h"
#include "DubitoPostGameView.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
	constexpr EAutomationTestFlags DubitoPostGameFlags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	FDubitoGameOverInfo MakeGameOver(int32 WinnerId, EDubitoGameOverReason Reason)
	{
		FDubitoGameOverInfo Info;
		Info.WinnerId = WinnerId;
		Info.Reason = Reason;
		return Info;
	}
}

// --- Default game-over is not presented ------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoPostGameInvalidTest, "Dubito.Unreal.PostGame.Invalid", DubitoPostGameFlags)
bool FDubitoPostGameInvalidTest::RunTest(const FString&)
{
	FDubitoPostGameView View = BuildDubitoPostGameView(FDubitoGameOverInfo(), 10);
	TestFalse(TEXT("A default game-over is not valid"), View.bValid);
	TestEqual(TEXT("No result for a default game-over"), View.Result, EDubitoPostGameResult::None);
	return true;
}

// --- Local perspective: win / loss / spectator -----------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoPostGamePerspectiveTest, "Dubito.Unreal.PostGame.Perspective", DubitoPostGameFlags)
bool FDubitoPostGamePerspectiveTest::RunTest(const FString&)
{
	const FDubitoGameOverInfo GameOver = MakeGameOver(10, EDubitoGameOverReason::PendingWinDoubtFailed);

	FDubitoPostGameView Winner = BuildDubitoPostGameView(GameOver, 10);
	TestTrue(TEXT("Valid game-over"), Winner.bValid);
	TestTrue(TEXT("Has a winner"), Winner.bHasWinner);
	TestEqual(TEXT("Winner id passes through"), Winner.WinnerId, 10);
	TestTrue(TEXT("Local player is the winner"), Winner.bLocalIsWinner);
	TestEqual(TEXT("Local win result"), Winner.Result, EDubitoPostGameResult::LocalWin);
	TestEqual(TEXT("Reason passes through"), Winner.Reason, EDubitoGameOverReason::PendingWinDoubtFailed);

	FDubitoPostGameView Loser = BuildDubitoPostGameView(GameOver, 20);
	TestFalse(TEXT("A non-winner is not the winner"), Loser.bLocalIsWinner);
	TestEqual(TEXT("Local loss result"), Loser.Result, EDubitoPostGameResult::LocalLoss);

	FDubitoPostGameView Observer = BuildDubitoPostGameView(GameOver, DubitoConstants::NoPlayerId);
	TestEqual(TEXT("No local identity is a spectator"), Observer.Result, EDubitoPostGameResult::Spectator);
	TestFalse(TEXT("A spectator is not the winner"), Observer.bLocalIsWinner);
	return true;
}

// --- No-winner game-over ----------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoPostGameNoWinnerTest, "Dubito.Unreal.PostGame.NoWinner", DubitoPostGameFlags)
bool FDubitoPostGameNoWinnerTest::RunTest(const FString&)
{
	const FDubitoGameOverInfo GameOver = MakeGameOver(DubitoConstants::NoPlayerId, EDubitoGameOverReason::NoPlayersRemaining);

	FDubitoPostGameView View = BuildDubitoPostGameView(GameOver, 10);
	TestTrue(TEXT("Valid even with no winner"), View.bValid);
	TestFalse(TEXT("No winner flagged"), View.bHasWinner);
	TestEqual(TEXT("No-winner result"), View.Result, EDubitoPostGameResult::NoWinner);
	TestFalse(TEXT("Local player is not a winner when none remain"), View.bLocalIsWinner);
	return true;
}

// --- Every rule-defined ending reason is a valid, presentable game-over -----
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoPostGameReasonsTest, "Dubito.Unreal.PostGame.Reasons", DubitoPostGameFlags)
bool FDubitoPostGameReasonsTest::RunTest(const FString&)
{
	// Last-card play then a correct/wrong final Doubt, a timeout confirmation, and a
	// last-player-standing ending all carry a winner and read as a valid result.
	const EDubitoGameOverReason WinningReasons[] = {
		EDubitoGameOverReason::PendingWinDoubtFailed,       // wrong final doubt confirms the win
		EDubitoGameOverReason::PendingWinDeclined,          // next player played instead of doubting
		EDubitoGameOverReason::PendingWinTimeout,           // final doubt window timed out
		EDubitoGameOverReason::PendingWinResponderDisconnected,
		EDubitoGameOverReason::LastPlayerStanding
	};

	for (const EDubitoGameOverReason Reason : WinningReasons)
	{
		FDubitoPostGameView View = BuildDubitoPostGameView(MakeGameOver(10, Reason), 10);
		TestTrue(TEXT("Winning reason is a valid game-over"), View.bValid);
		TestTrue(TEXT("Winning reason has a winner"), View.bHasWinner);
		TestEqual(TEXT("Winning reason reads as a local win"), View.Result, EDubitoPostGameResult::LocalWin);
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
