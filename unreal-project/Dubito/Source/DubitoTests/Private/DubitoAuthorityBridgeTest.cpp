#include "Misc/AutomationTest.h"
#include "DubitoAnnouncement.h"
#include "DubitoCard.h"
#include "DubitoConstants.h"
#include "DubitoGameMode.h"
#include "DubitoGameState.h"
#include "DubitoHand.h"
#include "DubitoPlayerController.h"
#include "DubitoPlayerState.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "UObject/Package.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
	constexpr EAutomationTestFlags DubitoAuthorityFlags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	FDubitoCard C(EDubitoSuit Suit, int32 Rank)
	{
		return FDubitoCard(Suit, Rank);
	}

	FDubitoHand MakeHand(const TArray<FDubitoCard>& Cards)
	{
		FDubitoHand Hand;
		for (const FDubitoCard& Card : Cards)
		{
			Hand.Add(Card);
		}
		return Hand;
	}

	TMap<int32, FDubitoHand> MakeTwoPlayerHands()
	{
		TMap<int32, FDubitoHand> Hands;
		Hands.Add(10, MakeHand({ C(EDubitoSuit::Clubs, 7), C(EDubitoSuit::Clubs, 2) }));
		Hands.Add(20, MakeHand({ C(EDubitoSuit::Spades, 7), C(EDubitoSuit::Hearts, 4) }));
		return Hands;
	}

	UWorld* CreateAuthorityTestWorld()
	{
		check(GEngine);

		const FName WorldName = MakeUniqueObjectName(GetTransientPackage(), UWorld::StaticClass(), TEXT("DubitoAuthorityTestWorld"));
		UWorld* World = NewObject<UWorld>(GetTransientPackage(), WorldName);
		World->WorldType = EWorldType::Game;
		World->AddToRoot();

		FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
		WorldContext.SetCurrentWorld(World);

		World->InitializeNewWorld(UWorld::InitializationValues()
			.InitializeScenes(false)
			.AllowAudioPlayback(false)
			.RequiresHitProxies(false)
			.CreatePhysicsScene(false)
			.CreateNavigation(false)
			.CreateAISystem(false)
			.ShouldSimulatePhysics(false)
			.EnableTraceCollision(false)
			.SetTransactional(false)
			.CreateFXSystem(false)
			.SetDefaultGameMode(ADubitoGameMode::StaticClass()));

		return World;
	}

	void DestroyAuthorityTestWorld(UWorld* World)
	{
		if (!World)
		{
			return;
		}

		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
		World->RemoveFromRoot();
	}

	ADubitoGameMode* SpawnAuthorityGameMode(UWorld* World)
	{
		if (!World)
		{
			return nullptr;
		}

		if (!World->GetGameState())
		{
			ADubitoGameState* PublicGameState = World->SpawnActor<ADubitoGameState>();
			World->SetGameState(PublicGameState);
		}

		ADubitoGameMode* GameMode = World->SpawnActor<ADubitoGameMode>();
		if (GameMode)
		{
			GameMode->GameState = World->GetGameState();
			World->CopyGameState(GameMode, World->GetGameState());
		}
		return GameMode;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoAuthorityBridgeStartTest, "Dubito.Unreal.AuthorityBridge.Start", DubitoAuthorityFlags)
bool FDubitoAuthorityBridgeStartTest::RunTest(const FString&)
{
	UWorld* World = CreateAuthorityTestWorld();
	ADubitoGameMode* GameMode = SpawnAuthorityGameMode(World);
	TestNotNull(TEXT("GameMode spawns in transient authority world"), GameMode);

	if (GameMode)
	{
		TestEqual(TEXT("Too few players rejected"), static_cast<int32>(GameMode->StartAuthoritativeMatchFromHands({ 10 }, MakeTwoPlayerHands())), static_cast<int32>(EDubitoAuthorityStartResult::InvalidPlayerCount));
		TestFalse(TEXT("Rejected start does not mark match started"), GameMode->HasAuthoritativeMatchStarted());

		TestEqual(TEXT("Duplicate player ids rejected"), static_cast<int32>(GameMode->StartAuthoritativeMatchFromHands({ 10, 10 }, MakeTwoPlayerHands())), static_cast<int32>(EDubitoAuthorityStartResult::DuplicatePlayerId));
		TestFalse(TEXT("Duplicate rejection does not mutate match started flag"), GameMode->HasAuthoritativeMatchStarted());

		TMap<int32, FDubitoHand> MissingHand = MakeTwoPlayerHands();
		MissingHand.Remove(20);
		TestEqual(TEXT("Missing dealt hand rejected"), static_cast<int32>(GameMode->StartAuthoritativeMatchFromHands({ 10, 20 }, MissingHand)), static_cast<int32>(EDubitoAuthorityStartResult::MissingHand));
		TestFalse(TEXT("Missing hand rejection does not mutate match started flag"), GameMode->HasAuthoritativeMatchStarted());

		TestEqual(TEXT("Valid start from hands succeeds"), static_cast<int32>(GameMode->StartAuthoritativeMatchFromHands({ 10, 20 }, MakeTwoPlayerHands())), static_cast<int32>(EDubitoAuthorityStartResult::Success));
		TestTrue(TEXT("Valid start marks match started"), GameMode->HasAuthoritativeMatchStarted());

		const FDubitoMatchState& State = GameMode->GetAuthoritativeMatchState();
		TestEqual(TEXT("GameMode owns PlayerTurn state"), static_cast<int32>(State.Phase), static_cast<int32>(EDubitoPhase::PlayerTurn));
		TestEqual(TEXT("Active player is first turn-order id"), State.ActivePlayerId(), 10);
		TestEqual(TEXT("Authoritative hidden hands are held server-side"), State.Hands.Num(), 2);
		TestEqual(TEXT("Public hand ledger mirrors dealt size for player 10"), State.PublicHandCounts[10], 2);
		TestEqual(TEXT("Public hand ledger mirrors dealt size for player 20"), State.PublicHandCounts[20], 2);
	}

	DestroyAuthorityTestWorld(World);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoAuthorityBridgePublicStateTest, "Dubito.Unreal.AuthorityBridge.PublicState", DubitoAuthorityFlags)
