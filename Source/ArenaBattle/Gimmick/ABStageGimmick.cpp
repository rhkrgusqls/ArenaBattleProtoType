// Fill out your copyright notice in the Description page of Project Settings.


#include "Gimmick/ABStageGimmick.h"
#include "Components/BoxComponent.h"
#include "Character/ABCharacterNonPlayer.h"
#include "Item/ABItemBox.h"

// Sets default values
AABStageGimmick::AABStageGimmick()
{
	Stage = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Stage"));
	RootComponent = Stage;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> StageMeshRef(TEXT("/Game/ArenaBattle/Environment/Stages/SM_SQUARE.SM_SQUARE"));
	if (StageMeshRef.Object)
	{
		Stage->SetStaticMesh(StageMeshRef.Object);
	}


	StageTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("StageTrigger"));
	StageTrigger->SetupAttachment(Stage);
	StageTrigger->SetBoxExtent(FVector(775, 775, 300));
	StageTrigger->SetRelativeLocation(FVector(0, 0, 250));
	StageTrigger->SetCollisionProfileName(TEXT("ABTrigger"));
	StageTrigger->OnComponentBeginOverlap.AddDynamic(this, &AABStageGimmick::OnStageTriggerBeginOverlap);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> GateMeshRef(TEXT("/Game/ArenaBattle/Environment/Props/SM_GATE.SM_GATE"));
	static FName GateSockets[] = { TEXT("+XGate"), TEXT("-XGate"), TEXT("+YGate"), TEXT("-YGate") };
	for (FName GateSocket : GateSockets)
	{
		UStaticMeshComponent* Gate = CreateDefaultSubobject<UStaticMeshComponent>(GateSocket);
		Gate->SetupAttachment(Stage, GateSocket);

		if (GateMeshRef.Object)
		{
			Gate->SetStaticMesh(GateMeshRef.Object);
		}
		Gate->SetRelativeLocation(FVector(0.0f, -80.0f, 0.0f));
		Gate->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

		Gates.Add(GateSocket, Gate);

		FName TriggerName = *GateSocket.ToString().Append(TEXT("Trigger"));
		UBoxComponent* GateTrigger = CreateDefaultSubobject<UBoxComponent>(TriggerName);
		GateTrigger->SetupAttachment(Stage, GateSocket);
		GateTrigger->SetBoxExtent(FVector(85.0f, 85.0f, 300.0f));
		GateTrigger->SetRelativeLocation(FVector(0.0f, 0.0f, 250.0f));
		GateTrigger->SetCollisionProfileName(TEXT("ABTrigger"));
		GateTrigger->ComponentTags.Add(GateSocket);
		GateTrigger->OnComponentBeginOverlap.AddDynamic(this, &AABStageGimmick::OnGateTriggerBeginOverlap);

		GateTriggers.Add(GateTrigger);
		
	}

	CurrentState = EStageState::READY;
	/*StateChangeActions.Add(EStageState::READY, FStageChangeDelegateWrapper(FStageChangeDelegate::CreateUObject(this, &AABStageGimmick::SetReady)));
	StateChangeActions.Add(EStageState::FIGHT, FStageChangeDelegateWrapper(FStageChangeDelegate::CreateUObject(this, &AABStageGimmick::SetFight)));
	StateChangeActions.Add(EStageState::REWARD, FStageChangeDelegateWrapper(FStageChangeDelegate::CreateUObject(this, &AABStageGimmick::SetReward)));
	StateChangeActions.Add(EStageState::NEXT, FStageChangeDelegateWrapper(FStageChangeDelegate::CreateUObject(this, &AABStageGimmick::SetNext)));*/

	OpponentSpawnTime = 2.0f;
	OpponentClass = AABCharacterNonPlayer::StaticClass();

	RewardBoxClass = AABItemBox::StaticClass();

	for (FName GateSocket : GateSockets)
	{
		FVector BoxLocation = Stage->GetSocketLocation(GateSocket) / 3.0f * 2.0f;
		RewardBoxLocations.Add(GateSocket, BoxLocation);
	}

	CurrentStageLevel = 1;

	//ItemBoxSpawnTime = 1.0f;
	//ItemBoxClass = AABItemBox::StaticClass();
}

