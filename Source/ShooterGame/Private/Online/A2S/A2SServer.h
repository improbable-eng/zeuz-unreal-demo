// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "CoreMinimal.h"
#include "Common/UdpSocketBuilder.h"
#include "Common/UdpSocketReceiver.h"
#include "Common/UdpSocketSender.h"

DECLARE_LOG_CATEGORY_EXTERN(LogA2S, Log, All);

class FA2SServer
{
public:
	FA2SServer();

	~FA2SServer()
	{
	};

	/** Start the A2S server */
	void Start();

	/** Stop the A2S server */
	void Stop();

private:
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

	int ListenPort = 29001;
	int SendPort = 39001;

	FSocket* SenderSocket;
	FSocket* ReceiverSocket;
	FUdpSocketReceiver* UDPReceiver;

	/** Packet size + some headroom, according to https://developer.valvesoftware.com/wiki/Server_queries#Protocol */
	int32 BufferSize = 1800 * 2;

	bool IsStarted = false;
};
