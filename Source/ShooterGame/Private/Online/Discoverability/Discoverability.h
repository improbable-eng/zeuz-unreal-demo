#pragma once

#include "CoreMinimal.h"

#include "DiscoverabilitySettings.h"
#include "HttpModule.h"

#include "Discoverability.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDiscoverability, Log, All);

UCLASS()
class UDiscoverability : public UObject
{
	GENERATED_BODY()

public:
	UDiscoverability();

	~UDiscoverability() { }

	/** Start notifying the matchmaker of availability */
	UFUNCTION(BlueprintCallable, Category = "Discoverability Functions")
	void Start();

	/** Stop notifying the matchmaker of availability */
	UFUNCTION(BlueprintCallable, Category = "Discoverability Functions")
	void Stop();

	/** Settings for the discoverability component */
	FDiscoverabilitySettings Settings;

private:
	/** Overwrite blueprint settings with any settings in CLI */ 
	void ParseCLIOptions();
	
	UFUNCTION()
	void SendUpdate();

	void OnResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	FString PayloadId = TEXT("UNSET-PAYLOAD-ID");
	FString PayloadIp = TEXT("UNSET-PAYLOAD-IP");

	FHttpModule* Http;
	FTimerHandle TimerHandle;
};
