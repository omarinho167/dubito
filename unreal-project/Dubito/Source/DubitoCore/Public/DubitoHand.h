#pragma once

#include "CoreMinimal.h"
#include "DubitoCard.h"
#include "DubitoHand.generated.h"

// A player's private hand: an ordered list of cards with add/remove semantics.
// Exact contents are owner-only knowledge; hidden-information handling lives in
// the network layer, not here.
USTRUCT(BlueprintType)
struct DUBITOCORE_API FDubitoHand
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Dubito")
	TArray<FDubitoCard> Cards;

	int32 Num() const { return Cards.Num(); }
	bool IsEmpty() const { return Cards.Num() == 0; }

	void Add(const FDubitoCard& Card) { Cards.Add(Card); }

	bool Contains(const FDubitoCard& Card) const { return Cards.Contains(Card); }

	// Removes a single matching card. Returns false if no matching card was present.
	bool Remove(const FDubitoCard& Card);

	// Number of cards in hand whose rank equals the given rank.
	int32 CountOfRank(int32 Rank) const;
};
