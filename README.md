# zeuz-unreal-demo
An adaptation of the Unreal Engine [ShooterGame](https://docs.unrealengine.com/en-US/Resources/SampleGames/ShooterGame/index.html) example game project to demonstrate how to support [_zeuz_](https://zeuz.io/) orchestration.

## Base Game Hosting
Without any changes, the base ShooterGame project can be built and published to zeuz. The payload command for this is:
```
/opt/zeuz/bin/payloadrunner
run
binaryactivename=ShooterServer
binaryexecpath=/opt/zeuz/gameserver/ShooterServer.sh
execargs=/Game/Maps/Highrise -log -PORT=${servicePort:PortName} -NOSTEAM
```

## Basic zeuz Support
[Code changes.](https://github.com/improbable/zeuz-unreal-demo/pull/3)

To support [automatic payload release](https://doc.zeuz.io/docs/payload-definition#automatic-payload-release), your game server executable should terminate at the end of the match. 
For this project, this is controlled in the [game mode (`ShooterGameMode`)](Source/ShooterGame/Private/Online/ShooterGameMode.cpp) by sending an RPC to clients to return to the main menu and then exiting the game server when all clients are disconnected (see `ShooterGameMode::DefaultTimer`).

Once the code changes are implemented, the payload command needs to be updated with an API key, so that the payload can automatically be unreserved:
```
/opt/zeuz/bin/payloadrunner
run
binaryactivename=ShooterServer
binaryexecpath=/opt/zeuz/gameserver/ShooterServer.sh
execargs=/Game/Maps/Highrise -log -PORT=${servicePort:PortName} -NOSTEAM
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
execargs=/Game/Maps/Highrise -log -PORT=${servicePort:PortName} -NOSTEAM -statsPort=${statsPort}
apikey=<API_KEY>
apisecret=<API_KEY_PASSWORD>
apiendpoint=https://zcp.zeuz.io/api/v1
payloadId=${payloadID}
```
In addition to `${statsPort}` being used in the `execargs`, the payload ID (accessible through `${payloadId}`) is used as the server name.
This is not crucial for zeuz to track CCUs, but useful if you wish to track CCUs per payload for your own queries.
