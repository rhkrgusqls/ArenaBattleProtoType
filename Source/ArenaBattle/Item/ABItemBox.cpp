// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ABItemBox.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Interface/ABCharacterItemInterface.h"
#include "Engine/AssetManager.h"

// Sets default values
AABItemBox::AABItemBox()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Trigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Trigger"));
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Effect = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Effect"));

	RootComponent = Trigger;
	Mesh->SetupAttachment(Trigger);
	Effect->bAutoActivate = false;
	Effect->SetupAttachment(Trigger);

	Trigger->SetCollisionProfileName(TEXT("ABTrigger"));
	Trigger->SetBoxExtent(FVector(40.0f, 45.0f, 30.0f));
	Trigger->OnComponentBeginOverlap.AddDynamic(this, &AABItemBox::OnOverlapBegin);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> BoxMeshRef(TEXT("/Game/ArenaBattle/Environment/Props/SM_Env_Breakables_Box1.SM_Env_Breakables_Box1"));
	if (BoxMeshRef.Object)
	{
		Mesh->SetStaticMesh(BoxMeshRef.Object);
		Mesh->SetCollisionProfileName(TEXT("NoCollision"));
	}

	static ConstructorHelpers::FObjectFinder<UParticleSystem> EffectRef(TEXT("/Game/ArenaBattle/Effect/P_TreasureChest_Open_Mesh.P_TreasureChest_Open_Mesh"));
	if (EffectRef.Object)
	{
		Effect->SetTemplate(EffectRef.Object);
		Effect->bAutoActivate = false;
	}
}

void AABItemBox::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

	if (ItemData == nullptr)
	{
		Destroy();
		return;
	}

	Effect->Activate(true);
	Mesh->SetHiddenInGame(true);
	SetActorEnableCollision(false);

	Effect->OnSystemFinished.AddDynamic(this, &AABItemBox::OnEffectFinished);

	IABCharacterItemInterface* OverlappingActor = Cast<IABCharacterItemInterface>(OtherActor);
	if (OverlappingActor)
	{
		OverlappingActor->TakeItem(ItemData);
	}
}

void AABItemBox::OnEffectFinished(UParticleSystemComponent* Psystem)
{
	Destroy();
}

void AABItemBox::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	UAssetManager& Manager = UAssetManager::Get();
	TArray<FPrimaryAssetId> AssetIds;
	Manager.GetPrimaryAssetIdList(TEXT("ABItemData"), AssetIds);

	ensure(AssetIds.Num() > 0);

	int32 RandomIndex = FMath::RandRange(0, AssetIds.Num() - 1);
	FSoftObjectPtr AssetPtr(Manager.GetPrimaryAssetPath(AssetIds[RandomIndex]));
	if (AssetPtr.IsPending())
	{
		AssetPtr.LoadSynchronous();
	}
	ItemData = Cast<UABItemData> (AssetPtr.Get());
}


