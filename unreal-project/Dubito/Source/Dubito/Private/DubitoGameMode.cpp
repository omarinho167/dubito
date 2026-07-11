#include "DubitoGameMode.h"

#include "DubitoConstants.h"
#include "DubitoDeck.h"
#include "DubitoGameState.h"
#include "DubitoPlayerController.h"
#include "DubitoPlayerState.h"

ADubitoGameMode::ADubitoGameMode()
{
	bReplicates = false;
	GameStateClass = ADubitoGameState::StaticClass();
	PlayerControllerClass = ADubitoPlayerController::StaticClass();
	PlayerStateClass = ADubitoPlayerState::StaticClass();
}

EDubitoAuthorityStartResult ADubitoGameMode::ValidatePlayerIds(const TArray<int32>& PlayerIds)
{
	if (PlayerIds.Num() < DubitoConstants::MinPlayers || PlayerIds.Num() > DubitoConstants::MaxPlayers)
	{
		return EDubitoAuthorityStartResult::InvalidPlayerCount;
	}

	TSet<int32> SeenIds;
	for (const int32 PlayerId : PlayerIds)
	{
		if (PlayerId == DubitoConstants::NoPlayerId)
		{
			return EDubitoAuthorityStartResult::InvalidPlayerId;
		}

		if (SeenIds.Contains(PlayerId))
		{
			return EDubitoAuthorityStartResult::DuplicatePlayerId;
		}

		SeenIds.Add(PlayerId);
	}

	return EDubitoAuthorityStartResult::Success;
}

EDubitoAuthorityStartResult ADubitoGameMode::StartAuthoritativeMatchFromHands(const TArray<int32>& PlayerIds, const TMap<int32, FDubitoHand>& DealtHands)
{
	const EDubitoAuthorityStartResult PlayerValidation = ValidatePlayerIds(PlayerIds);
	if (PlayerValidation != EDubitoAuthorityStartResult::Success)
	{
		return PlayerValidation;
	}

	for (const int32 PlayerId : PlayerIds)
	{
		if (!DealtHands.Contains(PlayerId))
		{
			return EDubitoAuthorityStartResult::MissingHand;
		}
	}

	FDubitoMatchState NewState;
	DubitoRules::SetupMatch(NewState, PlayerIds, DealtHands);

	AuthoritativeMatchState = MoveTemp(NewState);
	bAuthoritativeMatchStarted = true;
	RemovedDealCardCount = 0;
	RefreshTurnDeadlineForCurrentState();
	SyncReplicatedState();

	return EDubitoAuthorityStartResult::Success;
}

EDubitoAuthorityStartResult ADubitoGameMode::StartAuthoritativeMatchFromShuffledDeck(const TArray<int32>& PlayerIds, int32 ShuffleSeed)
{
	const EDubitoAuthorityStartResult PlayerValidation = ValidatePlayerIds(PlayerIds);
	if (PlayerValidation != EDubitoAuthorityStartResult::Success)
	{
		return PlayerValidation;
	}

	TArray<FDubitoCard> Deck = DubitoDeck::BuildStandardDeck();
	FRandomStream RandomStream(ShuffleSeed);
	DubitoDeck::Shuffle(Deck, RandomStream);

	int32 LocalRemovedDealCardCount = 0;
	const TArray<FDubitoHand> DealtHands = DubitoDeck::Deal(Deck, PlayerIds.Num(), LocalRemovedDealCardCount);

	TMap<int32, FDubitoHand> HandsByPlayerId;
	for (int32 Index = 0; Index < PlayerIds.Num(); ++Index)
	{
		HandsByPlayerId.Add(PlayerIds[Index], DealtHands[Index]);
	}

	const EDubitoAuthorityStartResult Result = StartAuthoritativeMatchFromHands(PlayerIds, HandsByPlayerId);
	if (Result == EDubitoAuthorityStartResult::Success)
	{
		RemovedDealCardCount = LocalRemovedDealCardCount;
	}

	return Result;
}

bool ADubitoGameMode::CanAuthorityPlay(int32 PlayerId) const
{
	return DubitoRules::CanPlay(AuthoritativeMatchState, PlayerId);
}

