// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "B/Weapons/BSword.h"
#include "BCharacter3rd.generated.h"

class ABSword;
class UInputComponent;
class USkeletalMeshComponent;
class USceneComponent;
class UCameraComponent;
class USpringArmComponent;
class USoundBase;
class UNiagaraSystem;
class UUserWidget;
class ABGameModeBase;

UENUM()
enum class ECharacterState : uint8
{
	NonCombat,
	Combat,
	Attack,
	Parry,
	Crouch,
	Dodge,
	Heal,
	Staggered,
	Dead
};

UCLASS(config=Game)
class B_API ABCharacter3rd : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABCharacter3rd();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	
	UFUNCTION(BlueprintCallable)
	float GetMaxHealth() const { return MaxHealth; }
	
	UFUNCTION(BlueprintCallable)
	float GetCurrentHealth() const { return CurrentHealth; }
	
	UFUNCTION(BlueprintCallable)
	float GetMaxStamina() const { return MaxStamina; }
	
	UFUNCTION(BlueprintCallable)
	float GetCurrentStamina() const { return CurrentStamina; }
	
	UFUNCTION(BlueprintCallable)
	float GetMaxPower() const { return MaxPower; }
	
	UFUNCTION(BlueprintCallable)
	float GetCurrentPower() const { return CurrentPower; }

	UFUNCTION(BlueprintCallable)
	bool GetIsInvincible() const  { return bInvincible; }
	
	UFUNCTION(BlueprintCallable, Category = CharacterState)
	ECharacterState GetCharacterState() const { return CurrentCharacterState; }
	
	UFUNCTION(BlueprintCallable, Category = CharacterState)
	ECharacterState GetCharacterPreviousState() const { return PreviousCharacterState; }
	
	/**
	 *  decreases the player health and if it reaches 0 changes the character state to Dead
	 *  @param Damage the damage that is dealt to the character
	 */
	bool CharacterDamage(float Damage);

	/** pauses or unpauses the game based on the current state of the game */
	UFUNCTION(BlueprintCallable)
	void PauseGame();

	/** increases the current power if the weapon is not empowered */
	UFUNCTION()
	void RecoverPower();
	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Called for forwards/backward input */
	void MoveForward(float Value);
	
	/** Called for side to side input */
	void MoveRight(float Value);

	/** called to heal the player */
	void Heal();

	/** changes the state of the character and the size of the capsule */
	void SetCrouch();
	/** removes the changes made by SetCrouch and returns the player state to PreviousState */
	void SetUnCrouch();

	/** changes the state of the character, the size of the capsule and gives the character invulnerability */
	void SetDodge();
	/** removes the changes made by SetDodge and returns the player state to PreviousState */
	void SetUnDodge();

	/** sheathes and unsheathes the character's weapon */
	void DrawSheatheWeapon();
	
	/** character attack */
	void Attack();
	
	/** character parry */
	void Parry();

