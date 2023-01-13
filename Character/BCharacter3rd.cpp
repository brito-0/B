// Fill out your copyright notice in the Description page of Project Settings.


#include "BCharacter3rd.h"

#include "CollisionDebugDrawingPublic.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "B/Framework/BGameModeBase.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values
ABCharacter3rd::ABCharacter3rd()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	GetCapsuleComponent()->InitCapsuleSize(CapsuleRadius, CapsuleHeight);
	
   	bUseControllerRotationPitch = false;
   	bUseControllerRotationYaw = false;
   	bUseControllerRotationRoll = false;
    
   	// configure character movement
   	GetCharacterMovement()->bOrientRotationToMovement = true;
   	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
   	GetCharacterMovement()->JumpZVelocity = 500.f;
   	GetCharacterMovement()->AirControl = 0.1f;

	// configure camera
   	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
   	CameraBoom->SetupAttachment(RootComponent);
   	CameraBoom->TargetArmLength = 500.0f;
   	CameraBoom->bUsePawnControlRotation = true;
   	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
   	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
   	FollowCamera->bUsePawnControlRotation = false;

	// set mesh and animation blueprint
	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));
	GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>MeshAsset(TEXT("SkeletalMesh'/Game/Mannequin/Character/Mesh/SK_Mannequin.SK_Mannequin'"));
	if (MeshAsset.Succeeded()) { GetMesh()->SetSkeletalMesh(MeshAsset.Object); }
	
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem>NiagaraAsset(TEXT("NiagaraSystem'/Game/Weapon_Pack/Effect/WeaponEffect.WeaponEffect'"));
	if (NiagaraAsset.Succeeded()) { WeaponSwitchEffect = NiagaraAsset.Object; }

	static ConstructorHelpers::FObjectFinder<USoundBase>HitSoundAsset(TEXT("SoundWave'/Game/Sounds/Player/grunt.grunt'"));
	if (HitSoundAsset.Succeeded()) { HitSound = HitSoundAsset.Object; }

	
	this->Tags.Add("Player");
	
	SetActorTickInterval(0.1f);
}

// Called when the game starts or when spawned
void ABCharacter3rd::BeginPlay()
{
	Super::BeginPlay();

	// GetWorld()->DebugDrawTraceTag = "TraceTag";
	
	SetCharacterState(ECharacterState::NonCombat);
	PreviousCharacterState = ECharacterState::NonCombat;

	GetCharacterMovement()->SetJumpAllowed(false);

	GetCharacterMovement()->MaxWalkSpeed = 500.0f;
	CurrentHealth = MaxHealth;
	CurrentStamina = MaxStamina;
	CurrentPower = MaxPower / 2;
	
	CharacterWeapon = GetWorld()->SpawnActor<ABSword>(WeaponObject);
	WeaponSocketAttachment("WeaponBackSocket");

	PauseMenu = CreateWidget<UUserWidget>(GetWorld(),  PauseWidget);

	GameMode = Cast<ABGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	
}

// Called every frame
void ABCharacter3rd::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// regen stamina
	if (bStaminaTick)
	{
		if (CurrentStamina + 1.f > MaxStamina)
		{
			bStaminaTick = false;
			CurrentStamina = MaxStamina;
		}
		else
		{
			CurrentStamina += 1.f;
		}
	}

	// heal tick - interrupt if damage is taken
	if (bHealTick)
	{
		if (CurrentHealth + 1.25f > MaxHealth)
		{
			bHealTick = false;
			CurrentHealth = MaxHealth;
		}
		else
		{
			CurrentHealth += 1.25f;
		}
	}

}


