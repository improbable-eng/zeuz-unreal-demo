// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "A2SServerSettings.h"
#include "Object.h"
#include "Common/UdpSocketBuilder.h"
#include "Common/UdpSocketReceiver.h"
#include "Common/UdpSocketSender.h"

#include "A2SServer.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogA2S, Log, All);

UCLASS()
class UA2SServer : public UObject
{
	GENERATED_BODY()

public:
	UA2SServer();

	~UA2SServer() { }

	/** Start the A2S server */
	UFUNCTION(BlueprintCallable, Category = "A2S Server Functions")
	void Start();

	/** Stop the A2S server */
	UFUNCTION(BlueprintCallable, Category = "A2S Server Functions")
	void Stop();

	/** Settings for the A2S server */
	FA2SServerSettings Settings;

private:
	/** Open a UDP socket for receiving */
	void OpenReceiveSocket();

	/** Open a UDP socket for sending */
	void OpenSendSocket();

	/** Close the UDP socket created by 'OpenSendSocket' */
	void CloseReceiveSocket();

	/** Close the UDP socket created by 'OpenReceiveSocket' */
	void CloseSendSocket();

	/** Handle an A2S query */
	void HandleRequest(const TArray<uint8>& Data, const FIPv4Endpoint& Endpoint);

	/** Validate an A2S query */
	bool IsQueryValid(const TArray<uint8>& Data);

	/** Build the raw bytes response to an A2S_INFO request */
	TArray<uint8> BuildA2SInfoResponse();

	FSocket* SenderSocket;
	FSocket* ReceiverSocket;
	TUniquePtr<FUdpSocketReceiver> UDPReceiver;

	FString PayloadId = TEXT("UNSET-PAYLOAD-ID");

	bool IsStarted = false;

	/** Mutex to ensure the server isn't cleaned up whilst handling a request */
	FCriticalSection Mutex;
};
