#pragma once

#include "ShooterGame.h"
#include "SlateBasics.h"
#include "SShooterMenuWidget.h"

class SShooterDirectConnect : public SShooterMenuWidget
{
public:
	SLATE_BEGIN_ARGS(SShooterDirectConnect)
	{}

	SLATE_ARGUMENT(TWeakObjectPtr<ULocalPlayer>, PlayerOwner)
    SLATE_ARGUMENT(TSharedPtr<SWidget>, OwnerWidget)

    SLATE_END_ARGS()

	/** needed for every widget */
    void Construct(const FArguments& InArgs);

	/** if we want to receive focus */
	virtual bool SupportsKeyboardFocus() const override { return true; }

protected:
	FReply OnConnectClicked();

	FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent);

	/** Pointer to owning player */
	TWeakObjectPtr<class ULocalPlayer> PlayerOwner;

	/** pointer to our parent widget */
	TSharedPtr<class SWidget> OwnerWidget;

	/** The edit text widget. */
	TSharedPtr<SEditableTextBox> AddressEditBox;
};
