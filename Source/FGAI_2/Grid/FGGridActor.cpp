#include "FGGridActor.h"

#include <string>

#include "FGGridBlockComponent.h"
#include "Components/StaticMeshComponent.h"
#include "StaticMeshDescription.h"

AFGGridActor::AFGGridActor()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetupAttachment(RootComponent);
	StaticMeshComponent->SetCastShadow(false);

	BlockStaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BlockStaticMeshComponent"));
	BlockStaticMeshComponent->SetupAttachment(RootComponent);
	BlockStaticMeshComponent->SetCastShadow(false);
}

void AFGGridActor::BeginPlay()
{
	Super::BeginPlay();
}

TArray<int32> AFGGridActor::GetNeighboursAroundCell(int32 Cell)
{
	int32 CellX;
	int32 CellY;
	GetXYFromIndex(Cell, CellX, CellY);

	TArray<int32> Neighbours;
	int32 Answer;
	if (GetTileIndexFromXY(CellX + 1, CellY, Answer)&&  !TileList[Answer].bBlock)
	{
		Neighbours.Add(Answer);
	}
	if (GetTileIndexFromXY(CellX - 1, CellY, Answer) && !TileList[Answer].bBlock)
	{
		Neighbours.Add(Answer);
	}
	if (GetTileIndexFromXY(CellX, CellY+1, Answer) && !TileList[Answer].bBlock)
	{
		Neighbours.Add(Answer);
	}
	if (GetTileIndexFromXY(CellX, CellY-1, Answer) && !TileList[Answer].bBlock)
	{
		Neighbours.Add(Answer);
	}
	return Neighbours;
}

int32 AFGGridActor::GetFValue(int32 CellFrom, int32 CellTo)
{
	int32 FromX;
	int32 FromY;
	int32 ToX;
	int32 ToY;

	GetXYFromIndex(CellFrom, FromX, FromY);
	GetXYFromIndex(CellTo, ToX, ToY);
	
	return TileList[CellFrom].Weight + FMath::Abs(FromX - ToX) + FMath::Abs(FromY - ToY);
}

int32 AFGGridActor::GetHValue(int32 CellFrom, int32 CellTo)
{
	int32 FromX;
	int32 FromY;
	int32 ToX;
	int32 ToY;

	GetXYFromIndex(CellFrom, FromX, FromY);
	GetXYFromIndex(CellTo, ToX, ToY);

	return FMath::Abs(FromX - ToX) + FMath::Abs(FromY - ToY);
}

float AFGGridActor::GetHValueEuclidian(int32 CellFrom, int32 CellTo)
{
	int32 FromX;
	int32 FromY;
	int32 ToX;
	int32 ToY;

	GetXYFromIndex(CellFrom, FromX, FromY);
	GetXYFromIndex(CellTo, ToX, ToY);

	return FMath::Sqrt(FMath::Abs(FromX - ToX)^2 + FMath::Abs(FromY - ToY)^2);
}

void AFGGridActor::GetXYFromIndex(int32 Index, int32& CellX, int32& CellY)
{
	CellX = Index % Width;
	CellY = Index / Width;
}

FVector AFGGridActor::GetWorldLocationFromIndex(int32 Index)
{
	int32 X;
	int32 Y;
	GetXYFromIndex(Index, X, Y);
	return GetWorldLocationFromXY(X, Y);
}

void AFGGridActor::ResetBoard()
{
	for (auto& Tile : TileList)
	{
		//UE_LOG(LogTemp, Log, TEXT("Resetting board"))
		Tile.Weight = Tile.DefaultWeight;
		Tile.ParentId = NULL;
		UE_LOG(LogTemp, Log, TEXT("Resetting board, Weight: %d"), Tile.Weight)
	}
}

void AFGGridActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (TileList.Num() == 0)
	{
		/*
		* If TileList is empty it probably means we just placed one in the level, so let's initialize it.
		*/

		TileList.SetNum(GetNumTiles());
	}

	GenerateGrid();

	DrawBlocks();
}

FVector AFGGridActor::GetWorldLocationFromXY(int32 TileX, int32 TileY) const
{
	const float X = ((static_cast<float>(TileX) - GetHalfWidth()) * TileSize) + GetTileSizeHalf();
	const float Y = ((static_cast<float>(TileY) - GetHalfHeight()) * TileSize) + GetTileSizeHalf();

	return GetActorTransform().TransformPosition(FVector(X, Y, 0));
}

