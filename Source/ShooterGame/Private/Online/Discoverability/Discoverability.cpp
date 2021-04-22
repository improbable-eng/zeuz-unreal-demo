#include "ShooterGame.h"
#include "Discoverability.h"

#include "IHttpResponse.h"

DEFINE_LOG_CATEGORY(LogDiscoverability);

UDiscoverability::UDiscoverability()
{
	Http = &FHttpModule::Get();
}

void UDiscoverability::Start()
{
	ParseCLIOptions();

	if (Settings.MatchmakerEndpoint.IsEmpty())
	{
		UE_LOG(LogDiscoverability, Warning, TEXT("No matchmaker address specified, not starting discoverability"));
		return;
	}

	const UWorld* World = GetOuter()->GetWorld();
	if (!World)
	{
		UE_LOG(LogDiscoverability, Warning, TEXT("Could not start discoverability as world is null"));
		return;
	}

	World->GetTimerManager().SetTimer(TimerHandle, this, &UDiscoverability::SendUpdate, Settings.Interval, true);

	UE_LOG(LogDiscoverability, Display,
	       TEXT("Started notifying matchmaker of availability (every %ds, endpoint: %s, payloadId: %s, payloadIp: %s)"),
	       Settings.Interval, *Settings.MatchmakerEndpoint, *PayloadId, *PayloadIp);
}

void UDiscoverability::Stop()
{
	if (Settings.MatchmakerEndpoint.IsEmpty())
	{
		UE_LOG(LogDiscoverability, Warning, TEXT("No matchmaker address specified, not starting discoverability"));
		return;
	}

	const UWorld* World = GetOuter()->GetWorld();
	if (!World)
	{
		UE_LOG(LogDiscoverability, Warning, TEXT("Could not stop discoverability as world is null"));
		return;
	}

	World->GetTimerManager().ClearTimer(TimerHandle);
	UE_LOG(LogDiscoverability, Display, TEXT("Stopped notifying matchmaker of availability"));
}

void UDiscoverability::ParseCLIOptions()
{
	if (FParse::Value(FCommandLine::Get(), TEXT("matchmakerAddr"), Settings.MatchmakerEndpoint))
	{
		UE_LOG(LogDiscoverability, Display,
		       TEXT("Overwriting matchmaker address with address given in CLI (-matchmakerAddr): %s"),
		       *Settings.MatchmakerEndpoint);
	}

	if (!FParse::Value(FCommandLine::Get(), TEXT("payloadId"), PayloadId))
	{
		UE_LOG(LogA2S, Display, TEXT("No payload ID given in CLI (-payloadId), defaulting to %s"), *PayloadId);
	}

	if (!FParse::Value(FCommandLine::Get(), TEXT("payloadIp"), PayloadIp))
	{
		UE_LOG(LogA2S, Display, TEXT("No payload IP given in CLI (-payloadIp), defaulting to %s"), *PayloadIp);
	}
}

void UDiscoverability::SendUpdate()
{
	const UWorld* World = GetOuter()->GetWorld();
	if (!World)
	{
		UE_LOG(LogDiscoverability, Warning, TEXT("World is null"));
		return;
	}

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	JsonObject->SetNumberField(TEXT("Ccu"), World->GetAuthGameMode()->GetNumPlayers());
	JsonObject->SetStringField(TEXT("IP"), PayloadIp);
	JsonObject->SetNumberField(TEXT("Port"), World->URL.Port);

	FString Body;
	const TSharedRef<TJsonWriter<TCHAR>> JsonWriter = TJsonWriterFactory<>::Create(&Body);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http->CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &UDiscoverability::OnResponse);
	Request->SetURL(FString::Printf(TEXT("%s/%s"), *Settings.MatchmakerEndpoint, *PayloadId));
	Request->SetVerb("POST");
	Request->SetHeader("Content-Type", TEXT("application/json"));
	Request->SetContentAsString(Body);
	Request->ProcessRequest();
}

void UDiscoverability::OnResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful || !Response.IsValid())
	{
		UE_LOG(LogDiscoverability, Warning, TEXT("Posting to matchmaker failed"));
		return;
	}

	const int32 ResponseCode = Response->GetResponseCode();
	if (ResponseCode < 200 || ResponseCode >= 300)
	{
		UE_LOG(LogDiscoverability, Warning, TEXT("Posting to matchmaker failed with error code %d (message: %s)"),
		       *Response->GetContentAsString());
	}
}
