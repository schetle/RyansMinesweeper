#include "SMinesweeper.h"

#include "SlateOptMacros.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"

#define LOCTEXT_NAMESPACE "SMinesweeper"

#define DEFAULT_WIDTH 10
#define DEFAULT_HEIGHT 10
#define DEFAULT_NUM_MINES 25
#define START_WITH_PLAYER_HINT true

static FSlateFontInfo ExtraLargeLayoutFont = FCoreStyle::GetDefaultFontStyle("Regular", 26);
static FSlateFontInfo LargeLayoutFont = FCoreStyle::GetDefaultFontStyle("Regular", 16);
static FSlateFontInfo MediumLayoutFont = FCoreStyle::GetDefaultFontStyle("Regular", 14);

void SMinesweeper::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.Padding(5)
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().Padding(10)
			.VAlign(VAlign_Center)
			.AutoWidth()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Minesweeper-Width", "Width"))
				.Font(LargeLayoutFont)
			]
			+ SHorizontalBox::Slot().Padding(5)
			.VAlign(VAlign_Center)
			.AutoWidth()
			[
				SNew(SSpinBox<int>)
				.Font(LargeLayoutFont)
				.MinDesiredWidth(48.f)
				.MinValue(3)
				.MaxValue(20)
				.MinSliderValue(TAttribute<TOptional<int>>(1))
				.MaxSliderValue(TAttribute<TOptional<int>>(20))
				.Delta(1)
				.Value(this, &SMinesweeper::GetDesiredWidth)
				.OnValueChanged(this, &SMinesweeper::OnDesiredWidthChanged)
			]
			+ SHorizontalBox::Slot().Padding(10)
			.VAlign(VAlign_Center)
			.AutoWidth()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Minesweeper-Height", "Height"))
				.Font(LargeLayoutFont)
			]
			+ SHorizontalBox::Slot().Padding(5)
			.VAlign(VAlign_Center)
			.AutoWidth()
			[
				SNew(SSpinBox<int>)
				.Font(LargeLayoutFont)
				.MinDesiredWidth(48.f)
				.MinValue(3)
				.MaxValue(20)
				.MinSliderValue(TAttribute<TOptional<int>>(1))
				.MaxSliderValue(TAttribute<TOptional<int>>(20))
				.Delta(1)
				.Value(this, &SMinesweeper::GetDesiredHeight)
				.OnValueChanged(this, &SMinesweeper::OnDesiredHeightChanged)
			]
			+ SHorizontalBox::Slot().Padding(10)
			.VAlign(VAlign_Center)
			.AutoWidth()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Minesweeper-NumMines", "Number of Mines"))
				.Font(LargeLayoutFont)
			]
			+ SHorizontalBox::Slot().Padding(5)
			.VAlign(VAlign_Center)
			.AutoWidth()
			[
				SNew(SSpinBox<int>)
				.Font(LargeLayoutFont)
				.MinDesiredWidth(64.f)
				.MinValue(3)
				.MinSliderValue(TAttribute<TOptional<int>>(1))
				.Delta(1)
				.Value(this, &SMinesweeper::GetDesiredMinesNum)
				.OnValueChanged(this, &SMinesweeper::OnDesiredMinesNumChanged)
			]
			+ SHorizontalBox::Slot().Padding(10)
			.VAlign(VAlign_Center)
			.AutoWidth()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().Padding(5, 0)
				.AutoHeight() [
					SNew(SCheckBox)
					.IsChecked(this, &SMinesweeper::GetPlayerHintState)
					.OnCheckStateChanged(this, &SMinesweeper::OnPlayerHintChanged)
					[
						SNew(STextBlock).Text(LOCTEXT("Minesweeper-Hint", "Start with Hint"))
					]
				]
				+ SVerticalBox::Slot().Padding(5, 0)
				.AutoHeight() [
					SNew(SCheckBox)
					.IsChecked(this, &SMinesweeper::GetDebugMinesState)
					.OnCheckStateChanged(this, &SMinesweeper::OnDebugMinesChanged)
					[
						SNew(STextBlock).Text(LOCTEXT("Minesweeper-DebugMines", "(Cheat) Show Mines"))
					]
				]
			]
			+ SHorizontalBox::Slot().Padding(10)
			.VAlign(VAlign_Center)
			.AutoWidth()
			[
				SNew(SButton)
				.OnClicked(this, &SMinesweeper::OnGenerateGridClicked)
				[
					SNew(STextBlock)
					.Font(MediumLayoutFont)
					.Text(LOCTEXT("Minesweeper-NewGame", "New Game"))
				]
			]
		]
		+ SVerticalBox::Slot()
		.FillHeight(1)
		[
			SNew(SOverlay)
			+ SOverlay::Slot() [
				SAssignNew(GridPanel, SUniformGridPanel)
					.MinDesiredSlotWidth(24.f)
					.MinDesiredSlotHeight(24.f)
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Visibility_Lambda([this]()
				{
					return CanPlay() ? EVisibility::Hidden : EVisibility::Visible;
				})
				.Font(ExtraLargeLayoutFont)
				.Text(LOCTEXT("Minesweeper-GameOver", "Game Over!"))
			]
		]
	];

	// We can start with a player hint by setting this to true
	// Which will "activate" a non-mine cell randomly on the board (including cascade)
	OnPlayerHintChanged(START_WITH_PLAYER_HINT ? ECheckBoxState::Checked : ECheckBoxState::Unchecked);
	
	// Let's set some default values
	OnDesiredWidthChanged(DEFAULT_WIDTH);
	OnDesiredHeightChanged(DEFAULT_HEIGHT);
	OnDesiredMinesNumChanged(DEFAULT_NUM_MINES);
	GenerateGrid();
}