void ABCharacter3rd::SetCharacterState(ECharacterState State)
{
	if (CurrentCharacterState == State || CurrentCharacterState == ECharacterState::Dead) { return; }
	GetWorldTimerManager().ClearTimer(CanAttackHandle);
	switch (State)
	{
	case ECharacterState::NonCombat:
		{
			if (GetCharacterState() == ECharacterState::Combat) { PreviousCharacterState = CurrentCharacterState; }
			
			CurrentCharacterState = ECharacterState::NonCombat;
			WeaponSocketAttachment("WeaponBackSocket");

			GetCharacterMovement()->SetMovementMode(MOVE_Walking);
			GetCharacterMovement()->MaxWalkSpeed = 500.0f;
			bCanCrouch = true;
			bCanDodge = false;
			
			FTimerHandle CanDodgeHandle;
			GetWorldTimerManager().SetTimer(CanDodgeHandle, this, &ABCharacter3rd::SetCanDodgeTrue, 0.2f, false);
			
			bCrouchSkipCapsuleChange = false;
			bCanHeal = true;
			bHealTick = false;
			bInvincible = false;
			bCanAttack = true;
			bCanParry = false;
			
			UE_LOG(LogTemp, Warning, TEXT("Player -> NonCombat"));
			break;
		}
	case ECharacterState::Combat:
		{
			if (GetCharacterState() == ECharacterState::NonCombat) { PreviousCharacterState = CurrentCharacterState; }
			
			CurrentCharacterState = ECharacterState::Combat;
			WeaponSocketAttachment("WeaponHandSocket");

			GetCharacterMovement()->SetMovementMode(MOVE_Walking);
			GetCharacterMovement()->MaxWalkSpeed = 500.0f;
			bCanCrouch = false;
			bCanDodge = true;
			bCrouchSkipCapsuleChange = false;
			bCanHeal = false;
			bHealTick = false;
			bInvincible = false;
			bCanAttack = false;
			GetWorldTimerManager().SetTimer(CanAttackHandle, this, &ABCharacter3rd::SetCanAttackTrue, .2f, false);
			bCanParry = true;
			
			UE_LOG(LogTemp, Warning, TEXT("Player -> Combat"));
			break;
		}
	case ECharacterState::Attack:
		{
			if (GetCharacterState() == ECharacterState::NonCombat || GetCharacterState() == ECharacterState::Combat) { PreviousCharacterState = CurrentCharacterState; }
			
			CurrentCharacterState = ECharacterState::Attack;
			GetWorldTimerManager().ClearTimer(AttackIndexResetHandle);
			if (PreviousCharacterState == ECharacterState::NonCombat) { WeaponSocketAttachment("WeaponHandSocket"); }

			GetCharacterMovement()->SetMovementMode(MOVE_None);
			GetCharacterMovement()->MaxWalkSpeed = 0.0f;
			bCanCrouch = false;
			bCanDodge = false;
			bCrouchSkipCapsuleChange = false;
			bCanHeal = false;
			bHealTick = false;
			bInvincible = false;
			
			bCanAttack = false;
			bCanParry = false;

			// delay to return to combat state, based on the animation time
			FTimerHandle AttackHandle;
			GetWorldTimerManager().SetTimer(AttackHandle, this, &ABCharacter3rd::ChangeCurrentStateCombat, 1.5f, false);

			// resets the attack index if another attack is not performed
			GetWorldTimerManager().SetTimer(AttackIndexResetHandle, this, &ABCharacter3rd::WeaponIndexReset, 2.0f, false);
			
			UE_LOG(LogTemp, Warning, TEXT("Player -> Attack"));
			break;
		}
	case ECharacterState::Parry:
		{
			PreviousCharacterState = ECharacterState::Combat;
			
			CurrentCharacterState = ECharacterState::Parry;

			GetCharacterMovement()->SetMovementMode(MOVE_None);
			GetCharacterMovement()->MaxWalkSpeed = 0.0f;
			bCanCrouch = false;
			bCanDodge = false;
			bCrouchSkipCapsuleChange = false;
			bCanHeal = false;
			bHealTick = false;
			bInvincible = false;
			bCanAttack = false;
			bCanParry = false;

			// delay to return to combat state, based on the animation time
			FTimerHandle ParryHandle;
			GetWorldTimerManager().SetTimer(ParryHandle, this, &ABCharacter3rd::ChangeCurrentStateCombat, 0.6f, false);
			
			UE_LOG(LogTemp, Warning, TEXT("Player -> Parry"));
			break;
		}
	case ECharacterState::Crouch:
		{
			if (GetCharacterState() == ECharacterState::NonCombat) { PreviousCharacterState = CurrentCharacterState; }
			
			if (!bCrouchSkipCapsuleChange) { ChangeCapsule(ECharacterState::Crouch); }
			CurrentCharacterState = ECharacterState::Crouch;
			
			GetCharacterMovement()->MaxWalkSpeed = 250.0f;
			bCanCrouch = false;
			bCanDodge = false;
			bCrouchSkipCapsuleChange = false;
			bCanHeal = false;
			bHealTick = false;
			bInvincible = false;
			bCanAttack = false;
			bCanParry = false;
			
			UE_LOG(LogTemp, Warning, TEXT("Player -> Crouch"));
			break;
		}
	case ECharacterState::Dodge:
		{
			if (GetCharacterState() == ECharacterState::NonCombat || GetCharacterState() == ECharacterState::Combat) { PreviousCharacterState = CurrentCharacterState; }
			
			ChangeCapsule(ECharacterState::Dodge);
			CurrentCharacterState = ECharacterState::Dodge;

			GetCharacterMovement()->MaxWalkSpeed = 550.0f;
			bCanCrouch = false;
			bCanDodge = false;
			bCrouchSkipCapsuleChange = true;
			bCanHeal = false;
			bHealTick = false;
			bInvincible = true;
			FTimerHandle InvincibilityHandle;
			GetWorldTimerManager().SetTimer(InvincibilityHandle, this, &ABCharacter3rd::SetInvincibleFalse, 0.4f, false);
			bCanAttack = false;
			bCanParry = false;
			
			UE_LOG(LogTemp, Warning, TEXT("Player -> Dodge"));
			
			FTimerHandle DodgeHandle;
			GetWorldTimerManager().SetTimer(DodgeHandle, this, &ABCharacter3rd::SetUnDodge, 1.0f, false);
			break;
		}
	case ECharacterState::Heal:
		{
			PreviousCharacterState = ECharacterState::NonCombat;
			
			CurrentCharacterState = ECharacterState::Heal;
			
			GetCharacterMovement()->MaxWalkSpeed = 300.0f;
			bCanCrouch = false;
			bCanDodge = false;
			bCrouchSkipCapsuleChange = false;
			bCanHeal = false;
			bHealTick = true;
			bInvincible = false;
			bCanAttack = false;
			bCanParry = false;

			FTimerHandle HealDodgeHandle;
			GetWorldTimerManager().SetTimer(HealDodgeHandle, this, &ABCharacter3rd::SetCanDodgeTrue, 0.5f, false);

			UE_LOG(LogTemp, Warning, TEXT("Player -> Heal"));

			FTimerHandle HealHandle;
			GetWorldTimerManager().SetTimer(HealHandle, this, &ABCharacter3rd::ChangeCurrentStateNonCombat, 2.0f, false);
			break;
		}
	case ECharacterState::Staggered:
		{
			if (GetCharacterState() == ECharacterState::NonCombat || GetCharacterState() == ECharacterState::Combat) { PreviousCharacterState = CurrentCharacterState; }
			
			CurrentCharacterState = ECharacterState::Staggered;

			GetCharacterMovement()->SetMovementMode(MOVE_None);
			GetCharacterMovement()->MaxWalkSpeed = 0.0f;
			bCanCrouch = false;
			bCanDodge = false;
			bCrouchSkipCapsuleChange = false;
			bCanHeal = false;
			bHealTick = false;
			bInvincible = false;
			bCanAttack = false;
			bCanParry = false;

			UE_LOG(LogTemp, Warning, TEXT("Player -> Staggered"));

			FTimerHandle StaggerHandle;
			GetWorldTimerManager().SetTimer(StaggerHandle, this, &ABCharacter3rd::ChangeCurrentStatePreviousCharacterState, .5f, false);
			break;
		}
	case ECharacterState::Dead:
		{
			CurrentCharacterState = ECharacterState::Dead;

			GetCharacterMovement()->SetMovementMode(MOVE_None);
			GetCharacterMovement()->MaxWalkSpeed = 0.0f;
			bCanCrouch = false;
			bCanDodge = false;
			bCrouchSkipCapsuleChange = false;
			bCanHeal = false;
			bHealTick = false;
			bInvincible = false;
			bCanAttack = false;
			bCanParry = false;

			GameMode->GameEndLoss();
			
			UE_LOG(LogTemp, Warning, TEXT("Player -> Dead"));
			break;
		}
	}
}


