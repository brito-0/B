// Fill out your copyright notice in the Description page of Project Settings.


#include "BBoss.h"

#include "AIController.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationSystem.h"
#include "B/Weapons/BBossFists.h"
#include "B/Weapons/BBossSword.h"
#include "B/Framework/BGameModeBase.h"

// Sets default values
ABBoss::ABBoss()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	this->Tags.Add("Boss");

	GetCapsuleComponent()->InitCapsuleSize(CapsuleRadius, CapsuleHeight);

	// set colliders
	BoxCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Collider"));
	BoxCollider->SetupAttachment(RootComponent);
	BoxCollider->SetVisibleFlag(true);
	BoxCollider->SetBoxExtent(FVector(CapsuleRadius + 10,CapsuleRadius + 10,CapsuleHeight - 20), true);

	ParryCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("Parry Collider"));
	ParryCollider->SetupAttachment(RootComponent);
	ParryCollider->SetVisibility(true);
	ParryCollider->SetBoxExtent(FVector(200.f,100.f,100.f), true);
	ParryCollider->SetRelativeLocation(FVector(100.f, ParryCollider->GetRelativeLocation().Y, ParryCollider->GetRelativeLocation().Z));

	// configure movement
	GetCharacterMovement()->SetJumpAllowed(false);
	GetCharacterMovement()->MaxWalkSpeed = BossBaseSpeed;
	GetCharacterMovement()->MaxAcceleration = 25000.f;

	// set mesh
	GetMesh()->SetRelativeScale3D(FVector(2.f, 2.f, 2.f));
	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -183.0f));
    GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
    static ConstructorHelpers::FObjectFinder<USkeletalMesh>MeshAsset(TEXT("SkeletalMesh'/Game/Boss_Mannequin/Character/Mesh/Maw_J_Laygo.Maw_J_Laygo'"));
	if (MeshAsset.Succeeded()) { GetMesh()->SetSkeletalMesh(MeshAsset.Object); }
 
	static ConstructorHelpers::FObjectFinder<USoundBase>ChargeSoundAsset(TEXT("SoundWave'/Game/Sounds/Boss/320905__suzenako__the-ding.320905__suzenako__the-ding'"));
	if (ChargeSoundAsset.Succeeded()) { ChargeFlurrySound = ChargeSoundAsset.Object; }

	static ConstructorHelpers::FObjectFinder<USoundBase>ParrySoundAsset(TEXT("SoundWave'/Game/Sounds/Boss/518992_amaiguri_swordclash.518992_amaiguri_swordclash'"));
	if (ParrySoundAsset.Succeeded()) { ParrySound = ParrySoundAsset.Object; }

	static ConstructorHelpers::FObjectFinder<USoundBase>SlashSoundAsset(TEXT("SoundWave'/Game/Sounds/Player/574821_wesleyextreme-gamer_slash1.574821_wesleyextreme-gamer_slash1'"));
	if (SlashSoundAsset.Succeeded()) { SlashSound = SlashSoundAsset.Object; }
	
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem>EyeEffectAsset(TEXT("NiagaraSystem'/Game/Boss_Mannequin/Effect/EyeEffect.EyeEffect'"));
	if (EyeEffectAsset.Succeeded())
	{
		Phase3EyeL = EyeEffectAsset.Object;
		Phase3EyeR = EyeEffectAsset.Object;
	}

	SetActorTickInterval(.1f);
}

// Called when the game starts or when spawned
void ABBoss::BeginPlay()
{
	Super::BeginPlay();

	// GetWorld()->DebugDrawTraceTag = "TraceTag";

	CurrentHealth = MaxHealth;
	
	BossController = Cast<AAIController>(GetController());

	BCharacter = Cast<ABCharacter3rd>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	SetBossState(EBossState::Rest);
	
	BoxCollider->OnComponentBeginOverlap.AddDynamic(this, &ABBoss::OnBeginOverlap);

	BossFistsWeapon = GetWorld()->SpawnActor<ABBossFists>(FistsObject);
	const FAttachmentTransformRules AttachRules(EAttachmentRule::KeepRelative, true);
	BossFistsWeapon->AttachToComponent(GetMesh(), AttachRules, "BossFistsSocket");
	
	
	BossSwordWeapon = GetWorld()->SpawnActor<ABBossSword>(SwordObject);
	WeaponSocketAttachment("BossSwordHideSocket");

	GameMode = Cast<ABGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	
}

