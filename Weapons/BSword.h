// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BSword.generated.h"

class ABCharacter3rd;
class USceneComponent;
class USkeletalMeshComponent;

UCLASS()
class B_API ABSword : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABSword();

	/** checks for collisions and deals damage to any enemies hit */
	void SwordAttack();

	/** checks for collision and, returns true if an attack is blocked or false if it isn't */
	void SwordParry();

	/** changes the value of AttackIndex to zero */
	void ResetAttackIndex() {AttackIndex = 0;}
	
	UFUNCTION()
	float GetAttackIndex() const { return AttackIndex; }

	bool GetIsEmpowered() const { return bIsEmpowered; }

	/** changes the sword material and sets bIsEmpowered to true */
	void EmpowerSword();
	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere)
	uint8 AttackIndex = 0;
	
	float AttackSize = 85.f;
	float AttackOffset = 125.f;
	float AttackPowerBase = 25.f;

	// float with the final value of attack power
	UPROPERTY(VisibleAnywhere)
	float AttackPowerFinal;
	
	float ParrySize = 75.f;
	float ParryOffset = 125.f;

	UFUNCTION()
	void SwordAttackDelay();
	
	UPROPERTY()
	ABCharacter3rd* BCharacter;

	UPROPERTY()
	UMaterial* SwordNormalMaterial;
	UPROPERTY()
	UMaterial* SwordEmpoweredMaterial;
	
	UPROPERTY(VisibleAnywhere, Category = Empower)
	uint8 EmpoweredAttackCounter = 0;
	
	UPROPERTY(VisibleAnywhere, Category = Empower)
	bool bIsEmpowered = false;

	/** changes the sword material and sets bIsEmpowered to false */
	void UnEmpowerSword();
	
};
