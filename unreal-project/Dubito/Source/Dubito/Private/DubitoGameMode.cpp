#include "DubitoGameMode.h"

#include "DubitoConstants.h"
#include "DubitoDeck.h"
#include "DubitoGameState.h"
#include "DubitoHUD.h"
#include "DubitoPlayerController.h"
#include "DubitoPlayerState.h"
#include "Engine/World.h"
#include "TimerManager.h"

namespace
{
	ADubitoGameState* ResolvePublicGameState(ADubitoGameMode* GameMode)
	{
		UWorld* World = GameMode ? GameMode->GetWorld() : nullptr;
		return World ? World->GetGameState<ADubitoGameState>() : nullptr;
	}

	void BroadcastPublicReveal(ADubitoGameMode* GameMode, const FDubitoRevealInfo& Reveal)
	{
		if (ADubitoGameState* PublicGameState = ResolvePublicGameState(GameMode))
		{
			PublicGameState->PublishPublicReveal(Reveal);
		}
	}

	void BroadcastGameOver(ADubitoGameMode* GameMode, EDubitoGameOverReason Reason)
	{
		if (!GameMode)
		{
			return;
		}

		if (ADubitoGameState* PublicGameState = ResolvePublicGameState(GameMode))
		{
			FDubitoGameOverInfo GameOver;
			GameOver.WinnerId = GameMode->GetAuthoritativeMatchState().WinnerId;
			GameOver.Reason = Reason;
			PublicGameState->PublishGameOver(GameOver);
		}
	}

	void BroadcastGameOverIfNeeded(ADubitoGameMode* GameMode, EDubitoPhase PreviousPhase, EDubitoGameOverReason Reason)
	{
		if (GameMode
			&& PreviousPhase != EDubitoPhase::GameOver
			&& GameMode->GetAuthoritativeMatchState().Phase == EDubitoPhase::GameOver)
		{
			BroadcastGameOver(GameMode, Reason);
		}
	}
}

ADubitoGameMode::ADubitoGameMode()
{
	bReplicates = false;
	GameStateClass = ADubitoGameState::StaticClass();
	PlayerControllerClass = ADubitoPlayerController::StaticClass();
	PlayerStateClass = ADubitoPlayerState::StaticClass();
	HUDClass = ADubitoHUD::StaticClass();
}

void ADubitoGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (!IsSessionSeatingMap())
	{
		return;
	}

	// Only seat players while still gathering the lobby; no late joins into a live match.
	if (bAuthoritativeMatchStarted || AuthoritativeMatchState.Phase != EDubitoPhase::Lobby)
	{
		return;
	}

	ADubitoPlayerController* Controller = Cast<ADubitoPlayerController>(NewPlayer);
	ADubitoPlayerState* PlayerState = NewPlayer ? NewPlayer->GetPlayerState<ADubitoPlayerState>() : nullptr;
	if (!Controller || !PlayerState)
	{
		return;
	}

	if (PlayerStatesById.Num() >= DubitoConstants::MaxPlayers)
	{
		return; // the table is full
	}

	const int32 SeatIndex = FirstFreeSeatIndex();
	const int32 PlayerId = SeatIndex + 1; // 1..8, never NoPlayerId; seat 0 is the host
	RegisterAuthorityPlayerState(PlayerState, PlayerId, SeatIndex);
	RegisterAuthorityPlayerController(Controller, PlayerId);
}

void ADubitoGameMode::Logout(AController* Exiting)
{
	if (IsSessionSeatingMap())
	{
		if (ADubitoPlayerController* Controller = Cast<ADubitoPlayerController>(Exiting))
		{
			const int32 PlayerId = Controller->GetAuthorityPlayerId();
			if (PlayerId != DubitoConstants::NoPlayerId)
			{
				// If a live match is running, resolve the disconnect through the rules first so the
				// pending-win / last-player-standing outcomes fire before we drop the registration.
				if (bAuthoritativeMatchStarted
					&& AuthoritativeMatchState.Phase != EDubitoPhase::GameOver
					&& AuthoritativeMatchState.TurnOrder.Contains(PlayerId))
				{
					AuthorityHandleDisconnect(PlayerId);
				}

				PlayerControllersById.Remove(PlayerId);
				PlayerStatesById.Remove(PlayerId);
			}
		}
	}

	Super::Logout(Exiting);
}

bool ADubitoGameMode::IsSessionSeatingMap() const
{
	const UWorld* World = GetWorld();
	// World->GetMapName() carries the PIE prefix (e.g. UEDPIE_0_Table); match by suffix, mirroring
	// ADubitoHUD::IsTableMap so the seating map and the table HUD agree.
	return World && World->GetMapName().EndsWith(TEXT("Table"));
}

