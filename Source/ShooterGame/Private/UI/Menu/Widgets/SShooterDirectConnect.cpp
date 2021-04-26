#include "SShooterDirectConnect.h"

#include "ShooterStyle.h"

const FRegexPattern IPPortRegex(TEXT(
	"^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5]):[0-9]+"
));

void SShooterDirectConnect::Construct(const FArguments& InArgs)
{
	PlayerOwner = InArgs._PlayerOwner;
	OwnerWidget = InArgs._OwnerWidget;

	FSlateFontInfo TextEnterFont = FShooterStyle::Get().GetFontStyle("ShooterGame.ChatFont");
	TextEnterFont.Size = 14;

	FSlateFontInfo ButtonFont = FShooterStyle::Get().GetFontStyle("ShooterGame.MenuTextStyle");
	ButtonFont.Size = 18;
	const FMargin ButtonPadding = FMargin(10.f);

	ChildSlot
	.VAlign(VAlign_Fill)
	.HAlign(HAlign_Fill)
	[
		SNew(SVerticalBox)

		// Address text box
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SAssignNew(AddressEditBox, SEditableTextBox)
			.MinDesiredWidth(350.f)
			.ClearKeyboardFocusOnCommit(false)
			.HintText(NSLOCTEXT("DCWidget", "EnterAddress", "Enter server address with port..."))
			.Font(TextEnterFont)
		]

		// Connect button
		+ SVerticalBox::Slot()
		.Padding(ButtonPadding)
		[
			SNew(SButton)
			.OnClicked(this, &SShooterDirectConnect::OnConnectClicked)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("DCWidget", "Connect", "CONNECT"))
				.Font(ButtonFont)
				.Justification(ETextJustify::Center)
			]
		]
	];
}

FReply SShooterDirectConnect::OnConnectClicked()
{
	if (AddressEditBox->GetText().IsEmpty())
	{
		return FReply::Handled();
	}

	const FString Address = AddressEditBox->GetText().ToString();
	FRegexMatcher Matcher(IPPortRegex, Address);
	if (!Matcher.FindNext())
	{
		// On-screen messaging for this would be better, but for now just handle gracefully 
		UE_LOG(LogOnline, Warning, TEXT("Did not match IP:Port regex (address: %s), will not attempt to connect"),
		       *Address)
		return FReply::Handled();
	}

	UE_LOG(LogOnline, Display, TEXT("Direct connecting to %s"), *Address);

	if (PlayerOwner->PlayerController)
	{
		APlayerController* PlayerController = PlayerOwner->PlayerController;
		if (GEngine && GEngine->GameViewport)
		{
			GEngine->GameViewport->RemoveViewportWidgetContent(OwnerWidget.ToSharedRef());
		}

		UShooterGameInstance* ShooterGameInstance = Cast<UShooterGameInstance>(PlayerController->GetGameInstance());
		if (ShooterGameInstance)
		{
			ShooterGameInstance->DirectConnectToSession(Address);
		}
	}

	return FReply::Handled();
}

FReply SShooterDirectConnect::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();
	if ((Key == EKeys::Escape || Key == EKeys::Virtual_Back) && !InKeyEvent.IsRepeat())
	{
		// Explicitly state unhandled so call propagates to SShooterMenuWidget, which will close this widget
		return FReply::Unhandled();
	}

	return FReply::Handled();
}
