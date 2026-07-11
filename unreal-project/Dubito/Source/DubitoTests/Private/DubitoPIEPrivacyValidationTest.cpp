#include "CQTest.h"
#include "Components/PIENetworkComponent.h"

#include "DubitoAnnouncement.h"
#include "DubitoCard.h"
#include "DubitoConstants.h"
#include "DubitoGameMode.h"
#include "DubitoGameState.h"
#include "DubitoHand.h"
#include "DubitoPlayerController.h"
#include "DubitoPlayerState.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"

#if ENABLE_PIE_NETWORK_TEST

namespace
{
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

	TArray<int32> MakePIEPlayerIds()
	{
		return { 10, 20, 30 };
	}

	TMap<int32, FDubitoHand> MakePIEHands()
	{
		TMap<int32, FDubitoHand> Hands;
		Hands.Add(10, MakeHand({ C(EDubitoSuit::Clubs, 7), C(EDubitoSuit::Clubs, 2) }));
		Hands.Add(20, MakeHand({ C(EDubitoSuit::Spades, 7), C(EDubitoSuit::Hearts, 4) }));
		Hands.Add(30, MakeHand({ C(EDubitoSuit::Diamonds, 7), C(EDubitoSuit::Spades, 4) }));
		return Hands;
	}

	TArray<ADubitoPlayerController*> CollectDubitoControllers(UWorld* World)
	{
		TArray<ADubitoPlayerController*> Controllers;
		if (!World)
		{
			return Controllers;
		}

		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			if (ADubitoPlayerController* Controller = Cast<ADubitoPlayerController>(It->Get()))
			{
				Controllers.Add(Controller);
			}
		}

