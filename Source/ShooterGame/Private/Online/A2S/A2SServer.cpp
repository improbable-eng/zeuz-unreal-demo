#include "ShooterGame.h"
#include "A2SServer.h"

DEFINE_LOG_CATEGORY(LogA2S);

UA2SServer::UA2SServer()
{
	if (!FParse::Value(FCommandLine::Get(), TEXT("payloadId"), PayloadId))
	{
		UE_LOG(LogA2S, Warning, TEXT("No payload ID given in CLI (-payloadId), defaulting to %s"), *PayloadId)
	}
	if (!FParse::Value(FCommandLine::Get(), TEXT("statsPort"), Settings.Port))
	{
		UE_LOG(LogA2S, Warning, TEXT("No stats listen port given in CLI (-statsPort), defaulting to %d"), Settings.Port)
	}
}

void UA2SServer::Start()
{
	if (IsStarted)
	{
		UE_LOG(LogA2S, Display, TEXT("Ignoring duplicated `A2SServer->Start` call"))
		return;
	}

	OpenReceiveSocket();
	OpenSendSocket();

	UDPReceiver->Start();
	IsStarted = true;
	UE_LOG(LogA2S, Display, TEXT("Started A2S server on port %d"), Settings.Port)
}

void UA2SServer::Stop()
{
	UDPReceiver->Stop();

	CloseReceiveSocket();
	CloseSendSocket();

	UE_LOG(LogA2S, Display, TEXT("Stopping A2S server"))
}

void UA2SServer::OpenReceiveSocket()
{
	const FIPv4Endpoint Endpoint(FIPv4Address::Any, Settings.Port);
	ReceiverSocket = FUdpSocketBuilder(FString(TEXT("ue4-a2s-receive")))
	                 .AsNonBlocking()
	                 .AsReusable()
	                 .BoundToEndpoint(Endpoint)
	                 .WithReceiveBufferSize(Settings.BufferSize);

	const FTimespan ThreadWaitTime = FTimespan::FromMilliseconds(100);
	const FString ThreadName = FString::Printf(TEXT("UDP-RECEIVER-A2S"));
	UDPReceiver = new FUdpSocketReceiver(ReceiverSocket, ThreadWaitTime, *ThreadName);

	UDPReceiver->OnDataReceived().BindLambda([this](const FArrayReaderPtr& DataPtr, const FIPv4Endpoint& Endpoint)
	{
		TArray<uint8> Data;
		Data.AddUninitialized(DataPtr->TotalSize());
		DataPtr->Serialize(Data.GetData(), DataPtr->TotalSize());

		this->HandleRequest(Data, Endpoint);
	});
}

void UA2SServer::OpenSendSocket()
{
	SenderSocket = FUdpSocketBuilder(FString(TEXT("ue4-a2s-send")))
	               .AsNonBlocking()
	               .BoundToPort(Settings.Port)
	               .AsReusable();

	SenderSocket->SetSendBufferSize(Settings.BufferSize, Settings.BufferSize);
	SenderSocket->SetReceiveBufferSize(Settings.BufferSize, Settings.BufferSize);
}

void UA2SServer::CloseReceiveSocket()
{
	delete UDPReceiver;
	UDPReceiver = nullptr;

	ReceiverSocket->Close();
	ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ReceiverSocket);
	ReceiverSocket = nullptr;
}

void UA2SServer::CloseSendSocket()
{
	SenderSocket->Close();
	ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(SenderSocket);
	SenderSocket = nullptr;
}

void UA2SServer::HandleRequest(const TArray<uint8> Data, const FIPv4Endpoint& Endpoint)
{
	UE_LOG(LogA2S, Display, TEXT("Handling A2S request from %s"), *Endpoint.ToString())

	if (!IsQueryValid(Data))
	{
		// Do not respond
		return;
	}

	TArray<uint8> ResponseBytes = BuildA2SInfoResponse();

	const FString SenderIP = Endpoint.Address.ToString();
	TSharedPtr<FInternetAddr> RemoteAddress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();

	bool bIsValid;
	RemoteAddress->SetIp(*SenderIP, bIsValid);
	RemoteAddress->SetPort(Endpoint.Port);
	if (!bIsValid)
	{
		UE_LOG(LogA2S, Error, TEXT("UDP address is invalid <%s:%d>"), *SenderIP, Endpoint.Port);
		return;
	}

	int32 BytesSent;
	SenderSocket->SendTo(ResponseBytes.GetData(), ResponseBytes.Num(), BytesSent, *RemoteAddress);

	if (BytesSent != ResponseBytes.Num())
	{
		UE_LOG(LogA2S, Warning, TEXT("Not all bytes sent in A2S response (expected: %d, actual: %d)"), ResponseBytes.Num(), BytesSent)
	}
}