bool AFGGridActor::GetXYFromWorldLocation(const FVector& WorldLocation, int32& TileX, int32& TileY) const
{
	if (!IsWorldLocationInsideGrid(WorldLocation))
		return false;

	const FVector RelativeGridLocation = GetActorTransform().InverseTransformPositionNoScale(WorldLocation);

	const float HeightOffset = (Height % 2) == 1 ? 0.5f : 0.0f;
	const float WidthOffset = (Width % 2) == 1 ? 0.5f : 0.0f;

	const float X = FMath::FloorToInt(WidthOffset + (RelativeGridLocation.X / TileSize)) + GetHalfWidth() - WidthOffset;
	const float Y = FMath::FloorToInt(HeightOffset + (RelativeGridLocation.Y / TileSize)) + GetHalfHeight() - HeightOffset;

	TileX = FMath::Clamp(static_cast<int32>(X), 0, Width - 1);
	TileY = FMath::Clamp(static_cast<int32>(Y), 0, Height - 1);

	return true;
}

int32 AFGGridActor::GetTileIndexFromWorldLocation(const FVector& WorldLocation) const
{
	int32 X = 0, Y = 0;
	if (GetXYFromWorldLocation(WorldLocation, X, Y))
	{
		//UE_LOG(LogTemp, Log, TEXT("We are clicking on tile %i"), GetTileIndexFromXY(X, Y));
		int32 TileIndex;
		GetTileIndexFromXY(X, Y, TileIndex);
		return TileIndex;
	}

	return 0;
}

bool AFGGridActor::TransformWorldLocationToTileLocation(const FVector& InWorldLocation, FVector& OutTileWorldLocation) const
{
	if (!IsWorldLocationInsideGrid(InWorldLocation))
		return false;

	int32 X = 0, Y = 0;
	if (GetXYFromWorldLocation(InWorldLocation, X, Y))
	{
		OutTileWorldLocation = GetWorldLocationFromXY(X, Y);
		return true;
	}

	return false;
}

void AFGGridActor::GetOverlappingTiles(const FVector& Origin, const FVector& Extent, TArray<int32>& OutOverlappingTiles) const
{
	const FBox BlockBox = FBox::BuildAABB(Origin, Extent);
	
	const FVector TileExtent(GetTileSizeHalf(), GetTileSizeHalf(), GetTileSizeHalf());

	FBox TileBox;

	for (int32 Y = Height - 1; Y >= 0; --Y)
	{
		for (int32 X = 0; X < Width; ++X)
		{
			const FVector TileWorldLocation = GetWorldLocationFromXY(X, Y);
			
			TileBox = FBox::BuildAABB(TileWorldLocation, TileExtent);
			if (TileBox.IntersectXY(BlockBox))
			{
				int32 ArrayIndex;
				if(GetTileIndexFromXY(X, Y, ArrayIndex))
					OutOverlappingTiles.Add(ArrayIndex);
			}
		}
	}
}

void AFGGridActor::DrawBlocks()
{
	const int32 NumBlocks = TileList.Num();

	if (NumBlocks == 0)
		return;

	if (BlockMeshDescription == nullptr)
		BlockMeshDescription = UStaticMesh::CreateStaticMeshDescription(this);

	if (BlockMesh == nullptr)
		BlockMesh = NewObject<UStaticMesh>(this, UStaticMesh::StaticClass());
		
	BlockMeshDescription->Empty();

	BlockStaticMeshComponent->SetStaticMesh(nullptr);

	FPolygonGroupID BlockPGID = BlockMeshDescription->CreatePolygonGroup();
	FPolygonID PID;

	const float BlockSize = TileSize * 0.25f;
	const FVector BlockExtent = FVector(BlockSize, BlockSize, BlockSize * 0.25f);

	for (int32 Y = Height - 1; Y >= 0; --Y)
	{
		for (int32 X = 0; X < Width; ++X)
		{
			const FVector TileRelativeLocation = GetActorTransform().InverseTransformPositionNoScale(GetWorldLocationFromXY(X, Y));
			int32 ArrayIndex;
			GetTileIndexFromXY(X, Y, ArrayIndex);
			const bool bIsBlocked = TileList[ArrayIndex].bBlock;

			if (bIsBlocked)
			{
				BlockMeshDescription->CreateCube(TileRelativeLocation, BlockExtent, BlockPGID, PID, PID, PID, PID, PID, PID);
			}
		}
	}

	if (!BlockMeshDescription->IsEmpty())
	{
		TArray<UStaticMeshDescription*> BlockMeshDescriptionList;
		BlockMeshDescriptionList.Add(BlockMeshDescription);
		BlockMesh->BuildFromStaticMeshDescriptions(BlockMeshDescriptionList);
		BlockStaticMeshComponent->SetStaticMesh(BlockMesh);
	}
}

