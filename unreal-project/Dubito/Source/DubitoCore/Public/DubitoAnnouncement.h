#pragma once

#include "CoreMinimal.h"
#include "DubitoConstants.h"
#include "DubitoAnnouncement.generated.h"

// A player's public announcement when playing: the claimed value and claimed count.
// The claimed value must match the locked round value once a round is open; the
// claimed count (1..4) is a bluffable number that may differ from the actual count.
USTRUCT(BlueprintType)
struct DUBITOCORE_API FDubitoAnnouncement
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Dubito")
	int32 ClaimedValue = DubitoConstants::NoRoundValue;

	UPROPERTY(BlueprintReadOnly, Category = "Dubito")
	int32 ClaimedCount = 0;

	FDubitoAnnouncement() = default;
	FDubitoAnnouncement(int32 InValue, int32 InCount)
		: ClaimedValue(InValue), ClaimedCount(InCount)
	{
	}

	// Claimed count must be within the legal play size (1..4).
	bool HasValidCount() const
	{
		return ClaimedCount >= DubitoConstants::MinCardsPerPlay && ClaimedCount <= DubitoConstants::MaxCardsPerPlay;
	}

	// Claimed value must name a real card rank (1..13).
	bool HasValidValue() const
	{
		return ClaimedValue >= DubitoConstants::MinCardRank && ClaimedValue <= DubitoConstants::MaxCardRank;
	}

	bool IsWellFormed() const
	{
		return HasValidCount() && HasValidValue();
	}
};