bool FDubitoAuthorityBridgePublicStateTest::RunTest(const FString&)
{
	UWorld* World = CreateAuthorityTestWorld();
	ADubitoGameMode* GameMode = SpawnAuthorityGameMode(World);
	ADubitoGameState* PublicGameState = World ? World->GetGameState<ADubitoGameState>() : nullptr;
	TestNotNull(TEXT("GameMode spawns in transient authority world"), GameMode);
	TestNotNull(TEXT("Dubito GameState exists in authority world"), PublicGameState);

	if (GameMode && PublicGameState)
	{
		TestEqual(TEXT("GameMode declares Dubito GameState class"), GameMode->GameStateClass.Get(), ADubitoGameState::StaticClass());

		auto TestReplicatedProperty = [this](const TCHAR* PropertyName)
		{
			const FProperty* Property = FindFProperty<FProperty>(ADubitoGameState::StaticClass(), PropertyName);
			TestNotNull(FString::Printf(TEXT("%s property exists"), PropertyName), Property);
			if (Property)
			{
				TestTrue(FString::Printf(TEXT("%s is marked for replication"), PropertyName), Property->HasAnyPropertyFlags(CPF_Net));
			}
		};

		TestReplicatedProperty(TEXT("PublicPhase"));
		TestReplicatedProperty(TEXT("ActivePlayerId"));
		TestReplicatedProperty(TEXT("RoundValue"));
		TestReplicatedProperty(TEXT("bHasPreviousPublicClaim"));
		TestReplicatedProperty(TEXT("PreviousClaimantId"));
		TestReplicatedProperty(TEXT("PreviousPublicAnnouncement"));
		TestReplicatedProperty(TEXT("ClaimedPileCount"));
		TestReplicatedProperty(TEXT("bHasPendingWin"));
		TestReplicatedProperty(TEXT("TurnDeadlineServerTimeSeconds"));

		TestEqual(TEXT("Valid start succeeds"), static_cast<int32>(GameMode->StartAuthoritativeMatchFromHands({ 10, 20 }, MakeTwoPlayerHands())), static_cast<int32>(EDubitoAuthorityStartResult::Success));

		TestEqual(TEXT("Public phase mirrors authoritative phase"), static_cast<int32>(PublicGameState->GetPublicPhase()), static_cast<int32>(EDubitoPhase::PlayerTurn));
		TestEqual(TEXT("Public active player mirrors turn"), PublicGameState->GetActivePlayerId(), 10);
		TestEqual(TEXT("Public round starts unset"), PublicGameState->GetRoundValue(), DubitoConstants::NoRoundValue);
		TestFalse(TEXT("No previous public claim after setup"), PublicGameState->HasPreviousPublicClaim());
		TestEqual(TEXT("Public claimed pile ledger starts empty"), PublicGameState->GetClaimedPileCount(), 0);
		TestFalse(TEXT("No pending win after setup"), PublicGameState->HasPendingWin());
		TestTrue(TEXT("Turn deadline is published for active turn"), PublicGameState->GetTurnDeadlineServerTimeSeconds() > PublicGameState->GetServerWorldTimeSeconds());

		TestEqual(TEXT("Play with bluffable claimed count succeeds"), static_cast<int32>(GameMode->AuthorityPlayCards(10, { C(EDubitoSuit::Clubs, 7) }, FDubitoAnnouncement(7, 2))), static_cast<int32>(EDubitoPlayValidity::Valid));

		const FDubitoAnnouncement PublicClaim = PublicGameState->GetPreviousPublicAnnouncement();
		TestEqual(TEXT("Public active player advances"), PublicGameState->GetActivePlayerId(), 20);
		TestEqual(TEXT("Public round value locks to claim value"), PublicGameState->GetRoundValue(), 7);
		TestTrue(TEXT("Previous public claim is available"), PublicGameState->HasPreviousPublicClaim());
		TestEqual(TEXT("Previous public claimant is replicated"), PublicGameState->GetPreviousClaimantId(), 10);
		TestEqual(TEXT("Previous public claim value is replicated"), PublicClaim.ClaimedValue, 7);
		TestEqual(TEXT("Previous public claim count is replicated as claimed, not actual"), PublicClaim.ClaimedCount, 2);
		TestEqual(TEXT("Claimed pile ledger follows claimed count"), PublicGameState->GetClaimedPileCount(), 2);
		TestFalse(TEXT("No pending win while actual hand still has a card"), PublicGameState->HasPendingWin());

		TestTrue(TEXT("Active player discards through core"), GameMode->AuthorityDiscard(20));
		TestEqual(TEXT("Discard clears public claimed pile ledger"), PublicGameState->GetClaimedPileCount(), 0);
		TestEqual(TEXT("Discard clears public round"), PublicGameState->GetRoundValue(), DubitoConstants::NoRoundValue);
		TestFalse(TEXT("Discard clears previous public claim"), PublicGameState->HasPreviousPublicClaim());
		TestEqual(TEXT("Discard publishes next active player"), PublicGameState->GetActivePlayerId(), 10);
	}

	DestroyAuthorityTestWorld(World);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoAuthorityBridgePendingWinPublicStateTest, "Dubito.Unreal.AuthorityBridge.PendingWinPublicState", DubitoAuthorityFlags)