/** This server only supports A2S_INFO requests */
bool UA2SServer::IsQueryValid(const TArray<uint8> Data)
{
	if (Data.Num() < 5)
	{
		// At least 4 bytes needed packet Header + 1 byte for request Header
		UE_LOG(LogA2S, Warning, TEXT("A2S request too short: %d bytes"), Data.Num())
		return false;
	}

	// Check the Header (the first 4 bytes) are 0xFF
	// (this indicates a single-packet request, multi-packet requests are not supported)
	if (Data[0] != 0xFF || Data[1] != 0xFF || Data[2] != 0xFF || Data[3] != 0xFF)
	{
		UE_LOG(LogA2S, Warning, TEXT("A2S packet header invalid: %02x %02x %02x %02x"), Data[0], Data[1], Data[2],
		       Data[3])
		return false;
	}

	// Check request Header (the 5th byte) denotes an A2S_INFO request
	if (Data[4] != 0x54)
	{
		UE_LOG(LogA2S, Warning, TEXT("A2S request header invalid: %x"), Data[4])
		return false;
	}

	return true;
}

void AddStringToByteArray(TArray<uint8>& Bytes, FString InString)
{
	// Encode string as UTF-8 (default encoding is UTF-16)
	const FTCHARToUTF8 InStringUtf8(*InString);
	const FString OutString(InStringUtf8.Get());

	// Copy string into byte buffer
	// Note: Unreal's 'StringToBytes' method doesn't undo the +1 offset of its internal UTF-8 encoding so write our own
	// conversion
	TArray<uint8> Buffer;
	Buffer.AddUninitialized(OutString.Len());

	int32 NumBytes = 0;
	const TCHAR* CharPos = *OutString;

	while (*CharPos && NumBytes < OutString.Len())
	{
		Bytes.Add(static_cast<int8>(*CharPos));
		CharPos++;
		++NumBytes;
	}

	// Add string terminating byte
	Bytes.Add(0x00);
}

TArray<uint8> UA2SServer::BuildA2SInfoResponse()
{
	TArray<uint8> Bytes;

	// Add packet header (to show single packet response)
	uint8 Header[] = {0xFF, 0xFF, 0xFF, 0xFF};
	Bytes.Append(Header, UE_ARRAY_COUNT(Header));

	const UWorld* World = GetOuter()->GetWorld();
	if (!World)
	{
		UE_LOG(LogA2S, Warning, TEXT("World is null"))
		return Bytes;
	}

	Bytes.Add(0x49); // Add response header (for A2S_INFO only)
	Bytes.Add(0x11); // Add protocol version (17)
	AddStringToByteArray(Bytes, PayloadId); // Add server name
	AddStringToByteArray(Bytes, World->GetMapName()); // Add map name
	AddStringToByteArray(Bytes, FString(TEXT("ShooterGame"))); // Add name of folder containing game files
	AddStringToByteArray(Bytes, FString(TEXT("ShooterGame"))); // Add name of game
	Bytes.Add(0x0); // Add Steam ID of game (unset) (first part of a 2-byte short)
	Bytes.Add(0x0); // Add Steam ID of game (unset) (second part of a 2-byte short)
	Bytes.Add(World->GetAuthGameMode()->GetNumPlayers()); // Add number of players
	Bytes.Add(World->GetAuthGameMode()->GameSession->MaxPlayers); // Add max. number of players
	Bytes.Add(0x0); // Add number of bots
	Bytes.Add(0x64); // Add server type (dedicated = 'd' = 0x64 UTF-8)
	Bytes.Add(0x6C); // Add server OS (linux = 'l' = 0x6C UTF-8)
	Bytes.Add(0x0); // Add visibility (indicates if the server needs a password)
	Bytes.Add(0x0); // Add VAC (Valve Anti Cheat)
	AddStringToByteArray(Bytes, FString(TEXT("1.0.0"))); // Add version of the game

	// Add EDF (Extra Data Flag, determines which of the next optional fields feature)
	// Only specify that the port field is included
	Bytes.Add(0x80);

	// Port is uint16 and shorts are little endian so split port (is uint32 but can fit into uint16) and add in parts
	Bytes.Add(World->URL.Port & 0xFF); // Add port (first part, lower bits)
	Bytes.Add(World->URL.Port >> 8); // Add port (second part, higher bits)

	// Add terminating 0x0
	Bytes.Add(0x0);

	return Bytes;
}
