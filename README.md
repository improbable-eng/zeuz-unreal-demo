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