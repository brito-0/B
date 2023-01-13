// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "B/Weapons/BBossAttack.h"
#include "B/Enemies/BBoss.h"

class ABBossAttack;
class ABBoss;

/**
 * 
 */
// UClass()
class B_API BTest
{
public:
	BTest();
	virtual  ~BTest();
	

	FName GetWeaponName() const { return WeaponName; }
	
	ABBossAttack* GetCurrentAttack() const { return CurrentAttack; }

	void SelectAttack();

	void TryParry();
	
	// void SetAttackParriedTrue() { bAttackParried = true; }

protected:

	FName WeaponName;

	void SetWeaponName(FName Name) { WeaponName = Name; }
	
	ABBoss* BBoss;

	/** an array with the attacks the boss can use */
	TArray<ABBossAttack*> Attacks;
	/** array to store following attacks from a combo */
	TArray<ABBossAttack*> ComboFollowingAttacks;
	
	ABBossAttack* CurrentAttack;
	
	virtual void WeaponAttack();
	void WeaponAttackDelay();
	
	uint8 ComboCounter = 0;

	bool bParryWindow = false;
	void SetParryWindowTrue() { bParryWindow = true; }
	void SetParryWindowFalse() { bParryWindow = false; }
	// void SetParryWindowFalse() { bParryWindow = false; }
	
	// bool bCanBeParried = false;
	bool bAttackParried = false;
	// void CanBeParriedFalse() { bCanBeParried = false; }
};
