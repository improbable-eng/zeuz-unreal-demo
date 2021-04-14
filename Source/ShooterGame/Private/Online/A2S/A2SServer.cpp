#include "ShooterGame.h"
#include "A2SServer.h"

DEFINE_LOG_CATEGORY(LogA2S);

void UA2SServer::ParseCLIOptions()
{
	// Get the port to listen on
	if (!FParse::Value(FCommandLine::Get(), TEXT("statsPort"), Settings.ListenPort))
	{
		UE_LOG(LogA2S, Warning, TEXT("No stats listen port given in CLI (-statsPort), defaulting to %d"), Settings.ListenPort)
	}
	if (!FParse::Value(FCommandLine::Get(), TEXT("statsSendPort"), Settings.ListenPort))
	{
		UE_LOG(LogA2S, Warning, TEXT("No stats send port given in CLI (-statsSendPort), defaulting to %d"), Settings.SendPort)
	}
}

void UA2SServer::Start()
{
	if (IsStarted)
	{
		UE_LOG(LogA2S, Display, TEXT("Ignoring duplicated `A2SServer->Start` call"))
		return;
	}

	ParseCLIOptions();

	OpenReceiveSocket();
	OpenSendSocket();

	UDPReceiver->Start();
	IsStarted = true;
	UE_LOG(LogA2S, Display, TEXT("Started A2S server on port %d (sending on port %d)"), Settings.ListenPort, Settings.SendPort)
}

void UA2SServer::Stop()
{
	UDPReceiver->Stop();

	CloseReceiveSocket();
	CloseSendSocket();

	UE_LOG(LogA2S, Display, TEXT("Stopping A2S server"))
}

bool UA2SServer::OpenReceiveSocket()
{
	FIPv4Endpoint Endpoint(FIPv4Address::Any, Settings.ListenPort);
	ReceiverSocket = FUdpSocketBuilder(FString(TEXT("ue4-a2s-receive")))
	                 .AsBlocking()
	                 .AsReusable()
	                 .BoundToEndpoint(Endpoint)
	                 .WithReceiveBufferSize(Settings.BufferSize);

	const FTimespan ThreadWaitTime = FTimespan::FromMilliseconds(100);
	const FString ThreadName = FString::Printf(TEXT("UDP-RECEIVER-A2S"));
	UDPReceiver = new FUdpSocketReceiver(ReceiverSocket, ThreadWaitTime, *ThreadName);

	UDPReceiver->OnDataReceived().BindLambda([this](const FArrayReaderPtr& DataPtr, const FIPv4Endpoint& Endpoint)
	{
		UE_LOG(LogA2S, Display, TEXT("Received a UDP packet from %s"), *Endpoint.ToString())
		TArray<uint8> Data;
		Data.AddUninitialized(DataPtr->TotalSize());
		DataPtr->Serialize(Data.GetData(), DataPtr->TotalSize());

		this->HandleRequest(Data, Endpoint);
	});

	return true;
}

bool UA2SServer::OpenSendSocket()
{
	SenderSocket = FUdpSocketBuilder(FString(TEXT("ue4-a2s-send"))).AsReusable();

	SenderSocket->SetSendBufferSize(Settings.BufferSize, Settings.BufferSize);
	SenderSocket->SetReceiveBufferSize(Settings.BufferSize, Settings.BufferSize);
	return true;
}

bool UA2SServer::CloseReceiveSocket()
{
	delete UDPReceiver;
	UDPReceiver = nullptr;

	ReceiverSocket->Close();
	ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ReceiverSocket);
	ReceiverSocket = nullptr;

	return true;
}

bool UA2SServer::CloseSendSocket()
{
	SenderSocket->Close();
	ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(SenderSocket);
	SenderSocket = nullptr;

	return true;
}

void UA2SServer::HandleRequest(const TArray<uint8> Data, const FIPv4Endpoint& Endpoint)
{
	UE_LOG(LogA2S, Display, TEXT("Handling A2S request from %s"), *Endpoint.ToString())

	const FString SenderIP = Endpoint.Address.ToString();
	TSharedPtr<FInternetAddr> RemoteAddress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();

	bool bIsValid;
	RemoteAddress->SetIp(*SenderIP, bIsValid);
	RemoteAddress->SetPort(Endpoint.Port);
	if (!bIsValid)
	{
		UE_LOG(LogTemp, Error, TEXT("UDP address is invalid <%s:%d>"), *SenderIP, Endpoint.Port);
		return;
	}

	int32 todo; // TODO
	SenderSocket->SendTo(Data.GetData(), Data.GetAllocatedSize(), todo, *RemoteAddress.Get());
}
