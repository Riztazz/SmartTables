// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTableObjectModel.h"

#include "SmartTableConstants.h"

#include "Algo/Transform.h"
#include "Logging/StructuredLog.h"
#include "Misc/AssertionMacros.h"
#include "SmartTableLog.h"
#include "SmartTableRowSetDiff.h"
#include "UObject/Class.h"
#include "UObject/UObjectBaseUtility.h"

void USmartTableObjectModel::SetItems( const TArray< UObject * > & InItems )
{
    const TArray< FName > Before = RowIds();

    Items.Reset( InItems.Num() );

    int32 NullCount = 0;
    for ( UObject * Item : InItems )
    {
        if ( !Item )
        {
            ++NullCount;
            continue;
        }

        Items.Add( Item );
    }

    if ( NullCount > 0 )
    {
        UE_LOGFMT( LogSmartTablesData, Warning, "SetItems dropped {Count} null item(s) of the {Given} handed to it, and those rows are missing. Look at the loop that fills the array. A failed build or cast leaves a gap in it.", NullCount, InItems.Num() );
    }

    RebuildItemRows();

    UE_LOGFMT( LogSmartTablesData, Verbose, "The items model carries {Count} item(s) from here on.", Items.Num() );

    BindingsByClass.Reset();
    NotifyRowSetChanged( FSmartTableRowSetDiff::Between( Before, RowIds() ) );
}

void USmartTableObjectModel::AddItem( UObject * Item )
{
    if ( !Item )
    {
        UE_LOGFMT( LogSmartTablesData, Warning, "AddItem passed over a null item, and no row came up. Look at what produced it. A failed build or cast is the usual cause." );
        return;
    }

    RowByItem.FindOrAdd( TObjectKey< UObject >( Item ), Items.Num() );
    Items.Add( Item );

    FSmartTableRowSetDiff Diff;
    Diff.Arrived.Add( { RowIdOf( Item ), Items.Num() - 1 } );

    NotifyRowSetChanged( Diff );
}

void USmartTableObjectModel::RemoveItem( UObject * Item )
{
    if ( Items.Remove( Item ) == 0 )
    {
        UE_LOGFMT( LogSmartTablesData, Verbose, "RemoveItem({Item}) took out nothing. That item is not in this table.", GetNameSafe( Item ) );
        return;
    }

    RebuildItemRows();

    FSmartTableRowSetDiff Diff;
    Diff.Left.Add( RowIdOf( Item ) );

    NotifyRowSetChanged( Diff );
}

void USmartTableObjectModel::ClearItems()
{
    UE_LOGFMT( LogSmartTablesData, Verbose, "Items model cleared. {Count} item(s) gone.", Items.Num() );

    Items.Reset();
    RowByItem.Reset();

    NotifyRowSetChanged( FSmartTableRowSetDiff::Replaced() );
}

FName USmartTableObjectModel::RowIdOf( const UObject * Item )
{

    return Item ? FName( TEXT( "Item" ), Item->GetUniqueID() + 1 ) : NAME_None;
}

TArray< FName > USmartTableObjectModel::RowIds() const
{
    TArray< FName > Ids;
    Algo::Transform( Items, Ids, []( const TObjectPtr< UObject > & Item )
    {
        return RowIdOf( Item );
    } );

    return Ids;
}

void USmartTableObjectModel::RebuildItemRows()
{
    RowByItem.Reset();
    RowByItem.Reserve( Items.Num() );

    for ( int32 Row = 0; Row < Items.Num(); ++Row )
    {
        RowByItem.FindOrAdd( TObjectKey< UObject >( Items[ Row ] ), Row );
    }
}

int32 USmartTableObjectModel::IndexOfItem( const UObject * Item ) const
{
    const int32 * Row = RowByItem.Find( TObjectKey< UObject >( Item ) );

    return Row ? *Row : INDEX_NONE;
}

