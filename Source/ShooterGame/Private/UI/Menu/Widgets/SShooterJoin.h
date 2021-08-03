#pragma once

#include "ShooterGame.h"
#include "SlateBasics.h"
#include "Framework/SlateDelegates.h"
#include "SShooterMenuWidget.h"

class SShooterJoin : public SShooterMenuWidget
{
public:
	SLATE_BEGIN_ARGS(SShooterJoin) {}

	SLATE_ARGUMENT(TWeakObjectPtr<ULocalPlayer>, PlayerOwner)
	SLATE_ARGUMENT(TSharedPtr<SWidget>, OwnerWidget)

	SLATE_END_ARGS()

	/** Build the widget */
	void Construct(const FArguments& InArgs);

	/** The Tick function, used to refresh the existing servers */
	virtual void Tick(float DeltaTime) override;

	/** If we want to receive focus, suppresses error */
	virtual bool SupportsKeyboardFocus() const override { return true; }

	/** Matchmaker endpoint to fetch server addresses from */
	FString MatchmakerEndpoint;

	/** The available servers to join */
	FString* servers;

	/** The number of available servers */
	int32 serversNumber;

protected:
	/** When the 'Join' button corresponding to a given server is clicked, join that server */
	FReply OnJoinClicked(FString Address);

	/** Gets the available server addresses from the matchmaker, and passes that value to servers */
	void GetServerAddresses();

	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

	/** Pointer to owning player */
	TWeakObjectPtr<class ULocalPlayer> PlayerOwner;

	/** pointer to our parent widget */
	TSharedPtr<class SWidget> OwnerWidget;
};
