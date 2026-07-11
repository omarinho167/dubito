#pragma once

#include "CoreMinimal.h"
#include "DubitoConstants.h"
#include "DubitoCard.generated.h"

// The four suits of a standard 52-card deck (no jokers).
UENUM(BlueprintType)
enum class EDubitoSuit : uint8
{
	Clubs,
	Diamonds,
	Hearts,
	Spades
};

// A single playing card: suit + rank (1..13). Rank 0 represents "no card".
// Kept as a plain USTRUCT so it can later replicate as owner-only hand state
// without depending on the Engine module.
USTRUCT(BlueprintType)
struct DUBITOCORE_API FDubitoCard
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Dubito")
	EDubitoSuit Suit = EDubitoSuit::Clubs;

	UPROPERTY(BlueprintReadOnly, Category = "Dubito")
	int32 Rank = 0;

	FDubitoCard() = default;
	FDubitoCard(EDubitoSuit InSuit, int32 InRank)
		: Suit(InSuit), Rank(InRank)
	{
	}

	// A card is valid when its rank falls inside the standard deck range.
	bool IsValid() const
	{
		return Rank >= DubitoConstants::MinCardRank && Rank <= DubitoConstants::MaxCardRank;
	}

	bool operator==(const FDubitoCard& Other) const
	{
		return Suit == Other.Suit && Rank == Other.Rank;
	}

	bool operator!=(const FDubitoCard& Other) const
	{
		return !(*this == Other);
	}

	friend uint32 GetTypeHash(const FDubitoCard& Card)
	{
		return HashCombine(::GetTypeHash(static_cast<uint8>(Card.Suit)), ::GetTypeHash(Card.Rank));
	}
};
