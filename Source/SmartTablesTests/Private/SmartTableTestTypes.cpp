// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTableTestTypes.h"

#include "Logging/StructuredLog.h"
#include "SmartTableDataTableModel.h"
#include "SmartTableLog.h"
#include "SmartTableObjectModel.h"

void USmartTableTestStructModel::SetRows( TArray< FSmartTableTestRow > InRows )
{
    Rows = MoveTemp( InRows );

    NotifyNumRowsChanged();
}

void USmartTableTestStructModel::SetMass( int32 NaturalRow, double MassTonnes )
{
    if ( !Rows.IsValidIndex( NaturalRow ) )
    {
        UE_LOGFMT( LogSmartTables, Verbose, "SetMass did nothing. Row {Row} sits past the end of {Count} row(s).", NaturalRow, Rows.Num() );
        return;
    }

    Rows[ NaturalRow ].MassTonnes = MassTonnes;

    NotifyRowChanged( NaturalRow );
}

int32 USmartTableTestStructModel::GetNumRows_Implementation()
{
    return Rows.Num();
}

FText USmartTableTestStructModel::GetCellText_Implementation( int32 NaturalRow, FName ColumnId )
{
    if ( !Rows.IsValidIndex( NaturalRow ) )
    {
        return FText::GetEmpty();
    }

    const FSmartTableTestRow & Row = Rows[ NaturalRow ];

    if ( ColumnId == TEXT( "Callsign" ) )
    {
        return FText::FromString( Row.Callsign );
    }

    if ( ColumnId == TEXT( "MassTonnes" ) )
    {
        return FText::AsNumber( Row.MassTonnes );
    }

    return FText::GetEmpty();
}

FSmartTableSortKey USmartTableTestStructModel::GetCellSortKey_Implementation( int32 NaturalRow, FName ColumnId )
{
    if ( !Rows.IsValidIndex( NaturalRow ) )
    {
        return FSmartTableSortKey::MakeEmpty();
    }

    if ( ColumnId == TEXT( "MassTonnes" ) )
    {
        return FSmartTableSortKey::MakeNumber( Rows[ NaturalRow ].MassTonnes );
    }

    return Super::GetCellSortKey_Implementation( NaturalRow, ColumnId );
}

FName USmartTableTestStructModel::GetRowId_Implementation( int32 NaturalRow )
{
    return Rows.IsValidIndex( NaturalRow ) ? FName( *Rows[ NaturalRow ].Callsign ) : NAME_None;
}

USmartTableDataTableRow * SmartTable::Test::NamedRow( const TCHAR * Name )
{
    USmartTableDataTableRow * Row = NewObject< USmartTableDataTableRow >();
    Row->RowName                  = Name;

    return Row;
}

USmartTableObjectModel * SmartTable::Test::NamedRowModel( const TArray< UObject * > & Items )
{
    USmartTableObjectModel * Model = NewObject< USmartTableObjectModel >();

    FSmartTableColumn Column;
    Column.ColumnId = TEXT( "RowName" );
    Model->SetColumns( { Column } );
    Model->SetItems( Items );

    return Model;
}

USmartTableTestHarness * SmartTable::Test::NamedRowTable( const TArray< UObject * > & Items )
{
    USmartTableTestHarness * Table = NewObject< USmartTableTestHarness >();

    FSmartTableColumn Column;
    Column.ColumnId = TEXT( "RowName" );
    Table->Author( { Column } );
    Table->SetItems( Items );
    Table->SetSelectionMode( ESmartTableSelectionMode::Multi );

    return Table;
}

FSmartTableKeptRow SmartTable::Test::KeptRow( const TCHAR * RowId, int32 NaturalRow )
{
    return { RowId, NaturalRow };
}
