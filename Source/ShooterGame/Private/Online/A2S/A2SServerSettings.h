#pragma once

#include "A2SServerSettings.generated.h"

//UDP Connection Settings
USTRUCT(BlueprintType)
struct FA2SServerSettings
{
	GENERATED_USTRUCT_BODY()

	/** Port for A2S server to listen and send on */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "A2S Server Properties")
	int Port = 29001;

	/** UDP socket buffer size */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "A2S Server Properties")
	int32 BufferSize = 1800; // A2S packet size + some headroom
};