		Controllers.Sort([](const ADubitoPlayerController& Left, const ADubitoPlayerController& Right)
		{
			if (Left.IsLocalController() != Right.IsLocalController())
			{
				return !Left.IsLocalController();
			}

			return Left.GetName() < Right.GetName();
		});
		return Controllers;
	}

	int32 CountDubitoControllers(UWorld* World)
	{
		int32 Count = 0;
		if (!World)
		{
			return Count;
		}

		for (TActorIterator<ADubitoPlayerController> It(World); It; ++It)
		{
			++Count;
		}
		return Count;
	}

	ADubitoPlayerController* GetLocalDubitoController(UWorld* World)
	{
		return World ? Cast<ADubitoPlayerController>(World->GetFirstPlayerController()) : nullptr;
	}

	int32 GetLocalDubitoPlayerId(UWorld* World)
	{
		const ADubitoPlayerController* Controller = GetLocalDubitoController(World);
		const ADubitoPlayerState* PlayerState = Controller ? Controller->GetPlayerState<ADubitoPlayerState>() : nullptr;
		return PlayerState ? PlayerState->GetDubitoPlayerId() : DubitoConstants::NoPlayerId;
	}

	ADubitoPlayerState* FindDubitoPlayerState(UWorld* World, int32 PlayerId)
	{
		const AGameStateBase* GameState = World ? World->GetGameState() : nullptr;
		if (!GameState)
		{
			return nullptr;
		}

		for (APlayerState* PlayerState : GameState->PlayerArray)
		{
			ADubitoPlayerState* DubitoPlayerState = Cast<ADubitoPlayerState>(PlayerState);
			if (DubitoPlayerState && DubitoPlayerState->GetDubitoPlayerId() == PlayerId)
			{
				return DubitoPlayerState;
			}
		}

		return nullptr;
	}

	bool PrivateHandMatches(const ADubitoPlayerController* Controller, int32 PlayerId)
	{
		if (!Controller)
		{
			return false;
		}

		const TMap<int32, FDubitoHand> Hands = MakePIEHands();
		const FDubitoHand* ExpectedHand = Hands.Find(PlayerId);
		if (!ExpectedHand || Controller->GetPrivateHandCount() != ExpectedHand->Num())
		{
			return false;
		}

		for (const FDubitoCard& ExpectedCard : ExpectedHand->Cards)
		{
			if (!Controller->GetPrivateHand().Contains(ExpectedCard))
			{
				return false;
			}
		}

		return true;
	}

	bool ClientsSeeInitialDeal(UWorld* World)
	{
		const ADubitoGameState* PublicGameState = World ? World->GetGameState<ADubitoGameState>() : nullptr;
		const ADubitoPlayerController* Controller = GetLocalDubitoController(World);
		const int32 PlayerId = GetLocalDubitoPlayerId(World);
		if (!PublicGameState || !Controller || (PlayerId != 10 && PlayerId != 20))
		{
			return false;
		}

		if (CountDubitoControllers(World) != 1
			|| PublicGameState->GetPublicPhase() != EDubitoPhase::PlayerTurn
			|| PublicGameState->GetActivePlayerId() != 10
			|| PublicGameState->GetClaimedPileCount() != 0
			|| PublicGameState->HasPreviousPublicClaim())
		{
			return false;
		}

		for (const int32 PublicPlayerId : MakePIEPlayerIds())
		{
			const ADubitoPlayerState* PlayerState = FindDubitoPlayerState(World, PublicPlayerId);
			if (!PlayerState || PlayerState->GetPublicHandCount() != 2)
			{
				return false;
			}
		}

		return PrivateHandMatches(Controller, PlayerId);
	}

	bool ClientsSeePostPlayState(UWorld* World)
	{
		const ADubitoGameState* PublicGameState = World ? World->GetGameState<ADubitoGameState>() : nullptr;
		const ADubitoPlayerController* Controller = GetLocalDubitoController(World);
		const int32 PlayerId = GetLocalDubitoPlayerId(World);
		if (!PublicGameState || !Controller || (PlayerId != 10 && PlayerId != 20))
		{
			return false;
		}

		const FDubitoAnnouncement PublicClaim = PublicGameState->GetPreviousPublicAnnouncement();
		if (CountDubitoControllers(World) != 1
			|| PublicGameState->GetPublicPhase() != EDubitoPhase::PlayerTurn
			|| PublicGameState->GetActivePlayerId() != 20
			|| PublicGameState->GetRoundValue() != 7
			|| !PublicGameState->HasPreviousPublicClaim()
			|| PublicGameState->GetPreviousClaimantId() != 10
			|| PublicClaim.ClaimedValue != 7
			|| PublicClaim.ClaimedCount != 2
			|| PublicGameState->GetClaimedPileCount() != 2)
		{
			return false;
		}

		const ADubitoPlayerState* Player10 = FindDubitoPlayerState(World, 10);
		const ADubitoPlayerState* Player20 = FindDubitoPlayerState(World, 20);
		const ADubitoPlayerState* Player30 = FindDubitoPlayerState(World, 30);
		if (!Player10 || !Player20 || !Player30
			|| Player10->GetPublicHandCount() != 0
			|| Player20->GetPublicHandCount() != 2
			|| Player30->GetPublicHandCount() != 2)
		{
			return false;
		}

		if (PlayerId == 10)
		{
			return Controller->GetPrivateHandCount() == 1
				&& Controller->GetPrivateHand().Contains(C(EDubitoSuit::Clubs, 2))
				&& !Controller->GetPrivateHand().Contains(C(EDubitoSuit::Clubs, 7));
		}

		return PrivateHandMatches(Controller, PlayerId);
	}
}

struct FDubitoPIEPrivacyState : FBasePIENetworkComponentState
{
	int32 RejectionsBefore = 0;
};