void ABCharacter3rd::ChangeCapsule(ECharacterState State)
{
	switch (State)
	{
	case ECharacterState::Dead:
		{ break; }
	case ECharacterState::Parry:
		{ break; }
	case ECharacterState::Attack:
		{ break; }
	case ECharacterState::Heal:
		{ break; }
	case ECharacterState::Staggered:
		{ break; }
	case ECharacterState::Dodge:
		{}
	case ECharacterState::Crouch:
		{
			GetCapsuleComponent()->SetCapsuleSize(CrouchCapsuleRadius, CrouchCapsuleHeight);		
			GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, GetMesh()->GetRelativeLocation().Z + CrouchOffset));
			const FVector CapsuleLocation = GetCapsuleComponent()->GetRelativeLocation();
			GetCapsuleComponent()->SetRelativeLocation(FVector(CapsuleLocation.X, CapsuleLocation.Y, CapsuleLocation.Z - CrouchOffset));
			break;
		}
	case ECharacterState::NonCombat:
		{}
	case ECharacterState::Combat:
		{
			if (CurrentCharacterState == ECharacterState::Crouch || CurrentCharacterState == ECharacterState::Dodge)
			{
				FHitResult CSResult;
				FCollisionQueryParams CSQueryP;
				CSQueryP.TraceTag = "TraceTag";
				FCollisionResponseParams CSResponseP;
				if (!GetWorld()->LineTraceSingleByChannel(CSResult, GetActorLocation(), (GetActorLocation() + (GetActorRotation().Vector().UpVector * CrouchStandLineLenght)), ECollisionChannel::ECC_Visibility, CSQueryP, CSResponseP))
				{
					// UE_LOG(LogTemp, Warning, TEXT("CAN STAND"));
					CanStandUpCapsuleChange();
				}
				else
				{
					// check for bStuck to prevent multiple checks
					if (!bStuck)
					{
						if (CurrentCharacterState == ECharacterState::Dodge) { ChangeCurrentStateCrouch(); }
						bStuck = true;
						CanStandUpDelay();
					}
				}
			}
			break;
		}
	}
}

