// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "SlateBasics.h"
#include "SlateExtras.h"
#include "CoreMinimal.h"

class LANTestWidget : public SBorder
{
	SLATE_BEGIN_ARGS(LANTestWidget)
	{}

	SLATE_ARGUMENT(TWeakObjectPtr<APlayerController>, PCOwner)

	SLATE_ATTRIBUTE(EShooterMatchState::Type, MatchState)

	SLATE_END_ARGS()

	/** needed for every widget */
	void Construct(const FArguments& InArgs);

protected:

	/** scoreboard tint color */
	FLinearColor ScoreboardTint;

	/** width of scoreboard item */
	int32 ScoreBoxWidth;

	/** scoreboard count up time */
	float ScoreCountUpTime;

	/** when the scoreboard was brought up. */
	double ScoreboardStartTime;

	/** pointer to our parent HUD */
	TWeakObjectPtr<class APlayerController> PCOwner;

	/** style for the scoreboard */
	const struct FShooterScoreboardStyle* ScoreboardStyle;
};