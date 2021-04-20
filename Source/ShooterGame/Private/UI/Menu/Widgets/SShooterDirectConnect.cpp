#include "SShooterDirectConnect.h"

#include "ShooterStyle.h"

void SShooterDirectConnect::Construct(const FArguments& InArgs)
{
	PlayerOwner = InArgs._PlayerOwner;
	OwnerWidget = InArgs._OwnerWidget;

	const FSlateFontInfo PlayerNameFontInfo = FShooterStyle::Get().GetFontStyle("ShooterGame.MenuProfileNameStyle");
	FSlateFontInfo ChatFont = FShooterStyle::Get().GetFontStyle("ShooterGame.ChatFont");
	ChatFont.Size = 18;

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
         		.Font(ChatFont)
			]

			// Connect button
			+ SVerticalBox::Slot()
			.Padding(ButtonPadding)
			[
				SNew(SButton)
				.OnClicked(this, &SShooterDirectConnect::OnConnectClicked)
				[
					SNew(STextBlock)
        			.Font(PlayerNameFontInfo)
        			.Text(NSLOCTEXT("DCWidget", "Connect", "CONNECT"))
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
	UE_LOG(LogTemp, Warning, TEXT("Direct connecting to %s"), *Address);

	APlayerController* PlayerController = PlayerOwner->PlayerController;
	if (PlayerController)
	{
		PlayerController->ConsoleCommand(FString("open ").Append(Address));
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
