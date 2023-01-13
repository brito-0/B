// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BGameModeBase.generated.h"

class UUserWidget;

/**
 * 
 */
UCLASS()
class B_API ABGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	ABGameModeBase();
	
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable)
	FString GetFinalGameText() const { return FinalGameText; }
	UFUNCTION(BlueprintCallable)
	FString GetFinalGameTime() const { return FinalGameTime; }

	/** shows the defeat widget */
	void  GameEndLoss();

	/** show the victory widget */
	void GameEndWin();
	
protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere)
	float GameTimeSeconds = 0.f;
	UPROPERTY(VisibleAnywhere)
    uint16 GameTimeMinutes = 0.f;

	UPROPERTY(VisibleAnywhere)
	FString FinalGameText;
	
	UPROPERTY(VisibleAnywhere)
	FString FinalGameTime;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> FinalWidget;
	UPROPERTY()
	UUserWidget* FinalMenu;
	
	void CreateFinalWidget();

	UPROPERTY(VisibleAnywhere)
	FString EndTime = "";
	
};
