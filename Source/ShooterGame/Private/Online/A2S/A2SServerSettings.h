#pragma once

#include "A2SServerSettings.generated.h"

//UDP Connection Settings
USTRUCT(BlueprintType)
struct FA2SServerSettings
{
	GENERATED_USTRUCT_BODY()

	/** Port for A2S server to listen on */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "A2S Server Properties")
	int ListenPort = 29001;

	/** Port for A2S server to respond to queries on */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "A2S Server Properties")
	int SendPort = 39001;

	/** UDP socket buffer size */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "A2S Server Properties")
	int32 BufferSize = 1800; // A2S packet size + some headroom
};