void ABBoss::SetBossState(EBossState State)
{
	GetWorldTimerManager().ClearTimer(AttackRestHandle);
	if (CurrentBossState == State || CurrentBossState == EBossState::Dead) { return; }
	switch (State)
	{
	case EBossState::Rest:
		{
			if (bChargeFlurry)
			{
				ChangeCurrentStateQuickRest();
				return;
			}
			
			CurrentBossState = EBossState::Rest;

			BossController->StopMovement();

			GetCharacterMovement()->SetMovementMode(MOVE_None);

			FTimerHandle RestHandle;
			switch (CurrentPhase)
			{
			default: {}
			case 0: {}
			case 1:
				{
					GetCharacterMovement()->MaxWalkSpeed = BossBaseSpeed;
					GetWorldTimerManager().SetTimer(RestHandle, this, &ABBoss::NextDecision, NormalRest, false);
					
					break;
				}
			case 2:
				{
					GetCharacterMovement()->MaxWalkSpeed = BossPhase3Speed;
					GetWorldTimerManager().SetTimer(RestHandle, this, &ABBoss::NextDecision, Phase3Rest, false);
					
					break;
				}
			}

			UE_LOG(LogTemp, Warning, TEXT("Boss -> Rest"));
			break;
		}
	case EBossState::QuickRest:
		{
			CurrentBossState = EBossState::QuickRest;

			GetCharacterMovement()->SetMovementMode(MOVE_None);

			PreviousBossLocation = GetActorLocation();
			SetActorLocation(BelowMapLocation);
			SetActorRotation(FRotator(0.f, FMath::RandRange(0.f, 360.f), 0.f));

			GetCharacterMovement()->MaxWalkSpeed = BossPhase3Speed;

			if (++ChargeFlurryCount == 5) { bChargeFlurry = false; }

			FTimerHandle ChargeFlurryHandle;
			GetWorldTimerManager().SetTimer(ChargeFlurryHandle, this, &ABBoss::ChargeFlurryAttack, 1.5f, false);

			UE_LOG(LogTemp, Warning, TEXT("Boss -> QuickRest"));
			break;
		}
	case EBossState::Wander:
		{
			CurrentBossState = EBossState::Wander;

			GetCharacterMovement()->SetMovementMode(MOVE_Walking);
			GetCharacterMovement()->MaxWalkSpeed = BossBaseSpeed;
			
			MoveToRandom();

			FTimerHandle WanderHandle;
			GetWorldTimerManager().SetTimer(WanderHandle, this, &ABBoss::CheckVelocity, 4.f, false);

			UE_LOG(LogTemp, Warning, TEXT("Boss -> Wander"));
			break;
		}
	case EBossState::Follow:
		{
			CurrentBossState = EBossState::Follow;

			GetCharacterMovement()->SetMovementMode(MOVE_Walking);
			GetCharacterMovement()->MaxWalkSpeed = BossBaseSpeed;
			
			NextDecision();

			UE_LOG(LogTemp, Warning, TEXT("Boss -> Follow"));
			break;
		}
	case EBossState::Charge:
		{
			BossController->StopMovement();
			CurrentBossState = EBossState::Charge;

			GetCharacterMovement()->SetMovementMode(MOVE_Walking);
			
			switch (CurrentPhase)
			{
			default: {}
			case 0:{}
			case 1:
				{
					GetCharacterMovement()->MaxWalkSpeed = BossChargeSpeed;
					break;
				}
			case 2:
				{
					GetCharacterMovement()->MaxWalkSpeed = BossPhase3ChargeSpeed;
					break;
				}
			}

			ChargePlayer();

			FTimerHandle ChangeToRestHandle;
			GetWorldTimerManager().SetTimer(ChangeToRestHandle, this, &ABBoss::CheckVelocity, 2.f, false);

			UE_LOG(LogTemp, Warning, TEXT("Boss -> Charge"));
			break;
		}
	case EBossState::Attack:
		{
			BossController->StopMovement();
			CurrentBossState = EBossState::Attack;

			GetCharacterMovement()->SetMovementMode(MOVE_Walking);
			GetCharacterMovement()->MaxWalkSpeed = BossAttackSpeed;

			switch (CurrentPhase)
			{
			default: {}
			case 0:
				{
					bBossSwordEquipped = false;
					CurrentAttackID = FMath::RandRange(0, 2);
					BossFistsWeapon->SelectAttack(CurrentAttackID);
					
					break;
				}
			case 1:
				{
					bBossSwordEquipped = true;
					CurrentAttackID = FMath::RandRange(0, 3);
					BossSwordWeapon->SelectAttack(CurrentAttackID);
					
					break;
				}
			case 2:
				{
					switch (FMath::RandRange(0,1))
					{
					default: {}
					case 0:
						{
							bBossSwordEquipped = false;
							WeaponSocketAttachment("BossSwordHideSocket");
							CurrentAttackID = FMath::RandRange(0, 2);
							BossFistsWeapon->SelectAttack(CurrentAttackID);
							
							break;
						}
					case 1:
						{
							bBossSwordEquipped = true;
							WeaponSocketAttachment("BossSwordHandSocket");
							CurrentAttackID = FMath::RandRange(0, 3);
							BossSwordWeapon->SelectAttack(CurrentAttackID);
							
							break;
						}
					}
					break;
				}
			}
			
			GetWorldTimerManager().SetTimer(AttackRestHandle, this, &ABBoss::ChangeCurrentStateRest, 3.f, false);

			UE_LOG(LogTemp, Warning, TEXT("Boss -> Attack"));
			break;
		}
	case EBossState::Staggered:
		{
			CurrentBossState = EBossState::Staggered;

			GetCharacterMovement()->SetMovementMode(MOVE_None);
			
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), ParrySound, GetActorLocation(), 1, 1);
			
			FTimerHandle StaggerHandle;
			GetWorldTimerManager().SetTimer(StaggerHandle, this, &ABBoss::ChangeCurrentStateRest, 1.f, false);
			
			UE_LOG(LogTemp, Warning, TEXT("Boss -> Staggered"));
			break;
		}
	case EBossState::Dead:
		{
			CurrentBossState = EBossState::Dead;

			GetCharacterMovement()->SetMovementMode(MOVE_None);

			if (BCharacter->GetCharacterState() == ECharacterState::Dead) { GameMode->GameEndLoss(); }
			else { GameMode->GameEndWin(); }

			UE_LOG(LogTemp, Warning, TEXT("Boss -> Dead"));
			break;
		}
	}
}