bool FDubitoAuthorityBridgePendingWinPublicStateTest::RunTest(const FString&)
{
	UWorld* World = CreateAuthorityTestWorld();
	ADubitoGameMode* GameMode = SpawnAuthorityGameMode(World);
	ADubitoGameState* PublicGameState = World ? World->GetGameState<ADubitoGameState>() : nullptr;
	TestNotNull(TEXT("GameMode spawns in transient authority world"), GameMode);
	TestNotNull(TEXT("Dubito GameState exists in authority world"), PublicGameState);

	if (GameMode && PublicGameState)
	{
		TMap<int32, FDubitoHand> Hands;
		Hands.Add(10, MakeHand({ C(EDubitoSuit::Clubs, 7) }));
		Hands.Add(20, MakeHand({ C(EDubitoSuit::Spades, 7), C(EDubitoSuit::Hearts, 4) }));

		TestEqual(TEXT("Valid start succeeds"), static_cast<int32>(GameMode->StartAuthoritativeMatchFromHands({ 10, 20 }, Hands)), static_cast<int32>(EDubitoAuthorityStartResult::Success));
		TestEqual(TEXT("Last-card play succeeds"), static_cast<int32>(GameMode->AuthorityPlayCards(10, { C(EDubitoSuit::Clubs, 7) }, FDubitoAnnouncement(7, 1))), static_cast<int32>(EDubitoPlayValidity::Valid));

		TestTrue(TEXT("Pending-win flag is public"), PublicGameState->HasPendingWin());
		TestEqual(TEXT("Pending-win response player is active"), PublicGameState->GetActivePlayerId(), 20);
		TestTrue(TEXT("Pending-win claim remains public"), PublicGameState->HasPreviousPublicClaim());
		TestEqual(TEXT("Pending-win claimant remains public"), PublicGameState->GetPreviousClaimantId(), 10);
		TestTrue(TEXT("Pending-win response has a turn deadline"), PublicGameState->GetTurnDeadlineServerTimeSeconds() > PublicGameState->GetServerWorldTimeSeconds());

		GameMode->AuthorityResolveTimeout(20);
		TestEqual(TEXT("Timeout during pending-win response publishes GameOver"), static_cast<int32>(PublicGameState->GetPublicPhase()), static_cast<int32>(EDubitoPhase::GameOver));
		TestFalse(TEXT("GameOver clears pending-win flag"), PublicGameState->HasPendingWin());
		TestEqual(TEXT("GameOver clears public timer deadline"), PublicGameState->GetTurnDeadlineServerTimeSeconds(), 0.0);
	}

	DestroyAuthorityTestWorld(World);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoAuthorityBridgePrivateHandTest, "Dubito.Unreal.AuthorityBridge.PrivateHand", DubitoAuthorityFlags)
bool FDubitoAuthorityBridgePrivateHandTest::RunTest(const FString&)
{
	UWorld* World = CreateAuthorityTestWorld();
	ADubitoGameMode* GameMode = SpawnAuthorityGameMode(World);
	ADubitoPlayerController* Controller10 = World ? World->SpawnActor<ADubitoPlayerController>() : nullptr;
	ADubitoPlayerController* Controller20 = World ? World->SpawnActor<ADubitoPlayerController>() : nullptr;
	ADubitoPlayerController* DuplicateController10 = World ? World->SpawnActor<ADubitoPlayerController>() : nullptr;
	TestNotNull(TEXT("GameMode spawns in transient authority world"), GameMode);
	TestNotNull(TEXT("Controller 10 spawns"), Controller10);
	TestNotNull(TEXT("Controller 20 spawns"), Controller20);

	if (GameMode && Controller10 && Controller20)
	{
		TestEqual(TEXT("GameMode declares Dubito PlayerController class"), GameMode->PlayerControllerClass.Get(), ADubitoPlayerController::StaticClass());

		const FProperty* PrivateHandProperty = FindFProperty<FProperty>(ADubitoPlayerController::StaticClass(), TEXT("PrivateHand"));
		TestNotNull(TEXT("PrivateHand property exists"), PrivateHandProperty);
		if (PrivateHandProperty)
		{
			TestTrue(TEXT("PrivateHand is marked for replication"), PrivateHandProperty->HasAnyPropertyFlags(CPF_Net));
			TestEqual(TEXT("PrivateHand uses owner-only replication"), static_cast<int32>(ADubitoPlayerController::GetPrivateHandReplicationCondition()), static_cast<int32>(COND_OwnerOnly));
		}

		TestTrue(TEXT("Controller 10 registers for private hand sync"), GameMode->RegisterAuthorityPlayerController(Controller10, 10));
		TestTrue(TEXT("Controller 20 registers for private hand sync"), GameMode->RegisterAuthorityPlayerController(Controller20, 20));
		TestFalse(TEXT("Duplicate controller cannot bind to same player id"), GameMode->RegisterAuthorityPlayerController(DuplicateController10, 10));
		TestEqual(TEXT("Registered controller has no private hand before match start"), Controller10->GetPrivateHandCount(), 0);

		TestEqual(TEXT("Valid start succeeds"), static_cast<int32>(GameMode->StartAuthoritativeMatchFromHands({ 10, 20 }, MakeTwoPlayerHands())), static_cast<int32>(EDubitoAuthorityStartResult::Success));
		TestEqual(TEXT("Controller 10 receives exact private hand"), Controller10->GetPrivateHandCount(), 2);
		TestTrue(TEXT("Controller 10 sees owned Clubs 7"), Controller10->GetPrivateHand().Contains(C(EDubitoSuit::Clubs, 7)));
		TestTrue(TEXT("Controller 10 sees owned Clubs 2"), Controller10->GetPrivateHand().Contains(C(EDubitoSuit::Clubs, 2)));
		TestEqual(TEXT("Controller 20 receives exact private hand"), Controller20->GetPrivateHandCount(), 2);

		TestEqual(TEXT("Play succeeds"), static_cast<int32>(GameMode->AuthorityPlayCards(10, { C(EDubitoSuit::Clubs, 7) }, FDubitoAnnouncement(7, 4))), static_cast<int32>(EDubitoPlayValidity::Valid));
		TestEqual(TEXT("Controller 10 private hand removes actual card only"), Controller10->GetPrivateHandCount(), 1);
		TestFalse(TEXT("Played actual card leaves owner hand"), Controller10->GetPrivateHand().Contains(C(EDubitoSuit::Clubs, 7)));
		TestTrue(TEXT("Unplayed owned card remains in owner hand"), Controller10->GetPrivateHand().Contains(C(EDubitoSuit::Clubs, 2)));
		TestEqual(TEXT("Controller 20 private hand is unchanged before responding"), Controller20->GetPrivateHandCount(), 2);

		FDubitoRevealInfo Reveal;
		TestTrue(TEXT("Active next player resolves Doubt"), GameMode->AuthorityResolveDoubt(20, Reveal));
		TestTrue(TEXT("Doubt detects the count lie"), Reveal.bWasLie);
		TestEqual(TEXT("Controller 10 private hand receives actual pile card back"), Controller10->GetPrivateHandCount(), 2);
		TestTrue(TEXT("Transferred actual card is visible to its owner again"), Controller10->GetPrivateHand().Contains(C(EDubitoSuit::Clubs, 7)));

		GameMode->AuthorityHandleDisconnect(20);
		TestEqual(TEXT("Disconnected owner private hand is cleared"), Controller20->GetPrivateHandCount(), 0);
	}

	DestroyAuthorityTestWorld(World);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoAuthorityBridgeServerActionsTest, "Dubito.Unreal.AuthorityBridge.ServerActions", DubitoAuthorityFlags)
