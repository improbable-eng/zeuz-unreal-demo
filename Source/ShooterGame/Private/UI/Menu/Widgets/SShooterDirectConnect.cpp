#include "SShooterDirectConnect.h"

#include "ShooterStyle.h"

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
	UE_LOG(LogTemp, Warning, TEXT("Direct connecting to %s"), *Address);

	APlayerController* PlayerController = PlayerOwner->PlayerController;
	if (PlayerController)
	{
		if (GEngine && GEngine->GameViewport)
		{
			GEngine->GameViewport->RemoveAllViewportWidgets();
		}

		UShooterGameInstance* ShooterGameInstance = Cast<UShooterGameInstance>(PlayerController->GetGameInstance());
		ShooterGameInstance->DirectConnectToSession(Address);
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
