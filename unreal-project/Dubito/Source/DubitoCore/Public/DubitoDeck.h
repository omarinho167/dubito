#pragma once

#include "CoreMinimal.h"
#include "Math/RandomStream.h"
#include "DubitoCard.h"
#include "DubitoHand.h"

// Pure deck operations: build, deterministic shuffle, and deal.
// All functions are deterministic given their inputs (shuffle is driven by an
// explicit FRandomStream), so they are fully unit-testable without a level.
namespace DubitoDeck
{
	// Builds an ordered standard 52-card deck (4 suits x ranks 1..13), no jokers.
	DUBITOCORE_API TArray<FDubitoCard> BuildStandardDeck();

	// In-place Fisher-Yates shuffle driven by the given stream. Same seed => same order.
	DUBITOCORE_API void Shuffle(TArray<FDubitoCard>& Deck, FRandomStream& Stream);

	// Deals the deck as evenly as possible to NumPlayers, round-robin.
	// Leftover cards that cannot be dealt evenly are removed from the game;
	// their count is written to OutRemovedCount. Returns one hand per player.
	DUBITOCORE_API TArray<FDubitoHand> Deal(const TArray<FDubitoCard>& Deck, int32 NumPlayers, int32& OutRemovedCount);
}