bool FDubitoAuthorityBridgeServerActionsTest::RunTest(const FString&)
{
	UWorld* World = CreateAuthorityTestWorld();
	ADubitoGameMode* GameMode = SpawnAuthorityGameMode(World);
	ADubitoGameState* PublicGameState = World ? World->GetGameState<ADubitoGameState>() : nullptr;
	ADubitoPlayerController* Controller10 = World ? World->SpawnActor<ADubitoPlayerController>() : nullptr;
	ADubitoPlayerController* Controller20 = World ? World->SpawnActor<ADubitoPlayerController>() : nullptr;
	ADubitoPlayerState* Player10 = World ? World->SpawnActor<ADubitoPlayerState>() : nullptr;
	ADubitoPlayerState* Player20 = World ? World->SpawnActor<ADubitoPlayerState>() : nullptr;
	TestNotNull(TEXT("GameMode spawns in transient authority world"), GameMode);
	TestNotNull(TEXT("Dubito GameState exists in authority world"), PublicGameState);
	TestNotNull(TEXT("Controller 10 spawns"), Controller10);
	TestNotNull(TEXT("Controller 20 spawns"), Controller20);
	TestNotNull(TEXT("Player 10 state spawns"), Player10);
	TestNotNull(TEXT("Player 20 state spawns"), Player20);

	if (GameMode && PublicGameState && Controller10 && Controller20 && Player10 && Player20)
	{
		auto TestServerRpc = [this](const TCHAR* FunctionName)
		{
			const UFunction* Function = ADubitoPlayerController::StaticClass()->FindFunctionByName(FunctionName);
			TestNotNull(FString::Printf(TEXT("%s RPC exists"), FunctionName), Function);
			if (Function)
			{
				TestTrue(FString::Printf(TEXT("%s is a net RPC"), FunctionName), Function->HasAnyFunctionFlags(FUNC_Net));
				TestTrue(FString::Printf(TEXT("%s runs on the server"), FunctionName), Function->HasAnyFunctionFlags(FUNC_NetServer));
				TestTrue(FString::Printf(TEXT("%s is reliable"), FunctionName), Function->HasAnyFunctionFlags(FUNC_NetReliable));
				TestFalse(FString::Printf(TEXT("%s does not use abusive RPC validation for ordinary gameplay"), FunctionName), Function->HasAnyFunctionFlags(FUNC_NetValidate));
			}
		};

		TestServerRpc(TEXT("ServerSetReady"));
		TestServerRpc(TEXT("ServerStartMatch"));
		TestServerRpc(TEXT("ServerPlayCards"));
		TestServerRpc(TEXT("ServerDoubt"));
		TestServerRpc(TEXT("ServerDiscard"));

		Controller10->SetPlayerState(Player10);
		Controller20->SetPlayerState(Player20);

		TestTrue(TEXT("Player 10 public identity registers"), GameMode->RegisterAuthorityPlayerState(Player10, 10, 0));
		TestTrue(TEXT("Player 20 public identity registers"), GameMode->RegisterAuthorityPlayerState(Player20, 20, 1));
		TestTrue(TEXT("Controller 10 private identity registers"), GameMode->RegisterAuthorityPlayerController(Controller10, 10));
		TestTrue(TEXT("Controller 20 private identity registers"), GameMode->RegisterAuthorityPlayerController(Controller20, 20));
		TestEqual(TEXT("Controller 10 resolves its server player id"), Controller10->ResolveAuthorityPlayerId(), 10);
		TestEqual(TEXT("Controller 20 resolves its server player id"), Controller20->ResolveAuthorityPlayerId(), 20);

		Controller10->RequestReady(true);
		TestTrue(TEXT("Ready request reaches PlayerState through server action"), Player10->IsReady());
		Controller10->RequestStartMatch(404);
		TestFalse(TEXT("Start request waits for all registered players to be ready"), GameMode->HasAuthoritativeMatchStarted());

		Controller20->RequestReady(true);
		TestTrue(TEXT("Second ready request reaches PlayerState through server action"), Player20->IsReady());
		Controller10->RequestStartMatch(404);
		TestTrue(TEXT("Start request reaches core setup through server action"), GameMode->HasAuthoritativeMatchStarted());
		TestEqual(TEXT("Registered seat order drives first active player"), GameMode->GetAuthoritativeMatchState().ActivePlayerId(), 10);
		TestEqual(TEXT("Public GameState syncs after server start action"), PublicGameState->GetActivePlayerId(), 10);
		TestEqual(TEXT("Owner private hand syncs after server start action"), Controller10->GetPrivateHandCount(), 26);

		const FDubitoCard Player10OpeningCard = Controller10->GetPrivateHand().Cards[0];
		Controller10->RequestPlayCards({ Player10OpeningCard }, FDubitoAnnouncement(Player10OpeningCard.Rank, 1));
		TestEqual(TEXT("Play request advances authoritative turn"), GameMode->GetAuthoritativeMatchState().ActivePlayerId(), 20);
		TestEqual(TEXT("Play request updates public active player"), PublicGameState->GetActivePlayerId(), 20);
		TestTrue(TEXT("Play request publishes previous claim"), PublicGameState->HasPreviousPublicClaim());
		TestEqual(TEXT("Play request updates owner private hand"), Controller10->GetPrivateHandCount(), 25);

		Controller20->RequestDoubt();
		TestEqual(TEXT("Doubt request resolves through core and returns turn after wrong Doubt"), GameMode->GetAuthoritativeMatchState().ActivePlayerId(), 10);
		TestEqual(TEXT("Doubt request clears public claim"), PublicGameState->GetClaimedPileCount(), 0);
		TestEqual(TEXT("Doubt loser receives actual pile in private hand"), Controller20->GetPrivateHandCount(), 27);

		const FDubitoCard Player10SecondCard = Controller10->GetPrivateHand().Cards[0];
		Controller10->RequestPlayCards({ Player10SecondCard }, FDubitoAnnouncement(Player10SecondCard.Rank, 1));
		TestEqual(TEXT("Second play makes player 20 active"), GameMode->GetAuthoritativeMatchState().ActivePlayerId(), 20);

		Controller20->RequestDiscard();
		TestEqual(TEXT("Discard request clears authoritative pile"), GameMode->GetAuthoritativeMatchState().ActualPile.Num(), 0);
		TestEqual(TEXT("Discard request clears public pile ledger"), PublicGameState->GetClaimedPileCount(), 0);
		TestEqual(TEXT("Discard request resets public round value"), PublicGameState->GetRoundValue(), DubitoConstants::NoRoundValue);
		TestEqual(TEXT("Discard request skips active player"), PublicGameState->GetActivePlayerId(), 10);
	}

	DestroyAuthorityTestWorld(World);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoAuthorityBridgeControlledRejectionTest, "Dubito.Unreal.AuthorityBridge.ControlledRejection", DubitoAuthorityFlags)
