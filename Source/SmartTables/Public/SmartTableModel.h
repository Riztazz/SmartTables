// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#pragma once

#include "SmartTableDispatcher.h"
#include "SmartTableKeptRow.h"
#include "SmartTableSortKey.h"
#include "SmartTableTypes.h"
#include "UObject/Object.h"
#include "SmartTableModel.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam( FOnSmartTablePresentationChanged, const FSmartTablePresentationChange & );
DECLARE_MULTICAST_DELEGATE_OneParam( FOnSmartTableModelBusy, bool );

UCLASS( Abstract, Blueprintable, BlueprintType, meta = ( ToolTip = "" ) )
class SMARTTABLES_API USmartTableModel : public UObject
{
    GENERATED_BODY()

public:

    UFUNCTION( BlueprintNativeEvent, BlueprintPure, Category = "Smart Tables|Model" )
    int32 GetNumRows();

    UFUNCTION( BlueprintNativeEvent, BlueprintPure, Category = "Smart Tables|Model" )
    FText GetCellText( int32 NaturalRow, FName ColumnId );

    UFUNCTION( BlueprintNativeEvent, BlueprintPure, Category = "Smart Tables|Model" )
    ESmartTableCellEditor GetCellEditor( int32 NaturalRow, FName ColumnId );

    UFUNCTION( BlueprintNativeEvent, Category = "Smart Tables|Model" )
    bool SetCellText( int32 NaturalRow, FName ColumnId, const FText & Value );

    UFUNCTION( BlueprintNativeEvent, BlueprintPure, Category = "Smart Tables|Model" )
    FLinearColor GetCellColor( int32 NaturalRow, FName ColumnId );

    UFUNCTION( BlueprintNativeEvent, BlueprintPure, Category = "Smart Tables|Model" )
    FLinearColor GetRowColor( int32 NaturalRow );

    UFUNCTION( BlueprintNativeEvent, BlueprintPure, Category = "Smart Tables|Model" )
    FSmartTableSortKey GetCellSortKey( int32 NaturalRow, FName ColumnId );

    UFUNCTION( BlueprintNativeEvent, BlueprintPure, Category = "Smart Tables|Model" )
    FName GetRowId( int32 NaturalRow );

    FSmartTableKeptRow KeepRow( int32 NaturalRow );

    int32 FindKeptRow( const FSmartTableKeptRow & Row );

    UFUNCTION( BlueprintNativeEvent, BlueprintPure, Category = "Smart Tables|Model" )
    UObject * GetRowItem( int32 NaturalRow );

    UFUNCTION( BlueprintNativeEvent, BlueprintPure, Category = "Smart Tables|Model" )
    int32 NaturalRowOfItem( UObject * Item );

    UFUNCTION( BlueprintNativeEvent, Category = "Smart Tables|Model" )
    bool SortRows( const FSmartTableSortSpec & Spec );

    UFUNCTION( BlueprintNativeEvent, Category = "Smart Tables|Model" )
    bool ApplyTextFilter( const FText & FilterText, const TArray< FName > & AllColumnIds );

    UFUNCTION( BlueprintNativeEvent, Category = "Smart Tables|Model" )
    bool MoveRows( const TArray< int32 > & NaturalRows, int32 InsertBeforeNaturalRow );

    static TArray< int32 > PlanRowMove( int32 NumRows, const TArray< int32 > & NaturalRows, int32 InsertBeforeNaturalRow );

    UFUNCTION( BlueprintCallable, Category = "Smart Tables|Model" )
    void NotifyRowChanged( int32 NaturalRow );

    UFUNCTION( BlueprintCallable, Category = "Smart Tables|Model" )
    void NotifyCellChanged( int32 NaturalRow, FName ColumnId );

    void NotifyValuesChanged( const FSmartTablePresentationChange & Change );

    UFUNCTION( BlueprintCallable, Category = "Smart Tables|Model" )
    void NotifyRowsChanged();

    UFUNCTION( BlueprintCallable, Category = "Smart Tables|Model" )
    void NotifyRowsAdded( const TArray< int32 > & NaturalRows );

    UFUNCTION( BlueprintCallable, Category = "Smart Tables|Model" )
    void NotifyRowsRemoved( const TArray< FName > & RowIds );

    UFUNCTION( BlueprintCallable, Category = "Smart Tables|Model" )
    void NotifyNumRowsChanged();

    void NotifyRowSetChanged( const FSmartTableRowSetDiff & Diff );

    UFUNCTION( BlueprintPure, Category = "Smart Tables|Model" )
    FSmartTableSortSpec GetActiveSortSpec() const
    {
        return ActiveSortSpec;
    }

    const FSmartTableSortSpec & ActiveSortSpecRef() const
    {
        return ActiveSortSpec;
    }

    UFUNCTION( BlueprintPure, Category = "Smart Tables|Model" )
    FText GetActiveFilterText() const
    {
        return ActiveFilterText;
    }

    UFUNCTION( BlueprintPure, Category = "Smart Tables|Model" )
    bool IsBusy() const
    {
        return bBusy;
    }

    UFUNCTION( BlueprintPure, Category = "Smart Tables|Model" )
    int32 GetNumPresentedRows() const
    {
        return PresentedToNatural.Num();
    }

    UFUNCTION( BlueprintPure, Category = "Smart Tables|Model" )
    int32 PresentedToNaturalRow( int32 PresentedRow ) const;

    UFUNCTION( BlueprintPure, Category = "Smart Tables|Model" )
    int32 NaturalToPresentedRow( int32 NaturalRow ) const;

    TConstArrayView< int32 > GetPresentedRows() const
    {
        return PresentedToNatural;
    }

    FOnSmartTablePresentationChanged & OnPresentationChanged()
    {
        return PresentationChanged;
    }

    FOnSmartTableModelBusy & OnBusyChanged()
    {
        return BusyChanged;
    }

    void SetWorkDispatcher( FSmartTableWorkDispatcher InDispatcher )
    {
        WorkDispatcher = MoveTemp( InDispatcher );
    }

    void RebuildPresentation( const FSmartTablePresentationChange & WhenDone = FSmartTablePresentationChange::Order() );

protected:

    void ApplySortSpecToPresentation( const FSmartTableSortSpec & Spec );
    void ApplyFilterToPresentation( const FText & FilterText, const TArray< FName > & AllColumnIds );

    TArray< int32 > PresentedToNatural;

    TArray< int32 > NaturalToPresented;

    UPROPERTY( Transient )
    FSmartTableSortSpec ActiveSortSpec;

    UPROPERTY( Transient )
    FText ActiveFilterText;

    TArray< FName > FilterColumnIds;

private:
    void BroadcastPresentationChanged( const FSmartTablePresentationChange & Change );

    TArray< int32 > CollectFilteredRows();

    void ExtractSortKeys( TArray< TArray< FSmartTableSortKey > > & OutLevels, TArray< ESmartTableSortMode > & OutModes );

    void ApplySortedRows( int32 Token, TArray< int32 > && Rows, bool bWasDispatched );

    void SetBusy( bool bInBusy );

    FOnSmartTablePresentationChanged PresentationChanged;
    FOnSmartTableModelBusy BusyChanged;

    FSmartTableWorkDispatcher WorkDispatcher;

    int32 SortToken = 0;

    FSmartTablePresentationChange PendingChange = FSmartTablePresentationChange::Order();

    bool bBusy = false;

    int32 SortsInFlight = 0;

    int32 LastDispatchedToken = INDEX_NONE;
};
