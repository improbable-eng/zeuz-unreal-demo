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

### The Matchmaker
The matchmaker expects periodic updates from the game server indicating that it is accepting client connections ('ready').
If it does not receive any updates from a game server after a set interval, it marks the game server as 'not ready' and no longer routes clients to it.
Clients can query the matchmaker for available game servers.

Any matchmaker used alongside zeuz and this game server needs to support two operations:
- `GET /`: Request a game server IP and port.
    - The matchmaker uses zeuz to find and, if necessary, reserve a payload for the player to connect to.
        - The body for the `GET` response is as follows (filled with example data):
      ```json
      {
          "IP": "123.45.67.89",
          "Port": 29000
      }
      ```
- `POST /<PAYLOAD ID>`: Add a payload to the collection of ready game servers.
    - The body for the `POST` request is as follows (filled with example data):
    ```json
    {
        "Ccu": 12,
        "IP": "123.45.67.89",
        "Port": 29000
    }
    ```

The REST operations that the matchmaker must support can be made available on the same or two different endpoints.
To specify a `GET` endpoint, for the game client to request a game server to play on, edit the `MatchmakerEndpoint` entry in [DefaultGame.ini](../Config/DefaultGame.ini) (e.g. `MatchmakerEndpoint="http://123.45.67.89:9000/servers"`).
To specify the `POST` endpoint, for the game server to advertise its readiness, supply a command line argument for `-matchmakerAddr` when starting the server (e.g. `-matchmakerAddr http://123.45.67.89:9000`).

### Dependencies
This component requires the `HTTP` and `Json` packages.
There were added to [ShooterGame.Build.cs](../Source/ShooterGame/ShooterGame.Build.cs).
