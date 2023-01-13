// Copyright Epic Games, Inc. All Rights Reserved.


#include "BGameModeBase.h"

#include "Engine/RendererSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"

ABGameModeBase::ABGameModeBase()
{
	PrimaryActorTick.bCanEverTick = true;

}


void ABGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	UGameplayStatics::GetPlayerController(GetWorld(), 0)->SetInputMode(FInputModeGameOnly());

	SetActorTickInterval(.1f);
}

void ABGameModeBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!UGameplayStatics::IsGamePaused(GetWorld()))
	{
		if (GameTimeSeconds < 60.f)
		{
			GameTimeSeconds += .1f;
			if (GameTimeSeconds >= 60.f)
			{
				++GameTimeMinutes;
				GameTimeSeconds = 0.f;
			}
		}
	}
}

void ABGameModeBase::GameEndLoss()
{
	SetActorTickEnabled(false);

	FinalGameText.Append("Defeat");
	
	FinalGameTime.Append("");

	FTimerHandle WidgetHandle;
	GetWorldTimerManager().SetTimer(WidgetHandle, this, &ABGameModeBase::CreateFinalWidget, .5, false);
}

void ABGameModeBase::GameEndWin()
{
	SetActorTickEnabled(false);

	FinalGameText.Append("Victory");
	
	FinalGameTime.AppendInt(GameTimeMinutes);
	FinalGameTime.Append(":");
	FinalGameTime.Append(FString::SanitizeFloat(GameTimeSeconds, 0));
	
	FTimerHandle WidgetHandle;
	GetWorldTimerManager().SetTimer(WidgetHandle, this, &ABGameModeBase::CreateFinalWidget, .5, false);
}

void ABGameModeBase::CreateFinalWidget()
{
	FinalMenu = CreateWidget<UUserWidget>(GetWorld(),  FinalWidget);

	FinalMenu->AddToViewport();

	UGameplayStatics::GetPlayerController(GetWorld(), 0)->SetInputMode(FInputModeUIOnly());
	UGameplayStatics::GetPlayerController(GetWorld(), 0)->SetShowMouseCursor(true);
}