TSharedRef<SWidget> SMinesweeper::ConstructCellButton(int32 Idx)
{
	return SNew(SButton)
		.IsEnabled_Lambda([this, Idx]()
		{
			check(Idx < MinesData.Num())
			
			return CanPlay() && MinesData[Idx].GetNearbyMinesCount() == -1;
		})
		.OnClicked_Lambda([this, Idx]()
		{
			if (CanPlay())
			{
				ActivateCell(Idx);
			}
			
			return FReply::Handled();
		})
		[
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.Padding(10)
			[
				SNew(STextBlock)
				.Font(MediumLayoutFont)
				.Text_Lambda([this, Idx]()
				{
					check(Idx < MinesData.Num())

					const FCellData* Cell = &MinesData[Idx];

					// Reveal mines at the end of a game, or if debug mines
					if (Cell->IsMine() && (!CanPlay() || IsDebugMinesEnabled()))
					{
						return FText::FromString("M");
					}
					
					return Cell->GetNearbyMinesCount() > 0 ? FText::FromString(FString::FromInt(Cell->GetNearbyMinesCount())) : FText();
				})
			]
		];
}

void SMinesweeper::GenerateGrid()
{
	bCanPlay = true;

	// Generate our cell data, as well as mine placement
	int32 StartingPoint = GenerateMinesData();
	
	GridPanel->ClearChildren();
	
	// We'll generate our buttons, and provide each button with a cell index for reference later
	for (int32 CellIndex = 0; CellIndex < DesiredHeight * DesiredWidth; CellIndex++)
	{
		int32 Row = CellIndex / DesiredWidth;
		int32 Col = CellIndex % DesiredWidth;
		
		GridPanel->AddSlot(Col, Row) [
			ConstructCellButton(CellIndex)
		];
	}

	// We'll give the player a random starting point hint if it's enabled and we actually have one
	// The scenarios in which we don't have one would be if the grid is entirely filled with mines, which
	// can happen depending on some tweaks to the control widgets
	if (IsPlayerHintEnabled() && StartingPoint > -1)
	{
		ActivateCell(MinesData[StartingPoint].GetIndex());
	}
}

int32 SMinesweeper::GenerateMinesData()
{
	MinesData.Empty(DesiredHeight * DesiredWidth);

	// Here's the algorithm we've come up with for mine placement
	// 1. Store Cell data in a row-major ordered array
	// 2. Uses a temporary array of pointers of "known not to have a mine" or "clean" cells
	// 3. For n number of mines we want, we pull from our temporary list, set it as a mine, and then remove it from our temp list
	// 4. If we still have any clean cells left over,
	//    return a random index from our clean cells list so that we can provide a player hint if enabled.

	// Temporary array of cell pointers, so that we can randomly pull a cell out
	// and mark it as a mine
	TArray<FCellData*> CleanCells;

	// Prefill out cells
	for (int32 cell = 0; cell < DesiredHeight * DesiredWidth; cell++)
	{
		MinesData.Add(FCellData(cell / DesiredWidth, cell % DesiredWidth, cell));
		CleanCells.Add(&MinesData[cell]);
	}

	check(DesiredMinesCount < CleanCells.Num());

	// Randomly choose cells to become mines
	for (int32 i = 0; i < DesiredMinesCount; i++)
	{
		int32 randomIndex = FMath::RandRange(0, CleanCells.Num()-1);
		CleanCells[randomIndex]->SetMine();
		CleanCells.RemoveAt(randomIndex);
	}

	// We'll return a starting point that can be used to give the initial mine hint to a player, if we can.
	// If the entire grid is mines, we can't.
	if (CleanCells.Num() > 0)
	{
		return CleanCells[FMath::RandRange(0, CleanCells.Num() - 1)]->GetIndex();
	}

	return -1;
}