void ABBoss::NextDecision()
{
	if (CurrentBossState == EBossState::Dead) { return; }
	
	CurrentDistanceToPlayer = GetDistanceTo(BCharacter);
	if (CurrentDistanceToPlayer > MaxDistance)
	{
		if (CurrentDistanceToPlayer >= ChargeDistance) { ChangeCurrentStateCharge(); }
		else
		{
			if (CurrentBossState == EBossState::Follow)
			{
				MoveToPlayer();
				FTimerHandle CheckLocationHandle;
				GetWorldTimerManager().SetTimer(CheckLocationHandle, this, &ABBoss::NextDecision, .5f, false);
			}
			else { ChangeCurrentStateFollow(); }
		}
	}
	else
	{
		int32 Range;
		if (CurrentPhase == 2) { Range = 6; } else { Range = FMath::RandRange(1, 6); }
	
		switch (Range)
		{
		case 1:
			{
				ChangeCurrentStateWander();
				break;
			}
		case 2: {}
		case 3: {}
		case 4: {}
		case 5: {}
		default: {}
		case 6:
			{
				ChangeCurrentStateAttack();
				break;
			}
		}
	}
}


void ABBoss::MoveToPlayer()
{
	BossController->MoveToActor(BCharacter, Distance, true, true, true);
}

FVector ABBoss::GetRandomPositionFrom(FVector Location)
{
	const FVector RandomPosition = Location + FRotator(0.f,FMath::RandRange(GetActorRotation().Yaw + 90.f, GetActorRotation().Yaw + 270.f),0.f).Vector() * FMath::RandRange(1700, 2700); //2200;

	// check if location is within the map bounds
	FHitResult CSResult;
	FCollisionQueryParams CSQueryP;
	CSQueryP.TraceTag = "TraceTag";
	CSQueryP.AddIgnoredActor(BCharacter);
	FCollisionResponseParams CSResponseP;
	GetWorld()->LineTraceSingleByChannel(CSResult, Location,
										RandomPosition,
										ECollisionChannel::ECC_Visibility, CSQueryP, CSResponseP);
	
	if (CSResult.Distance == 0.f)
	{
		return RandomPosition;
	}
	
	return GetRandomPositionFrom(Location);
}