void AFGGridActor::UpdateBlockingTiles()
{
	TArray<UFGGridBlockComponent*> AllBlocks;
	GetComponents(AllBlocks);

	TileList.Empty();
	TileList.SetNum(GetNumTiles());

	TArray<int32> BlockIndices;

	for (const auto Block : AllBlocks)
	{
		const FVector Origin = Block->GetComponentLocation();
		const FVector Extents = Block->Extents * 0.5f;

		BlockIndices.Reset();
		GetOverlappingTiles(Origin, Extents, BlockIndices);

		for (int32 Index = 0, Num = BlockIndices.Num(); Index < Num; ++Index)
		{
			TileList[BlockIndices[Index]].bBlock = true;
		}
	}

	DrawBlocks();
}

void AFGGridActor::GenerateGrid()
{
	if (Width < 1 || Height < 1)
		return;

	if (MeshDescription == nullptr)
		MeshDescription = UStaticMesh::CreateStaticMeshDescription(this);

	if (GridMesh == nullptr)
		GridMesh = NewObject<UStaticMesh>(this, UStaticMesh::StaticClass());

	MeshDescription->Empty();

	FPolygonGroupID PGID = MeshDescription->CreatePolygonGroup();
	FPolygonID PID;

	float Location_X = -((Width * TileSize) * 0.5f);
	float Location_Y = -((Height * TileSize) * 0.5f);

	for (int X = 0; X < Width + 1; ++X)
	{
		float LocationOffset = X * TileSize;
		FVector Center = FVector(Location_X + LocationOffset, 0.0f, 0.0f);
		FVector Test1 = FVector(BorderSize, GetHeightSize(), BorderSize);
		MeshDescription->CreateCube(Center, GetWidthExtends(), PGID, PID, PID, PID, PID, PID, PID);
	}

	for (int Y = 0; Y < Height + 1; ++Y)
	{
		float LocationOffset = Y * TileSize;
		FVector Center = FVector(0.0f, Location_Y + LocationOffset, BorderSize);
		FVector Test = FVector(GetWidthSize(), BorderSize, BorderSize);
		MeshDescription->CreateCube(Center, GetHeightExtends(), PGID, PID, PID, PID, PID, PID, PID);
	}

	TArray<UStaticMeshDescription*> MeshDescriptionList;
	MeshDescriptionList.Add(MeshDescription);
	GridMesh->BuildFromStaticMeshDescriptions(MeshDescriptionList);
	StaticMeshComponent->SetStaticMesh(GridMesh);
}

bool AFGGridActor::IsWorldLocationInsideGrid(const FVector& WorldLocation) const
{
	const FVector RelativeGridLocation = GetActorTransform().InverseTransformPositionNoScale(WorldLocation);

	if (RelativeGridLocation.X < -GetWidthSize())
		return false;
	else if (RelativeGridLocation.X > GetWidthSize())
		return false;
	else if (RelativeGridLocation.Y < -GetHeightSize())
		return false;
	else if (RelativeGridLocation.Y > GetHeightSize())
		return false;

	return true;
}

bool AFGGridActor::GetTileIndexFromXY(int32 TileX, int32 TileY, int32& OutTileIndex) const
{
	if (TileX < 0 || TileX >= Width)
		return false;

	if (TileY < 0 || TileY >= Height)
		return false;

	const int32 TileIndex = (TileY * Width) + TileX;
	

	if (!IsTileIndexValid(TileIndex))
	{
		OutTileIndex = -1;
		return false;
	}

	OutTileIndex = TileIndex;
	return true;
}

bool AFGGridActor::IsTileIndexValid(int32 TileIndex) const
{
	const int32 NumTiles = TileList.Num();

	if (TileIndex < 0 || TileIndex >= NumTiles)
	{
		return false;
	}

	return true;
}

#if WITH_EDITOR
void AFGGridActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	UpdateBlockingTiles();
}
#endif // WITH_EDITOR