void ABCharacter3rd::CanStandUpDelay()
{
	StuckLocation = GetCharacterMovement()->GetActorLocation();
	FTimerHandle CanStandUpHandle;
	GetWorldTimerManager().SetTimer(CanStandUpHandle, this, &ABCharacter3rd::CanStandUp, StandUpDelay, false);
}

void ABCharacter3rd::CanStandUp()
{
	FVector PlayerPos = GetCharacterMovement()->GetActorLocation();
	// if the location is different from the previous location check for collisions
	if (PlayerPos.X != StuckLocation.X || PlayerPos.Y != StuckLocation.Y)
	{
		FHitResult CSResult;
		FCollisionQueryParams CSQueryP;
		CSQueryP.TraceTag = "TraceTag";
		FCollisionResponseParams CSResponseP;
		if (!GetWorld()->LineTraceSingleByChannel(CSResult, GetActorLocation(), (GetActorLocation() + (GetActorRotation().Vector().UpVector * CrouchStandLineLenght)), ECollisionChannel::ECC_Visibility, CSQueryP, CSResponseP))
		{
			bStuck = false;
			// UE_LOG(LogTemp, Warning, TEXT("CAN FINALLY STAND"));
			CanStandUpCapsuleChange();
		}
		else
		{
			CanStandUpDelay();
		}
	}
	else
	{
		CanStandUpDelay();
	}
}

