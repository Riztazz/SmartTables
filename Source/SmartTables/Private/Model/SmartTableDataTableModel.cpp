// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTableDataTableModel.h"

#include "SmartTableConstants.h"

#include "Engine/DataTable.h"
#include "Logging/StructuredLog.h"
#include "Misc/AssertionMacros.h"
#include "SmartTableLog.h"
#include "SmartTableRowSetDiff.h"
#include "UObject/Class.h"
#include "UObject/UObjectBaseUtility.h"

void USmartTableDataTableModel::SetDataTable( UDataTable * InDataTable )
{

    if ( DataTable && DataTableChangedHandle.IsValid() )
    {
        DataTable->OnDataTableChanged().Remove( DataTableChangedHandle );
        DataTableChangedHandle.Reset();
    }

    DataTable = InDataTable;

    if ( DataTable )
    {
        DataTableChangedHandle = DataTable->OnDataTableChanged().AddUObject( this, &USmartTableDataTableModel::HandleDataTableChanged );
    }

    RowProxies.Reset();
    RowNames = DataTable ? DataTable->GetRowNames() : TArray< FName >();

    UE_LOGFMT( LogSmartTablesData, Verbose, "DataTable model reads {Asset} from here on: {Rows} row(s).", GetNameSafe( InDataTable ), RowNames.Num() );

    ResolveBindings();
    NotifyRowSetChanged( FSmartTableRowSetDiff::Replaced() );
}

void USmartTableDataTableModel::SetColumns( const TArray< FSmartTableColumn > & InColumns )
{
    Columns = InColumns;

    ColumnOrderById.Reset();
    ColumnOrderById.Reserve( Columns.Num() );
    for ( int32 Index = 0; Index < Columns.Num(); ++Index )
    {
        ColumnOrderById.FindOrAdd( Columns[ Index ].ColumnId, Index );
    }

    UE_LOGFMT( LogSmartTablesData, Verbose, "DataTable model given {Count} column(s). Every binding resolves once more.", Columns.Num() );

    ResolveBindings();
    NotifyRowsChanged();
}

void USmartTableDataTableModel::ResolveBindings()
{
    ValueBindings.Reset( Columns.Num() );
    SortBindings.Reset( Columns.Num() );

    if ( !DataTable )
    {
        UnresolvedWarnings.Reset();
        NoRowStructWarnedFor.Reset();

        UE_LOGFMT( LogSmartTablesData, Verbose, "No DataTable is set, and {Count} column(s) have nothing to bind against.", Columns.Num() );
        return;
    }

    const UScriptStruct * RowStruct = DataTable->GetRowStruct();

    if ( !RowStruct )
    {
        const FString Asset = DataTable->GetPathName();
        if ( NoRowStructWarnedFor != Asset )
        {
            NoRowStructWarnedFor = Asset;

            UE_LOGFMT( LogSmartTablesData, Warning, "DataTable {Asset} carries no row struct, so every column over it draws blank. Pick the Row Structure on the asset. This usually means the struct it named was deleted or would not load.", GetNameSafe( DataTable.Get() ) );
        }

        UnresolvedWarnings.Reset();
        return;
    }

    NoRowStructWarnedFor.Reset();

    TMap< FName, FString > StillUnresolved;

    TSet< FName > Shadowed;

    for ( const FSmartTableColumn & Column : Columns )
    {
        const SmartTable::FBinding Value = SmartTable::ResolveBinding( RowStruct, Column.GetValueBinding() );

        ValueBindings.Add( Value );
        SortBindings.Add( SmartTable::ResolveBinding( RowStruct, Column.GetSortBinding() ) );

        bool bShadowed = false;
        if ( !Column.ColumnId.IsNone() )
        {
            Shadowed.Add( Column.ColumnId, &bShadowed );
        }

        if ( bShadowed )
        {
            continue;
        }

        const bool bValueBroken = SmartTable::ShouldWarnForUnresolvedBinding( Column, Value );
        const bool bSortBroken  = SmartTable::ShouldWarnForUnresolvedSortBinding( Column, SortBindings.Last() );

        if ( !bValueBroken && !bSortBroken )
        {
            continue;
        }

        const FString Signature = UnresolvedSignature( Column, *RowStruct );
        StillUnresolved.Add( Column.ColumnId, Signature );

        const FString * Warned = UnresolvedWarnings.Find( Column.ColumnId );
        if ( Warned && *Warned == Signature )
        {

            continue;
        }

        if ( bValueBroken )
        {
            UE_LOGFMT( LogSmartTablesData, Warning, "Column '{Column}' draws nothing: it binds '{Binding}', which is no field on row struct {Struct}. Set the BindingName of the column to one of these. Available: {Available}", Column.ColumnId, Column.GetValueBinding(), RowStruct->GetName(), SmartTable::DescribeBindableNames( RowStruct ) );
        }

        if ( bSortBroken )
        {

            UE_LOGFMT( LogSmartTablesSort, Warning, "Column '{Column}' will not sort: its SortBindingName '{Binding}' is no field on row struct {Struct}. The column still DRAWS, because its values come from '{Value}', only the order is dead. Set SortBindingName to one of these, or clear it to sort by the shown value. Available: {Available}", Column.ColumnId, Column.SortBindingName,
                RowStruct->GetName(), Column.GetValueBinding(), SmartTable::DescribeBindableNames( RowStruct ) );
        }
    }

    for ( const TPair< FName, FString > & Was : UnresolvedWarnings )
    {

        const int32 Index      = IndexOfColumn( Was.Key );
        const bool bBindsAgain = Index != INDEX_NONE && !Columns[ Index ].GetValueBinding().IsNone() && ValueBindings.IsValidIndex( Index ) && ValueBindings[ Index ].IsValid();

        if ( bBindsAgain )
        {
            UE_LOGFMT( LogSmartTablesData, Verbose, "Column '{Column}' binds after all: {Struct} now carries the field it names.", Was.Key, RowStruct->GetName() );
        }
    }

    UnresolvedWarnings = MoveTemp( StillUnresolved );
}

