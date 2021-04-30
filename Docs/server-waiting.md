## Server waiting
We wish to make the server wait until a player has joined before the pre-match countdown begins.

The first change is to track whether or not a player has joined yet (in the [`ShooterGameMode` class](../Source/ShooterGame/Public/Online/ShooterGameMode.h)) and update it when a player connects.
```c++
// Source/ShooterGame/Public/Online/ShooterGameMode.h

class AShooterGameMode : public AGameMode
{
    // ...
    /** Tracks if a game has joined the server at least once */
    bool bHasPlayerConnected = false;
    // ...
}
```

```c++
// Source/ShooterGame/Public/Online/ShooterGameMode.cpp

void AShooterGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

	bHasPlayerConnected = true;
	// ...
}
```

Lastly, we can use this variable in the timer, to specify whether to decrement the timer, or not.
```c++
// Source/ShooterGame/Public/Online/ShooterGameMode.cpp

void AShooterGameMode::DefaultTimer()
{
    // ...
    if (MyGameState && MyGameState->RemainingTime > 0 && !MyGameState->bTimerPaused)
	{
		// If we are waiting for match to start (MatchState::WaitingToStart) and there has not yet been a connected
		// player, do not decrement the counter
		if (GetMatchState() == MatchState::WaitingToStart && !bHasPlayerConnected)
		{
			return;
		}

		MyGameState->RemainingTime--;
    // ...
    }
    // ...
}
```
