#include "DubitoHand.h"

bool FDubitoHand::Remove(const FDubitoCard& Card)
{
	const int32 Index = Cards.IndexOfByKey(Card);
	if (Index == INDEX_NONE)
	{
		return false;
	}

	Cards.RemoveAt(Index);
	return true;
}

int32 FDubitoHand::CountOfRank(int32 Rank) const
{
	int32 Count = 0;
	for (const FDubitoCard& Card : Cards)
	{
		if (Card.Rank == Rank)
		{
			++Count;
		}
	}
	return Count;
}
