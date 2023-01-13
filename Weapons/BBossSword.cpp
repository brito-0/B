// Fill out your copyright notice in the Description page of Project Settings.


#include "BBossSword.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ABBossSword::ABBossSword()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SetWeaponName("Sword");
	
}

// Called when the game starts or when spawned
void ABBossSword::BeginPlay()
{
	Super::BeginPlay();

	BBoss = Cast<ABBoss>(UGameplayStatics::GetActorOfClass(GetWorld(), ABBoss::StaticClass()));
	
	// Attacks.Empty();
	
	SwordAttacks.Add(NewObject<ABBossAttack>());
	SwordAttacks.Add(NewObject<ABBossAttack>());
	SwordAttacks.Add(NewObject<ABBossAttack>());
	SwordAttacks.Add(NewObject<ABBossAttack>());
	
	SwordComboFollowingAttacks.Add(NewObject<ABBossAttack>());
	SwordComboFollowingAttacks.Add(NewObject<ABBossAttack>());
	
	// initialise each attack
	SwordAttacks[0]->InitializeBBossAttack(0, false, BBoss->GetBossBaseDamage(), 200.f, 250.f, false, 0, false, .7f);
	SwordAttacks[1]->InitializeBBossAttack(1, false, BBoss->GetBossInterDamage(), 200.f, 250.f, false, 0, false, .6f);
	SwordAttacks[2]->InitializeBBossAttack(2, true, BBoss->GetBossBaseDamage(), 200.f, 250.f, false, 0, false, .6f);
	SwordAttacks[3]->InitializeBBossAttack(3, false, BBoss->GetBossBaseDamage(), 200.f, 250.f, true, 0, false, .7f);

	// initialise combo attacks
	SwordComboFollowingAttacks[0]->InitializeBBossAttack(0, false, BBoss->GetBossMinDamage(), 150.f, 200.f, true, 1, false, .8f);
	SwordComboFollowingAttacks[1]->InitializeBBossAttack(0, true, BBoss->GetBossInterDamage(), 250.f, 300.f, true, 0, true, .7f);

	CurrentAttack = SwordAttacks[0];
	
}

void ABBossSword::SelectAttack(uint16 AttackID)
{
	//  add a fail safe  to this where the comment is
	
	if (!SwordAttacks.IsEmpty() && SwordAttacks.IsValidIndex(AttackID))
	{
		CurrentAttack = SwordAttacks[AttackID];
		WeaponAttack();
		return;
	}
	
	UE_LOG(LogTemp, Warning, TEXT(" \nOOOOO\n00000\nINVALID ATTACK ID\n00000\nOOOOO"));
	BBoss->SetBossStateRest();
	
	// WeaponAttack();
}

void ABBossSword::WeaponAttack()
{
	BBossWeapon::WeaponAttack();

	GetWorldTimerManager().ClearAllTimersForObject(this);
	// GetWorldTimerManager().ClearTimer(ParryWindowSwordTrueHandle);
	// GetWorldTimerManager().ClearTimer(ParryWindowSwordFalseHandle);
	// GetWorldTimerManager().ClearTimer(SwordAttackHandle);
	// GetWorldTimerManager().ClearTimer(SwordRestHandle);

	SwordAttackDelay = GetCurrentAttack()->GetDelayRate();
	// UE_LOG(LogTemp, Warning, TEXT("--- Sword Attack Delay -> %s"), *FString::SanitizeFloat(SwordAttackDelay));
	
	// parry window
	if (GetCurrentAttack()->GetCanBeParried())
	{
		GetWorldTimerManager().SetTimer(ParryWindowSwordTrueHandle, this, &ABBossSword::SetParryWindowTrue, SwordAttackDelay - .2f, false);
		GetWorldTimerManager().SetTimer(ParryWindowSwordFalseHandle, this, &ABBossSword::SetParryWindowFalse, SwordAttackDelay - .1f, false);
	}

	// delay to match the animation
	GetWorldTimerManager().SetTimer(SwordAttackHandle, this, &ABBossSword::WeaponAttackDelay, SwordAttackDelay, false);


	if (!GetCurrentAttack()->GetIsCombo() || GetCurrentAttack()->GetIsLastComboAttack())
	{
		GetWorldTimerManager().SetTimer(SwordRestHandle, this, &ABBossSword::BossSetToRest, SwordAttackDelay + .4f, false);
	}
}

void ABBossSword::WeaponAttackDelay()
{
	BBossWeapon::WeaponAttackDelay();

	if (GetCurrentAttack()->GetIsCombo() && !GetCurrentAttack()->GetIsLastComboAttack())
	{
		if (!SwordComboFollowingAttacks.IsEmpty())
		{
			CurrentAttack = SwordComboFollowingAttacks[GetCurrentAttack()->GetComboFollowingID()];
			WeaponAttack();
		}
		else { BBoss->SetBossStateRest(); }
	}
}