FString USmartTableDataTableModel::UnresolvedSignature( const FSmartTableColumn & Column, const UScriptStruct & RowStruct )
{
    const uint32 Fields = GetTypeHash( SmartTable::DescribeBindableNames( &RowStruct ) );

    return FString::Printf( TEXT( "%s|%s|%s|%u" ), *Column.GetValueBinding().ToString(), *Column.SortBindingName.ToString(), *RowStruct.GetPathName(), Fields );
}

int32 USmartTableDataTableModel::IndexOfColumn( FName ColumnId ) const
{

    const int32 * Index = ColumnOrderById.Find( ColumnId );

    return Index ? *Index : INDEX_NONE;
}

uint8 * USmartTableDataTableModel::RowData( int32 NaturalRow ) const
{
    if ( !DataTable || !RowNames.IsValidIndex( NaturalRow ) )
    {

        UE_LOGFMT( LogSmartTablesData, VeryVerbose, "Row {Row} carries no row data: {Reason}.", NaturalRow, DataTable ? TEXT( "that row is past the end of the table" ) : TEXT( "no DataTable is set" ) );
        return nullptr;
    }

    return DataTable->FindRowUnchecked( RowNames[ NaturalRow ] );
}

int32 USmartTableDataTableModel::GetNumRows_Implementation()
{
    return RowNames.Num();
}

FText USmartTableDataTableModel::GetCellText_Implementation( int32 NaturalRow, FName ColumnId )
{
    const int32 ColumnIndex = IndexOfColumn( ColumnId );
    if ( !ValueBindings.IsValidIndex( ColumnIndex ) )
    {

        UE_LOGFMT( LogSmartTablesData, VeryVerbose, "'{Column}' resolved to no binding on this model, and row {Row} draws nothing.", ColumnId, NaturalRow );
        return FText::GetEmpty();
    }

    checkf( ValueBindings.Num() == Columns.Num(), TEXT( "%d resolved value binding(s) against %d declared column(s) - the two must stay parallel" ), ValueBindings.Num(), Columns.Num() );

    if ( !ValueBindings[ ColumnIndex ].IsValid() )
    {

#if UE_BUILD_SHIPPING
        return FText::GetEmpty();
#else
        return SmartTable::Text::UnresolvedBinding( Columns[ ColumnIndex ].GetValueBinding() );
#endif
    }

    return SmartTable::ReadStructAsText( ValueBindings[ ColumnIndex ], RowData( NaturalRow ), Columns[ ColumnIndex ] );
}

