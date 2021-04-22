#pragma once

#include "DiscoverabilitySettings.generated.h"

USTRUCT(BlueprintType)
struct FDiscoverabilitySettings
{
	GENERATED_USTRUCT_BODY()

	/** Endpoint of the matchmaker to send updates to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discoverability Properties")
	FString MatchmakerEndpoint = TEXT("");

	/** Time in seconds between updates sent to the matchmaker */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discoverability Properties")
	int Interval = 5;
};