void ABBoss::MoveToRandom()
{
	const FVector MoveLocation = GetRandomPositionFrom(GetActorLocation());

	BossController->MoveToLocation(MoveLocation, 0.f, true, true, false, false);
	// UKismetSystemLibrary::DrawDebugSphere(GetWorld(), MoveLocation, 25, 1, FColor::Cyan, 10.f, 1.0f);
}

void ABBoss::CheckVelocity()
{
	const FVector BossVelocity = GetCharacterMovement()->Velocity;
	if (BossVelocity.X == 0.f && BossVelocity.Y == 0.f) { ChangeCurrentStateRest(); }
	else
	{
		FTimerHandle CheckVelocityHandle;
		GetWorldTimerManager().SetTimer(CheckVelocityHandle, this, &ABBoss::CheckVelocity, .1f, false);
	}
}

void ABBoss::ChargePlayer()
{
	FTimerHandle ChargeHandle;
	GetWorldTimerManager().SetTimer(ChargeHandle, this, &ABBoss::ChargeDelay, 1.5f, false);
}

void ABBoss::ChargeDelay()
{
	FVector ChargeLocation;
	const FVector CharacterSpeed = BCharacter->GetCharacterMovement()->Velocity;

	LookAtPlayer();
	
	FHitResult CSResult;
	FCollisionQueryParams CSQueryP;
	CSQueryP.TraceTag = "TraceTag";
	CSQueryP.AddIgnoredActor(BCharacter);
	FCollisionResponseParams CSResponseP;
	GetWorld()->LineTraceSingleByChannel(CSResult, BCharacter->GetActorLocation(),
										BCharacter->GetActorLocation() + (GetRootComponent()->GetForwardVector() + BCharacter->GetRootComponent()->GetForwardVector()) * 500,
											ECollisionChannel::ECC_Visibility, CSQueryP, CSResponseP);
	DistanceOffSet = CSResult.Distance;
	
	if (CharacterSpeed.X != 0.f || CharacterSpeed.Y != 0.f)
	{
		// check if the line trace hits any wall
		if (CSResult.Distance != 0.f)
		{
			ChargeLocation = BCharacter->GetActorLocation() + (GetRootComponent()->GetForwardVector() + BCharacter->GetRootComponent()->GetForwardVector()) * CSResult.Distance/2;
		}
		else { ChargeLocation = BCharacter->GetActorLocation() + (GetRootComponent()->GetForwardVector() + BCharacter->GetRootComponent()->GetForwardVector()) * 500; }
	}
	else
	{
		// check if the line trace hits any wall
		if (CSResult.Distance != 0.f)
		{
			ChargeLocation = BCharacter->GetActorLocation() + GetRootComponent()->GetForwardVector() * CSResult.Distance/2;
		}
		else { ChargeLocation = BCharacter->GetActorLocation() + GetRootComponent()->GetForwardVector() * 500; }
	}

	// UKismetSystemLibrary::DrawDebugSphere(GetWorld(), ChargeLocation, 25, 1, FColor::Orange, 10.f, 1.0f);

	ChargeCanDamage = true;
	
	BossController->MoveToLocation(ChargeLocation, 0.f, true, true, false, false);
}

void ABBoss::ChargeFlurryAttack()
{
	const FVector ChargeLocation = GetRandomPositionFrom(PreviousBossLocation);
	
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), ChargeFlurrySound, ChargeLocation, 1, 1);
	
	SetActorLocation(ChargeLocation);

	ChangeCurrentStateCharge();
}

void ABBoss::OnBeginOverlap(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// UE_LOG(LogTemp, Warning, TEXT(" \nOOOOO\nCOLLISION\nOOOOO"));
	if (CurrentBossState == EBossState::Charge)
	{
		if (OtherActor->ActorHasTag("Player") && ChargeCanDamage)
		{
			if (Cast<ABCharacter3rd>(OtherActor)->CharacterDamage(BossMaxDamage)) { ChargeCanDamage = false; }
		}
	}
}

