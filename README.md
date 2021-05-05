# zeuz-unreal-demo
An adaptation of the Unreal Engine [ShooterGame](https://docs.unrealengine.com/en-US/Resources/SampleGames/ShooterGame/index.html) example game project to demonstrate how to support [_zeuz_](https://zeuz.io/) orchestration.

For information on uploading and hosting game servers on zeuz, please see [doc.zeuz.io](https://doc.zeuz.io/).


## Before getting started
Each section addresses a different behaviour missing from the unmodified [ShooterGame](https://docs.unrealengine.com/en-US/Resources/SampleGames/ShooterGame/index.html) project.
You should use this project as an example to read the changes you need to make to your game to better support zeuz orchestration.

The payload commands in each section show only the fields required to enable the section's behaviour.
To enable all of the behaviours, add a matchmaker endpoint (`MatchmakerEndpoint`) to [DefaultGame.ini](Config/DefaultGame.ini) and use the following payload command:
```
/opt/zeuz/bin/payloadrunner
run
binaryactivename=ShooterServer
binaryexecpath=/opt/zeuz/gameserver/ShooterServer.sh
execargs=/Game/Maps/Highrise -log -NOSTEAM -PORT=${servicePort:PortName} -statsPort ${statsPort} -payloadIp ${serviceIP} -payloadId ${payloadID} -matchmakerAddr <MATCHMAKER ENDPOINT>
apikey=<API_KEY>
apisecret=<API_KEY_PASSWORD>
apiendpoint=https://zcp.zeuz.io/api/v1
```

Before you follow this example, there are a couple of general points to note:
- The '**Files changed**' list for each section only notes the most significant changes.
    - For example, the files changed to edit the on-screen messaging text at the end of the game in ['Basic zeuz support'](#basic-zeuz-support) are omitted.
- The menu of the original ShooterGame project has been modified to reflect the connections available when this game is used with zeuz and a matchmaker. For more information, see the [UI Changes section](#ui-changes).
    - Note: These UI changes are **not** necessary to make your game support zeuz orchestration.

Lastly, if you wish to follow along, making similar changes to the [ShooterGame](https://docs.unrealengine.com/en-US/Resources/SampleGames/ShooterGame/index.html) project, you should ensure you are able to package the unmodified game as a dedicated server and connect with a client.
There is an [Unreal Engine tutorial](https://docs.unrealengine.com/en-US/InteractiveExperiences/Networking/HowTo/DedicatedServers/index.html) for this.


## Base game hosting
Without any changes, the unmodified [ShooterGame project](https://docs.unrealengine.com/en-US/Resources/SampleGames/ShooterGame/index.html) can be built and published to zeuz. The payload command for this is:
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

Whilst this is a functional game hosted on zeuz, there are a few problems which are addressed in this project:
- The server keeps restarting matches.
    - See [Basic zeuz support](#basic-zeuz-support).
- zeuz cannot read the number of players connected to the server.
    - See [CCU tracking (A2S protocol)](#ccu-tracking-a2s-protocol).
- Since the payload image is started whenever a payload is ready (and unreserved), matches can start for unreserved payloads.
    - See [Server waiting](#server-waiting).


## Basic zeuz support
> **For a full description of the changes with code snippets, see the [full docs](Docs/basic-zeuz-support.md) of this change.**

To support [automatic payload release](https://doc.zeuz.io/docs/payload-definition#automatic-payload-release), your game server executable should terminate at the end of the match.

**Files changed:** [ShooterGameMode.cpp](Source/ShooterGame/Private/Online/ShooterGameMode.cpp)

Server lifetime controlled in the [game mode (`ShooterGameMode`)](Source/ShooterGame/Private/Online/ShooterGameMode.cpp) by sending an RPC to clients to return to the main menu and then exiting the game server when all clients are disconnected (see [`ShooterGameMode::DefaultTimer`](Source/ShooterGame/Private/Online/ShooterGameMode.cpp)).

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


## CCU tracking (A2S protocol)
> **For a full description of the changes with code snippets, see the [full docs](Docs/ccu-tracking.md) of this change.**

With zeuz, you can use [CCU tracking](https://doc.zeuz.io/docs/ccu-tracking) to view how many concurrent players are connected to your game servers.
When creating or editing an allocation, you can select various CCU tracking options at the end of the 'Payload Definition' section.
If you don't see this, speak to your zeuz account manager.

**Files changed:** [A2SServer.h](Source/ShooterGame/Private/Online/A2S/A2SServer.h),
[A2SServer.cpp](Source/ShooterGame/Private/Online/A2S/A2SServer.cpp),
[A2SServerSettings.h](Source/ShooterGame/Private/Online/A2S/A2SServerSettings.h),
[ShooterGameMode.h](Source/ShooterGame/Public/Online/ShooterGameMode.h),
[ShooterGameMode.cpp](Source/ShooterGame/Private/Online/ShooterGameMode.cpp),
[ShooterGame.Build.cs](Source/ShooterGame/ShooterGame.Build.cs)

To support CCU tracking, the game server must implement the [A2S (Any to Server) protocol](https://developer.valvesoftware.com/wiki/Server_queries).
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
execargs=/Game/Maps/Highrise -log -NOSTEAM -PORT=${servicePort:PortName} -statsPort ${statsPort} -payloadId ${payloadID}
```
In addition to `${statsPort}` being used in the `execargs`, the payload ID (accessible through `${payloadId}`) is used as the server name.
This is not crucial for zeuz to track CCUs, but useful if you wish to track CCUs per payload for your own queries.


## Server waiting
> **For a full description of the changes with code snippets, see the [full docs](Docs/server-waiting.md) of this change.**

When a zeuz payload starts, the image it is launched with is started straight away, meaning that the game server starts executing before the payload is reserved.

**Files changed:** [ShooterGameMode.h](Source/ShooterGame/Public/Online/ShooterGameMode.h),
[ShooterGameMode.cpp](Source/ShooterGame/Private/Online/ShooterGameMode.cpp)

A ShooterGame server moves through three phases ('pre-match', 'in-match', 'post-match') after specified intervals of time.
This can cause problems if a payload spends a long amount of time as unreserved before players begin to connect to it as the server may not be in its pre-match phase when a player connects.

To overcome this, the timer of the game (see [`ShooterGameMode::DefaultTimer`](Source/ShooterGame/Private/Online/ShooterGameMode.cpp)) doesn't count down when the match is waiting to start **and** there has not yet been a connected player.
After the first player connects, the timer will count down to the start of the game.


## UI changes
Whilst not necessary for supporting zeuz orchestration, the base ShooterGame example UI has been modified to allow players to connect to zeuz-hosted game servers.

**Files changed:** [DefaultGame.ini](Config/DefaultGame.ini),
[ShooterGameInstance.h](Source/ShooterGame/Public/ShooterGameInstance.h),
[ShooterGameInstance.cpp](Source/ShooterGame/Private/ShooterGameInstance.cpp),
[ShooterMainMenu.cpp](Source/ShooterGame/Private/UI/Menu/ShooterMainMenu.cpp),
[SShooterDirectConnect.cpp](Source/ShooterGame/Private/UI/Menu/Widgets/SShooterDirectConnect.cpp),
[SShooterDirectConnect.h](Source/ShooterGame/Private/UI/Menu/Widgets/SShooterDirectConnect.h)

The option for 'DIRECT CONNECT' allows players to enter the address of the game server they wish to connect to.
Unsupported options for 'HOST', 'LEADERBOARDS', 'ONLINE STORE' and 'DEMOS' have been removed from the menu, but their source code still exists.

This is a new [`DirectConnect` widget](Source/ShooterGame/Private/UI/Menu/Widgets/SShooterDirectConnect.cpp) that is added to the [main menu](Source/ShooterGame/Private/UI/Menu/ShooterMainMenu.cpp).
