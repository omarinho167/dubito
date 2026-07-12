#include "DubitoCardVisuals.h"

#include "Engine/Texture2D.h"

namespace
{
	FString SuitAssetName(EDubitoSuit Suit)
	{
		switch (Suit)
		{
		case EDubitoSuit::Clubs: return TEXT("Clubs");
		case EDubitoSuit::Diamonds: return TEXT("Diamonds");
		case EDubitoSuit::Hearts: return TEXT("Hearts");
		case EDubitoSuit::Spades: return TEXT("Spades");
		default: return TEXT("Clubs");
		}
	}
}

FString DubitoCardFaceTexturePath(const FDubitoCard& Card)
{
	const FString Suit = SuitAssetName(Card.Suit);
	// Full object path (Package.Object) so LoadObject resolves the Texture2D directly.
	return FString::Printf(TEXT("/Game/Cards/Faces/T_Card_%s_%02d.T_Card_%s_%02d"), *Suit, Card.Rank, *Suit, Card.Rank);
}

UTexture2D* LoadDubitoCardFaceTexture(const FDubitoCard& Card)
{
	if (!Card.IsValid())
	{
		return nullptr;
	}

	return LoadObject<UTexture2D>(nullptr, *DubitoCardFaceTexturePath(Card));
}

UTexture2D* LoadDubitoCardBackTexture()
{
	return LoadObject<UTexture2D>(nullptr, TEXT("/Game/Cards/T_Card_Back.T_Card_Back"));
}
