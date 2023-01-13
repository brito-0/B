// Fill out your copyright notice in the Description page of Project Settings.


#include "BBossFists.h"

#include "BBossSword.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ABBossFists::ABBossFists()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SetWeaponName("Fists");
	
}

// Called when the game starts or when spawned
void ABBossFists::BeginPlay()
{
	Super::BeginPlay();
	
	BBoss = Cast<ABBoss>(UGameplayStatics::GetActorOfClass(GetWorld(), ABBoss::StaticClass()));
	
	// Attacks.Empty();
	
	FistsAttacks.Add(NewObject<ABBossAttack>());
	FistsAttacks.Add(NewObject<ABBossAttack>());
	FistsAttacks.Add(NewObject<ABBossAttack>());

	// initialise each attack
	FistsAttacks[0]->InitializeBBossAttack(0, false, BBoss->GetBossBaseDamage(), 200.f, 250.f, false, 0, false, .6f);
	FistsAttacks[1]->InitializeBBossAttack(1, false, BBoss->GetBossMinDamage(), 200.f, 250.f, false, 0, false, .4f);
	FistsAttacks[2]->InitializeBBossAttack(2, false, BBoss->GetBossBaseDamage(), 200.f, 250.f, false, 0, false, .7f);

	CurrentAttack = FistsAttacks[0];
	
}

void ABBossFists::SelectAttack(uint16 AttackID)
{	
	if (!FistsAttacks.IsEmpty() && FistsAttacks.IsValidIndex(AttackID))
	{
		CurrentAttack = FistsAttacks[AttackID];
		WeaponAttack();
		return;
	}
	
	UE_LOG(LogTemp, Warning, TEXT(" \nOOOOO\n00000\nINVALID ATTACK ID\n00000\nOOOOO"));
	BBoss->SetBossStateRest();
	
	// WeaponAttack();
}


void ABBossFists::WeaponAttack()
{
	BBossWeapon::WeaponAttack();

	GetWorldTimerManager().ClearAllTimersForObject(this);
	// GetWorldTimerManager().ClearTimer(ParryWindowFistsTrueHandle);
	// GetWorldTimerManager().ClearTimer(ParryWindowFistsFalseHandle);
	// GetWorldTimerManager().ClearTimer(FistsAttackHandle);
	// GetWorldTimerManager().ClearTimer(FistsRestHandle);

	FistsAttackDelay = GetCurrentAttack()->GetDelayRate();
	// UE_LOG(LogTemp, Warning, TEXT("--- Fists Attack Delay -> %s"), *FString::SanitizeFloat(FistsAttackDelay));
	
	// parry window
	if (GetCurrentAttack()->GetCanBeParried())
	{
		GetWorldTimerManager().SetTimer(ParryWindowFistsTrueHandle, this, &ABBossFists::SetParryWindowTrue, FistsAttackDelay - .2f, false);
		GetWorldTimerManager().SetTimer(ParryWindowFistsFalseHandle, this, &ABBossFists::SetParryWindowFalse, FistsAttackDelay - .1f, false);
	}

	// delay to match the animation
	GetWorldTimerManager().SetTimer(FistsAttackHandle, this, &ABBossFists::WeaponAttackDelay, FistsAttackDelay, false);


	if (!GetCurrentAttack()->GetIsCombo() || GetCurrentAttack()->GetIsLastComboAttack())
	{
		GetWorldTimerManager().SetTimer(FistsRestHandle, this, &ABBossFists::BossSetToRest, FistsAttackDelay + .4f, false);
	}
}

void ABBossFists::WeaponAttackDelay()
{
	BBossWeapon::WeaponAttackDelay();

	if (GetCurrentAttack()->GetIsCombo() && !GetCurrentAttack()->GetIsLastComboAttack())
	{
		if (!FistsComboFollowingAttacks.IsEmpty())
		{
			CurrentAttack = FistsComboFollowingAttacks[GetCurrentAttack()->GetComboFollowingID()];
			WeaponAttack();
		}
		else { BBoss->SetBossStateRest(); }
	}
}
