#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DubitoLobbyView.h"
#include "DubitoLobbyWidget.generated.h"

class ADubitoGameState;
class ADubitoPlayerController;
class UButton;
class UTextBlock;
class UVerticalBox;

/**
 * Phase 5.6 pre-match lobby panel on the Table map.
 *
 * Shows the seated players and their ready state, lets the local player toggle Ready, and lets the
 * host (seat 0) Start once everyone is ready. It renders the pure BuildDubitoLobbyView and drives
 * the existing RequestReady / RequestStartMatch server paths; it holds no rule authority. It hides
 * itself once the match leaves the lobby, handing the screen to the table HUD and action bar.
 */
UCLASS()
class DUBITO_API UDubitoLobbyWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UDubitoLobbyWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION()
	void HandleReadyToggle();

	UFUNCTION()
	void HandleStart();

private:
	void EnsureLobbyTree();
	void BindToGameState();
	void UnbindFromGameState();
	void HandlePublicMatchStateChanged();
	void Refresh();
	FDubitoLobbyInputs GatherInputs() const;
	void ApplyViewToWidgets();

	ADubitoGameState* ResolveGameState() const;
	ADubitoPlayerController* ResolveDubitoController() const;
	int32 ResolveLocalPlayerId() const;

	FDubitoLobbyView CachedView;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> SeatBox = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TitleText = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> StatusText = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UButton> ReadyButton = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ReadyLabel = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UButton> StartButton = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> StartLabel = nullptr;

	TWeakObjectPtr<ADubitoGameState> BoundGameState;
	FDelegateHandle PublicStateChangedHandle;
	FTimerHandle RefreshTimerHandle;
};
