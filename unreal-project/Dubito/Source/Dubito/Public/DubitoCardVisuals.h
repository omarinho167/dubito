#pragma once

#include "CoreMinimal.h"
#include "DubitoCard.h"

class UTexture2D;

/**
 * Phase 5 first visual pass: map a pure FDubitoCard to its imported face texture.
 *
 * The 52 faces live at `/Game/Cards/Faces/T_Card_<Suit>_<NN>` (NN 01=Ace..13=King) and the shared
 * single card back at `/Game/Cards/T_Card_Back` (see the Phase 3.5 asset follow-up). These loaders
 * return nullptr when an asset is missing so callers can fall back to a text label, keeping the UI
 * working even if the owner-local card pack is absent (the pack is not yet committed to LFS).
 *
 * The shared back must be reused for every face-down card to preserve the hidden-information rules.
 */
DUBITO_API FString DubitoCardFaceTexturePath(const FDubitoCard& Card);
DUBITO_API UTexture2D* LoadDubitoCardFaceTexture(const FDubitoCard& Card);
DUBITO_API UTexture2D* LoadDubitoCardBackTexture();
