// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "B/Character/BCharacter3rd.h"
#include "Components/BoxComponent.h"
#include "BBoss.generated.h"

class ABCharacter3rd;
class UBoxComponent;
class UNiagaraSystem;
class USoundBase;
class ABBossFists;
class ABBossSword;
class ABGameModeBase;

UENUM()
enum class EBossState : uint8
{
	Rest,
	QuickRest,
	Wander,
	Follow,
	Charge,
	Attack,
	Staggered,
	Dead
};

UCLASS()
class B_API ABBoss : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABBoss();
	
	UFUNCTION(BlueprintCallable)
	float GetMaxHealth() const { return MaxHealth; }
	
	UFUNCTION(BlueprintCallable)
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintCallable)
	float GetBossMinDamage() const { return BossMinDamage; }

	UFUNCTION(BlueprintCallable)
	float GetBossBaseDamage() const { return BossBaseDamage; }

	UFUNCTION(BlueprintCallable)
	float GetBossInterDamage() const { return BossInterDamage; }

	
	UFUNCTION(BlueprintCallable, Category = CharacterState)
	EBossState GetBossState() const { return CurrentBossState; }

	
	UFUNCTION(BlueprintCallable)
	ABBossFists* GetBossWeaponFists() const { return BossFistsWeapon; }

	UFUNCTION(BlueprintCallable)
	ABBossSword* GetBossWeaponSword() const { return BossSwordWeapon; }
	
	UFUNCTION(BlueprintCallable)
	bool GetBossSwordEquipped() const { return bBossSwordEquipped; }
	UFUNCTION(BlueprintCallable)
	float GetCurrentAttackID() const { return CurrentAttackID; }

	/** makes the boss move forward a certain amount */
	void AttackDashForward();

	/** makes the character move toward the right or the  left depending on a random number */
	void AttackSideStep();

	void SetBossStateRest() { ChangeCurrentStateRest(); }
	void SetBossStateStaggered() { ChangeCurrentStateStaggered(); }

	
	void LookAtPlayer();

	/**
	 * decreases the boss' health and if it reaches 0 changes the boss' state to Dead
	 * @param Value the value to be deducted from the boss' health
	 */
	void BossTakeDamage(float Value);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	AAIController* BossController;
	
	UPROPERTY(EditAnywhere, Category = Mesh)
	USkeletalMeshComponent* SkeletalMesh;

	
	UPROPERTY(EditAnywhere, Category = Weapon)
	TSubclassOf<ABBossSword> SwordObject;
	UPROPERTY(VisibleAnywhere, Category = Weapon)
	ABBossSword* BossSwordWeapon;

	UPROPERTY(EditAnywhere, Category = Weapon)
	TSubclassOf<ABBossFists> FistsObject;
	UPROPERTY(VisibleAnywhere, Category = Weapon)
	ABBossFists* BossFistsWeapon;
	
	UPROPERTY(VisibleAnywhere)
	bool bBossSwordEquipped =  false;
	UPROPERTY(VisibleAnywhere)
	uint16 CurrentAttackID = 0;

	
	UPROPERTY(VisibleAnywhere, Category = Effects)
	UNiagaraSystem* Phase3EyeL;

    UPROPERTY(VisibleAnywhere, Category = Effects)
    UNiagaraSystem* Phase3EyeR;
	
	UPROPERTY()
	ABCharacter3rd* BCharacter;

	float CapsuleRadius = 115.f;
	float CapsuleHeight = 182.f;
	
	UPROPERTY(VisibleAnywhere)
	EBossState CurrentBossState = EBossState::Staggered;
	
	/**
	 * changes the boss state
	 * @param State the new boss state
	 */
	void SetBossState(EBossState State);

	// functions to change the current state of the boss - used with world timers
	void ChangeCurrentStateRest() { SetBossState(EBossState::Rest); }
	void ChangeCurrentStateQuickRest() { SetBossState(EBossState::QuickRest); }
	void ChangeCurrentStateWander() { SetBossState(EBossState::Wander); }
	void ChangeCurrentStateFollow() { SetBossState(EBossState::Follow); }
	void ChangeCurrentStateCharge() { SetBossState(EBossState::Charge); }
	void ChangeCurrentStateAttack() { SetBossState(EBossState::Attack); }
	void ChangeCurrentStateStaggered() { SetBossState(EBossState::Staggered); }
	void ChangeCurrentStateDead() { SetBossState(EBossState::Dead); }

	FTimerHandle AttackRestHandle;
	
	/** collider used with the charge attack */
	UPROPERTY()
	UBoxComponent* BoxCollider;

	/** collider used to check for parries */
	UPROPERTY()
	UBoxComponent* ParryCollider;

	UPROPERTY(VisibleAnywhere)
	float CurrentDistanceToPlayer;

	UPROPERTY(VisibleAnywhere)
	float DistanceOffSet = 0.f;
	
	float ChargeDistance = 1500.f;
	float MaxDistance = 700.f;
	float Distance = 500.f;

	bool Charging = false;
	bool ChargeCanDamage = false;

	float BossBaseSpeed = 350.f;
	float BossPhase3Speed = 550.f;
	float BossAttackSpeed = 850.f;
	float BossChargeSpeed = 7500.f;
	float BossPhase3ChargeSpeed = 10500.f;
	
	
    float MaxHealth = 800.f;
    UPROPERTY(VisibleAnywhere)
	float CurrentHealth;
	float Phase2CutOff = 550.f;
	float Phase3CutOff = 300.f;

    float BossMinDamage = 15.f;
	float BossBaseDamage = 25.f;
	float BossInterDamage = 35.f;
	float BossMaxDamage = 50.f;

	/** 0 - phase 1 / 1 - phase 2 / 2 - phase 3 */
	UPROPERTY(VisibleAnywhere)
	uint8 CurrentPhase = 0;

	/** 0 - fists / 1 - sword */
	UPROPERTY(VisibleAnywhere)
	uint8 CurrentWeapon = 0;
	
	// rest times
	/** Phase 1 and 2 rest times */
	float NormalRest = 4.f;
	/** Phase 1 and 2 rest times for combo attacks */
	float QuickRest = .7f;
	/** Phase 3 rest times */
	float Phase3Rest = 3.f;
	/** Phase 3 rest times for combo attacks */
	float Phase3QuickRest = .3f;

	
	UPROPERTY(VisibleAnywhere)
	bool bSideStepDash = false;

	UPROPERTY(VisibleAnywhere)
	bool bChargeFlurry = false;
	UPROPERTY(VisibleAnywhere)
	uint8 ChargeFlurryCount = 0;
	UPROPERTY(VisibleAnywhere)
	USoundBase* ChargeFlurrySound;

	UPROPERTY(VisibleAnywhere)
	USoundBase* ParrySound;

	UPROPERTY(VisibleAnywhere)
	USoundBase* SlashSound;

	FVector BelowMapLocation = FVector(0.f, 0.f, -718.f);
	FVector PreviousBossLocation;
	

	// decides on what to do based on the distance to the player
	void NextDecision();

	UFUNCTION(BlueprintCallable)
	void MoveToPlayer();

	/**
	 * Generates a random position around the parameter entered
	 * @param Location the location from which the random position is going to be searched
	 */
	FVector GetRandomPositionFrom(FVector Location);

	void MoveToRandom();
	
	void CheckVelocity();

	
	void ChargePlayer();
	void ChargeDelay();


	void ChargeFlurryAttack();


	UFUNCTION()
	void OnBeginOverlap(class UPrimitiveComponent* HitComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult);
	
	/**
	 * changes the socket that the weapon is attached to
	 * @param SocketName name of the socket:
	 * - hidden - BossSwordHideSocket
	 * - hand - BossSwordHandSocket
	 */
	UFUNCTION()
	void WeaponSocketAttachment(FName SocketName) const;

	UPROPERTY()
	ABGameModeBase* GameMode;
	
};
