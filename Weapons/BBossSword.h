// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BBossWeapon.h"
#include "BBossSword.generated.h"

class BBossWeapon;
class ABBossAttack;

UCLASS()
class B_API ABBossSword : public AActor, public BBossWeapon
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABBossSword();

	virtual void SelectAttack(uint16 AttackID) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void WeaponAttack() override;

	virtual void WeaponAttackDelay() override;

private:
	FTimerHandle ParryWindowSwordTrueHandle;
	FTimerHandle ParryWindowSwordFalseHandle;
	FTimerHandle SwordAttackHandle;
	FTimerHandle SwordRestHandle;

	UPROPERTY(VisibleAnywhere)
	float SwordAttackDelay;

	/** an array with the attacks the boss can use */
	UPROPERTY(VisibleAnywhere, Category = Attacks)
	TArray<ABBossAttack*> SwordAttacks;
	/** array to store following attacks from a combo */
	UPROPERTY(VisibleAnywhere, Category = Attacks)
	TArray<ABBossAttack*> SwordComboFollowingAttacks;
	
};
