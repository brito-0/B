// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BBossAttack.generated.h"

UCLASS()
class B_API ABBossAttack : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABBossAttack();

	/**
		 * Constructor for the attacks
		 * @param ID value of AttackID
		 * @param BP value of bCanBeParried
		 * @param AD value of AttackDamage
		 * @param AS value of AttackSize
		 * @param AO value of AttackOffset
		 * @param C value of bIsCombo
		 * @param CFID value of ComboFollowingID
		 * @param LC value of bIsLastComboAttack
		 * @param DR value of DelayRate
		 */
	void InitializeBBossAttack(uint16 ID, bool BP = false, float AD = .0f, float AS = 200.f, float AO = 250.f, bool C = false, uint16 CFID = 0, bool LC = false, float DR = .8f);
	
	uint16 GetAttackId() const { return AttackId; }
	
	bool GetCanBeParried() const { return  bCanBeParried; }
	
	void SetCanBeParriedFalse() { bCanBeParried = false; }

	float GetAttackDamage() const { return AttackDamage; }

	float  GetAttackSize() const { return AttackSize; }
	float  GetAttackOffset() const { return AttackOffset; }

	bool GetIsCombo() const { return bIsCombo; }
	uint16 GetComboFollowingID() const { return ComboFollowingID; }
	bool GetIsLastComboAttack() const { return bIsLastComboAttack; }

	float GetDelayRate() const { return DelayRate; }

private:
	UPROPERTY()
	uint16 AttackId;

	UPROPERTY()
	bool bCanBeParried = false;

	UPROPERTY()
	float AttackDamage;

	UPROPERTY()
	float AttackSize;

	UPROPERTY()
	float AttackOffset;

	UPROPERTY()
	bool bIsCombo = false;

	UPROPERTY()
	uint16 ComboFollowingID;

	UPROPERTY()
	bool bIsLastComboAttack = false;

	UPROPERTY()
	float DelayRate;
	
};