NETWORK_TEST_CLASS(FDubitoPIEPrivacyValidationTest, "Dubito.Unreal.PIE.PrivacyValidation")
{
	FPIENetworkComponent<FDubitoPIEPrivacyState> Network{ TestRunner, TestCommandBuilder, bInitializing };

	BEFORE_EACH()
	{
		FNetworkComponentBuilder<FDubitoPIEPrivacyState>()
			.WithClients(2)
			.AsListenServer()
			.WithGameMode(ADubitoGameMode::StaticClass())
			.Build(Network);
	}

	TEST_METHOD(ListenServer_ReplicatesPublicStateAndOwnerOnlyHands)
	{
		const TOptional<FTimespan> NetworkTimeout = FTimespan::FromSeconds(20.0);

		Network
			.UntilServer(TEXT("Server has listen and remote controllers"), [](FDubitoPIEPrivacyState& State)
			{
				const TArray<ADubitoPlayerController*> Controllers = CollectDubitoControllers(State.World);
				if (Controllers.Num() != 3)
				{
					return false;
				}

				for (const ADubitoPlayerController* Controller : Controllers)
				{
					if (!Controller || !Controller->GetPlayerState<ADubitoPlayerState>())
					{
						return false;
					}
				}

				return State.World && State.World->GetAuthGameMode<ADubitoGameMode>() != nullptr && State.World->GetGameState<ADubitoGameState>() != nullptr;
			}, NetworkTimeout)
			.ThenServer(TEXT("Register deterministic players and start match"), [this](FDubitoPIEPrivacyState& State)
			{
				ADubitoGameMode* GameMode = State.World ? State.World->GetAuthGameMode<ADubitoGameMode>() : nullptr;
				const TArray<ADubitoPlayerController*> Controllers = CollectDubitoControllers(State.World);
				const TArray<int32> PlayerIds = MakePIEPlayerIds();
				if (!this->AddErrorIfFalse(GameMode != nullptr, TEXT("PIE server has Dubito GameMode"))
					|| !this->AddErrorIfFalse(Controllers.Num() == PlayerIds.Num(), TEXT("PIE server has three player controllers")))
				{
					return;
				}

				for (int32 Index = 0; Index < PlayerIds.Num(); ++Index)
				{
					ADubitoPlayerController* Controller = Controllers[Index];
					ADubitoPlayerState* PlayerState = Controller ? Controller->GetPlayerState<ADubitoPlayerState>() : nullptr;
					const int32 PlayerId = PlayerIds[Index];
					this->AddErrorIfFalse(PlayerState != nullptr, FString::Printf(TEXT("Player %d has Dubito PlayerState"), PlayerId));
					this->AddErrorIfFalse(GameMode->RegisterAuthorityPlayerState(PlayerState, PlayerId, Index), FString::Printf(TEXT("Player %d public state registers"), PlayerId));
					this->AddErrorIfFalse(GameMode->RegisterAuthorityPlayerController(Controller, PlayerId), FString::Printf(TEXT("Player %d private controller registers"), PlayerId));
				}

				const EDubitoAuthorityStartResult StartResult = GameMode->StartAuthoritativeMatchFromHands(PlayerIds, MakePIEHands());
				this->AddErrorIfFalse(StartResult == EDubitoAuthorityStartResult::Success, TEXT("PIE deterministic match starts"));
			})
			.UntilClients(TEXT("Clients receive public deal and owner-only hands"), [](FDubitoPIEPrivacyState& State)
			{
				return ClientsSeeInitialDeal(State.World);
			}, NetworkTimeout)
			.ThenServer(TEXT("Server applies a bluffable claimed-count play"), [this](FDubitoPIEPrivacyState& State)
			{
				ADubitoGameMode* GameMode = State.World ? State.World->GetAuthGameMode<ADubitoGameMode>() : nullptr;
				if (!this->AddErrorIfFalse(GameMode != nullptr, TEXT("PIE server still has Dubito GameMode")))
				{
					return;
				}

				const EDubitoPlayValidity PlayResult = GameMode->AuthorityPlayCards(10, { C(EDubitoSuit::Clubs, 7) }, FDubitoAnnouncement(7, 2));
				this->AddErrorIfFalse(PlayResult == EDubitoPlayValidity::Valid, TEXT("Server play advances turn and publishes claim"));
			})
			.UntilClients(TEXT("Clients receive public claim without opponent private hand"), [](FDubitoPIEPrivacyState& State)
			{
				return ClientsSeePostPlayState(State.World);
			}, NetworkTimeout)
			.ThenClients(TEXT("Stale player attempts illegal discard through owning client RPC"), [](FDubitoPIEPrivacyState& State)
			{
				ADubitoPlayerController* Controller = GetLocalDubitoController(State.World);
				if (Controller && GetLocalDubitoPlayerId(State.World) == 10)
				{
					State.RejectionsBefore = Controller->GetActionRejectionCount();
					Controller->RequestDiscard();
				}
			})
			.UntilClients(TEXT("Illegal action rejects without disconnect and keeps state synced"), [](FDubitoPIEPrivacyState& State)
			{
				ADubitoPlayerController* Controller = GetLocalDubitoController(State.World);
				const int32 PlayerId = GetLocalDubitoPlayerId(State.World);
				if (!Controller || PlayerId == DubitoConstants::NoPlayerId)
				{
					return false;
				}

				if (PlayerId != 10)
				{
					return ClientsSeePostPlayState(State.World);
				}

				return Controller->GetActionRejectionCount() == State.RejectionsBefore + 1
					&& Controller->GetLastRejectedAction() == EDubitoServerAction::Discard
					&& Controller->GetLastRejectionReason() == EDubitoActionRejectionReason::DiscardUnavailable
					&& Controller->DidLastRejectionRequestResync()
					&& ClientsSeePostPlayState(State.World);
			}, NetworkTimeout);
	}
};

#endif // ENABLE_PIE_NETWORK_TEST
