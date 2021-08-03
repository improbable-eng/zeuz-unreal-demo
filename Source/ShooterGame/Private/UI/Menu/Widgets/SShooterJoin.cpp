#include "ShooterGame.h"
#include "ShooterGameInstance.h"
#include "IHttpResponse.h"
#include "Framework/SlateDelegates.h"
#include "SShooterJoin.h"
#include "ShooterStyle.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "Serialization/JsonSerializer.h"

void SShooterJoin::Construct(const FArguments& InArgs)
{
	PlayerOwner = InArgs._PlayerOwner;
	OwnerWidget = InArgs._OwnerWidget;

	FSlateFontInfo ButtonFont = FShooterStyle::Get().GetFontStyle("ShooterGame.MenuTextStyle");
	ButtonFont.Size = 18;
	const FMargin ButtonPadding = FMargin(10.f);

	// maybe tick this, refresh rate ~1 sec, maybe refresh button (probably won't work on its own)
	//

	TSharedPtr<SVerticalBox> Container = SNew(SVerticalBox);

	for (int32 server = 0; server < this->serversNumber; server++)
	{
		Container->AddSlot().Padding(ButtonPadding)
		[
			SNew(SButton).OnClicked(this, &SShooterJoin::OnJoinClicked, *(this->servers + server))
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("DCWidget", "Join", "JOIN"))  /*  *(this->servers + server).Right(5)  */
				.Font(ButtonFont)
				.Justification(ETextJustify::Center)
			]
		];
	}

	ChildSlot
	.VAlign(VAlign_Fill)
	.HAlign(HAlign_Fill)
	[
		Container.ToSharedRef()
	];
}

void SShooterJoin::Tick(float DeltaTime)
{
	GetServerAddresses();
}

void SShooterJoin::GetServerAddresses()
{
	// Allow the possibility to set the matchmaker endpoint from the command line or the zeuz CLI
	// In the current working of zeuz, this will not work
	/* if (FParse::Value(FCommandLine::Get(), TEXT("matchmakerAddr"), MatchmakerEndpoint))
	{
		UE_LOG(LogOnline, Display, TEXT("Overwriting matchmaker endpoint from zeuz CLI to %s"), *MatchmakerEndpoint)
	}

	else
	{
		UE_LOG(LogOnline, Warning, TEXT("No matchmaker endpoint specified in the zeuz CLI"))
	} */

	UE_LOG(LogOnline, Display, TEXT("Fetching server to join from matchmaker at %s"), *MatchmakerEndpoint);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(MatchmakerEndpoint);
	Request->SetVerb("GET");
	Request->SetHeader("Content-Type", TEXT("application/json"));

	// Add callback to join the server if the fetch was a success
	Request->OnProcessRequestComplete().BindLambda([&](FHttpRequestPtr _, FHttpResponsePtr Response, bool bSuccessful)
		{
			if (!bSuccessful || !Response.IsValid())
			{
				UE_LOG(LogOnline, Warning, TEXT("Posting to matchmaker failed"));
				return;
			}

			const int32 ResponseCode = Response->GetResponseCode();
			if (ResponseCode < 200 || ResponseCode >= 300)
			{
				UE_LOG(LogOnline, Warning, TEXT("Posting to matchmaker failed with error code %d (message: %s)"),
					ResponseCode, *Response->GetContentAsString());
				return;
			}

			TArray<TSharedPtr<FJsonValue>> JsonArray;
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
			if (FJsonSerializer::Deserialize(Reader, JsonArray))
			{
				FString* Addresses = new FString[JsonArray.Num()];
				
				for (int32 i = 0; i < JsonArray.Num(); ++i) {
					const FString IP = JsonArray[i]->AsObject()->GetStringField("IP");
					const int32 Port = JsonArray[i]->AsObject()->GetIntegerField("Port");
					const FString Address = FString::Printf(TEXT("%s:%d"), *IP, Port);
					
					Addresses[i] = Address;
					UE_LOG(LogOnline, Warning, TEXT("Address found: %s"), *Address);
				}

				this->serversNumber = JsonArray.Num();
				this->servers = Addresses;

				return;
			}
			else
			{
				UE_LOG(LogOnline, Warning, TEXT("Could not deserialise JSON (raw: %s)"),
					*Response->GetContentAsString());
				return;
			}
		});

	Request->ProcessRequest();
}

FReply SShooterJoin::OnJoinClicked(FString Address)
{
	if (APlayerController* PlayerController = PlayerOwner->PlayerController)
	{
		if (GEngine && GEngine->GameViewport)
		{
			GEngine->GameViewport->RemoveAllViewportWidgets();
		}
		else
		{
			UE_LOG(LogOnline, Warning, TEXT("Could not get game viewport, no widgets removed"));
		}

		if (UShooterGameInstance* GInstance = Cast<UShooterGameInstance>(PlayerController->GetGameInstance()))
		{
			GInstance->DirectConnectToSession(Address);
		}
		else
		{
			UE_LOG(LogOnline, Warning, TEXT("Could not get game instance, joining server failed"));
		}
	}

	return FReply::Handled();
}

FReply SShooterJoin::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();
	if ((Key == EKeys::Escape || Key == EKeys::Virtual_Back) && !InKeyEvent.IsRepeat())
	{
		// Explicitly state unhandled so call propagates to SShooterMenuWidget, which will close this widget
		return FReply::Unhandled();
	}

	return FReply::Handled();
}