void ABCharacter3rd::CanStandUpCapsuleChange()
{
	// UE_LOG(LogTemp, Warning, TEXT("Standing up"));
	GetCapsuleComponent()->SetCapsuleSize(CapsuleRadius, CapsuleHeight);
	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, GetMesh()->GetRelativeLocation().Z - CrouchOffset));
	const FVector CapsuleLocation = GetCapsuleComponent()->GetRelativeLocation();
	GetCapsuleComponent()->SetRelativeLocation(FVector(CapsuleLocation.X, CapsuleLocation.Y, CapsuleLocation.Z + CrouchOffset));
	
	SetCharacterState(PreviousCharacterState);
}

bool ABCharacter3rd::CharacterDamage(float Damage)
{
	if (bInvincible) { return false; }
	if (bHealTick) { bHealTick = false; }
	if (CurrentHealth - Damage > 0)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitSound, GetActorLocation(), 1, 1);
		
		CurrentHealth -= Damage;
		if (CurrentCharacterState != ECharacterState::Dodge && CurrentCharacterState != ECharacterState::Crouch) { ChangeCurrentStateStaggered(); }
	} else
	{
		CurrentHealth = 0;
		ChangeCurrentStateDead();
	}
	return true;
}

void ABCharacter3rd::WeaponIndexReset() const { CharacterWeapon->ResetAttackIndex(); }

void ABCharacter3rd::WeaponSocketAttachment(FName SocketName) const
{
	if (CharacterWeapon->GetAttachParentSocketName() == SocketName) {return;}
	UNiagaraFunctionLibrary::SpawnSystemAttached(WeaponSwitchEffect, GetMesh(), CharacterWeapon->GetAttachParentSocketName(),
										FVector(0.0f, 0.0f, 0.0f), FRotator(0.0f, 0.0f, 0.0f),
										EAttachLocation::KeepRelativeOffset, true, true);
	
	const FAttachmentTransformRules AttachRules(EAttachmentRule::KeepRelative, true);
	CharacterWeapon->AttachToComponent(GetMesh(), AttachRules, SocketName);
}

void ABCharacter3rd::RecoverPower()
{
	if (!CharacterWeapon->GetIsEmpowered() && CurrentPower < MaxPower)
	{
		if (CurrentPower + PowerRecovery > MaxPower) { CurrentPower = MaxPower; }
		else { CurrentPower += PowerRecovery; }
	}
}


// Called to bind functionality to input
void ABCharacter3rd::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ABCharacter3rd::SetCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ABCharacter3rd::SetUnCrouch);

	PlayerInputComponent->BindAction("Dodge", IE_Pressed, this, &ABCharacter3rd::SetDodge);

	PlayerInputComponent->BindAxis("MoveForward", this, &ABCharacter3rd::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ABCharacter3rd::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	PlayerInputComponent->BindAction("Heal", IE_Pressed, this, &ABCharacter3rd::Heal);

	PlayerInputComponent->BindAction("Draw/Sheath", IE_Pressed, this, &ABCharacter3rd::DrawSheatheWeapon);
	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &ABCharacter3rd::Attack);
	PlayerInputComponent->BindAction("Parry", IE_Pressed, this, &ABCharacter3rd::Parry);
	PlayerInputComponent->BindAction("Empower", IE_Pressed, this, &ABCharacter3rd::EmpowerWeapon);

	PlayerInputComponent->BindAction("Pause", IE_Pressed, this, &ABCharacter3rd::PauseGame).bExecuteWhenPaused = true;
}

void ABCharacter3rd::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ABCharacter3rd::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) )
    	{
    		// find out which way is right
    		const FRotator Rotation = Controller->GetControlRotation();
    		const FRotator YawRotation(0, Rotation.Yaw, 0);
    	
    		// get right vector 
    		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
    		// add movement in that direction
    		AddMovementInput(Direction, Value);
    	}
}

void ABCharacter3rd::SetCrouch()
{
	if (bCanCrouch && GetCharacterMovement()->IsMovingOnGround())
	{
		ChangeCurrentStateCrouch();
	}
}

void ABCharacter3rd::SetUnCrouch()
{

	if (CurrentCharacterState == ECharacterState::Crouch)
	{
		ChangeCapsule(ECharacterState::NonCombat);
	}
}

