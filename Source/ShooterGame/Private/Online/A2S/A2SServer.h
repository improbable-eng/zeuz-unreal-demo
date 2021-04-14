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
	UA2SServer(){};

	~UA2SServer(){}

	/** Start the A2S server */
	UFUNCTION(BlueprintCallable, Category = "A2S Server Functions")
	void Start();

	/** Stop the A2S server */
	UFUNCTION(BlueprintCallable, Category = "A2S Server Functions")
	void Stop();

	/** Setting for the A2S server */
	FA2SServerSettings Settings;

private:
	/** Parse port options from CLI */
	void ParseCLIOptions();
	
	/** Open a UDP socket for receiving */
	bool OpenReceiveSocket();

	/** Open a UDP socket for sending */
	bool OpenSendSocket();

	/** Close the UDP socket created by 'OpenSendSocket' */
	bool CloseReceiveSocket();

	/** Close the UDP socket created by 'OpenReceiveSocket' */
	bool CloseSendSocket();

	/** Handle an A2S query */
	void HandleRequest(const TArray<uint8> Data, const FIPv4Endpoint& Endpoint);

	FSocket* SenderSocket;
	FSocket* ReceiverSocket;
	FUdpSocketReceiver* UDPReceiver;

	bool IsStarted = false;
};
