// Fill out your copyright notice in the Description page of Project Settings.


#include "FGPathfinder.h"
#include "FGAI_2/Grid/FGGridActor.h"
#include "DrawDebugHelpers.h"

// Sets default values for this component's properties
UFGPathfinder::UFGPathfinder()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

//This method is called by some additions to your sphere drawer blueprint. Check it out, it is ugly!
void UFGPathfinder::StartPathfinding(int32 StartCell, int32 EndCell)
{
	Grid->ResetBoard();
	OpenList.Empty();
	ClosedList.Empty();
	Path.Empty();

	int32 CurrentCell = StartCell;

	OpenList.Add(CurrentCell);

	while (OpenList.Num() > 0)
	{
		TArray<int32> CurrentNeighbours = Grid->GetNeighboursAroundCell(CurrentCell);
		for (auto TroddenTile : ClosedList)
		{
			if(CurrentNeighbours.Contains(TroddenTile))
			{
				CurrentNeighbours.Remove(TroddenTile);
			}
		}
		for (int32 CurrentNeighbour : CurrentNeighbours)
		{
			//This used to be very wrong for a long while, being g(current) + F(current to end) + F(Neighbour to end)
			int32 NeighborCurrentCost = Grid->TileList[CurrentCell].Weight + Grid->GetHValue(CurrentCell, Grid->TileList[CurrentCell].ParentId);
			if(OpenList.Contains(CurrentNeighbour))
			{
				if(Grid->TileList[CurrentNeighbour].Weight <= NeighborCurrentCost)
				{
					continue;
				}
				else if(ClosedList.Contains(CurrentNeighbour))
				{
					if(Grid->GetFValue(CurrentNeighbour, EndCell) <= NeighborCurrentCost)
					{
						continue;
					}
					ClosedList.Remove(CurrentNeighbour);
					OpenList.Add(CurrentNeighbour);
				}
			}
			else
			{
				OpenList.Add(CurrentNeighbour);
			}
			Grid->TileList[CurrentNeighbour].Weight = NeighborCurrentCost;
			Grid->TileList[CurrentNeighbour].ParentId = CurrentCell;
		}
		ClosedList.Add(CurrentCell);
		OpenList.Remove(CurrentCell);

		int32 F = INT_MAX;
		for (int32 Index : OpenList)
		{
			if(Grid->GetFValue(Index, EndCell) < F)
			{
				CurrentCell = Index;
				F = Grid->GetFValue(Index, EndCell);
			}
		}
		if(CurrentCell == EndCell)
		{
			UE_LOG(LogTemp, Log, TEXT("We found the end!"));
			
			break;
		}
	}
	if(CurrentCell != EndCell)
	{
		UE_LOG(LogTemp, Log, TEXT("Couldn't find end"));
		
	}
	//Terrible, dangerous way to add the path in its own list,
	//Has caused several infinite loops during development.
	while(CurrentCell != StartCell)
    {
		//If I ever find that the cell after this one already exists in the path list,
		//Then something has gone terribly wrong and we abort the process
    	if(Path.Contains(Grid->TileList[CurrentCell].ParentId))
    	{
    		UE_LOG(LogTemp, Log, TEXT("The path has somehow split, that's not good"))
    		break;
    	}
    	Path.Add(CurrentCell);
		UE_LOG(LogTemp,Log, TEXT("%d, %d"), CurrentCell, Grid->TileList[CurrentCell].Weight );
    	CurrentCell = Grid->TileList[CurrentCell].ParentId;
    }
	//this is a remnant when I only drew the path in between the start and end, and when I
	//switched rendering method I couldn't/didn't bother to find a way to add the last cell
	//while inside the while loop
	Path.Add(CurrentCell);

	//Old rendering method before switching to tick render, alongside aborted attempt
	//To draw every element one step at a time with a timed delay in between
	//
	//Is there an easy way to iterate thru a list in UE4 with a pause between iteration?
	//I could have done this easily with a Coroutine in Unity, is there a UE equivalent?
	
	//UE_LOG(LogTemp,Log, TEXT("%d, %d"), CurrentCell, Grid->TileList[CurrentCell].Weight );
	//int32 X;
	//int32 Y;
	//Grid->GetXYFromIndex(StartCell, X, Y);
	//DrawDebugSphere(GetWorld(), Grid->GetWorldLocationFromXY(X, Y), 100.0f, 12, FColor::Yellow);
	//Grid->GetXYFromIndex(EndCell, X, Y);
	//DrawDebugSphere(GetWorld(), Grid->GetWorldLocationFromXY(X, Y), 100.0f, 12, FColor::Green);
	//for (auto PathTile : Path)
	//{
	//	Grid->GetXYFromIndex(PathTile, X, Y);
	//	DrawDebugSphere(GetWorld(), Grid->GetWorldLocationFromXY(X, Y), 100.0f, 12, FColor::Cyan, false, 10.0f);
	//	//Grid->ResetBoard();
	//}
	/*for (size_t i = 0; i < ClosedList.Num(); i++)
	{
		GetOwner()->GetWorldTimerManager().SetTimer(DrawClosedListHandle, this, &UFG);
	}*/
}


// Called when the game starts
void UFGPathfinder::BeginPlay()
{
	Super::BeginPlay();
	Grid = Cast<AFGGridActor>(GetOwner());
}


void UFGPathfinder::DrawSphere(int32 tileId, FLinearColor Color) //Courtesy of Esteban Morales via Johan Liljedahl
{
	FVector drawPosition = Grid->GetWorldLocationFromIndex(tileId);
	FVector newDrawPosition = FVector(drawPosition.X, drawPosition.Y, 100);
	const FColor constColor = Color.ToFColor(true);
	if (Grid->TileList[tileId].ParentId)
	{
		FVector parentDrawPosition = Grid->GetWorldLocationFromIndex(Grid->TileList[tileId].ParentId);
		FVector newparentDrawPosition = FVector(parentDrawPosition.X, parentDrawPosition.Y, 100);
		DrawDebugDirectionalArrow(GetWorld(), newDrawPosition, newparentDrawPosition, 5200, constColor, false, -1, 0, 20);
	}
	DrawDebugSphere(GetWorld(), drawPosition, 100.0f, 12, constColor, false, 0.0f, 0, 0);

}

// Called every frame
void UFGPathfinder::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//Right now I render everything from every list, and counting on the fact that
	//drawn debugs render over each other when called later.
	//A cheaper way would be to render first the path from start to finish, then
	//the evaulated tiles in the closed list except for the ones also in path.
	//Open can be rendererd at any time
	
	
	for (auto Index : OpenList)
	{
		DrawSphere(Index, FColor::Black);
	}
	for (auto Index : ClosedList)
	{
		DrawSphere(Index, FColor::Red);
	}
	for (auto Index : Path)
	{
		DrawSphere(Index, FColor::Blue);
	}
}