void ABBoss::WeaponSocketAttachment(FName SocketName) const
{
	if (BossSwordWeapon->GetAttachParentSocketName() == SocketName) {return;}
	
	const FAttachmentTransformRules AttachRules(EAttachmentRule::KeepRelative, true);
	BossSwordWeapon->AttachToComponent(GetMesh(), AttachRules, SocketName);
}

void ABBoss::AttackDashForward()
{
	if (!bSideStepDash) { LookAtPlayer(); }

	bSideStepDash = false;
	
	// UE_LOG(LogTemp, Warning, TEXT(" \nOOOOO\nDash\nOOOOO"));
	
	const FVector DashDestination = GetActorLocation() + GetRootComponent()->GetForwardVector() * 500;
	
	BossController->MoveToLocation(DashDestination, 0.f, true, true, false, false);
	
	// UKismetSystemLibrary::DrawDebugSphere(GetWorld(), DashDestination, 25, 1, FColor::Cyan, 10.f, 1.0f);
}

void ABBoss::AttackSideStep()
{
	LookAtPlayer();
	
	FVector SideStepDestination;
	
	switch (FMath::RandRange(0,1))
	{
	default: {}
	case  0:
		{
			// sidestep left
			SideStepDestination = GetActorLocation() + GetRootComponent()->GetRightVector() * -250;
			
			break;
		}
	case 1:
		{
			// sidestep right
			SideStepDestination = GetActorLocation() + GetRootComponent()->GetRightVector() * 250;
			
			break;
		}
	}

	bUseControllerRotationYaw = false;

	BossController->MoveToLocation(SideStepDestination, 0.f, true, true, false, false);
	// UKismetSystemLibrary::DrawDebugSphere(GetWorld(), SideStepDestination, 25, 1, FColor::Magenta, 10.f, 1.0f);

	bSideStepDash = true;

	FTimerHandle LookHandle;
	GetWorldTimerManager().SetTimer(LookHandle, this, &ABBoss::LookAtPlayer, .2f, false);
}

void ABBoss::LookAtPlayer()
{
	bUseControllerRotationYaw = true;
	
	const FRotator Direction = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), BCharacter->GetActorLocation());
	const FRotator YawRotation (0.f, Direction.Yaw, 0.f);
	GetRootComponent()->SetRelativeRotation(YawRotation);

	if (bSideStepDash) { AttackDashForward(); }
}

void ABBoss::BossTakeDamage(float Value)
{
	// UE_LOG(LogTemp, Warning, TEXT("BOSS HIT"));
	
	if (CurrentHealth - Value > 0)
	{
		CurrentHealth -= Value;

		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SlashSound, GetActorLocation(), 1, 1);

		if (CurrentHealth <= Phase2CutOff && CurrentPhase == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("--- PHASE 2 ---"));
			CurrentPhase = 1;
			CurrentWeapon = 1;
			WeaponSocketAttachment("BossSwordHandSocket");
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), ChargeFlurrySound, GetActorLocation(), 1, .1);
		}
		else if (CurrentHealth <= Phase3CutOff && CurrentPhase == 1)
		{
			UE_LOG(LogTemp, Warning, TEXT("--- PHASE 3 ---"));
			CurrentPhase = 2;
			bChargeFlurry = true;

			UGameplayStatics::PlaySoundAtLocation(GetWorld(), ChargeFlurrySound, GetActorLocation(), 1, .1);

			UNiagaraFunctionLibrary::SpawnSystemAttached(Phase3EyeL, GetMesh(), "BossLEyeSocket",
													FVector(0.0f, 0.0f, 0.0f), FRotator(0.0f, 0.0f, 0.0f),
													EAttachLocation::KeepRelativeOffset, false, true);

			UNiagaraFunctionLibrary::SpawnSystemAttached(Phase3EyeR, GetMesh(), "BossREyeSocket",
													FVector(0.0f, 0.0f, 0.0f), FRotator(0.0f, 0.0f, 0.0f),
													EAttachLocation::KeepRelativeOffset, true, true);
		}
	}
	else
	{
		CurrentHealth = 0.f;
		ChangeCurrentStateDead();
	} 
}
