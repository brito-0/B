// Fill out your copyright notice in the Description page of Project Settings.


#include "BBossWeapon.h"
#include "B/Character/BCharacter3rd.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/Actor.h"

BBossWeapon::BBossWeapon()
{
}

BBossWeapon::~BBossWeapon()
{
}

void BBossWeapon::SelectAttack(uint16 AttackID)
{
	//
	// if (!Attacks.IsEmpty() && Attacks.IsValidIndex(AttackID))
	// {
	// 	CurrentAttack = Attacks[AttackID];
	// 	WeaponAttack();
	// 	return;
	// }
	//
	// UE_LOG(LogTemp, Warning, TEXT(" \nOOOOO\n00000\nINVALID ATTACK ID\n00000\nOOOOO"));
	// BBoss->SetBossStateRest();
	//
	// // WeaponAttack();
}

void BBossWeapon::WeaponAttack()
{

	if (!GetCurrentAttack()->GetIsCombo())
	{
		switch (FMath::RandRange(0, 2))
		{
		case 0: {}
		case 1:
			{
				// UE_LOG(LogTemp, Warning, TEXT(" \nOOOOO\nFORWARD\nOOOOO"));
				BBoss->AttackDashForward();
				break;
			}
		case 2:
			{
				// UE_LOG(LogTemp, Warning, TEXT(" \nOOOOO\nSIDESTEP\nOOOOO"));
				BBoss->AttackSideStep();
			}
		}
	} else { BBoss->AttackDashForward(); }

	
	// delays are set within the child classes
	
}

void BBossWeapon::WeaponAttackDelay()
{

	BBoss->LookAtPlayer();
	
	bParryWindow = false;
	
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType((ECollisionChannel::ECC_WorldDynamic)));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType((ECollisionChannel::ECC_PhysicsBody)));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType((ECollisionChannel::ECC_Pawn)));
	const TArray<AActor*> IgnoreActor = {BBoss};

	TArray<AActor*> HitActors;

	const FVector CharPos = (BBoss->GetActorLocation() + (BBoss->GetRootComponent()->GetForwardVector() * GetCurrentAttack()->GetAttackOffset()));
	UKismetSystemLibrary::SphereOverlapActors(BBoss->GetWorld(),CharPos, GetCurrentAttack()->GetAttackSize(), ObjectTypes, nullptr, IgnoreActor,HitActors);
	// UKismetSystemLibrary::DrawDebugSphere(BBoss->GetWorld(), CharPos, GetCurrentAttack()->GetAttackSize(), 12, FColor::Red, true, 1.0f);
	
	if (bAttackParried)
	{

		bAttackParried = false;
		
		BBoss->SetBossStateStaggered();

		return;
	}

	if (!HitActors.IsEmpty())
	{
		if (HitActors[0]->ActorHasTag("Player"))
		{
			// UE_LOG(LogTemp, Warning, TEXT("PLAYER HIT"));
			Cast<ABCharacter3rd>(HitActors[0])->CharacterDamage(GetCurrentAttack()->GetAttackDamage());
		}
	}
	
	// if (GetCurrentAttack()->GetIsCombo() && !GetCurrentAttack()->GetIsLastComboAttack())
	// {
	// 	if (!ComboFollowingAttacks.IsEmpty())
	// 	{
	// 		CurrentAttack = ComboFollowingAttacks[GetCurrentAttack()->GetComboFollowingID()];
	// 		WeaponAttack();
	// 	}
	// 	else { BBoss->SetBossStateRest(); }
	// }
	
}

void BBossWeapon::TryParry()
{
	if (bParryWindow)
	{
		bAttackParried = true;
	}
}