void ABCharacter3rd::SetDodge()
{
	if (bCanDodge && GetCharacterMovement()->IsMovingOnGround() && CurrentStamina > 0)
	{
		// reset stamina timer handle, to prevent stamina regeneration when dodge is being used
		GetWorldTimerManager().ClearTimer(StaminaRegenHandle);
		
		bStaminaTick = false;
		if (CurrentStamina - DodgeCost < 0)
		{
			CurrentStamina = 0;
			GetWorldTimerManager().SetTimer(StaminaRegenHandle, this, &ABCharacter3rd::SetStaminaTickTrue, 4.f, false);
		} else
		{
			CurrentStamina -= DodgeCost;
			GetWorldTimerManager().SetTimer(StaminaRegenHandle, this, &ABCharacter3rd::SetStaminaTickTrue, 1.1f, false);
		}
		if (CurrentCharacterState == ECharacterState::Heal) { bHealTick = false; }
		ChangeCurrentStateDodge();
	}
}

void ABCharacter3rd::SetUnDodge()
{
	ChangeCapsule(PreviousCharacterState);
}

void ABCharacter3rd::Heal()
{
	if (CurrentHealth == MaxHealth) { return; }
	if (bCanHeal && GetCharacterMovement()->IsMovingOnGround() && CurrentPower >= HealCost)
	{
		// UE_LOG(LogTemp, Warning, TEXT("CAN HEAL"));
		CurrentPower -= HealCost;
		ChangeCurrentStateHeal();
	}
	else
	{
		// UE_LOG(LogTemp, Warning, TEXT("CANNOT HEAL"));
	}
}

void ABCharacter3rd::DrawSheatheWeapon()
{
	if (GetCharacterMovement()->IsMovingOnGround())
	{
		if (CurrentCharacterState == ECharacterState::NonCombat) { ChangeCurrentStateCombat(); }
		else if (CurrentCharacterState == ECharacterState::Combat) { ChangeCurrentStateNonCombat(); }
	}
}

void ABCharacter3rd::Attack()
{
	if (bCanAttack && GetCharacterMovement()->IsMovingOnGround())
	{
		ChangeCurrentStateAttack();
		CharacterWeapon->SwordAttack();
	}
}

void ABCharacter3rd::Parry()
{
	if (bCanParry && GetCharacterMovement()->IsMovingOnGround())
	{
		ChangeCurrentStateParry();
		CharacterWeapon->SwordParry();
	}
}

void ABCharacter3rd::EmpowerWeapon()
{
	if (CurrentCharacterState == ECharacterState::Combat && CurrentPower >= EmpoweredAttackCost && !CharacterWeapon->GetIsEmpowered())
	{
		UE_LOG(LogTemp, Warning, TEXT("EMPOWER WEAPON"));
		CurrentPower -= EmpoweredAttackCost;
		CharacterWeapon->EmpowerSword();
	}
}

void ABCharacter3rd::PauseGame()
{
	// UE_LOG(LogTemp, Warning, TEXT("Game Paused: %d"), UGameplayStatics::IsGamePaused(GetWorld()));
	if (UGameplayStatics::IsGamePaused(GetWorld()))
	{
		UE_LOG(LogTemp, Warning, TEXT("-GAME UNPAUSED-"));

		PauseMenu->RemoveFromViewport();

		UGameplayStatics::GetPlayerController(GetWorld(), 0)->SetInputMode(FInputModeGameOnly());
		UGameplayStatics::GetPlayerController(GetWorld(), 0)->SetShowMouseCursor(false);
		
		UGameplayStatics::SetGamePaused(GetWorld(), false);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("-GAME PAUSED-"));

		PauseMenu->AddToViewport();
		
		UGameplayStatics::GetPlayerController(GetWorld(), 0)->SetInputMode(FInputModeUIOnly());
		UGameplayStatics::GetPlayerController(GetWorld(), 0)->SetShowMouseCursor(true);
		
		UGameplayStatics::SetGamePaused(GetWorld(), true);
	}
	
}