int32 SMinesweeper::FindNearbyMinesCount(int32 CellIndex)
{
	check(CellIndex < MinesData.Num())
	const FCellData* CurrentCell = &MinesData[CellIndex];
	int32 FoundMines = 0;
	
	for (int32 i = -1; i < 2; i++)
	{
		for (int32 j = -1; j < 2; j++)
		{
			int32 AdjacentCellIndex = -1;
			if (TryGetAdjacentCellIndex(CurrentCell, i, j, AdjacentCellIndex))
			{
				if (MinesData[AdjacentCellIndex].IsMine())
				{
					FoundMines++;
				}
			}
		}
	}

	return FoundMines;
}

void SMinesweeper::ActivateNearbyCells(int32 CellIndex)
{
	check(CellIndex < MinesData.Num())
	const FCellData* CurrentCell = &MinesData[CellIndex];

	for (int32 i = -1; i < 2; i++)
	{
		for (int32 j = -1; j < 2; j++)
		{
			int32 AdjacentCellIndex = -1;
			if (TryGetAdjacentCellIndex(CurrentCell, i, j, AdjacentCellIndex))
			{
				if (!MinesData[AdjacentCellIndex].IsMine() && !MinesData[AdjacentCellIndex].WasActivated())
				{
					ActivateCell(AdjacentCellIndex);
				}	
			}
		}
	}
}

bool SMinesweeper::CanPlay() const
{
	return bCanPlay;
}

void SMinesweeper::ActivateCell(int32 Idx)
{
	check(Idx < MinesData.Num())
	
	FCellData* Cell = &MinesData[Idx];

	if (Cell->IsMine())
	{
		// We've hit a mine!
		bCanPlay = false;
		return;
	}

	int32 NearbyMinesCount = FindNearbyMinesCount(Idx);
	Cell->SetNearbyMinesCount(NearbyMinesCount);

	// Cascade outward until we've found nearby mines
	if (NearbyMinesCount == 0)
	{
		ActivateNearbyCells(Idx);
	}
}

bool SMinesweeper::TryGetAdjacentCellIndex(const FCellData* CurrentCell, int32 Row, int32 Col, int32& OutIndex) const
{
	// Reset out index as the first thing, so it's not forgotten or if code changes, it's guaranteed to be set
	// by this function
	OutIndex = -1;
	
	// Convert our current cell and the requested row,col to an index
	int32 Index = (CurrentCell->GetRow() + Row) * DesiredWidth + (CurrentCell->GetCol() + Col);
	
	if (Index < 0 || Index >= MinesData.Num())
	{
		return false;
	}

	// Quick bounds check to make sure we didn't wrap
	if (FMath::Abs(CurrentCell->GetRow() - MinesData[Index].GetRow()) >= 2 || FMath::Abs(CurrentCell->GetCol() - MinesData[Index].GetCol()) >= 2)
	{
		return false;
	}

	OutIndex = Index;
	return true;
}

int32 SMinesweeper::GetDesiredWidth() const
{
	return DesiredWidth;
}

void SMinesweeper::OnDesiredWidthChanged(int32 NewVal)
{
	DesiredWidth = NewVal;

	// We need to make sure to update Mines as well, if we're changing this value
	// We'll just pass in the existing value. It could still be valid.
	OnDesiredMinesNumChanged(DesiredMinesCount);
}

int32 SMinesweeper::GetDesiredHeight() const
{
	return DesiredHeight;
}

void SMinesweeper::OnDesiredHeightChanged(int32 NewVal)
{
	DesiredHeight = NewVal;

	// We need to make sure to update Mines as well, if we're changing this value
	// We'll just pass in the existing value. It could still be valid.
	OnDesiredMinesNumChanged(DesiredMinesCount);
}

int32 SMinesweeper::GetDesiredMinesNum() const
{
	return DesiredMinesCount;
}

void SMinesweeper::OnDesiredMinesNumChanged(int32 NewVal)
{
	// Yes, you CAN fill the entire grid with mines.
	// No, it will not be a fun game.
	DesiredMinesCount = FMath::Clamp(NewVal, 1, (GetDesiredWidth() * GetDesiredHeight()) - 3);
}

bool SMinesweeper::IsDebugMinesEnabled() const
{
	return DebugMinesState == ECheckBoxState::Checked;
}

ECheckBoxState SMinesweeper::GetDebugMinesState() const
{
	return DebugMinesState;
}

void SMinesweeper::OnDebugMinesChanged(ECheckBoxState NewState)
{
	DebugMinesState = NewState;
}

bool SMinesweeper::IsPlayerHintEnabled() const
{
	return PlayerHintState == ECheckBoxState::Checked;
}

ECheckBoxState SMinesweeper::GetPlayerHintState() const
{
	return PlayerHintState;
}

void SMinesweeper::OnPlayerHintChanged(ECheckBoxState NewState)
{
	PlayerHintState = NewState;
}

FReply SMinesweeper::OnGenerateGridClicked()
{
	GenerateGrid();
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
