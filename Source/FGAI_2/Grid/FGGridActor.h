#pragma once

#include "GameFramework/Actor.h"
#include "FGGridActor.generated.h"

USTRUCT(BlueprintType)
struct FFGTileinfo
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, Category = "Tile")
	bool bBlock = false;
	int32 Weight;
	int32 DefaultWeight = 10;
	int32 ParentId;

};

class UStaticMeshComponent;
class UStaticMesh;
class UStaticMeshDescription;

UCLASS()
class FGAI_2_API AFGGridActor : public AActor
{
	GENERATED_BODY()
public:
	AFGGridActor();

	virtual void BeginPlay() override;

	//These are my own methods here up top

	//Only gets 4 neighbours
	TArray<int32> GetNeighboursAroundCell(int32 Cell);

	//These should be obvious
	int32 GetFValue(int32 CellFrom, int32 CellTo);
	int32 GetHValue(int32 CellFrom, int32 CellTo);

	//Added my owner helper based on your methods,
	//working with a grid in a regular list was a new one for me
	void GetXYFromIndex(int32 Index, int32& CellX, int32& CellY);
	//This was for drawing the things out
	FVector GetWorldLocationFromIndex(int32 Index);
	//This almost ruined everything, thanks for pointing out structs
	//tend to give copies rather than refs
	void ResetBoard();

	//Unused, didn't produce desired result
	float GetHValueEuclidian(int32 CellFrom, int32 CellTo);
	
	/*
	* Called whenever placed in the editor or world, having its transform changed etc.
	* Responsible for eventually calling the infamous ConstructionScript in blueprint.
	*/
	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY()
	UStaticMeshComponent* StaticMeshComponent = nullptr;

	UPROPERTY()
	UStaticMeshComponent* BlockStaticMeshComponent = nullptr;

	UPROPERTY()
	UStaticMesh* GridMesh = nullptr;

	UPROPERTY()
	UStaticMesh* BlockMesh = nullptr;

	UPROPERTY()
	UStaticMeshDescription* MeshDescription = nullptr;

	UPROPERTY()
	UStaticMeshDescription* BlockMeshDescription = nullptr;

	UFUNCTION(BlueprintPure, Category = "Grid")
	FVector GetWorldLocationFromXY(int32 TileX, int32 TileY) const;

	UFUNCTION(BlueprintPure, Category = "Grid")
	bool GetXYFromWorldLocation(const FVector& WorldLocation, int32& TileX, int32& TileY) const;

	UFUNCTION(BlueprintPure, Category = "Grid")
	int32 GetTileIndexFromWorldLocation(const FVector& WorldLocation) const;

	UFUNCTION(BlueprintPure, Category = "Grid")
	bool TransformWorldLocationToTileLocation(const FVector& InWorldLocation, FVector& OutTileWorldLocation) const;

	/*
	* Returns a list of indices correlating to the location of a tile within the TileList
	*/
	void GetOverlappingTiles(const FVector& Origin, const FVector& Extent, TArray<int32>& OutOverlappingTiles) const;

	void DrawBlocks();

	void UpdateBlockingTiles();

	void GenerateGrid();

	bool IsWorldLocationInsideGrid(const FVector& WorldLocation) const;

	UFUNCTION(BlueprintPure, Category = "Grid")
	bool GetTileIndexFromXY(int32 TileX, int32 TileY, int32& OutTileIndes) const;
	bool IsTileIndexValid(int32 TileIndex) const;

#if WITH_EDITOR
	/*
	* This is called whenever a property, on this Actor, is edited in the editor.
	* Only available in the editor. If you forget WITH_EDITOR you will get a compile error when compiling the non-editor build
	*/
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

	UFUNCTION(BlueprintPure, Category = "Grid")
	float GetTileSizeHalf() const { return TileSize * 0.5f; }
	UFUNCTION(BlueprintPure, Category = "Grid")
	int32 GetNumTiles() const { return Width * Height; }
	UFUNCTION(BlueprintPure, Category = "Grid")
	float GetGridSize() const { return GetNumTiles() * TileSize; }

	UFUNCTION(BlueprintPure, Category = "Grid")
	float GetHalfWidth() const { return static_cast<float>(Width) * 0.5f; }
	UFUNCTION(BlueprintPure, Category = "Grid")
	float GetHalfHeight() const { return static_cast<float>(Height) * 0.5f; }

	UFUNCTION(BlueprintPure, Category = "Grid")
	float GetWidthSize() const { return (static_cast<float>(Width) * GetTileSizeHalf()) + BorderSize; }
	UFUNCTION(BlueprintPure, Category = "Grid")
	float GetHeightSize() const { return (static_cast<float>(Height) * GetTileSizeHalf()) + BorderSize; }

	UFUNCTION(BlueprintPure, Category = "Grid")
	FVector GetWidthExtends() const { return FVector(BorderSize, GetHeightSize(), BorderSize); }
	UFUNCTION(BlueprintPure, Category = "Grid")
	FVector GetHeightExtends() const { return FVector(GetWidthSize(), BorderSize, BorderSize); }

	/*
	* Initializes to the size of the number of tiles in the grid. 
	*/
	UPROPERTY(BlueprintReadOnly, Category = "Grid")
	TArray<FFGTileinfo> TileList;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Grid, meta = (ClampMin = 1))
	int Width = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Grid, meta = (ClampMin = 1))
	int Height = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Grid, meta = (ClampMin = 0.1))
	float BorderSize = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Grid)
	float TileSize = 500.0f;
};
