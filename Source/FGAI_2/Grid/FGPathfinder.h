// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FGPathfinder.generated.h"

class AFGGridActor;
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FGAI_2_API UFGPathfinder : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UFGPathfinder();
	UFUNCTION(BlueprintCallable, Category="Grid")
	void StartPathfinding(int32 StartCell, int32 EndCell);
	void DrawSphere(int tileId, FLinearColor Color);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	UPROPERTY(Transient)
	AFGGridActor* Grid;
	
	TArray<int32> OpenList;
	TArray<int32> ClosedList;
	TArray<int32> Path;

	FTimerHandle DrawClosedListHandle;
	FTimerHandle DrawPathHandle;
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