bool ADubitoGameMode::CanAuthorityDoubt(int32 PlayerId) const
{
	return DubitoRules::CanDoubt(AuthoritativeMatchState, PlayerId);
}

bool ADubitoGameMode::CanAuthorityDiscard(int32 PlayerId) const
{
	return DubitoRules::CanDiscard(AuthoritativeMatchState, PlayerId);
}

EDubitoPlayValidity ADubitoGameMode::AuthorityPlayCards(int32 PlayerId, const TArray<FDubitoCard>& ActualCards, const FDubitoAnnouncement& Announcement)
{
	const EDubitoPlayValidity Validation = DubitoRules::ValidatePlay(AuthoritativeMatchState, PlayerId, ActualCards, Announcement);
	if (Validation != EDubitoPlayValidity::Valid)
	{
		return Validation;
	}

	DubitoRules::NoteVoluntaryAction(AuthoritativeMatchState, PlayerId);
	DubitoRules::ApplyPlay(AuthoritativeMatchState, PlayerId, ActualCards, Announcement);
	RefreshTurnDeadlineForCurrentState();
	SyncReplicatedState();
	return EDubitoPlayValidity::Valid;
}

bool ADubitoGameMode::AuthorityResolveDoubt(int32 PlayerId, FDubitoRevealInfo& OutReveal)
{
	OutReveal = FDubitoRevealInfo();

	if (!DubitoRules::CanDoubt(AuthoritativeMatchState, PlayerId))
	{
		return false;
	}

	DubitoRules::NoteVoluntaryAction(AuthoritativeMatchState, PlayerId);
	OutReveal = DubitoRules::ResolveDoubt(AuthoritativeMatchState, PlayerId);
	RefreshTurnDeadlineForCurrentState();
	SyncReplicatedState();
	return true;
}

bool ADubitoGameMode::AuthorityDiscard(int32 PlayerId)
{
	if (!DubitoRules::CanDiscard(AuthoritativeMatchState, PlayerId))
	{
		return false;
	}

	DubitoRules::NoteVoluntaryAction(AuthoritativeMatchState, PlayerId);
	DubitoRules::ApplyDiscard(AuthoritativeMatchState, PlayerId);
	RefreshTurnDeadlineForCurrentState();
	SyncReplicatedState();
	return true;
}

void ADubitoGameMode::AuthorityResolveTimeout(int32 PlayerId)
{
	if (AuthoritativeMatchState.Phase != EDubitoPhase::PlayerTurn || AuthoritativeMatchState.ActivePlayerId() != PlayerId)
	{
		return;
	}

	DubitoRules::ResolveTimeout(AuthoritativeMatchState, PlayerId);
	RefreshTurnDeadlineForCurrentState();
	SyncReplicatedState();
}

void ADubitoGameMode::AuthorityHandleDisconnect(int32 PlayerId)
{
	if (!AuthoritativeMatchState.TurnOrder.Contains(PlayerId))
	{
		return;
	}

	DubitoRules::HandleDisconnect(AuthoritativeMatchState, PlayerId);
	RefreshTurnDeadlineForCurrentState();
	SyncReplicatedState();
}

bool ADubitoGameMode::RegisterAuthorityPlayerState(ADubitoPlayerState* PlayerState, int32 PlayerId, int32 SeatIndex)
{
	if (!PlayerState || PlayerId == DubitoConstants::NoPlayerId || SeatIndex < 0)
	{
		return false;
	}

	if (const TWeakObjectPtr<ADubitoPlayerState>* Existing = PlayerStatesById.Find(PlayerId))
	{
		if (Existing->IsValid() && Existing->Get() != PlayerState)
		{
			return false;
		}
	}

	PlayerStatesById.Add(PlayerId, PlayerState);
	PlayerState->SetPublicIdentity(PlayerId, SeatIndex);

	if (const int32* PublicCount = AuthoritativeMatchState.PublicHandCounts.Find(PlayerId))
	{
		PlayerState->SetPublicHandCount(*PublicCount);
	}

	return true;
}

