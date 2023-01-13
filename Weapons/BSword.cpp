// Fill out your copyright notice in the Description page of Project Settings.


#include "BSword.h"

#include "BBossSword.h"
#include "B/Enemies/BBoss.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values
ABSword::ABSword()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	static ConstructorHelpers::FObjectFinder<UMaterial>MaterialNormalAsset(TEXT("Material'/Game/Weapon_Pack/Materials/Weapons/M_WeaponSet_1.M_WeaponSet_1'"));
	if (MaterialNormalAsset.Succeeded()) { SwordNormalMaterial = MaterialNormalAsset.Object; }
	
	static ConstructorHelpers::FObjectFinder<UMaterial>MaterialEmpoweredAsset(TEXT("Material'/Game/Weapon_Pack/Materials/Weapons/M_WeaponSet_Empowered.M_WeaponSet_Empowered'"));
	if (MaterialEmpoweredAsset.Succeeded()) { SwordEmpoweredMaterial = MaterialEmpoweredAsset.Object; }
	
}

// Called when the game starts or when spawned
void ABSword::BeginPlay()
{
	Super::BeginPlay();

	BCharacter = Cast<ABCharacter3rd>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	
}

void ABSword::SwordAttack()
{
	// delay the attack to match the animation
	FTimerHandle SwordAttackDelayHandle;
	GetWorldTimerManager().SetTimer(SwordAttackDelayHandle, this, &ABSword::SwordAttackDelay, 0.7f, false);
	
}

void ABSword::SwordAttackDelay()
{
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType((ECollisionChannel::ECC_WorldDynamic)));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType((ECollisionChannel::ECC_PhysicsBody)));
	const TArray<AActor*> IgnoreActor = {BCharacter};
	
	TArray<AActor*> HitActors;
	
	FVector CharPos = (BCharacter->GetActorLocation() + (BCharacter->GetRootComponent()->GetForwardVector() * AttackOffset));
	UKismetSystemLibrary::SphereOverlapActors(GetWorld(),CharPos, AttackSize, ObjectTypes, nullptr, IgnoreActor,HitActors);
	// UKismetSystemLibrary::DrawDebugSphere(GetWorld(), CharPos, AttackSize, 12, FColor::Red, true, 1.0f);

	if (HitActors.IsEmpty() || !HitActors[0]->ActorHasTag("Boss")) {return;}
	
	if (bIsEmpowered)
	{
		AttackPowerFinal = (AttackPowerBase + AttackIndex) * 1.5;
		if (EmpoweredAttackCounter < 2) { ++EmpoweredAttackCounter; }
		else
		{
			FTimerHandle UnEmpowerHandle;
			GetWorldTimerManager().SetTimer(UnEmpowerHandle, this, &ABSword::UnEmpowerSword, .3f, false);
		}
	}
	else { AttackPowerFinal = AttackPowerBase + AttackIndex; }

	// UE_LOG(LogTemp, Warning, TEXT("Enemy Hit"));
	
	Cast<ABBoss>(HitActors[0])->BossTakeDamage(AttackPowerFinal);

	BCharacter->RecoverPower();

	if (AttackIndex < 2) { ++AttackIndex; } else { AttackIndex = 0; }
}

void ABSword::SwordParry()
{	
	FVector CharPos = (BCharacter->GetActorLocation() + (BCharacter->GetRootComponent()->GetForwardVector() * ParryOffset));
	// UKismetSystemLibrary::SphereOverlapActors(GetWorld(),CharPos, AttackSize[AttackIndex], ObjectTypes, nullptr,IgnoreActor,HitActors);
	
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	const TArray<AActor*> IgnoreActor = {BCharacter};
	TArray<UPrimitiveComponent*> HitComponent;
	UKismetSystemLibrary::SphereOverlapComponents(GetWorld(), CharPos, ParrySize, ObjectTypes, nullptr, IgnoreActor, HitComponent);
	// UKismetSystemLibrary::DrawDebugSphere(GetWorld(), CharPos, ParrySize, 12, FColor::Blue, true, 1.0f);

	if (HitComponent.IsEmpty()) {return;}

	for (auto Object : HitComponent)
	{
		if (Object->GetName() == "Parry Collider")
		{
			Cast<ABBoss>(Object->GetOwner())->GetBossWeaponSword()->TryParry();
		}
	}
}

void ABSword::EmpowerSword()
{
	bIsEmpowered = true;
	
	USkeletalMeshComponent* SwordMesh = FindComponentByClass<USkeletalMeshComponent>();
	SwordMesh->SetMaterial(0, SwordEmpoweredMaterial);
}

void ABSword::UnEmpowerSword()
{	
	bIsEmpowered = false;
	EmpoweredAttackCounter = 0;

	USkeletalMeshComponent* SwordMesh = FindComponentByClass<USkeletalMeshComponent>();
	SwordMesh->SetMaterial(0, SwordNormalMaterial);
}
