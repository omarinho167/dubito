#include "DubitoGameMode.h"

#include "DubitoConstants.h"
#include "DubitoDeck.h"

ADubitoGameMode::ADubitoGameMode()
{
	bReplicates = false;
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
	return true;
}

void ADubitoGameMode::AuthorityResolveTimeout(int32 PlayerId)
{
	DubitoRules::ResolveTimeout(AuthoritativeMatchState, PlayerId);
}

void ADubitoGameMode::AuthorityHandleDisconnect(int32 PlayerId)
{
	DubitoRules::HandleDisconnect(AuthoritativeMatchState, PlayerId);
}