bool FDubitoAuthorityBridgeControlledRejectionTest::RunTest(const FString&)
{
	UWorld* World = CreateAuthorityTestWorld();
	ADubitoGameMode* GameMode = SpawnAuthorityGameMode(World);
	ADubitoGameState* PublicGameState = World ? World->GetGameState<ADubitoGameState>() : nullptr;
	ADubitoPlayerController* Controller10 = World ? World->SpawnActor<ADubitoPlayerController>() : nullptr;
	ADubitoPlayerController* Controller20 = World ? World->SpawnActor<ADubitoPlayerController>() : nullptr;
	ADubitoPlayerState* Player10 = World ? World->SpawnActor<ADubitoPlayerState>() : nullptr;
	ADubitoPlayerState* Player20 = World ? World->SpawnActor<ADubitoPlayerState>() : nullptr;
	TestNotNull(TEXT("GameMode spawns in transient authority world"), GameMode);
	TestNotNull(TEXT("Dubito GameState exists in authority world"), PublicGameState);
	TestNotNull(TEXT("Controller 10 spawns"), Controller10);
	TestNotNull(TEXT("Controller 20 spawns"), Controller20);
	TestNotNull(TEXT("Player 10 state spawns"), Player10);
	TestNotNull(TEXT("Player 20 state spawns"), Player20);

	if (GameMode && PublicGameState && Controller10 && Controller20 && Player10 && Player20)
	{
		Controller10->SetPlayerState(Player10);
		Controller20->SetPlayerState(Player20);

		TestTrue(TEXT("Player 10 public identity registers"), GameMode->RegisterAuthorityPlayerState(Player10, 10, 0));
		TestTrue(TEXT("Player 20 public identity registers"), GameMode->RegisterAuthorityPlayerState(Player20, 20, 1));
		TestTrue(TEXT("Controller 10 private identity registers"), GameMode->RegisterAuthorityPlayerController(Controller10, 10));
		TestTrue(TEXT("Controller 20 private identity registers"), GameMode->RegisterAuthorityPlayerController(Controller20, 20));

		Controller10->RequestReady(true);
		Controller10->RequestStartMatch(505);
		TestEqual(TEXT("Start before every registered player is ready is rejected"), Controller10->GetLastRejectedAction(), EDubitoServerAction::StartMatch);
		TestEqual(TEXT("Start rejection explains not-all-ready"), Controller10->GetLastRejectionReason(), EDubitoActionRejectionReason::StartNotAllReady);
		TestTrue(TEXT("Start rejection requests authority resync"), Controller10->DidLastRejectionRequestResync());
		TestFalse(TEXT("Not-all-ready start does not mutate match started"), GameMode->HasAuthoritativeMatchStarted());

		Controller20->RequestReady(true);
		Controller10->RequestStartMatch(505);
		TestTrue(TEXT("Start succeeds once registered players are ready"), GameMode->HasAuthoritativeMatchStarted());
		TestEqual(TEXT("Active player after start is seat 0"), PublicGameState->GetActivePlayerId(), 10);

		const FDubitoCard Player20OwnedCard = GameMode->GetAuthoritativeMatchState().Hands[20].Cards[0];
		Controller20->ClearPrivateHand();
		TestEqual(TEXT("Test intentionally desyncs owner hand before rejection"), Controller20->GetPrivateHandCount(), 0);

		const int32 Controller20RejectionsBeforePlay = Controller20->GetActionRejectionCount();
		Controller20->RequestPlayCards({ Player20OwnedCard }, FDubitoAnnouncement(Player20OwnedCard.Rank, 1));
		TestEqual(TEXT("Out-of-turn play records a Play rejection"), Controller20->GetLastRejectedAction(), EDubitoServerAction::Play);
		TestEqual(TEXT("Out-of-turn play rejection reason is NotYourTurn"), Controller20->GetLastRejectionReason(), EDubitoActionRejectionReason::PlayNotYourTurn);
		TestEqual(TEXT("Out-of-turn play increments rejection count"), Controller20->GetActionRejectionCount(), Controller20RejectionsBeforePlay + 1);
		TestTrue(TEXT("Out-of-turn play requests authority resync"), Controller20->DidLastRejectionRequestResync());
		TestEqual(TEXT("Resync restores owner private hand from authoritative state"), Controller20->GetPrivateHandCount(), 26);
		TestEqual(TEXT("Rejected play keeps active player unchanged"), PublicGameState->GetActivePlayerId(), 10);
		TestEqual(TEXT("Rejected play keeps pile empty"), GameMode->GetAuthoritativeMatchState().ActualPile.Num(), 0);

		const int32 Controller10RejectionsBeforeBadCount = Controller10->GetActionRejectionCount();
		Controller10->RequestPlayCards({}, FDubitoAnnouncement(7, 1));
		TestEqual(TEXT("Zero-card play records a Play rejection"), Controller10->GetLastRejectedAction(), EDubitoServerAction::Play);
		TestEqual(TEXT("Zero-card play rejection reason is BadActualCount"), Controller10->GetLastRejectionReason(), EDubitoActionRejectionReason::PlayBadActualCount);
		TestEqual(TEXT("Zero-card play increments rejection count"), Controller10->GetActionRejectionCount(), Controller10RejectionsBeforeBadCount + 1);
		TestEqual(TEXT("Zero-card play keeps owner hand unchanged"), Controller10->GetPrivateHandCount(), 26);

		const int32 Controller10RejectionsBeforeDiscard = Controller10->GetActionRejectionCount();
		Controller10->RequestDiscard();
		TestEqual(TEXT("Discard on empty pile records a Discard rejection"), Controller10->GetLastRejectedAction(), EDubitoServerAction::Discard);
		TestEqual(TEXT("Discard on empty pile rejection reason is unavailable"), Controller10->GetLastRejectionReason(), EDubitoActionRejectionReason::DiscardUnavailable);
		TestEqual(TEXT("Discard on empty pile increments rejection count"), Controller10->GetActionRejectionCount(), Controller10RejectionsBeforeDiscard + 1);

		const FDubitoCard Player10OpeningCard = Controller10->GetPrivateHand().Cards[0];
		Controller10->RequestPlayCards({ Player10OpeningCard }, FDubitoAnnouncement(Player10OpeningCard.Rank, 1));
		TestEqual(TEXT("Valid play still reaches core after rejected attempts"), PublicGameState->GetActivePlayerId(), 20);
		TestEqual(TEXT("Valid play does not add a rejection"), Controller10->GetActionRejectionCount(), Controller10RejectionsBeforeDiscard + 1);

		const int32 Controller10RejectionsBeforeStaleDoubt = Controller10->GetActionRejectionCount();
		Controller10->RequestDoubt();
		TestEqual(TEXT("Non-immediate stale Doubt records a Doubt rejection"), Controller10->GetLastRejectedAction(), EDubitoServerAction::Doubt);
		TestEqual(TEXT("Non-immediate stale Doubt rejection reason is unavailable"), Controller10->GetLastRejectionReason(), EDubitoActionRejectionReason::DoubtUnavailable);
		TestEqual(TEXT("Non-immediate stale Doubt increments rejection count"), Controller10->GetActionRejectionCount(), Controller10RejectionsBeforeStaleDoubt + 1);
		TestEqual(TEXT("Stale Doubt keeps active responder unchanged"), PublicGameState->GetActivePlayerId(), 20);

		const FDubitoCard Player20FollowCard = Controller20->GetPrivateHand().Cards[0];
		const int32 WrongLockedValue = Player10OpeningCard.Rank == DubitoConstants::MaxCardRank ? DubitoConstants::MinCardRank : Player10OpeningCard.Rank + 1;
		const int32 Controller20RejectionsBeforeValueLocked = Controller20->GetActionRejectionCount();
		Controller20->RequestPlayCards({ Player20FollowCard }, FDubitoAnnouncement(WrongLockedValue, 1));
		TestEqual(TEXT("Wrong locked claim records a Play rejection"), Controller20->GetLastRejectedAction(), EDubitoServerAction::Play);
		TestEqual(TEXT("Wrong locked claim rejection reason is ValueLocked"), Controller20->GetLastRejectionReason(), EDubitoActionRejectionReason::PlayValueLocked);
		TestEqual(TEXT("Wrong locked claim increments rejection count"), Controller20->GetActionRejectionCount(), Controller20RejectionsBeforeValueLocked + 1);

		const int32 Controller20RejectionsBeforeBadAnnouncement = Controller20->GetActionRejectionCount();
		Controller20->RequestPlayCards({ Player20FollowCard }, FDubitoAnnouncement(DubitoConstants::NoRoundValue, 1));
		TestEqual(TEXT("Malformed claim records a Play rejection"), Controller20->GetLastRejectedAction(), EDubitoServerAction::Play);
		TestEqual(TEXT("Malformed claim rejection reason is BadAnnouncement"), Controller20->GetLastRejectionReason(), EDubitoActionRejectionReason::PlayBadAnnouncement);
		TestEqual(TEXT("Malformed claim increments rejection count"), Controller20->GetActionRejectionCount(), Controller20RejectionsBeforeBadAnnouncement + 1);

		const FDubitoCard Player10UnownedCard = Controller10->GetPrivateHand().Cards[0];
		const int32 Controller20RejectionsBeforeUnowned = Controller20->GetActionRejectionCount();
		Controller20->RequestPlayCards({ Player10UnownedCard }, FDubitoAnnouncement(Player10OpeningCard.Rank, 1));
		TestEqual(TEXT("Unowned actual card records a Play rejection"), Controller20->GetLastRejectedAction(), EDubitoServerAction::Play);
		TestEqual(TEXT("Unowned actual card rejection reason is DontOwnCards"), Controller20->GetLastRejectionReason(), EDubitoActionRejectionReason::PlayDontOwnCards);
		TestEqual(TEXT("Unowned actual card increments rejection count"), Controller20->GetActionRejectionCount(), Controller20RejectionsBeforeUnowned + 1);
		TestEqual(TEXT("Rejected follow-up attempts keep responder active"), PublicGameState->GetActivePlayerId(), 20);
	}

	DestroyAuthorityTestWorld(World);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoAuthorityBridgePlayerStateTest, "Dubito.Unreal.AuthorityBridge.PlayerState", DubitoAuthorityFlags)