bool ADubitoGameMode::RegisterAuthorityPlayerController(ADubitoPlayerController* PlayerController, int32 PlayerId)
{
	if (!PlayerController || PlayerId == DubitoConstants::NoPlayerId)
	{
		return false;
	}

	if (const TWeakObjectPtr<ADubitoPlayerController>* Existing = PlayerControllersById.Find(PlayerId))
	{
		if (Existing->IsValid() && Existing->Get() != PlayerController)
		{
			return false;
		}
	}

	PlayerControllersById.Add(PlayerId, PlayerController);

	if (const FDubitoHand* Hand = AuthoritativeMatchState.Hands.Find(PlayerId))
	{
		PlayerController->SetPrivateHand(*Hand);
	}
	else
	{
		PlayerController->ClearPrivateHand();
	}

	return true;
}

bool ADubitoGameMode::SetAuthorityPlayerReady(int32 PlayerId, bool bReady)
{
	TWeakObjectPtr<ADubitoPlayerState>* PlayerStatePtr = PlayerStatesById.Find(PlayerId);
	ADubitoPlayerState* PlayerState = PlayerStatePtr ? PlayerStatePtr->Get() : nullptr;
	if (!PlayerState)
	{
		return false;
	}

	PlayerState->SetReady(bReady);
	return true;
}

void ADubitoGameMode::RefreshTurnDeadlineForCurrentState()
{
	if (!bAuthoritativeMatchStarted || AuthoritativeMatchState.Phase != EDubitoPhase::PlayerTurn || AuthoritativeMatchState.ActivePlayerId() == DubitoConstants::NoPlayerId)
	{
		TurnDeadlineServerTimeSeconds = 0.0;
		return;
	}

	const UWorld* World = GetWorld();
	const AGameStateBase* PublicGameState = World ? World->GetGameState() : nullptr;
	const double CurrentServerTime = PublicGameState ? PublicGameState->GetServerWorldTimeSeconds() : (World ? World->GetTimeSeconds() : 0.0);
	TurnDeadlineServerTimeSeconds = CurrentServerTime + static_cast<double>(DubitoConstants::TurnSeconds);
}

void ADubitoGameMode::SyncReplicatedState()
{
	SyncPublicGameState();
	SyncPublicPlayerStates();
	SyncPrivateHands();
}

void ADubitoGameMode::SyncPublicGameState()
{
	ADubitoGameState* PublicGameState = GetGameState<ADubitoGameState>();
	if (!PublicGameState && GetWorld())
	{
		PublicGameState = GetWorld()->GetGameState<ADubitoGameState>();
	}

	if (PublicGameState)
	{
		PublicGameState->SyncFromAuthoritativeState(AuthoritativeMatchState, TurnDeadlineServerTimeSeconds);
	}
}

void ADubitoGameMode::SyncPublicPlayerStates()
{
	for (auto It = PlayerStatesById.CreateIterator(); It; ++It)
	{
		ADubitoPlayerState* PlayerState = It.Value().Get();
		if (!PlayerState)
		{
			It.RemoveCurrent();
			continue;
		}

		const int32 PlayerId = It.Key();
		const int32* PublicCount = AuthoritativeMatchState.PublicHandCounts.Find(PlayerId);
		PlayerState->SetPublicHandCount(PublicCount ? *PublicCount : 0);

		const int32 SeatIndex = AuthoritativeMatchState.TurnOrder.IndexOfByKey(PlayerId);
		if (SeatIndex != INDEX_NONE && PlayerState->GetSeatIndex() != SeatIndex)
		{
			PlayerState->SetPublicIdentity(PlayerId, SeatIndex);
		}
	}
}

void ADubitoGameMode::SyncPrivateHands()
{
	for (auto It = PlayerControllersById.CreateIterator(); It; ++It)
	{
		ADubitoPlayerController* PlayerController = It.Value().Get();
		if (!PlayerController)
		{
			It.RemoveCurrent();
			continue;
		}

		const int32 PlayerId = It.Key();
		if (const FDubitoHand* Hand = AuthoritativeMatchState.Hands.Find(PlayerId))
		{
			PlayerController->SetPrivateHand(*Hand);
		}
		else
		{
			PlayerController->ClearPrivateHand();
		}
	}
}