void USmartTableObjectModel::SetColumns( const TArray< FSmartTableColumn > & InColumns )
{
    Columns = InColumns;

    ColumnOrderById.Reset();
    ColumnOrderById.Reserve( Columns.Num() );
    for ( int32 Index = 0; Index < Columns.Num(); ++Index )
    {

        ColumnOrderById.FindOrAdd( Columns[ Index ].ColumnId, Index );
    }

    UE_LOGFMT( LogSmartTablesData, Verbose, "Items model given {Count} column(s). Every cached binding goes.", Columns.Num() );

    BindingsByClass.Reset();
    NotifyRowsChanged();
}

void USmartTableObjectModel::ResolveBindingsFor( UClass * Class )
{
    UE_LOGFMT( LogSmartTablesData, Verbose, "{Count} column binding(s) resolve against {Class}. This runs once per item class, never per cell.", Columns.Num(), Class->GetName() );

    TArray< FColumnBinding > & Bindings = BindingsByClass.Add( Class );
    Bindings.Reserve( Columns.Num() );

    TSet< FName > Shadowed;

    for ( int32 Index = 0; Index < Columns.Num(); ++Index )
    {
        const FSmartTableColumn & Column = Columns[ Index ];

        FColumnBinding & Binding = Bindings.AddDefaulted_GetRef();
        Binding.Column           = Index;
        Binding.Value            = SmartTable::ResolveBinding( Class, Column.GetValueBinding() );
        Binding.Sort             = SmartTable::ResolveBinding( Class, Column.GetSortBinding() );

        bool bShadowed = false;
        if ( !Column.ColumnId.IsNone() )
        {
            Shadowed.Add( Column.ColumnId, &bShadowed );
        }

        if ( bShadowed )
        {
            continue;
        }

        if ( SmartTable::ShouldWarnForUnresolvedBinding( Column, Binding.Value ) )
        {
            UE_LOGFMT( LogSmartTablesData, Warning, "Column '{Column}' draws nothing: it binds '{Binding}', which is no property and no pure function on {Class} that takes no parameters. Set the BindingName of the column to one of these, or set the ExpectedItemClass of the table to get a dropdown of them. Available: {Available}", Column.ColumnId, Column.GetValueBinding(), Class->GetName(),
                SmartTable::DescribeBindableNames( Class ) );
        }

        if ( SmartTable::ShouldWarnForUnresolvedSortBinding( Column, Binding.Sort ) )
        {

            UE_LOGFMT( LogSmartTablesSort, Warning, "Column '{Column}' will not sort: its SortBindingName '{Binding}' is no property and no pure function on {Class} that takes no parameters. The column still DRAWS, because its values come from '{Value}', only the order is dead. Set SortBindingName to one of these, or clear it to sort by the shown value. Available: {Available}", Column.ColumnId,
                Column.SortBindingName, Class->GetName(), Column.GetValueBinding(), SmartTable::DescribeBindableNames( Class ) );
        }
    }
}

const USmartTableObjectModel::FColumnBinding * USmartTableObjectModel::FindBinding( const UObject * Item, FName ColumnId )
{
    if ( !Item )
    {
        return nullptr;
    }

    UClass * Class                            = Item->GetClass();
    const TArray< FColumnBinding > * Bindings = BindingsByClass.Find( Class );
    if ( !Bindings )
    {

        for ( auto It = BindingsByClass.CreateIterator(); It; ++It )
        {
            if ( !It.Key().IsValid() )
            {
                It.RemoveCurrent();
            }
        }

        ResolveBindingsFor( Class );
        Bindings = BindingsByClass.Find( Class );

        checkf( Bindings, TEXT( "Bindings for '%s' are missing immediately after being resolved" ), *Class->GetName() );
    }

    const int32 * Index = ColumnOrderById.Find( ColumnId );

    return Index && Bindings->IsValidIndex( *Index ) ? &( *Bindings )[ *Index ] : nullptr;
}

int32 USmartTableObjectModel::GetNumRows_Implementation()
{
    return Items.Num();
}