private:
	/** camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	UPROPERTY(EditAnywhere, Category = Mesh)
	USkeletalMeshComponent* SkeletalMesh;
	
	UPROPERTY(EditAnywhere, Category = Weapon)
	TSubclassOf<ABSword> WeaponObject;
	UPROPERTY(VisibleAnywhere, Category = Weapon)
	ABSword* CharacterWeapon;

	UPROPERTY(VisibleAnywhere, Category = Weapon)
	UNiagaraSystem* WeaponSwitchEffect;
	
	void EmpowerWeapon();

	
	UPROPERTY()
	float CapsuleRadius = 42.f; 
	UPROPERTY()
	float CapsuleHeight = 91.f;

	UPROPERTY()
	float CrouchCapsuleRadius = 42.f; 
	UPROPERTY()
	float CrouchCapsuleHeight = 70.f;
	/** offset to use to change the position of the mesh and the capsule */
	UPROPERTY()
	float CrouchOffset = 21.f;
	/** size of the raycast to check for collisions when increasing the size of the capsule */
	UPROPERTY()
	float CrouchStandLineLenght = 115.f;
	
	
	float MaxHealth = 100.f;
	UPROPERTY(VisibleAnywhere)
	float CurrentHealth;

	float MaxStamina = 100.f;
	UPROPERTY(VisibleAnywhere)
	float CurrentStamina;

	float MaxPower = 99.f;
	UPROPERTY(VisibleAnywhere)
	float CurrentPower;
	float PowerRecovery = 10.f;
	
	float HealCost = 49.5f;
	float DodgeCost = 45.f;
	float EmpoweredAttackCost = 66.f ;

	UPROPERTY(VisibleAnywhere)
	USoundBase* HitSound;
		

	UPROPERTY(VisibleAnywhere)
	ECharacterState CurrentCharacterState;

	UPROPERTY(VisibleAnywhere)
	ECharacterState PreviousCharacterState;
	
	/**
	 * changes the character state
	 * @param State the new character state
	 */
	void SetCharacterState(ECharacterState State);

	/**
	 * changes the size and the position of the capsule, and the position of the mesh, depending on the State
	 * @param State the state to which the capsule will need to be changed
	 */
	void ChangeCapsule(ECharacterState State);

	// functions and variables to use with change capsule
	bool bStuck = false;
	FVector StuckLocation;
	float StandUpDelay = 0.25f;
	/** delays CanStandUp when a collision is detected */
	void CanStandUpDelay();
	/** checks if there is still a collision */
	void CanStandUp();
	/** change the capsule when */
	void CanStandUpCapsuleChange();

	// functions to change the current state of the character - used with world timers
	void ChangeCurrentStatePreviousCharacterState() { SetCharacterState(PreviousCharacterState); }
	UFUNCTION()
	void ChangeCurrentStateNonCombat() { SetCharacterState(ECharacterState::NonCombat); }
	UFUNCTION()
	void ChangeCurrentStateCombat() { SetCharacterState(ECharacterState::Combat); }
	UFUNCTION()
	void ChangeCurrentStateAttack() { SetCharacterState(ECharacterState::Attack); }
	UFUNCTION()
	void ChangeCurrentStateParry() { SetCharacterState(ECharacterState::Parry); }
	UFUNCTION()
	void ChangeCurrentStateCrouch() { SetCharacterState(ECharacterState::Crouch); }
	UFUNCTION()
	void ChangeCurrentStateDodge() { SetCharacterState(ECharacterState::Dodge); }
	UFUNCTION()
	void ChangeCurrentStateHeal() { SetCharacterState(ECharacterState::Heal); }
	UFUNCTION()
	void ChangeCurrentStateStaggered() { SetCharacterState(ECharacterState::Staggered); }
	UFUNCTION()
	void ChangeCurrentStateDead() { SetCharacterState(ECharacterState::Dead); }
	
	// State Permissions 
	bool bCanCrouch = true;
	UPROPERTY(VisibleAnywhere)
	bool bCanDodge = true;
	void SetCanDodgeTrue() { bCanDodge = true; }
	bool bCrouchSkipCapsuleChange = false;
	bool bCanHeal = true;
	bool bHealTick = false;
	bool bStaminaTick = true;
	void SetStaminaTickTrue() { bStaminaTick = true; }
	FTimerHandle StaminaRegenHandle;
	UPROPERTY(VisibleAnywhere)
	bool bInvincible = false;
	void SetInvincibleFalse() { bInvincible = false; }
	bool bCanAttack = true;
	void SetCanAttackTrue() { bCanAttack = true; }
	FTimerHandle CanAttackHandle;
	FTimerHandle AttackIndexResetHandle;
	void WeaponIndexReset() const;
	bool bCanParry = false;
	void SetCanParryTrue() { bCanParry = true; }

	/**
	 * changes the socket that the weapon is attached to
	 * @param SocketName name of the socket:
	 * - back - WeaponBackSocket
	 * - hand - WeaponHandSocket
	 */
	UFUNCTION()
	void WeaponSocketAttachment(FName SocketName) const;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> PauseWidget;
	
	UPROPERTY()
	UUserWidget* PauseMenu;

	UPROPERTY()
	ABGameModeBase* GameMode;
};
