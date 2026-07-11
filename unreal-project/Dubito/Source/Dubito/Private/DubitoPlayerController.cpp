#include "DubitoPlayerController.h"

#include "DubitoGameMode.h"
#include "DubitoPlayerState.h"
#include "Net/UnrealNetwork.h"

ADubitoPlayerController::ADubitoPlayerController()
{
	SetReplicates(true);
}

void ADubitoPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ADubitoPlayerController, PrivateHand, PrivateHandReplicationCondition);
}

void ADubitoPlayerController::SetAuthorityPlayerId(int32 InPlayerId)
{
	AuthorityPlayerId = InPlayerId;
}

int32 ADubitoPlayerController::ResolveAuthorityPlayerId() const
{
	if (AuthorityPlayerId != DubitoConstants::NoPlayerId)
	{
		return AuthorityPlayerId;
	}

	const ADubitoPlayerState* DubitoPlayerState = GetPlayerState<ADubitoPlayerState>();
	return DubitoPlayerState ? DubitoPlayerState->GetDubitoPlayerId() : DubitoConstants::NoPlayerId;
}

void ADubitoPlayerController::RequestReady(bool bReady)
{
	if (HasAuthority())
	{
		ExecuteReadyAction(bReady);
		return;
	}

	ServerSetReady(bReady);
}

void ADubitoPlayerController::RequestStartMatch(int32 ShuffleSeed)
{
	if (HasAuthority())
	{
		ExecuteStartMatchAction(ShuffleSeed);
		return;
	}

	ServerStartMatch(ShuffleSeed);
}

void ADubitoPlayerController::RequestPlayCards(const TArray<FDubitoCard>& ActualCards, const FDubitoAnnouncement& Announcement)
{
	if (HasAuthority())
	{
		ExecutePlayAction(ActualCards, Announcement);
		return;
	}

	ServerPlayCards(ActualCards, Announcement);
}

void ADubitoPlayerController::RequestDoubt()
{
	if (HasAuthority())
	{
		ExecuteDoubtAction();
		return;
	}

	ServerDoubt();
}

void ADubitoPlayerController::RequestDiscard()
{
	if (HasAuthority())
	{
		ExecuteDiscardAction();
		return;
	}

	ServerDiscard();
}

void ADubitoPlayerController::ServerSetReady_Implementation(bool bReady)
{
	ExecuteReadyAction(bReady);
}

void ADubitoPlayerController::ServerStartMatch_Implementation(int32 ShuffleSeed)
{
	ExecuteStartMatchAction(ShuffleSeed);
}

void ADubitoPlayerController::ServerPlayCards_Implementation(const TArray<FDubitoCard>& ActualCards, const FDubitoAnnouncement& Announcement)
{
	ExecutePlayAction(ActualCards, Announcement);
}

void ADubitoPlayerController::ServerDoubt_Implementation()
{
	ExecuteDoubtAction();
}

void ADubitoPlayerController::ServerDiscard_Implementation()
{
	ExecuteDiscardAction();
}

void ADubitoPlayerController::SetPrivateHand(const FDubitoHand& InPrivateHand)
{
	PrivateHand = InPrivateHand;
	OnPrivateHandUpdated();
}

void ADubitoPlayerController::ClearPrivateHand()
{
	PrivateHand = FDubitoHand();
	OnPrivateHandUpdated();
}

void ADubitoPlayerController::OnRep_PrivateHand()
{
	OnPrivateHandUpdated();
}

ADubitoGameMode* ADubitoPlayerController::GetAuthorityGameMode() const
{
	const UWorld* World = GetWorld();
	return World ? World->GetAuthGameMode<ADubitoGameMode>() : nullptr;
}

bool ADubitoPlayerController::ExecuteReadyAction(bool bReady)
{
	ADubitoGameMode* GameMode = GetAuthorityGameMode();
	const int32 PlayerId = ResolveAuthorityPlayerId();
	return GameMode && PlayerId != DubitoConstants::NoPlayerId && GameMode->SetAuthorityPlayerReady(PlayerId, bReady);
}

bool ADubitoPlayerController::ExecuteStartMatchAction(int32 ShuffleSeed)
{
	ADubitoGameMode* GameMode = GetAuthorityGameMode();
	const int32 PlayerId = ResolveAuthorityPlayerId();
	return GameMode && PlayerId != DubitoConstants::NoPlayerId && GameMode->StartAuthoritativeMatchFromRegisteredPlayers(ShuffleSeed) == EDubitoAuthorityStartResult::Success;
}

bool ADubitoPlayerController::ExecutePlayAction(const TArray<FDubitoCard>& ActualCards, const FDubitoAnnouncement& Announcement)
{
	ADubitoGameMode* GameMode = GetAuthorityGameMode();
	const int32 PlayerId = ResolveAuthorityPlayerId();
	return GameMode && PlayerId != DubitoConstants::NoPlayerId && GameMode->AuthorityPlayCards(PlayerId, ActualCards, Announcement) == EDubitoPlayValidity::Valid;
}

bool ADubitoPlayerController::ExecuteDoubtAction()
{
	ADubitoGameMode* GameMode = GetAuthorityGameMode();
	const int32 PlayerId = ResolveAuthorityPlayerId();
	FDubitoRevealInfo Reveal;
	return GameMode && PlayerId != DubitoConstants::NoPlayerId && GameMode->AuthorityResolveDoubt(PlayerId, Reveal);
}

bool ADubitoPlayerController::ExecuteDiscardAction()
{
	ADubitoGameMode* GameMode = GetAuthorityGameMode();
	const int32 PlayerId = ResolveAuthorityPlayerId();
	return GameMode && PlayerId != DubitoConstants::NoPlayerId && GameMode->AuthorityDiscard(PlayerId);
}