bool FDubitoAuthorityBridgePlayerStateTest::RunTest(const FString&)
{
	UWorld* World = CreateAuthorityTestWorld();
	ADubitoGameMode* GameMode = SpawnAuthorityGameMode(World);
	ADubitoPlayerState* Player10 = World ? World->SpawnActor<ADubitoPlayerState>() : nullptr;
	ADubitoPlayerState* Player20 = World ? World->SpawnActor<ADubitoPlayerState>() : nullptr;
	ADubitoPlayerState* DuplicatePlayer10 = World ? World->SpawnActor<ADubitoPlayerState>() : nullptr;
	TestNotNull(TEXT("GameMode spawns in transient authority world"), GameMode);
	TestNotNull(TEXT("Player 10 state spawns"), Player10);
	TestNotNull(TEXT("Player 20 state spawns"), Player20);

	if (GameMode && Player10 && Player20)
	{
		TestEqual(TEXT("GameMode declares Dubito PlayerState class"), GameMode->PlayerStateClass.Get(), ADubitoPlayerState::StaticClass());

		auto TestReplicatedProperty = [this](const TCHAR* PropertyName)
		{
			const FProperty* Property = FindFProperty<FProperty>(ADubitoPlayerState::StaticClass(), PropertyName);
			TestNotNull(FString::Printf(TEXT("%s property exists"), PropertyName), Property);
			if (Property)
			{
				TestTrue(FString::Printf(TEXT("%s is marked for replication"), PropertyName), Property->HasAnyPropertyFlags(CPF_Net));
			}
		};

		TestReplicatedProperty(TEXT("DubitoPlayerId"));
		TestReplicatedProperty(TEXT("SeatIndex"));
		TestReplicatedProperty(TEXT("bReady"));
		TestReplicatedProperty(TEXT("PublicHandCount"));

		TestTrue(TEXT("Player 10 public identity registers"), GameMode->RegisterAuthorityPlayerState(Player10, 10, 0));
		TestTrue(TEXT("Player 20 public identity registers"), GameMode->RegisterAuthorityPlayerState(Player20, 20, 1));
		TestFalse(TEXT("Duplicate player id cannot bind to a different PlayerState"), GameMode->RegisterAuthorityPlayerState(DuplicatePlayer10, 10, 2));

		TestEqual(TEXT("Player 10 id is public"), Player10->GetDubitoPlayerId(), 10);
		TestEqual(TEXT("Player 10 seat is public"), Player10->GetSeatIndex(), 0);
		TestEqual(TEXT("Player 20 id is public"), Player20->GetDubitoPlayerId(), 20);
		TestEqual(TEXT("Player 20 seat is public"), Player20->GetSeatIndex(), 1);

		TestTrue(TEXT("Ready flag can be set for registered player"), GameMode->SetAuthorityPlayerReady(10, true));
		TestTrue(TEXT("Ready flag is replicated public state"), Player10->IsReady());
		TestFalse(TEXT("Unknown player ready update is rejected"), GameMode->SetAuthorityPlayerReady(30, true));

		TestEqual(TEXT("Valid start succeeds"), static_cast<int32>(GameMode->StartAuthoritativeMatchFromHands({ 10, 20 }, MakeTwoPlayerHands())), static_cast<int32>(EDubitoAuthorityStartResult::Success));
		TestEqual(TEXT("Player 10 public hand ledger mirrors dealt size"), Player10->GetPublicHandCount(), 2);
		TestEqual(TEXT("Player 20 public hand ledger mirrors dealt size"), Player20->GetPublicHandCount(), 2);

		TestEqual(TEXT("Claimed count can diverge from actual count"), static_cast<int32>(GameMode->AuthorityPlayCards(10, { C(EDubitoSuit::Clubs, 7) }, FDubitoAnnouncement(7, 4))), static_cast<int32>(EDubitoPlayValidity::Valid));
		TestEqual(TEXT("Player 10 ledger follows claimed count and clamps at zero"), Player10->GetPublicHandCount(), 0);
		TestEqual(TEXT("Player 20 ledger is unchanged before response"), Player20->GetPublicHandCount(), 2);

		FDubitoRevealInfo Reveal;
		TestTrue(TEXT("Active next player resolves Doubt"), GameMode->AuthorityResolveDoubt(20, Reveal));
		TestTrue(TEXT("Count lie was detected"), Reveal.bWasLie);
		TestEqual(TEXT("Liar public ledger receives claimed stake"), Player10->GetPublicHandCount(), 4);
		TestEqual(TEXT("Authoritative actual hand is not the public ledger"), GameMode->GetAuthoritativeMatchState().Hands[10].Num(), 2);

		GameMode->AuthorityHandleDisconnect(20);
		TestEqual(TEXT("Disconnected player public ledger is cleared"), Player20->GetPublicHandCount(), 0);
		TestEqual(TEXT("Disconnected player identity remains public for UI cleanup"), Player20->GetDubitoPlayerId(), 20);
	}

	DestroyAuthorityTestWorld(World);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoAuthorityBridgeDeckStartTest, "Dubito.Unreal.AuthorityBridge.DeckStart", DubitoAuthorityFlags)
