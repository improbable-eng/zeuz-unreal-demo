// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterGame.h"
#include "SShooterScoreboardWidget.h"
#include "ShooterStyle.h"
#include "ShooterScoreboardWidgetStyle.h"
#include "Widgets/ShooterMenuItem.h"
#include "UI/Widgets/LANTestWidget.h"

void LANTestWidget::Construct(const FArguments& InArgs)
{
	ScoreboardStyle = &FShooterStyle::Get().GetWidgetStyle<FShooterScoreboardStyle>("DefaultShooterScoreboardStyle");

	PCOwner = InArgs._PCOwner;
	ScoreboardTint = FLinearColor(0.0f, 0.0f, 0.0f, 0.4f);
	ScoreBoxWidth = 140.0f;
	ScoreCountUpTime = 2.0f;

	ScoreboardStartTime = FPlatformTime::Seconds();

	// add button.

	TSharedPtr<FShooterMenuItem> RootMenuItem;
	
}