#pragma once
#include "SMinesweeper.generated.h"

/* Runtime Cell Data which holds state information for each cell */
USTRUCT()
struct FCellData
{
	GENERATED_BODY()
	FCellData() : Row(0), Col(0), Idx(-1), bIsMine(false), NearbyMinesCount(-1) {}
	FCellData(int32 Row, int32 Col, int32 Idx, bool bIsMine = false, int32 NearbyMines = -1) : Row(Row), Col(Col), Idx(Idx), bIsMine(bIsMine), NearbyMinesCount(NearbyMines) {}

	int32 GetRow() const
	{
		return Row;
	}

	int32 GetCol() const
	{
		return Col;
	}

	int32 GetIndex() const
	{
		return Idx;
	}

	int32 IsMine() const
	{
		return bIsMine;
	}

	// Currently, we will never unset a mine, so we use SetMine rather than SetIsMine(bool)
	void SetMine()
	{
		bIsMine = true;
	}

	// NearbyMinesCount will only be set if we've activated it before and performed a sweep of adjacent cells
	bool WasActivated() const
	{
		return GetNearbyMinesCount() > -1;
	}

	int32 GetNearbyMinesCount() const
	{
		return NearbyMinesCount;
	}

	void SetNearbyMinesCount(int32 Mines)
	{
		NearbyMinesCount = Mines;
	}
	
private:
	int32 Row;
	int32 Col;
	int32 Idx;
	bool bIsMine;
	int32 NearbyMinesCount;
};

class SMinesweeper : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS( SMinesweeper ){}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	TSharedRef<SWidget> ConstructCellButton(int32 Idx);

	/* GenerateGrid is equivalent to starting a new game */
	void GenerateGrid();

	/* Generate the Data used by the grid
	 * Returns a random index that might be used as a player hint, -1 if we don't have a starting point
	 */
	int32 GenerateMinesData();

	/* Find the sum of mines within the adjacent cells */
	int32 FindNearbyMinesCount(int32 CellIndex);

	/* Activate nearby cells which haven't been activated yet, as long as they're not mines themselves */
	void ActivateNearbyCells(int32 CellIndex);

	/* Are we able to play? This controls the disabled state of the grid buttons */
	bool CanPlay() const;

	void ActivateCell(int32 Idx);

	/* Returns a bool if a valid adjacent cell was found, and sets OutIndex to a found adjacent cell */
	/* This is needed to convert from incoming row/col to a valid index in our data */
	bool TryGetAdjacentCellIndex(const FCellData* CurrentCell, int32 Row, int32 Col, int32& OutIndex) const;
	
	int32 GetDesiredWidth() const;
	void OnDesiredWidthChanged(int32 NewVal);

	int32 GetDesiredHeight() const;
	void OnDesiredHeightChanged(int32 NewVal);
	
	int32 GetDesiredMinesNum() const;
    void OnDesiredMinesNumChanged(int32 NewVal);

	bool IsDebugMinesEnabled() const;
	ECheckBoxState GetDebugMinesState() const;
	void OnDebugMinesChanged(ECheckBoxState NewState);

	bool IsPlayerHintEnabled() const;
	ECheckBoxState GetPlayerHintState() const;
	void OnPlayerHintChanged(ECheckBoxState NewState);

	FReply OnGenerateGridClicked();

	// The Only widget we may want to reference later, as we dynamically add / clear children
	TSharedPtr<class SUniformGridPanel> GridPanel;

	int32 DesiredWidth;
	int32 DesiredHeight;
	int32 DesiredMinesCount;
	bool bCanPlay;
	ECheckBoxState DebugMinesState;
	ECheckBoxState PlayerHintState;
	
	// row-major ordered array for our mine grid
	TArray<FCellData> MinesData;
};
