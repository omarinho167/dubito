#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DubitoAnnouncement.h"
#include "DubitoCard.h"
#include "DubitoMatchState.h"
#include "DubitoReveal.h"
#include "DubitoRules.h"
#include "DubitoGameMode.generated.h"

class ADubitoGameState;
class ADubitoPlayerController;
class ADubitoPlayerState;

/**
 * Result for server-authoritative match setup.
 *
 * These are ordinary setup failures, not abusive client RPC validation failures.
 */
enum class EDubitoAuthorityStartResult : uint8
{
	Success,
	InvalidPlayerCount,
	InvalidPlayerId,
	DuplicatePlayerId,
	MissingHand
};

/**
 * Phase 4.0 server authority bridge.
 *
 * GameMode exists only on the server/listen host. It owns the complete hidden
 * match state and routes every rule decision through DubitoCore. Replication,
 * RPC entry points, and UI binding are later Phase 4 sub-phases.
 */
UCLASS()
class DUBITO_API ADubitoGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ADubitoGameMode();

	const FDubitoMatchState& GetAuthoritativeMatchState() const { return AuthoritativeMatchState; }
	bool HasAuthoritativeMatchStarted() const { return bAuthoritativeMatchStarted; }
	int32 GetRemovedDealCardCount() const { return RemovedDealCardCount; }

	EDubitoAuthorityStartResult StartAuthoritativeMatchFromHands(const TArray<int32>& PlayerIds, const TMap<int32, FDubitoHand>& DealtHands);
	EDubitoAuthorityStartResult StartAuthoritativeMatchFromShuffledDeck(const TArray<int32>& PlayerIds, int32 ShuffleSeed);

	bool CanAuthorityPlay(int32 PlayerId) const;
	bool CanAuthorityDoubt(int32 PlayerId) const;
	bool CanAuthorityDiscard(int32 PlayerId) const;

	EDubitoPlayValidity AuthorityPlayCards(int32 PlayerId, const TArray<FDubitoCard>& ActualCards, const FDubitoAnnouncement& Announcement);
	bool AuthorityResolveDoubt(int32 PlayerId, FDubitoRevealInfo& OutReveal);
	bool AuthorityDiscard(int32 PlayerId);
	void AuthorityResolveTimeout(int32 PlayerId);
	void AuthorityHandleDisconnect(int32 PlayerId);

	bool RegisterAuthorityPlayerState(ADubitoPlayerState* PlayerState, int32 PlayerId, int32 SeatIndex);
	bool RegisterAuthorityPlayerController(ADubitoPlayerController* PlayerController, int32 PlayerId);
	bool SetAuthorityPlayerReady(int32 PlayerId, bool bReady);

private:
	static EDubitoAuthorityStartResult ValidatePlayerIds(const TArray<int32>& PlayerIds);

	void RefreshTurnDeadlineForCurrentState();
	void SyncReplicatedState();
	void SyncPublicGameState();
	void SyncPublicPlayerStates();
	void SyncPrivateHands();

	FDubitoMatchState AuthoritativeMatchState;
	bool bAuthoritativeMatchStarted = false;
	int32 RemovedDealCardCount = 0;
	double TurnDeadlineServerTimeSeconds = 0.0;
	TMap<int32, TWeakObjectPtr<ADubitoPlayerController>> PlayerControllersById;
	TMap<int32, TWeakObjectPtr<ADubitoPlayerState>> PlayerStatesById;
};
