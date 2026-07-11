#include "DubitoPlayerController.h"

#include "DubitoGameMode.h"
#include "DubitoPlayerState.h"
#include "Net/UnrealNetwork.h"

namespace
{
	EDubitoActionRejectionReason ToStartRejectionReason(EDubitoAuthorityStartResult Result)
	{
		switch (Result)
		{
		case EDubitoAuthorityStartResult::NotAllReady:
			return EDubitoActionRejectionReason::StartNotAllReady;
		case EDubitoAuthorityStartResult::InvalidPlayerCount:
		case EDubitoAuthorityStartResult::InvalidPlayerId:
		case EDubitoAuthorityStartResult::DuplicatePlayerId:
		case EDubitoAuthorityStartResult::MissingHand:
		default:
			return EDubitoActionRejectionReason::StartInvalidPlayers;
		}
	}

	EDubitoActionRejectionReason ToPlayRejectionReason(EDubitoPlayValidity Result)
	{
		switch (Result)
		{
		case EDubitoPlayValidity::WrongPhase:
			return EDubitoActionRejectionReason::PlayWrongPhase;
		case EDubitoPlayValidity::NotYourTurn:
			return EDubitoActionRejectionReason::PlayNotYourTurn;
		case EDubitoPlayValidity::BadActualCount:
			return EDubitoActionRejectionReason::PlayBadActualCount;
		case EDubitoPlayValidity::BadAnnouncement:
			return EDubitoActionRejectionReason::PlayBadAnnouncement;
		case EDubitoPlayValidity::ValueLocked:
			return EDubitoActionRejectionReason::PlayValueLocked;
		case EDubitoPlayValidity::DontOwnCards:
			return EDubitoActionRejectionReason::PlayDontOwnCards;
		case EDubitoPlayValidity::Valid:
		default:
			return EDubitoActionRejectionReason::None;
		}
	}
}

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

void ADubitoPlayerController::ClientReceiveActionRejected_Implementation(EDubitoServerAction Action, EDubitoActionRejectionReason Reason, bool bRequestedResync)
{
	RecordActionRejection(Action, Reason, bRequestedResync);
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
	if (!GameMode)
	{
		RejectAction(nullptr, EDubitoServerAction::Ready, EDubitoActionRejectionReason::MissingAuthority);
		return false;
	}

	if (PlayerId == DubitoConstants::NoPlayerId || !GameMode->SetAuthorityPlayerReady(PlayerId, bReady))
	{
		RejectAction(GameMode, EDubitoServerAction::Ready, EDubitoActionRejectionReason::MissingPlayer);
		return false;
	}

	return true;
}

bool ADubitoPlayerController::ExecuteStartMatchAction(int32 ShuffleSeed)
{
	ADubitoGameMode* GameMode = GetAuthorityGameMode();
	const int32 PlayerId = ResolveAuthorityPlayerId();
	if (!GameMode)
	{
		RejectAction(nullptr, EDubitoServerAction::StartMatch, EDubitoActionRejectionReason::MissingAuthority);
		return false;
	}

	if (PlayerId == DubitoConstants::NoPlayerId)
	{
		RejectAction(GameMode, EDubitoServerAction::StartMatch, EDubitoActionRejectionReason::MissingPlayer);
		return false;
	}

	const EDubitoAuthorityStartResult StartResult = GameMode->StartAuthoritativeMatchFromRegisteredPlayers(ShuffleSeed);
	if (StartResult != EDubitoAuthorityStartResult::Success)
	{
		RejectAction(GameMode, EDubitoServerAction::StartMatch, ToStartRejectionReason(StartResult));
		return false;
	}

	return true;
}

bool ADubitoPlayerController::ExecutePlayAction(const TArray<FDubitoCard>& ActualCards, const FDubitoAnnouncement& Announcement)
{
	ADubitoGameMode* GameMode = GetAuthorityGameMode();
	const int32 PlayerId = ResolveAuthorityPlayerId();
	if (!GameMode)
	{
		RejectAction(nullptr, EDubitoServerAction::Play, EDubitoActionRejectionReason::MissingAuthority);
		return false;
	}

	if (PlayerId == DubitoConstants::NoPlayerId)
	{
		RejectAction(GameMode, EDubitoServerAction::Play, EDubitoActionRejectionReason::MissingPlayer);
		return false;
	}

	const EDubitoPlayValidity PlayResult = GameMode->AuthorityPlayCards(PlayerId, ActualCards, Announcement);
	if (PlayResult != EDubitoPlayValidity::Valid)
	{
		RejectAction(GameMode, EDubitoServerAction::Play, ToPlayRejectionReason(PlayResult));
		return false;
	}

	return true;
}

bool ADubitoPlayerController::ExecuteDoubtAction()
{
	ADubitoGameMode* GameMode = GetAuthorityGameMode();
	const int32 PlayerId = ResolveAuthorityPlayerId();
	if (!GameMode)
	{
		RejectAction(nullptr, EDubitoServerAction::Doubt, EDubitoActionRejectionReason::MissingAuthority);
		return false;
	}

	if (PlayerId == DubitoConstants::NoPlayerId)
	{
		RejectAction(GameMode, EDubitoServerAction::Doubt, EDubitoActionRejectionReason::MissingPlayer);
		return false;
	}

	FDubitoRevealInfo Reveal;
	if (!GameMode->AuthorityResolveDoubt(PlayerId, Reveal))
	{
		RejectAction(GameMode, EDubitoServerAction::Doubt, EDubitoActionRejectionReason::DoubtUnavailable);
		return false;
	}

	return true;
}

bool ADubitoPlayerController::ExecuteDiscardAction()
{
	ADubitoGameMode* GameMode = GetAuthorityGameMode();
	const int32 PlayerId = ResolveAuthorityPlayerId();
	if (!GameMode)
	{
		RejectAction(nullptr, EDubitoServerAction::Discard, EDubitoActionRejectionReason::MissingAuthority);
		return false;
	}

	if (PlayerId == DubitoConstants::NoPlayerId)
	{
		RejectAction(GameMode, EDubitoServerAction::Discard, EDubitoActionRejectionReason::MissingPlayer);
		return false;
	}

	if (!GameMode->AuthorityDiscard(PlayerId))
	{
		RejectAction(GameMode, EDubitoServerAction::Discard, EDubitoActionRejectionReason::DiscardUnavailable);
		return false;
	}

	return true;
}

void ADubitoPlayerController::RejectAction(ADubitoGameMode* GameMode, EDubitoServerAction Action, EDubitoActionRejectionReason Reason)
{
	const bool bRequestedResync = GameMode != nullptr;
	if (bRequestedResync)
	{
		GameMode->ForceAuthorityStateResync();
	}

	if (GetNetMode() == NM_Standalone || IsLocalController())
	{
		RecordActionRejection(Action, Reason, bRequestedResync);
	}
	else
	{
		ClientReceiveActionRejected(Action, Reason, bRequestedResync);
	}
}

void ADubitoPlayerController::RecordActionRejection(EDubitoServerAction Action, EDubitoActionRejectionReason Reason, bool bRequestedResync)
{
	LastRejectedAction = Action;
	LastRejectionReason = Reason;
	bLastRejectionRequestedResync = bRequestedResync;
	++ActionRejectionCount;
	OnServerActionRejected(Action, Reason);
}