int32 ADubitoGameMode::FirstFreeSeatIndex() const
{
	TSet<int32> UsedSeats;
	for (const TPair<int32, TWeakObjectPtr<ADubitoPlayerState>>& Entry : PlayerStatesById)
	{
		if (const ADubitoPlayerState* PlayerState = Entry.Value.Get())
		{
			UsedSeats.Add(PlayerState->GetSeatIndex());
		}
	}

	for (int32 Seat = 0; Seat < DubitoConstants::MaxPlayers; ++Seat)
	{
		if (!UsedSeats.Contains(Seat))
		{
			return Seat;
		}
	}

	return DubitoConstants::MaxPlayers - 1;
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

EDubitoAuthorityStartResult ADubitoGameMode::StartAuthoritativeMatchFromRegisteredPlayers(int32 ShuffleSeed)
{
	bool bAllRegisteredPlayersReady = true;
	const TArray<int32> PlayerIds = BuildRegisteredPlayerIdsBySeat(bAllRegisteredPlayersReady);

	const EDubitoAuthorityStartResult PlayerValidation = ValidatePlayerIds(PlayerIds);
	if (PlayerValidation != EDubitoAuthorityStartResult::Success)
	{
		return PlayerValidation;
	}

	if (!bAllRegisteredPlayersReady)
	{
		return EDubitoAuthorityStartResult::NotAllReady;
	}

	return StartAuthoritativeMatchFromShuffledDeck(PlayerIds, ShuffleSeed);
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

	const EDubitoPhase PreviousPhase = AuthoritativeMatchState.Phase;
	const bool bWasPendingWin = AuthoritativeMatchState.HasPendingWin();
	DubitoRules::NoteVoluntaryAction(AuthoritativeMatchState, PlayerId);
	DubitoRules::ApplyPlay(AuthoritativeMatchState, PlayerId, ActualCards, Announcement);
	RefreshTurnDeadlineForCurrentState();
	SyncReplicatedState();
	if (bWasPendingWin)
	{
		BroadcastGameOverIfNeeded(this, PreviousPhase, EDubitoGameOverReason::PendingWinDeclined);
	}
	return EDubitoPlayValidity::Valid;
}

bool ADubitoGameMode::AuthorityResolveDoubt(int32 PlayerId, FDubitoRevealInfo& OutReveal)
{
	OutReveal = FDubitoRevealInfo();

	if (!DubitoRules::CanDoubt(AuthoritativeMatchState, PlayerId))
	{
		return false;
	}

	const EDubitoPhase PreviousPhase = AuthoritativeMatchState.Phase;
	DubitoRules::NoteVoluntaryAction(AuthoritativeMatchState, PlayerId);
	OutReveal = DubitoRules::ResolveDoubt(AuthoritativeMatchState, PlayerId);
	RefreshTurnDeadlineForCurrentState();
	SyncReplicatedState();
	BroadcastPublicReveal(this, OutReveal);
	BroadcastGameOverIfNeeded(this, PreviousPhase, EDubitoGameOverReason::PendingWinDoubtFailed);
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

	const EDubitoPhase PreviousPhase = AuthoritativeMatchState.Phase;
	const bool bWasPendingWin = AuthoritativeMatchState.HasPendingWin();
	DubitoRules::ResolveTimeout(AuthoritativeMatchState, PlayerId);
	RefreshTurnDeadlineForCurrentState();
	SyncReplicatedState();
	BroadcastGameOverIfNeeded(this, PreviousPhase, bWasPendingWin ? EDubitoGameOverReason::PendingWinTimeout : EDubitoGameOverReason::LastPlayerStanding);
}

void ADubitoGameMode::AuthorityHandleDisconnect(int32 PlayerId)
{
	if (!AuthoritativeMatchState.TurnOrder.Contains(PlayerId))
	{
		return;
	}

	const EDubitoPhase PreviousPhase = AuthoritativeMatchState.Phase;
	const bool bWasPendingWinResponder = AuthoritativeMatchState.HasPendingWin() && AuthoritativeMatchState.ActivePlayerId() == PlayerId;
	DubitoRules::HandleDisconnect(AuthoritativeMatchState, PlayerId);
	RefreshTurnDeadlineForCurrentState();
	SyncReplicatedState();
	const EDubitoGameOverReason Reason = bWasPendingWinResponder
		? EDubitoGameOverReason::PendingWinResponderDisconnected
		: (AuthoritativeMatchState.WinnerId == DubitoConstants::NoPlayerId ? EDubitoGameOverReason::NoPlayersRemaining : EDubitoGameOverReason::LastPlayerStanding);
	BroadcastGameOverIfNeeded(this, PreviousPhase, Reason);
}

bool ADubitoGameMode::RegisterAuthorityPlayerState(ADubitoPlayerState* PlayerState, int32 PlayerId, int32 SeatIndex)
{
	if (!PlayerState || PlayerId == DubitoConstants::NoPlayerId || SeatIndex < 0)
	{
		return false;
	}

	for (const TPair<int32, TWeakObjectPtr<ADubitoPlayerState>>& RegisteredPlayerState : PlayerStatesById)
	{
		if (RegisteredPlayerState.Key != PlayerId && RegisteredPlayerState.Value.IsValid() && RegisteredPlayerState.Value.Get() == PlayerState)
		{
			return false;
		}
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

	for (const TPair<int32, TWeakObjectPtr<ADubitoPlayerController>>& RegisteredPlayerController : PlayerControllersById)
	{
		if (RegisteredPlayerController.Key != PlayerId && RegisteredPlayerController.Value.IsValid() && RegisteredPlayerController.Value.Get() == PlayerController)
		{
			return false;
		}
	}

	if (const TWeakObjectPtr<ADubitoPlayerController>* Existing = PlayerControllersById.Find(PlayerId))
	{
		if (Existing->IsValid() && Existing->Get() != PlayerController)
		{
			return false;
		}
	}

	PlayerControllersById.Add(PlayerId, PlayerController);
	PlayerController->SetAuthorityPlayerId(PlayerId);

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

TArray<int32> ADubitoGameMode::BuildRegisteredPlayerIdsBySeat(bool& bOutAllRegisteredPlayersReady) const
{
	struct FRegisteredPlayerStartInfo
	{
		int32 PlayerId = DubitoConstants::NoPlayerId;
		int32 SeatIndex = INDEX_NONE;
	};

	TArray<FRegisteredPlayerStartInfo> RegisteredPlayers;
	bOutAllRegisteredPlayersReady = true;

	for (const TPair<int32, TWeakObjectPtr<ADubitoPlayerState>>& RegisteredPlayerState : PlayerStatesById)
	{
		const ADubitoPlayerState* PlayerState = RegisteredPlayerState.Value.Get();
		if (!PlayerState || PlayerState->GetDubitoPlayerId() == DubitoConstants::NoPlayerId || PlayerState->GetSeatIndex() < 0)
		{
			continue;
		}

		bOutAllRegisteredPlayersReady &= PlayerState->IsReady();
		RegisteredPlayers.Add({ PlayerState->GetDubitoPlayerId(), PlayerState->GetSeatIndex() });
	}

	RegisteredPlayers.Sort([](const FRegisteredPlayerStartInfo& Left, const FRegisteredPlayerStartInfo& Right)
	{
		if (Left.SeatIndex == Right.SeatIndex)
		{
			return Left.PlayerId < Right.PlayerId;
		}

		return Left.SeatIndex < Right.SeatIndex;
	});

	TArray<int32> PlayerIds;
	PlayerIds.Reserve(RegisteredPlayers.Num());
	for (const FRegisteredPlayerStartInfo& RegisteredPlayer : RegisteredPlayers)
	{
		PlayerIds.Add(RegisteredPlayer.PlayerId);
	}

	return PlayerIds;
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

void ADubitoGameMode::ForceAuthorityStateResync()
{
	SyncReplicatedState();
}

void ADubitoGameMode::RefreshTurnDeadlineForCurrentState()
{
	if (!bAuthoritativeMatchStarted || AuthoritativeMatchState.Phase != EDubitoPhase::PlayerTurn || AuthoritativeMatchState.ActivePlayerId() == DubitoConstants::NoPlayerId)
	{
		TurnDeadlineServerTimeSeconds = 0.0;
		ClearTurnTimer();
		return;
	}

	const UWorld* World = GetWorld();
	const AGameStateBase* PublicGameState = World ? World->GetGameState() : nullptr;
	const double CurrentServerTime = PublicGameState ? PublicGameState->GetServerWorldTimeSeconds() : (World ? World->GetTimeSeconds() : 0.0);
	TurnDeadlineServerTimeSeconds = CurrentServerTime + static_cast<double>(DubitoConstants::TurnSeconds);

	// Keep the authoritative turn timer in lockstep with the deadline so an idle turn
	// auto-resolves as a timeout (Documentation/DESIGN.md timer rules).
	ScheduleTurnTimer();
}

void ADubitoGameMode::ScheduleTurnTimer()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Fire once, TurnSeconds from now, for the player whose turn is currently open. Resolving
	// a timeout advances the turn and reschedules a fresh timer for the next player.
	World->GetTimerManager().SetTimer(TurnTimerHandle, this, &ADubitoGameMode::HandleTurnTimerElapsed,
		static_cast<float>(DubitoConstants::TurnSeconds), false);
}

void ADubitoGameMode::ClearTurnTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TurnTimerHandle);
	}
}

void ADubitoGameMode::HandleTurnTimerElapsed()
{
	// The active player let the clock run out; resolve it as a rules timeout for whoever is
	// currently on the clock (AuthorityResolveTimeout re-checks phase/active player and is a
	// no-op if the state has already moved on).
	AuthorityResolveTimeout(AuthoritativeMatchState.ActivePlayerId());
}

bool ADubitoGameMode::IsTurnTimerActive() const
{
	const UWorld* World = GetWorld();
	return World && World->GetTimerManager().IsTimerActive(TurnTimerHandle);
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
