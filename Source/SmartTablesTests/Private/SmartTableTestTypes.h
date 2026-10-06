// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#pragma once

#include "Engine/DataTable.h"
#include "SmartTable.h"
#include "SmartTableCell.h"
#include "SmartTableCellDragDropOp.h"
#include "SmartTableLayoutStore.h"
#include "SmartTableModel.h"
#include "SmartTableRowWidget.h"
#include "SmartTableTestTypes.generated.h"

UENUM()
enum class ESmartTableTestStatus : uint8
{
    Unsurveyed,
    Surveyed,
    Claimed,
    Derelict
};

USTRUCT()
struct FSmartTableTestRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY()
    FString Callsign;

    UPROPERTY()
    FName Designation;

    UPROPERTY()
    FText DisplayName;

    UPROPERTY()
    double MassTonnes = 0.0;

    UPROPERTY()
    float Integrity = 0.0f;

    UPROPERTY()
    int32 CrewCapacity = 0;

    UPROPERTY()
    bool bVisited = false;

    UPROPERTY()
    ESmartTableTestStatus Status = ESmartTableTestStatus::Unsurveyed;
};

UCLASS()
class USmartTableTestStructModel : public USmartTableModel
{
    GENERATED_BODY()

public:
    void SetRows( TArray< FSmartTableTestRow > InRows );

    const TArray< FSmartTableTestRow > & GetRows() const
    {
        return Rows;
    }

    void SetMass( int32 NaturalRow, double MassTonnes );

protected:
    virtual int32 GetNumRows_Implementation() override;
    virtual FText GetCellText_Implementation( int32 NaturalRow, FName ColumnId ) override;
    virtual FSmartTableSortKey GetCellSortKey_Implementation( int32 NaturalRow, FName ColumnId ) override;
    virtual FName GetRowId_Implementation( int32 NaturalRow ) override;

private:
    TArray< FSmartTableTestRow > Rows;
};

UCLASS()
class USmartTableTestHarness : public USmartTable
{
    GENERATED_BODY()

public:
    void Author( TArray< FSmartTableColumn > InColumns )
    {
        Columns = MoveTemp( InColumns );
    }

    void LinkTo( const TSoftObjectPtr< UDataTable > & InSourceTable )
    {
        ColumnSource = ESmartTableColumnSource::DataTable;
        SourceTable  = InSourceTable;
    }

    void ExpectItemsOf( UClass * InExpectedItemClass )
    {
        ColumnSource      = ESmartTableColumnSource::ItemClass;
        ExpectedItemClass = InExpectedItemClass;
    }

    void PointAtFile( const FString & InPath )
    {
        ColumnSource        = ESmartTableColumnSource::File;
        SourceFile.FilePath = InPath;
    }

    UFUNCTION()
    void HearSelection( const TArray< int32 > & SelectedRows )
    {
        SelectionsHeard.Add( SelectedRows );
    }

    TArray< TArray< int32 > > SelectionsHeard;

    UFUNCTION()
    void HearCellDrop( USmartTableCellDragDropOp * Payload, int32 TargetNaturalRow, FName TargetColumnId )
    {
        DropSourcesHeard.Add( Payload ? Payload->SourceRow : INDEX_NONE );
    }

    TArray< int32 > DropSourcesHeard;

    UFUNCTION()
    void HearMenuPick( FName EntryId, int32 EntryIndex, UObject * Item, int32 NaturalRow )
    {
        MenuPicksHeard.Add( { Item, NaturalRow } );
    }

    TArray< TPair< const UObject *, int32 > > MenuPicksHeard;

    UFUNCTION()
    void HearRowMenuOpening( UObject * Item, int32 NaturalRow )
    {
        RowMenusHeard.Add( NaturalRow );
    }

    TArray< int32 > RowMenusHeard;

    void PickRowMenuEntry( FName EntryId, const FSmartTableKeptRow & Row )
    {
        ChooseRowMenuEntry( EntryId, 0, Row );
    }

#if WITH_EDITOR

    void RunCompileValidation( class IWidgetCompilerLog & CompileLog ) const
    {
        ValidateCompiledDefaults( CompileLog );
    }

    FText WhatSwitchingLoses( ESmartTableColumnSource NewSource ) const
    {
        return DescribeSourcesLostBySwitchingTo( NewSource );
    }
#endif
};

UCLASS()
class USmartTableTestCountingModel : public USmartTableModel
{
    GENERATED_BODY()

public:
    int32 ItemsAsked = 0;

protected:
    virtual int32 GetNumRows_Implementation() override
    {
        return 4;
    }

    virtual FText GetCellText_Implementation( int32 NaturalRow, FName ColumnId ) override
    {
        return FText::AsNumber( NaturalRow );
    }

    virtual UObject * GetRowItem_Implementation( int32 NaturalRow ) override
    {
        ++ItemsAsked;

        return nullptr;
    }
};

UCLASS()
class USmartTableTestLayoutStore : public USmartTableLayoutStore
{
    GENERATED_BODY()

public:
    virtual bool LoadLayout_Implementation( FName TableId, FSmartTableLayout & OutLayout ) const override
    {
        OutLayout = Stored;

        return true;
    }

    virtual void SaveLayout_Implementation( FName TableId, const FSmartTableLayout & Layout ) override
    {
        Stored = Layout;
    }

    FSmartTableLayout Stored;
};

UCLASS()
class USmartTableTestRowWidget : public USmartTableRowWidget
{
    GENERATED_BODY()
};

UCLASS()
class USmartTableTestHandedCell : public USmartTableCell
{
    GENERATED_BODY()

public:
    USmartTableTestHandedCell()
    {
        ItemSetter = GET_FUNCTION_NAME_CHECKED( USmartTableTestHandedCell, TakeItem );
    }

    void NameItemSetter( FName Setter )
    {
        ItemSetter = Setter;
    }

    TArray< FName > ItemSetterOptions() const
    {
        return GetItemSetterOptions();
    }

    UFUNCTION( BlueprintCallable, Category = "Smart Tables|Test" )
    void TakeItem( UObject * Item )
    {
        Handed.Add( Item );
    }

    UFUNCTION( BlueprintCallable, Category = "Smart Tables|Test" )
    void TakeTwo( UObject * Item, int32 Extra )
    {
        Handed.Add( Item );
    }

    TArray< const UObject * > Handed;
};

UCLASS()
class USmartTableTestTypedCell : public USmartTableCell
{
    GENERATED_BODY()

public:
    USmartTableTestTypedCell()
    {
        ItemSetter = GET_FUNCTION_NAME_CHECKED( USmartTableTestTypedCell, TakeStore );
    }

    UFUNCTION( BlueprintCallable, Category = "Smart Tables|Test" )
    void TakeStore( USmartTableTestLayoutStore * Store )
    {
        Handed.Add( Store );
    }

    TArray< const UObject * > Handed;
};

class USmartTableDataTableRow;
class USmartTableObjectModel;

namespace SmartTable::Test
{
    USmartTableDataTableRow * NamedRow( const TCHAR * Name );

    USmartTableObjectModel * NamedRowModel( const TArray< UObject * > & Items );

    USmartTableTestHarness * NamedRowTable( const TArray< UObject * > & Items );

    FSmartTableKeptRow KeptRow( const TCHAR * RowId, int32 NaturalRow );
}
