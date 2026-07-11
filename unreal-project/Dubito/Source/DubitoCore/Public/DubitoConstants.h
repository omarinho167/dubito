#pragma once

#include "CoreMinimal.h"

// V1 rule constants. Must match docs/DESIGN.md and docs/ARCHITECTURE.md.
namespace DubitoConstants
{
	inline constexpr int32 MinPlayers = 2;
	inline constexpr int32 MaxPlayers = 8;

	inline constexpr int32 DeckSize = 52;
	inline constexpr int32 NumSuits = 4;
	inline constexpr int32 MinCardRank = 1;  // Ace
	inline constexpr int32 MaxCardRank = 13; // King

	inline constexpr int32 MinCardsPerPlay = 1;
	inline constexpr int32 MaxCardsPerPlay = 4;

	inline constexpr int32 TurnSeconds = 45;
	inline constexpr int32 MaxConsecutiveTimeouts = 3;

	inline constexpr int32 NoRoundValue = 0;
	inline constexpr int32 NoPlayerId = -1;
}
