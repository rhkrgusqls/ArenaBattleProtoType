// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ABStageGimmick.generated.h"

DECLARE_DELEGATE(FStageChangeDelegate);

USTRUCT()
struct FStageChangeDelegateWrapper
{
	GENERATED_BODY()

	FStageChangeDelegateWrapper(){}
	FStageChangeDelegateWrapper(const FStageChangeDelegate& InDelegate) : StageDelegate(InDelegate) {}

	FStageChangeDelegate StageDelegate;
};

UENUM(BlueprintType)
enum class EStageState : uint8
{
	READY = 0,
	FIGHT,
	REWARD,
	NEXT
};

UCLASS()
class ARENABATTLE_API AABStageGimmick : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AABStageGimmick();

	FORCEINLINE int32 GetStaeLevel() const { return CurrentStageLevel; }
	FORCEINLINE void SetStageLevel(int32 InStageLevel) { CurrentStageLevel = InStageLevel; }

protected:

	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION()
	void OnStageTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnGateTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void SetState(EStageState InNewState);
	void SetReady();
	void SetFight();
	void SetReward();
	void SetNext();

	void OpenAllGate();
	void CloseAllGate();

protected:
	UPROPERTY(VisibleAnyWhere, Category = State)
	TObjectPtr<class UStaticMeshComponent> Stage;

	UPROPERTY(VisibleAnyWhere, Category = State)
	TObjectPtr<class UBoxComponent> StageTrigger;

	UPROPERTY(VisibleAnyWhere, Category = Gate)
	TMap < FName, TObjectPtr<class UStaticMeshComponent>> Gates;

	UPROPERTY(VisibleAnyWhere, Category = Gate)
	TArray<TObjectPtr<class UBoxComponent>> GateTriggers;

	UPROPERTY(EditAnywhere, Category = Stage)
	EStageState CurrentState;

	UPROPERTY(EditAnywhere, Category = Stage)
	TMap<EStageState, FStageChangeDelegateWrapper> StateChangeActions;

	TSubclassOf<class AABCharacterNonPlayer> OpponentClass;		// 넣어줄 수 있는 데이터 타입을 <>로 강제하고 싶을때 사용(클래스를 지정)

	FTimerHandle OpponentSpawnTimer;

	UPROPERTY(EditAnywhere, Category = Fight)
	float OpponentSpawnTime;

	void OnOpponentSpawn();

	UFUNCTION()
	void OnOpponentDestroyed(AActor* DestroyedActor);

	UPROPERTY(VisibleAnywhere, Category = Reward)
	TSubclassOf<class AABItemBox> RewardBoxClass;

	UPROPERTY(VisibleAnywhere, Category = Reward)
	TArray<TWeakObjectPtr<class AABItemBox>> RewardBoxes;	// 약참조

	TMap<FName, FVector> RewardBoxLocations;

	void SpawnRewardBoxes();

	UFUNCTION()
	void OnRewardBoxTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(VisibleInstanceOnly, Category = Stat)
	int32 CurrentStageLevel;


	//TSubclassOf<class AABItemBox> ItemBoxClass;
	//TArray<TSubclassOf<class AABItemBox>> ItemBoxClasses;

	/*FTimerHandle ItemBoxSpawnTimer;

	UPROPERTY(EditAnywhere, Category = Reward)
	float ItemBoxSpawnTime;
	
	void OnItemBoxSpawn();

	UFUNCTION()
	void OnItemBoxDestroyed(AActor* DestroyedActor);*/
};