void USmartTableDataTableModel::HandleDataTableChanged()
{
    const TArray< FName > Before = MoveTemp( RowNames );

    RowNames = DataTable ? DataTable->GetRowNames() : TArray< FName >();
    RowProxies.Reset();

    if ( DataTable )
    {
        ResolveBindings();
    }

    UE_LOGFMT( LogSmartTablesData, Verbose, "{Asset} changed underneath the table. {Rows} row(s) now.", GetNameSafe( DataTable.Get() ), RowNames.Num() );

    NotifyRowSetChanged( FSmartTableRowSetDiff::Between( Before, RowNames ) );
}

void USmartTableDataTableModel::BeginDestroy()
{
    if ( DataTable && DataTableChangedHandle.IsValid() )
    {
        DataTable->OnDataTableChanged().Remove( DataTableChangedHandle );
        DataTableChangedHandle.Reset();
    }

    Super::BeginDestroy();
}

ESmartTableCellEditor USmartTableDataTableModel::GetCellEditor_Implementation( int32 NaturalRow, FName ColumnId )
{
    const int32 ColumnIndex = IndexOfColumn( ColumnId );

    return ValueBindings.IsValidIndex( ColumnIndex ) ? SmartTable::EditorFor( ValueBindings[ ColumnIndex ] ) : ESmartTableCellEditor::None;
}

bool USmartTableDataTableModel::SetCellText_Implementation( int32 NaturalRow, FName ColumnId, const FText & Value )
{
    const int32 ColumnIndex = IndexOfColumn( ColumnId );
    if ( !ValueBindings.IsValidIndex( ColumnIndex ) )
    {
        return false;
    }

    if ( !SmartTable::WriteStructAsText( ValueBindings[ ColumnIndex ], RowData( NaturalRow ), Value ) )
    {
        return false;
    }

    NotifyRowChanged( NaturalRow );

    return true;
}

FSmartTableSortKey USmartTableDataTableModel::GetCellSortKey_Implementation( int32 NaturalRow, FName ColumnId )
{
    const int32 ColumnIndex = IndexOfColumn( ColumnId );
    if ( !SortBindings.IsValidIndex( ColumnIndex ) )
    {

        return FSmartTableSortKey::MakeEmpty();
    }

    checkf( SortBindings.Num() == Columns.Num(), TEXT( "%d resolved sort binding(s) against %d declared column(s) - the two must stay parallel" ), SortBindings.Num(), Columns.Num() );

    return SmartTable::ReadStructAsSortKey( SortBindings[ ColumnIndex ], RowData( NaturalRow ) );
}

FName USmartTableDataTableModel::GetRowId_Implementation( int32 NaturalRow )
{
    return RowNames.IsValidIndex( NaturalRow ) ? RowNames[ NaturalRow ] : NAME_None;
}

UObject * USmartTableDataTableModel::GetRowItem_Implementation( int32 NaturalRow )
{
    if ( !RowNames.IsValidIndex( NaturalRow ) )
    {
        return nullptr;
    }

    const FName RowName = RowNames[ NaturalRow ];
    if ( const TObjectPtr< USmartTableDataTableRow > * Existing = RowProxies.Find( RowName ) )
    {
        return *Existing;
    }

    USmartTableDataTableRow * Proxy = NewObject< USmartTableDataTableRow >( this );

    checkf( Proxy, TEXT( "Could not create a row proxy for '%s'" ), *RowName.ToString() );

    UE_LOGFMT( LogSmartTablesData, VeryVerbose, "A row proxy for '{Row}' went up, because a custom cell asked for the object.", RowName );

    Proxy->DataTable = DataTable;
    Proxy->RowName   = RowName;

    RowProxies.Add( RowName, Proxy );

    return Proxy;
}

int32 USmartTableDataTableModel::NaturalRowOfItem_Implementation( UObject * Item )
{

    const USmartTableDataTableRow * Proxy = Cast< USmartTableDataTableRow >( Item );
    if ( !Proxy || Proxy->DataTable != DataTable )
    {
        return INDEX_NONE;
    }

    return RowNames.IndexOfByKey( Proxy->RowName );
}
