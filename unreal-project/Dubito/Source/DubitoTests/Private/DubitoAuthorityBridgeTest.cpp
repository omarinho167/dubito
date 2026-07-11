#include "Misc/AutomationTest.h"
#include "DubitoAnnouncement.h"
#include "DubitoCard.h"
#include "DubitoConstants.h"
#include "DubitoGameMode.h"
#include "DubitoHand.h"
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
		return World ? World->SpawnActor<ADubitoGameMode>() : nullptr;
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
