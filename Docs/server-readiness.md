## Server Readiness
To notify a matchmaker when the server is ready to accept player connections, a Discoverability component is dropped into the server.
This keeps changes simple and makes the component much more manageable.

The component also has settings, configurable from the Unreal Engine editor or the command line (command line options prioritised).

### Discoverability Component Lifecycle
The Discoverability component is created and started during the game mode's `BeginPlay` event, since we only wish to have players connect when the game has started (the first state is pre-match).
The component is then added to the root, so that it isn't garbage collected.
```c++
// Source/ShooterGame/Private/Online/ShooterGameMode.cpp

void AShooterGameMode::BeginPlay()
{
	//...
	Discoverability = NewObject<UDiscoverability>(this);
	Discoverability->AddToRoot();
	Discoverability->Settings = DiscoverabilitySettings;
	Discoverability->Start();
}
```

When the match is over and the server enters the post-match state, then the game server does not wish to accept new client connections, so updates to the matchmaker are stopped. 
```c++
// Source/ShooterGame/Private/Online/ShooterGameMode.cpp::DefaultTimer

if (GetMatchState() == MatchState::WaitingPostMatch)
{
       // ...
       Discoverability->Stop();
 }
```

The component is removed from the root in the `EndPlay` game mode event so that it can be garbage collected.
```c++
// Source/ShooterGame/Private/Online/ShooterGameMode.cpp

void AShooterGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// ...
	Discoverability->RemoveFromRoot();
    // ...
}
```

### Notifying the Matchmaker
To advertise the game server's availability, it `POST`s its CCU (concurrent users) to the matchmaker.
```json
{
    "Ccu": 12,
    "IP": "123.45.67.89",
    "Port": 9000
}
```

On receiving this request, the matchmaker notes the game server as 'ready' and begins to route clients to it, until it is full (therefore, the matchmaker must know what the maximum player count is).
When the matchmaker does not receive an update from a server after a specified timeout, it marks the game server as 'not ready' and no longer routes clients to it.

### Requesting a Game Server
Clients can use the 'JOIN' button to request a game server from the matchmaker and connect to the game.
The request is a `GET` and the response body contains the connection information.
```json
{
    "IP": "123.45.67.89",
    "Port": 9000
}
```

### Dependencies
This component requires the `HTTP` and `Json` packages.
There were added to [ShooterGame.Build.cs](../Source/ShooterGame/ShooterGame.Build.cs).
