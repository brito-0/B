// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "B/Weapons/BBossAttack.h"
#include "B/Enemies/BBoss.h"

class ABBossAttack;
class ABBoss;

class B_API BBossWeapon
{
public:
	BBossWeapon();
	virtual  ~BBossWeapon();
	

	FName GetWeaponName() const { return WeaponName; }
	
	ABBossAttack* GetCurrentAttack() const { return CurrentAttack; }

	/**
	 * executes an attack based on the AttackID used
	 * @param  AttackID ID of the attack to be used
	 */
	virtual void SelectAttack(uint16 AttackID);

	/** sets bAttackParried to true if bParryWindow is true */
	void TryParry();

	protected:

	FName WeaponName;

	void SetWeaponName(FName Name) { WeaponName = Name; }
	
	ABBoss* BBoss;

	// /** an array with the attacks the boss can use */
	// TArray<ABBossAttack*> Attacks;
	// /** array to store following attacks from a combo */
	// TArray<ABBossAttack*> ComboFollowingAttacks;
	
	ABBossAttack* CurrentAttack;
	
	virtual void WeaponAttack();
	virtual void WeaponAttackDelay();
	void BossSetToRest() { BBoss->SetBossStateRest(); }
	
	uint8 ComboCounter = 0;

	bool bParryWindow = false;
	void SetParryWindowTrue() { bParryWindow = true; }
	void SetParryWindowFalse() { bParryWindow = false; }
	
	bool bAttackParried = false;

};
