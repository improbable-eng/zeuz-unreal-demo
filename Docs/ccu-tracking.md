## CCU tracking (A2S protocol)
To support the A2S (any to server) protocol, an A2S server component is dropped in.
This keeps changes simple and makes the component much more manageable.

The component also has settings, configurable from the Unreal Engine editor or the command line (command line options prioritised).

### A2S server lifecycle
The A2S server is created and started during the game mode's `BeginPlay` event, since we only wish to serve A2S queries when the game has started.
The server component is then added to the root, so that it isn't garbage collected.
```c++
// Source/ShooterGame/Private/Online/ShooterGameMode.cpp

void AShooterGameMode::BeginPlay()
{
	// ...
	A2SServer = NewObject<UA2SServer>(this);
	A2SServer->AddToRoot();
	A2SServer->Settings = Settings;
	A2SServer->Start();
}
```

The settings assigned to the A2S server are the [A2S settings](../Source/ShooterGame/Private/Online/A2S/A2SServerSettings.h) which can be configured in the Unreal editor.
Following this assignment, in `A2SServer::Start()`, the command line flags are checked for any settings overwrites and applied if there are any.
This is to make the server configurable at runtime.

On the other hand, the A2S server is stopped and made eligible for garbage collection in the `EndPlay` event of the game mode.
```c++
// Source/ShooterGame/Private/Online/ShooterGameMode.cpp

void AShooterGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	A2SServer->Stop();
	A2SServer->RemoveFromRoot();
	// ...
}
```

### Starting the server
The [A2S server](../Source/ShooterGame/Private/Online/A2S/A2SServer.cpp) uses Unreal's UDP socket API to build a send (see `UA2SServer::OpenSendSocket`) and receive (see `UA2SServer::OpenReceiveSocket`) socket which use the same port.
It is advisable that the same port is used for both sending and receiving as some implementations of A2S clients expect responses from the same address they make queries to.

### Stopping the server
Unreal's UDP socket API also offers ways to close and destroy sockets, which we make use of.
It's also important to note that a mutex is used, to ensure that the socket isn't closed before a query is handled (i.e. a response sent).

### Building a response
Building an `A2S_INFO` response is low level and requires byte-level operations.
A fully documented example response is available in `UA2SServer::BuildA2SInfoResponse` but it is strongly advised that you read [the protocol documentation](https://developer.valvesoftware.com/wiki/Server_queries) when implementing your own response.

### Gotcha!
Unreal's internal representation of strings offsets UTF-8 encoding by +1, as *"this keeps anything from being put into the string as a null terminator"* (see `UnrealString.h` in your Unreal Engine source build).
This causes problems when converting strings to bytes in A2S responses, so be sure to implement your own `BytesToString` equivalent.
For this purpose, the `AddStringToByteArray` utility function, which adds a string to an existing byte array, is defined.
```c++
// Source/ShooterGame/Private/Online/A2S/A2SServer.cpp

void AddStringToByteArray(TArray<uint8>& Bytes, FString InString)
{
	// Encode string as UTF-8 (default encoding is UTF-16)
	const FTCHARToUTF8 InStringUtf8(*InString);

	// Copy string into byte buffer
	// Note: Unreal's 'StringToBytes' method doesn't undo the +1 offset of its internal UTF-8 encoding so write our own
	// conversion
	int32 NumBytes = 0;
	const ANSICHAR* CharPos = InStringUtf8.Get();

	while (*CharPos && NumBytes < InStringUtf8.Length())
	{
		Bytes.Add(static_cast<uint8>(*CharPos));
		CharPos++;
		NumBytes++;;
	}

	// Add string terminating byte
	Bytes.Add(0x00);
}
```

### Dependencies
This component requires the `Networking` and `Sockets` packages.
There were added to [ShooterGame.Build.cs](../Source/ShooterGame/ShooterGame.Build.cs).
