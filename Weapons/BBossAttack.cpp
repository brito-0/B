// Fill out your copyright notice in the Description page of Project Settings.


#include "BBossAttack.h"

// Sets default values
ABBossAttack::ABBossAttack()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

void ABBossAttack::InitializeBBossAttack(uint16 ID, bool BP, float AD, float AS, float AO, bool C, uint16 CFID, bool LC, float DR)
{
	AttackId = ID;
	bCanBeParried = BP;
	AttackDamage  = AD;
	AttackSize = AS;
	AttackOffset = AO;
	bIsCombo = C;
	ComboFollowingID = CFID;
	bIsLastComboAttack = LC;
	DelayRate = DR;
}