bool FDubitoAuthorityBridgeDeckStartTest::RunTest(const FString&)
{
	UWorld* World = CreateAuthorityTestWorld();
	ADubitoGameMode* GameMode = SpawnAuthorityGameMode(World);
	TestNotNull(TEXT("GameMode spawns in transient authority world"), GameMode);

	if (GameMode)
	{
		TestEqual(TEXT("Valid shuffled deck start succeeds"), static_cast<int32>(GameMode->StartAuthoritativeMatchFromShuffledDeck({ 1, 2, 3 }, 12345)), static_cast<int32>(EDubitoAuthorityStartResult::Success));

		const FDubitoMatchState& State = GameMode->GetAuthoritativeMatchState();
		TestEqual(TEXT("Three player deal creates three hidden hands"), State.Hands.Num(), 3);
		TestEqual(TEXT("Three player deal removes one leftover card"), GameMode->GetRemovedDealCardCount(), 1);
		TestEqual(TEXT("Player 1 receives 17 cards"), State.Hands[1].Num(), 17);
		TestEqual(TEXT("Player 2 receives 17 cards"), State.Hands[2].Num(), 17);
		TestEqual(TEXT("Player 3 receives 17 cards"), State.Hands[3].Num(), 17);
		TestEqual(TEXT("Public ledger for player 1 uses dealt size"), State.PublicHandCounts[1], 17);
	}

	DestroyAuthorityTestWorld(World);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoAuthorityBridgePlayTest, "Dubito.Unreal.AuthorityBridge.Play", DubitoAuthorityFlags)
