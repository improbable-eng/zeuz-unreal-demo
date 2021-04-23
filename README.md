# zeuz-unreal-demo
An adaptation of the Unreal Engine [ShooterGame](https://docs.unrealengine.com/en-US/Resources/SampleGames/ShooterGame/index.html) example game project to demonstrate how to support [_zeuz_](https://zeuz.io/) orchestration.


## Before You Read
Before you follow this example, there are a couple of general points to note: 
- Unreal handles its internal command line arguments differently to custom ones. Read the payload commands in each section carefully for the correct use.
    - Internal flag values are assigned to a flag with an `=` between the flag key and value (e.g. `-PORT=29000`).
    - Custom flag values are assigned to a flag with a space between the flag key and value (e.g. `-payloadId my-payload-id-123`).
 - The menu of the original ShooterGame project has been modified to reflect the connections available when this game is used with zeuz and a matchmaker. For more information, see the [UI Changes section](#ui-changes).
    - Note: These UI changes are **not** necessary to make your game support zeuz orchestration.


## Base Game Hosting
Without any changes, the base ShooterGame project can be built and published to zeuz. The payload command for this is:
```
/opt/zeuz/bin/payloadrunner
run
binaryactivename=ShooterServer
binaryexecpath=/opt/zeuz/gameserver/ShooterServer.sh
execargs=/Game/Maps/Highrise -log -NOSTEAM -PORT=${servicePort:PortName}
```
The execution flags specify:
- The map to launch (`/Game/Maps/Highrise`).
- Enable logging (`-log`).
- Disable Steam (`-NOSTEAM`).
- The port to host on (`-PORT=${servicePort:PortName}`).
    - This makes use of the port variable zeuz makes available to you.


## Basic zeuz Support
[Code changes.](https://github.com/improbable/zeuz-unreal-demo/pull/3)

To support [automatic payload release](https://doc.zeuz.io/docs/payload-definition#automatic-payload-release), your game server executable should terminate at the end of the match. 
For this project, this is controlled in the [game mode (`ShooterGameMode`)](Source/ShooterGame/Private/Online/ShooterGameMode.cpp) by sending an RPC to clients to return to the main menu and then exiting the game server when all clients are disconnected (see [`ShooterGameMode::DefaultTimer`](Source/ShooterGame/Private/Online/ShooterGameMode.cpp)).

Once the code changes are implemented, the payload command needs to be updated with an API key, so that the payload can automatically be unreserved:
```
/opt/zeuz/bin/payloadrunner
run
binaryactivename=ShooterServer
binaryexecpath=/opt/zeuz/gameserver/ShooterServer.sh
execargs=/Game/Maps/Highrise -log -NOSTEAM -PORT=${servicePort:PortName}
apikey=<API_KEY>
apisecret=<API_KEY_PASSWORD>
apiendpoint=https://zcp.zeuz.io/api/v1
```


## CCU Tracking (A2S Protocol)
[Code changes.](https://github.com/improbable/zeuz-unreal-demo/pull/4)

With zeuz, you can use [CCU tracking](https://doc.zeuz.io/docs/ccu-tracking) to view how many concurrent players are connected to your game servers.
When creating or editing an allocation, you can select various CCU tracking options at the end of the 'Payload Definition' section.
If you don't see this, speak to your zeuz account manager.

To support CCU tracking, the game server must implement the [A2S protocol](https://developer.valvesoftware.com/wiki/Server_queries).
For this, an [`A2SServer` component](Source/ShooterGame/Private/Online/A2S/A2SServer.h) is created which utilises Unreal's `UDPSocket` API to listen to and respond to A2S queries.
This example only supports `A2S_INFO` queries, as is required by zeuz CCU tracking, but you may wish to support further A2S features for your own purposes.

This `A2SServer` component is created by the [game mode (`ShooterGameMode`)](Source/ShooterGame/Private/Online/ShooterGameMode.cpp) and has its lifetime controlled by the `BeginPlay/EndPlay` events, since the game mode exists only on the server.

Lastly, the payload command of the allocation needs updating to ensure that the game server responds to A2S queries on the port it expects. 
The `${statsPort}` variable is used in the payload command to specify this:
```
/opt/zeuz/bin/payloadrunner
run
binaryactivename=ShooterServer
binaryexecpath=/opt/zeuz/gameserver/ShooterServer.sh
execargs=/Game/Maps/Highrise -log -NOSTEAM -PORT=${servicePort:PortName} -statsPort ${statsPort}
apikey=<API_KEY>
apisecret=<API_KEY_PASSWORD>
apiendpoint=https://zcp.zeuz.io/api/v1
payloadId=${payloadID}
```
In addition to `${statsPort}` being used in the `execargs`, the payload ID (accessible through `${payloadId}`) is used as the server name.
This is not crucial for zeuz to track CCUs, but useful if you wish to track CCUs per payload for your own queries.


## Server Readiness
[Code changes.](https://github.com/improbable/zeuz-unreal-demo/pull/5)

A payload being ready doesn't mean that the game server running on it is ready to accept connections from players.
To overcome this, the game server has a [`Discoverability` component](Source/ShooterGame/Private/Online/Discoverability/Discoverability.h) which `POST`s to a matchmaker service ([description below](#the-matchmaker)) to indicate that it is ready to accept connections.

Similarly to the [A2S server](#ccu-tracking-a2s-protocol), this component is created by the [game mode (`ShooterGameMode`)](Source/ShooterGame/Private/Online/ShooterGameMode.cpp) and is started during the `BeginPlay` event.
However, when the match state is `WaitingPostMatch`, we do not wish to have any more players connect to the game server, so the updates are stopped (see [`ShooterGameMode::DefaultTimer`](Source/ShooterGame/Private/Online/ShooterGameMode.cpp)).
There will be a short time between the last update sent to the matchmaker and the matchmaker labelling the game server as 'not ready', in which the matchmaker may still route clients to the game server.
If a player connects in this case, they will see the end of game scoreboard and then disconnect gracefully, like any other player. 

The matchmaker endpoint, along with the payload ID and IP, is specified to the game server in the payload command, so it can interact with the matchmaker:
```
/opt/zeuz/bin/payloadrunner
run
binaryactivename=ShooterServer
binaryexecpath=/opt/zeuz/gameserver/ShooterServer.sh
execargs=/Game/Maps/Highrise -log -NOSTEAM -PORT=${servicePort:PortName} -statsPort ${statsPort} -payloadIp ${serviceIP} -payloadId ${payloadID} -matchmakerAddr "http://123.45.67.89:9000"
apikey=<API_KEY>
apisecret=<API_KEY_PASSWORD>
apiendpoint=https://zcp.zeuz.io/api/v1
```

### The Matchmaker
Any matchmaker used alongside zeuz and this game server needs to support two endpoints:
- `GET /v1/gameservers`: Request a game server IP and port.
    - The matchmaker uses zeuz to find and, if necessary, reserve a payload for the player to connect to. 
      - The body for the `GET` response is as follows (filled with example data):
      ```json
      {
        "IP": "123.45.67.89",
        "Port": 29000
      }
      ```
- `POST /v1/gameservers/<PAYLOAD ID>`: Add a payload to the collection of ready game servers.
    - The body for the `POST` request is as follows (filled with example data):
    ```json
    {
      "Ccu": 12,
      "IP": "123.45.67.89",
      "Port": 29000
    }
    ```

The matchmaker expects periodic updates from the game server indicating that it is accepting client connections ('ready').
If it does not receive any updates from a game server after a set interval, it marks the game server as 'not ready' and no longer routes clients to it.
Clients can query the matchmaker for available game servers.

The matchmaker is also used to allow players to find a server to play on, using the `GET /v1/gameservers` endpoint, see the [UI Changes section](#ui-changes).

For this example, the matchmaker used can be found [here](https://github.com/improbable/zeuz-demo).


## UI Changes
Whilst not necessary for supporting zeuz orchestration, the base ShooterGame example UI has been modified to allow players to connect to zeuz-hosted game servers.
These options are 'JOIN' and 'DIRECT CONNECT'.
Unsupported options for 'HOST', 'LEADERBOARDS', 'ONLINE STORE' and 'DEMOS' have been removed from the menu, but their source code still exists.

### JOIN
If a matchmaker is set up (see [The Matchmaker](#the-matchmaker)), clicking 'JOIN' will query the matchmaker for a game server and connect to it.
The endpoint of the matchmaker is specified in [DefaultGame.ini](Config/DefaultGame.ini).
If no endpoint is specified, the 'JOIN' button will not render.

A simple button is added to the [`MainMenu` component](Source/ShooterGame/Private/UI/Menu/ShooterMainMenu.cpp) and a HTTP GET request (see [FShooterMainMenu::OnJoinClicked](Source/ShooterGame/Private/UI/Menu/ShooterMainMenu.cpp)) is made to the matchmaker when it is clicked.

### DIRECT CONNECT
If you know the IP and port of the game server you wish to connect to, you can enter it in the text box that shows when 'DIRECT CONNECT' is chosen from the main menu.

This is a new [`DirectConnect` widget](Source/ShooterGame/Private/UI/Menu/Widgets/SShooterDirectConnect.cpp) that is added to the [main menu](Source/ShooterGame/Private/UI/Menu/ShooterMainMenu.cpp).