void AABStageGimmick::BeginPlay()
{
	Super::BeginPlay();

	StateChangeActions.Add(EStageState::READY, FStageChangeDelegateWrapper(FStageChangeDelegate::CreateUObject(this, &AABStageGimmick::SetReady)));
	StateChangeActions.Add(EStageState::FIGHT, FStageChangeDelegateWrapper(FStageChangeDelegate::CreateUObject(this, &AABStageGimmick::SetFight)));
	StateChangeActions.Add(EStageState::REWARD, FStageChangeDelegateWrapper(FStageChangeDelegate::CreateUObject(this, &AABStageGimmick::SetReward)));
	StateChangeActions.Add(EStageState::NEXT, FStageChangeDelegateWrapper(FStageChangeDelegate::CreateUObject(this, &AABStageGimmick::SetNext)));
}

void AABStageGimmick::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	SetState(CurrentState);
}

void AABStageGimmick::OnStageTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Log, TEXT("OnStageTriggerBeginOverlap"));

	SetState(EStageState::FIGHT);
}

void AABStageGimmick::OnGateTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Log, TEXT("OnGateTriggerBeginOverlap"));

	check(OverlappedComponent->ComponentTags.Num()==1)
	FName ComponentTag = OverlappedComponent->ComponentTags[0];

	FName SocketName = *ComponentTag.ToString().Left(2);

	FVector NewLocation = Stage->GetSocketLocation(SocketName);

	// 중복 예외처리
	TArray<FOverlapResult> OverlapResults;
	FCollisionQueryParams CollisionQueryParams(SCENE_QUERY_STAT(GateTrigger), false, this);
	bool IsOverlap = GetWorld()->OverlapMultiByObjectType(OverlapResults, NewLocation, FQuat::Identity,
		FCollisionObjectQueryParams::InitType::AllStaticObjects, 
		FCollisionShape::MakeSphere(800.0f), CollisionQueryParams);

#if ENABLE_DRAW_DEBUG
	DrawDebugBox(GetWorld(), NewLocation, FVector(800.0f, 800.0f, 800.0f), FColor::Red,
		false, 5.0f);
#endif

	if (IsOverlap == false)
	{
		AABStageGimmick* NewStageGimmick = GetWorld()->SpawnActor<AABStageGimmick>(NewLocation, FRotator::ZeroRotator);
		if (NewStageGimmick)
		{
			NewStageGimmick->SetStageLevel(CurrentStageLevel + 1);
		}
	}
	
}

void AABStageGimmick::SetState(EStageState InNewState)
{
	CurrentState = InNewState;
	
	if (StateChangeActions.Contains(CurrentState))
	{
		StateChangeActions[CurrentState].StageDelegate.ExecuteIfBound();
	}
	
}

void AABStageGimmick::SetReady()
{
	UE_LOG(LogTemp, Log, TEXT("SetReady"));

	// 스테이지 트리거
	StageTrigger->SetCollisionProfileName(TEXT("ABTrigger"));

	// 게이트 트리거
	for (auto GateTrigger : GateTriggers)
	{
		GateTrigger->SetCollisionProfileName(TEXT("NoCollision"));
	}
	
	// 문 여닫음
	OpenAllGate();

}

void AABStageGimmick::SetFight()
{
	UE_LOG(LogTemp, Log, TEXT("SetFight"));

	// 스테이지 트리거
	StageTrigger->SetCollisionProfileName(TEXT("NoCollision"));

	// 게이트 트리거
	for (auto GateTrigger : GateTriggers)
	{
		GateTrigger->SetCollisionProfileName(TEXT("NoCollision"));
	}

	// 문 여닫음
	CloseAllGate();

	GetWorld()->GetTimerManager().SetTimer(OpponentSpawnTimer, this,
		&AABStageGimmick::OnOpponentSpawn, OpponentSpawnTime, false);
}

void AABStageGimmick::SetReward()
{
	UE_LOG(LogTemp, Log, TEXT("SetReward"));

	// 스테이지 트리거
	StageTrigger->SetCollisionProfileName(TEXT("NoCollision"));

	// 게이트 트리거
	for (auto GateTrigger : GateTriggers)
	{
		GateTrigger->SetCollisionProfileName(TEXT("NoCollision"));
	}

	// 문 여닫음
	CloseAllGate();

	SpawnRewardBoxes();

	/*GetWorld()->GetTimerManager().SetTimer(ItemBoxSpawnTimer, this,
		&AABStageGimmick::OnItemBoxSpawn, ItemBoxSpawnTime, false);*/
	
}

