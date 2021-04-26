## Basic zeuz Support
In order to make the game server exit when the match is over, we can adapt the [default game mode timer](../Source/ShooterGame/Private/Online/ShooterGameMode.cpp) which handles the transitions between the game's states.
To gracefully achieve this, the client should be sent back to the main menu before the server exits.

### Send Client Back to the Main Menu
```c++
// Source/ShooterGame/Private/Online/ShooterGameMode.cpp::DefaultTimer

if (GetMatchState() == MatchState::WaitingPostMatch)
{
    // Send the players back to the main menu
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        (*It)->ClientReturnToMainMenuWithTextReason(NSLOCTEXT("GameMessages", "MatchEnded", "The match has ended."));
        AShooterPlayerController* ShooterPlayerController = Cast<AShooterPlayerController>(*It);
        ShooterPlayerController->HandleReturnToMainMenu();
    }
    // ...
 }
```

Instead of the game server calling `RestartGame()` when it is transitioning out of its post-match state, we instead instruct clients to return to the main menu, using the RPC `ClientReturnToMainMenuWithTextReason`.
Note that this RPC implementation is the same as the existing `ShooterPlayerController` definition of `ClientReturnToMainMenu` with added message propagation (in fact the deprecated `ClientReturnToMainMenu` method was updated with the new `ClientReturnToMainMenuWithTextReason` method definition).

### Exit the Process
```c++
// Source/ShooterGame/Private/Online/ShooterGameMode.cpp::DefaultTimer

// ...
else if (GetMatchState() == MatchState::WaitingPostMatch)
{
    // The post match is over (no time left) so wait for clients to be disconnected 
    if (GetNumPlayers() == 0)
    {
        // All players have disconnected, exit the server
        FGenericPlatformMisc::RequestExit(false);
    }
}
```

Once the game server has instructed the clients to return to the main menu, we then wait for all of the players to disconnect.
This *waiting* also occurs in the `DefaultTimer` method, as once we instruct players to return to the main menu (and therefore disconnect), we do not transition the game state, staying in post-match.
Subsequent calls to `DefaultTimer` then enter the case in the snippet above, where the number of connected players is checked and the game server is exited if there are no connected players.

### Other Changes
Besides the behavioural changes above, the on-screen messaging was changed to reflect the new player flow.
Instead of *'Starting new match...'* displaying during the countdown on the post-match scoreboard, the text now reads *'Match ending...'*.
Method names were also updated to reflect the change.

This change was made in the [scoreboard widget](../Source/ShooterGame/Private/UI/Widgets/SShooterScoreboardWidget.cpp) (see method `SShooterScoreboardWidget::GetMatchEndText`, which was previously `SShooterScoreboardWidget::GetMatchRestartText`).