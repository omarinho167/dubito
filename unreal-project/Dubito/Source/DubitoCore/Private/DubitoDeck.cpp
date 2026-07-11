#include "DubitoDeck.h"
#include "DubitoConstants.h"

namespace DubitoDeck
{
	TArray<FDubitoCard> BuildStandardDeck()
	{
		TArray<FDubitoCard> Deck;
		Deck.Reserve(DubitoConstants::DeckSize);

		for (int32 SuitIndex = 0; SuitIndex < DubitoConstants::NumSuits; ++SuitIndex)
		{
			for (int32 Rank = DubitoConstants::MinCardRank; Rank <= DubitoConstants::MaxCardRank; ++Rank)
			{
				Deck.Emplace(static_cast<EDubitoSuit>(SuitIndex), Rank);
			}
		}

		return Deck;
	}

	void Shuffle(TArray<FDubitoCard>& Deck, FRandomStream& Stream)
	{
		for (int32 Index = Deck.Num() - 1; Index > 0; --Index)
		{
			const int32 SwapIndex = Stream.RandRange(0, Index);
			Deck.Swap(Index, SwapIndex);
		}
	}

	TArray<FDubitoHand> Deal(const TArray<FDubitoCard>& Deck, int32 NumPlayers, int32& OutRemovedCount)
	{
		TArray<FDubitoHand> Hands;

		if (NumPlayers <= 0)
		{
			OutRemovedCount = Deck.Num();
			return Hands;
		}

		Hands.SetNum(NumPlayers);

		const int32 PerPlayer = Deck.Num() / NumPlayers;
		const int32 DealtCount = PerPlayer * NumPlayers;
		OutRemovedCount = Deck.Num() - DealtCount;

		for (int32 Index = 0; Index < DealtCount; ++Index)
		{
			Hands[Index % NumPlayers].Add(Deck[Index]);
		}

		return Hands;
	}
}
