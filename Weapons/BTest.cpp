// Fill out your copyright notice in the Description page of Project Settings.


#include "BTest.h"
#include "B/Character/BCharacter3rd.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/Actor.h"

BTest::BTest()
{
}

BTest::~BTest()
{
}

void BTest::SelectAttack()
{
	// randomly select one of the possible attack ids set it as the CurrentAttack and attack
	if (!Attacks.IsEmpty())
	{
		// CurrentAttack = Attacks[FMath::RandRange(0, Attacks.Num() - 1)];
		// CurrentAttack = Attacks[FMath::RandRange(0, Attacks.Last()->GetAttackId())];

		CurrentAttack = Attacks[3];

		// check each one by one
		
	}

	WeaponAttack();
}

void BTest::WeaponAttack()
{
	// // make boss dash forward
	// BBoss->AttackDashForward();

	if (!GetCurrentAttack()->GetIsCombo())
	{
		switch (FMath::RandRange(0, 3))
		{
		case 0: {}
		case 1:
			{
				UE_LOG(LogTemp, Warning, TEXT(" \nOOOOO\nFORWARD\nOOOOO"));
				BBoss->AttackDashForward();
				break;
			}
		case 2:
			{
				UE_LOG(LogTemp, Warning, TEXT(" \nOOOOO\nSIDESTEP\nOOOOO"));
				BBoss->AttackSideStep();
			}
		}
	} else { BBoss->AttackDashForward(); }
	
	
	// FTimerHandle ParryWindowHandle;
	// GetWorldTimerManager().SetTimer(ParryWindowHandle, this, &BTest::SetParryWindowTrue, GetCurrentAttack()->GetDelayRate() - .3f, false);
	//
	// FTimerHandle WeaponAttackHandle;
	// GetWorldTimerManager().SetTimer(WeaponAttackHandle, this, &BTest::WeaponAttackDelay, GetCurrentAttack()->GetDelayRate(), false);

	// figure out how to do the combo attack
}

void BTest::WeaponAttackDelay()
{
	UE_LOG(LogTemp, Warning, TEXT(" \nOOOOO\nBOSS ATTACK\nOOOOO"));

	// if (!GetCurrentAttack()->GetIsCombo())
	// {
	// 	switch (FMath::RandRange(0, 3))
	// 	{
	// 	case 0: {}
	// 	case 1:
	// 		{
	// 			UE_LOG(LogTemp, Warning, TEXT(" \nOOOOO\nFORWARD\nOOOOO"));
	// 			BBoss->AttackDashForward();
	// 			break;
	// 		}
	// 	case 2:
	// 		{
	// 			UE_LOG(LogTemp, Warning, TEXT(" \nOOOOO\nSIDESTEP\nOOOOO"));
	// 			BBoss->AttackSideStep();
	// 		}
	// 	}
	// } else { BBoss->AttackDashForward(); }

	BBoss->LookAtPlayer();
	
	bParryWindow = false;
	bool bDealDamage = true;
	
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType((ECollisionChannel::ECC_WorldDynamic)));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType((ECollisionChannel::ECC_PhysicsBody)));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType((ECollisionChannel::ECC_Pawn)));
	const TArray<AActor*> IgnoreActor = {BBoss};

	TArray<AActor*> HitActors;

	const FVector CharPos = (BBoss->GetActorLocation() + (BBoss->GetRootComponent()->GetForwardVector() * GetCurrentAttack()->GetAttackOffset()));
	UKismetSystemLibrary::SphereOverlapActors(BBoss->GetWorld(),CharPos, GetCurrentAttack()->GetAttackSize(), ObjectTypes, nullptr, IgnoreActor,HitActors);
	UKismetSystemLibrary::DrawDebugSphere(BBoss->GetWorld(), CharPos, GetCurrentAttack()->GetAttackSize(), 12, FColor::Red, true, 1.0f);
	
	if (bAttackParried)
	{
		// bDealDamage = false;

		bAttackParried = false;

		// change boss state to staggered
		BBoss->SetBossStateStaggered();

		return;
	}

	// if (HitActors.IsEmpty() || !HitActors[0]->ActorHasTag("Player")) { bDealDamage = false; }
	//
	// if (bDealDamage) { Cast<ABCharacter3rd>(HitActors[0])->CharacterDamage(GetCurrentAttack()->GetAttackDamage()); }

	if (!HitActors.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT(" \nOOOOO\nPLAYER HIT\nOOOOO"));
		if (HitActors[0]->ActorHasTag("Player")) { Cast<ABCharacter3rd>(HitActors[0])->CharacterDamage(GetCurrentAttack()->GetAttackDamage()); }
	}

	// if attack is combo and 
	if (GetCurrentAttack()->GetIsCombo() && !GetCurrentAttack()->GetIsLastComboAttack())
	{
		if (!ComboFollowingAttacks.IsEmpty())
		{
			CurrentAttack = ComboFollowingAttacks[GetCurrentAttack()->GetComboFollowingID()];
			WeaponAttack();
		}
		else { BBoss->SetBossStateRest(); }
		return;
	}

	BBoss->SetBossStateRest();
}

void BTest::TryParry()
{
	if (bParryWindow)
	{
		bAttackParried = true;
	}
}