bool FDubitoAuthorityBridgePlayTest::RunTest(const FString&)
{
	UWorld* World = CreateAuthorityTestWorld();
	ADubitoGameMode* GameMode = SpawnAuthorityGameMode(World);
	TestNotNull(TEXT("GameMode spawns in transient authority world"), GameMode);

	if (GameMode)
	{
		TestEqual(TEXT("Valid start succeeds"), static_cast<int32>(GameMode->StartAuthoritativeMatchFromHands({ 10, 20 }, MakeTwoPlayerHands())), static_cast<int32>(EDubitoAuthorityStartResult::Success));

		const FDubitoMatchState BeforeIllegal = GameMode->GetAuthoritativeMatchState();
		TestEqual(TEXT("Non-active player play is rejected through DubitoCore"), static_cast<int32>(GameMode->AuthorityPlayCards(20, { C(EDubitoSuit::Spades, 7) }, FDubitoAnnouncement(7, 1))), static_cast<int32>(EDubitoPlayValidity::NotYourTurn));
		TestEqual(TEXT("Rejected play does not mutate active player"), GameMode->GetAuthoritativeMatchState().ActivePlayerId(), BeforeIllegal.ActivePlayerId());
		TestEqual(TEXT("Rejected play does not mutate pile"), GameMode->GetAuthoritativeMatchState().ActualPile.Num(), BeforeIllegal.ActualPile.Num());

		TestTrue(TEXT("Active player can play before action"), GameMode->CanAuthorityPlay(10));
		TestEqual(TEXT("Active player play succeeds through DubitoCore"), static_cast<int32>(GameMode->AuthorityPlayCards(10, { C(EDubitoSuit::Clubs, 7) }, FDubitoAnnouncement(7, 1))), static_cast<int32>(EDubitoPlayValidity::Valid));

		const FDubitoMatchState& State = GameMode->GetAuthoritativeMatchState();
		TestEqual(TEXT("Round value locked by core"), State.RoundValue, 7);
		TestEqual(TEXT("Hidden actual pile has actual card count"), State.ActualPile.Num(), 1);
		TestEqual(TEXT("Public claimed pile has claimed count"), State.ClaimedPileCount, 1);
		TestEqual(TEXT("Actual hand lost actual card"), State.Hands[10].Num(), 1);
		TestEqual(TEXT("Public hand ledger lost claimed count"), State.PublicHandCounts[10], 1);
		TestEqual(TEXT("Turn advanced to next player"), State.ActivePlayerId(), 20);
		TestTrue(TEXT("Immediate next player can Doubt"), GameMode->CanAuthorityDoubt(20));
		TestTrue(TEXT("Immediate next player can Discard non-empty pile"), GameMode->CanAuthorityDiscard(20));
	}

	DestroyAuthorityTestWorld(World);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoAuthorityBridgeResolveTest, "Dubito.Unreal.AuthorityBridge.Resolve", DubitoAuthorityFlags)
bool FDubitoAuthorityBridgeResolveTest::RunTest(const FString&)
{
	UWorld* World = CreateAuthorityTestWorld();
	ADubitoGameMode* GameMode = SpawnAuthorityGameMode(World);
	TestNotNull(TEXT("GameMode spawns in transient authority world"), GameMode);

	if (GameMode)
	{
		TestEqual(TEXT("Valid start succeeds"), static_cast<int32>(GameMode->StartAuthoritativeMatchFromHands({ 10, 20 }, MakeTwoPlayerHands())), static_cast<int32>(EDubitoAuthorityStartResult::Success));
		TestEqual(TEXT("Opening play succeeds"), static_cast<int32>(GameMode->AuthorityPlayCards(10, { C(EDubitoSuit::Clubs, 2) }, FDubitoAnnouncement(7, 1))), static_cast<int32>(EDubitoPlayValidity::Valid));

		FDubitoRevealInfo Reveal;
		TestFalse(TEXT("Non-active claimant cannot resolve Doubt"), GameMode->AuthorityResolveDoubt(10, Reveal));
		TestTrue(TEXT("Active next player resolves Doubt through core"), GameMode->AuthorityResolveDoubt(20, Reveal));
		TestTrue(TEXT("Reveal marks value lie"), Reveal.bWasLie);
		TestEqual(TEXT("Reveal claimant is previous player"), Reveal.ClaimantId, 10);
		TestEqual(TEXT("Reveal doubter is active player"), Reveal.DoubterId, 20);
		TestEqual(TEXT("Liar takes pile on correct Doubt"), Reveal.LoserId, 10);
		TestEqual(TEXT("Round reset after Doubt"), GameMode->GetAuthoritativeMatchState().RoundValue, DubitoConstants::NoRoundValue);
		TestEqual(TEXT("Pile emptied after Doubt"), GameMode->GetAuthoritativeMatchState().ActualPile.Num(), 0);

		TestEqual(TEXT("Restart valid match for discard path"), static_cast<int32>(GameMode->StartAuthoritativeMatchFromHands({ 10, 20 }, MakeTwoPlayerHands())), static_cast<int32>(EDubitoAuthorityStartResult::Success));
		TestEqual(TEXT("Opening play succeeds"), static_cast<int32>(GameMode->AuthorityPlayCards(10, { C(EDubitoSuit::Clubs, 7) }, FDubitoAnnouncement(7, 1))), static_cast<int32>(EDubitoPlayValidity::Valid));
		TestTrue(TEXT("Active next player discards through core"), GameMode->AuthorityDiscard(20));
		TestEqual(TEXT("Discard clears pile"), GameMode->GetAuthoritativeMatchState().ActualPile.Num(), 0);
		TestEqual(TEXT("Discard resets round"), GameMode->GetAuthoritativeMatchState().RoundValue, DubitoConstants::NoRoundValue);
		TestEqual(TEXT("Discard skips active player"), GameMode->GetAuthoritativeMatchState().ActivePlayerId(), 10);
	}

	DestroyAuthorityTestWorld(World);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