FText USmartTableObjectModel::GetCellText_Implementation( int32 NaturalRow, FName ColumnId )
{
    if ( !Items.IsValidIndex( NaturalRow ) )
    {

        UE_LOGFMT( LogSmartTablesData, VeryVerbose, "Row {Row} sits past the end of {Count} item(s), and '{Column}' draws nothing.", NaturalRow, Items.Num(), ColumnId );
        return FText::GetEmpty();
    }

    UObject * Item                 = Items[ NaturalRow ];
    const FColumnBinding * Binding = FindBinding( Item, ColumnId );
    if ( !Binding )
    {
        UE_LOGFMT( LogSmartTablesData, VeryVerbose, "'{Column}' is no declared column on this model, and row {Row} draws nothing.", ColumnId, NaturalRow );
        return FText::GetEmpty();
    }

    if ( !Binding->Value.IsValid() )
    {

#if UE_BUILD_SHIPPING
        return FText::GetEmpty();
#else
        return SmartTable::Text::UnresolvedBinding( Columns[ Binding->Column ].GetValueBinding() );
#endif
    }

    return SmartTable::ReadAsText( Binding->Value, Item, Columns[ Binding->Column ] );
}

ESmartTableCellEditor USmartTableObjectModel::GetCellEditor_Implementation( int32 NaturalRow, FName ColumnId )
{
    if ( !Items.IsValidIndex( NaturalRow ) )
    {
        return ESmartTableCellEditor::None;
    }

    const FColumnBinding * Binding = FindBinding( Items[ NaturalRow ], ColumnId );

    return Binding ? SmartTable::EditorFor( Binding->Value ) : ESmartTableCellEditor::None;
}

bool USmartTableObjectModel::SetCellText_Implementation( int32 NaturalRow, FName ColumnId, const FText & Value )
{
    if ( !Items.IsValidIndex( NaturalRow ) )
    {
        return false;
    }

    UObject * Item                 = Items[ NaturalRow ];
    const FColumnBinding * Binding = FindBinding( Item, ColumnId );
    if ( !Binding || !SmartTable::WriteAsText( Binding->Value, Item, Value ) )
    {
        return false;
    }

    NotifyRowChanged( NaturalRow );

    return true;
}

FSmartTableSortKey USmartTableObjectModel::GetCellSortKey_Implementation( int32 NaturalRow, FName ColumnId )
{
    if ( !Items.IsValidIndex( NaturalRow ) )
    {
        return FSmartTableSortKey::MakeEmpty();
    }

    UObject * Item                 = Items[ NaturalRow ];
    const FColumnBinding * Binding = FindBinding( Item, ColumnId );

    return Binding && Binding->Sort.IsValid() ? SmartTable::ReadAsSortKey( Binding->Sort, Item ) : FSmartTableSortKey::MakeEmpty();
}

FName USmartTableObjectModel::GetRowId_Implementation( int32 NaturalRow )
{
    return Items.IsValidIndex( NaturalRow ) ? RowIdOf( Items[ NaturalRow ] ) : NAME_None;
}

UObject * USmartTableObjectModel::GetRowItem_Implementation( int32 NaturalRow )
{
    return Items.IsValidIndex( NaturalRow ) ? Items[ NaturalRow ] : nullptr;
}

int32 USmartTableObjectModel::NaturalRowOfItem_Implementation( UObject * Item )
{

    return IndexOfItem( Item );
}

bool USmartTableObjectModel::MoveRows_Implementation( const TArray< int32 > & NaturalRows, int32 InsertBeforeNaturalRow )
{
    const TArray< int32 > Order = PlanRowMove( Items.Num(), NaturalRows, InsertBeforeNaturalRow );

    if ( Order.IsEmpty() )
    {
        UE_LOGFMT( LogSmartTablesData, Verbose, "MoveRows moved nothing. {Count} row(s) in front of row {Target} is not a move this set of {Rows} can make.", NaturalRows.Num(), InsertBeforeNaturalRow, Items.Num() );
        return false;
    }

    TArray< TObjectPtr< UObject > > Reordered;
    Reordered.Reserve( Order.Num() );

    for ( const int32 Row : Order )
    {
        Reordered.Add( Items[ Row ] );
    }

    Items = MoveTemp( Reordered );
    RebuildItemRows();

    return true;
}