void AABStageGimmick::SetNext()
{
	UE_LOG(LogTemp, Log, TEXT("SetNext"));

	// 스테이지 트리거
	StageTrigger->SetCollisionProfileName(TEXT("NoCollision"));

	// 게이트 트리거
	for (auto GateTrigger : GateTriggers)
	{
		GateTrigger->SetCollisionProfileName(TEXT("ABTrigger"));
	}

	// 문 여닫음
	OpenAllGate();

	
}

void AABStageGimmick::OpenAllGate()
{
	for (auto Gate : Gates)
	{
		Gate.Value->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	}

}

void AABStageGimmick::CloseAllGate()
{
	for (auto Gate : Gates)
	{
		Gate.Value->SetRelativeRotation(FRotator::ZeroRotator);
	}
}

void AABStageGimmick::OnOpponentSpawn()
{
	const FVector SpawnLocation = GetActorLocation() + FVector::UpVector * 100.0f;
	/*AActor* SpawnActor = GetWorld()->SpawnActor(OpponentClass, &SpawnLocation,
		&FRotator::ZeroRotator);
	AABCharacterNonPlayer* NPC = Cast<AABCharacterNonPlayer>(SpawnActor);*/

	const FTransform SpawnTransform(SpawnLocation);
	AABCharacterNonPlayer* SpawnNPC =
		GetWorld()->SpawnActorDeferred<AABCharacterNonPlayer>(OpponentClass, SpawnTransform);

	if (SpawnNPC)
	{
		SpawnNPC->OnDestroyed.AddDynamic(this, &AABStageGimmick::OnOpponentDestroyed);
		SpawnNPC->SetLevel(CurrentStageLevel);
		SpawnNPC->FinishSpawning(SpawnTransform);
	}
}

void AABStageGimmick::OnOpponentDestroyed(AActor* DestroyedActor)
{
	SetState(EStageState::REWARD);
}

void AABStageGimmick::SpawnRewardBoxes()
{
	for (auto RewardBoxLocation : RewardBoxLocations)
	{
		FVector SpawnLocation = GetActorLocation() + RewardBoxLocation.Value;
		AActor* ItemActor = GetWorld()->SpawnActor(RewardBoxClass, &SpawnLocation);
		AABItemBox* RewardBoxActor = Cast<AABItemBox>(ItemActor);

		RewardBoxActor->GetTrigger()->OnComponentBeginOverlap.AddDynamic(this, 
			&AABStageGimmick::OnRewardBoxTriggerBeginOverlap);

		RewardBoxes.Add(RewardBoxActor);
	}
}

void AABStageGimmick::OnRewardBoxTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	for (const auto& RewardBox : RewardBoxes)
	{
		if (RewardBox.IsValid())
		{
			AActor* OverlappedBox = OverlappedComponent->GetOwner();
			AABItemBox* ItemBox = RewardBox.Get();
			if (OverlappedBox != ItemBox)
			{
				ItemBox->Destroy();
			}
		}
		
	}

	SetState(EStageState::NEXT);
}









//void AABStageGimmick::OnItemBoxSpawn()
//{
//	//static FName ItemBox[] = { TEXT("WeaponBox"), TEXT("PotionBox"), TEXT("ScrollBox") };
//	
//	const FVector BoxSpawnLocation = GetActorLocation() + FVector::UpVector * 10.0f;
//	AActor* SpawnActor = GetWorld()->SpawnActor(ItemBoxClass, &BoxSpawnLocation, &FRotator::ZeroRotator);
//	AABItemBox* ITemBox = Cast<AABItemBox>(SpawnActor);
//	if (ITemBox)
//	{
//		ITemBox->OnDestroyed.AddDynamic(this, &AABStageGimmick::OnItemBoxDestroyed);
//	}
//}
//
//void AABStageGimmick::OnItemBoxDestroyed(AActor* DestroyedActor)
//{
//	SetState(EStageState::NEXT);
//}

