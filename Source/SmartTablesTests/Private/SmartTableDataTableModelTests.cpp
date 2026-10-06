// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTablesTestsMacros.h"

#if SMARTTABLES_WITH_TESTS

#include "AITestsCommon.h"

#include "Engine/DataTable.h"
#include "SmartTableDataTableModel.h"
#include "SmartTableTestTypes.h"
#include "UObject/UObjectHash.h"

#define LOCTEXT_NAMESPACE "SmartTablesTest"

UE_DISABLE_OPTIMIZATION_SHIP

namespace SmartTable::Test
{
    static UDataTable * TableOfThreeRows()
    {
        UDataTable * Table = NewObject< UDataTable >();
        Table->RowStruct   = FSmartTableTestRow::StaticStruct();

        FSmartTableTestRow Heavy;
        Heavy.Callsign     = TEXT( "Vanta-1" );
        Heavy.Designation  = TEXT( "OBJ-000001" );
        Heavy.DisplayName  = FText::FromString( TEXT( "Vanta One" ) );
        Heavy.MassTonnes   = 90000.0;
        Heavy.Integrity    = 12.5f;
        Heavy.CrewCapacity = 7;
        Heavy.bVisited     = true;
        Heavy.Status       = ESmartTableTestStatus::Claimed;
        Table->AddRow( TEXT( "Row_1" ), Heavy );

        FSmartTableTestRow Light;
        Light.Callsign   = TEXT( "Asteroid #2" );
        Light.MassTonnes = 120.0;
        Light.Status     = ESmartTableTestStatus::Unsurveyed;
        Table->AddRow( TEXT( "Row_2" ), Light );

        FSmartTableTestRow Middling;
        Middling.Callsign   = TEXT( "Asteroid #10" );
        Middling.MassTonnes = 4200.0;
        Middling.Status     = ESmartTableTestStatus::Derelict;
        Table->AddRow( TEXT( "Row_3" ), Middling );

        return Table;
    }

    static USmartTableDataTableModel * RowModelOver( UDataTable * Table, const TArray< FName > & ColumnIds )
    {
        USmartTableDataTableModel * Model = NewObject< USmartTableDataTableModel >();

        TArray< FSmartTableColumn > Columns;
        for ( FName Id : ColumnIds )
        {
            FSmartTableColumn & Column = Columns.AddDefaulted_GetRef();
            Column.ColumnId            = Id;
        }

        Model->SetColumns( Columns );
        Model->SetDataTable( Table );

        return Model;
    }

    static FString RowTextAt( USmartTableModel * Model, int32 PresentedRow, FName ColumnId )
    {
        return Model->GetCellText( Model->PresentedToNaturalRow( PresentedRow ), ColumnId ).ToString();
    }

    static int32 NaturalRowOf( USmartTableModel * Model, FName RowName )
    {
        for ( int32 NaturalRow = 0; NaturalRow < Model->GetNumRows(); ++NaturalRow )
        {
            if ( Model->GetRowId( NaturalRow ) == RowName )
            {
                return NaturalRow;
            }
        }

        return INDEX_NONE;
    }

    static FSmartTableSortSpec SortedBy( FName ColumnId, ESmartTableSortMode Mode )
    {
        FSmartTableSortSpec Spec;
        FSmartTableSortColumn & Level = Spec.Columns.AddDefaulted_GetRef();
        Level.ColumnId                = ColumnId;
        Level.Mode                    = Mode;

        return Spec;
    }

    struct FDataTableColumnsBindToTheRowStruct : FAITestBase
    {
        virtual bool InstantTest() override
        {
            UDataTable * Table                = TableOfThreeRows();
            USmartTableDataTableModel * Model = RowModelOver( Table, { TEXT( "Callsign" ), TEXT( "Designation" ), TEXT( "DisplayName" ), TEXT( "MassTonnes" ), TEXT( "CrewCapacity" ), TEXT( "bVisited" ), TEXT( "Status" ) } );

            AITEST_TRUE( "The model holds the table it was given", Model->GetDataTable() == Table );
            AITEST_EQUAL( "Every row is a row", Model->GetNumRows(), 3 );

            const int32 Row = NaturalRowOf( Model, TEXT( "Row_1" ) );
            AITEST_NOT_EQUAL( "The row is there to read", Row, int32( INDEX_NONE ) );

            AITEST_EQUAL( "A string field reads", Model->GetCellText( Row, TEXT( "Callsign" ) ).ToString(), FString( TEXT( "Vanta-1" ) ) );
            AITEST_EQUAL( "A name field reads", Model->GetCellText( Row, TEXT( "Designation" ) ).ToString(), FString( TEXT( "OBJ-000001" ) ) );
            AITEST_EQUAL( "A text field reads", Model->GetCellText( Row, TEXT( "DisplayName" ) ).ToString(), FString( TEXT( "Vanta One" ) ) );
            AITEST_EQUAL( "An int field reads", Model->GetCellText( Row, TEXT( "CrewCapacity" ) ).ToString(), FString( TEXT( "7" ) ) );
            AITEST_EQUAL( "A bool field reads as a word", Model->GetCellText( Row, TEXT( "bVisited" ) ).ToString(), FString( TEXT( "true" ) ) );
            AITEST_EQUAL( "An enum field reads as its name", Model->GetCellText( Row, TEXT( "Status" ) ).ToString(), FString( TEXT( "Claimed" ) ) );

            AITEST_TRUE( "An undeclared column reads as nothing", Model->GetCellText( Row, TEXT( "NotDeclared" ) ).IsEmpty() );
            AITEST_TRUE( "A row past the end reads as nothing", Model->GetCellText( 9, TEXT( "Callsign" ) ).IsEmpty() );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FDataTableColumnsBindToTheRowStruct, "SmartTables.DataTable.ColumnsBindToTheRowStruct" );

    struct FDataTableRowsAreIdentifiedByTheirOwnName : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTableDataTableModel * Model = RowModelOver( TableOfThreeRows(), { TEXT( "Callsign" ) } );

            AITEST_NOT_EQUAL( "Every row answers to its row name", NaturalRowOf( Model, TEXT( "Row_1" ) ), int32( INDEX_NONE ) );
            AITEST_NOT_EQUAL( "...", NaturalRowOf( Model, TEXT( "Row_2" ) ), int32( INDEX_NONE ) );
            AITEST_NOT_EQUAL( "...", NaturalRowOf( Model, TEXT( "Row_3" ) ), int32( INDEX_NONE ) );
            AITEST_EQUAL( "A name nothing answers to is nowhere", NaturalRowOf( Model, TEXT( "Row_9" ) ), INDEX_NONE );
            AITEST_TRUE( "...and a row past the end identifies nothing", Model->GetRowId( 9 ).IsNone() );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FDataTableRowsAreIdentifiedByTheirOwnName, "SmartTables.DataTable.RowsAreIdentifiedByTheirOwnName" );

    struct FDataTableOrdersByValueNotByRenderedText : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTableDataTableModel * Model = RowModelOver( TableOfThreeRows(), { TEXT( "Callsign" ), TEXT( "MassTonnes" ) } );
            const FName Before                = Model->GetRowId( 0 );

            AITEST_TRUE( "The model takes the sort", Model->SortRows( SortedBy( TEXT( "MassTonnes" ), ESmartTableSortMode::Ascending ) ) );

            AITEST_EQUAL( "The lightest is first", RowTextAt( Model, 0, TEXT( "Callsign" ) ), FString( TEXT( "Asteroid #2" ) ) );
            AITEST_EQUAL( "...then the middle", RowTextAt( Model, 1, TEXT( "Callsign" ) ), FString( TEXT( "Asteroid #10" ) ) );
            AITEST_EQUAL( "...then the heaviest", RowTextAt( Model, 2, TEXT( "Callsign" ) ), FString( TEXT( "Vanta-1" ) ) );

            Model->SortRows( SortedBy( TEXT( "Callsign" ), ESmartTableSortMode::Ascending ) );
            AITEST_EQUAL( "#2 comes before #10", RowTextAt( Model, 0, TEXT( "Callsign" ) ), FString( TEXT( "Asteroid #2" ) ) );
            AITEST_EQUAL( "...", RowTextAt( Model, 1, TEXT( "Callsign" ) ), FString( TEXT( "Asteroid #10" ) ) );

            AITEST_EQUAL( "Natural row 0 is where it started", Model->GetRowId( 0 ).ToString(), Before.ToString() );

            AITEST_TRUE( "The model takes a filter", Model->ApplyTextFilter( FText::FromString( TEXT( "Asteroid" ) ), { TEXT( "Callsign" ), TEXT( "MassTonnes" ) } ) );
            AITEST_EQUAL( "...and hides what does not match", Model->GetNumPresentedRows(), 2 );
            AITEST_EQUAL( "...without losing it", Model->GetNumRows(), 3 );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FDataTableOrdersByValueNotByRenderedText, "SmartTables.DataTable.OrdersByValueNotByRenderedText" );

    struct FRowProxiesAreMadeOnlyWhenACellAsks : FAITestBase
    {
        virtual bool InstantTest() override
        {
            UDataTable * Table                = TableOfThreeRows();
            USmartTableDataTableModel * Model = RowModelOver( Table, { TEXT( "Callsign" ) } );

            const int32 Row = NaturalRowOf( Model, TEXT( "Row_2" ) );
            UObject * Item  = Model->GetRowItem( Row );
            AITEST_NOT_NULL( "A custom cell gets an item", Item );
            AITEST_TRUE( "...and the same one next time", Model->GetRowItem( Row ) == Item );
            AITEST_TRUE( "...while a different row gets its own", Model->GetRowItem( NaturalRowOf( Model, TEXT( "Row_1" ) ) ) != Item );
            AITEST_TRUE( "A row past the end gets nothing", Model->GetRowItem( 9 ) == nullptr );

            USmartTableDataTableRow * Proxy = Cast< USmartTableDataTableRow >( Item );
            AITEST_NOT_NULL( "...of the type a Blueprint cell expects", Proxy );
            AITEST_TRUE( "...naming the table", Proxy->DataTable == Table );
            AITEST_EQUAL( "...and the row", Proxy->RowName.ToString(), FString( TEXT( "Row_2" ) ) );

            Model->SetDataTable( TableOfThreeRows() );
            AITEST_TRUE( "A new table invalidates the old proxies", Model->GetRowItem( Row ) != Item );

            Model->SetDataTable( nullptr );
            AITEST_EQUAL( "No table is no rows", Model->GetNumRows(), 0 );
            AITEST_TRUE( "...and no items", Model->GetRowItem( 0 ) == nullptr );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FRowProxiesAreMadeOnlyWhenACellAsks, "SmartTables.DataTable.RowProxiesAreMadeOnlyWhenACellAsks" );

    struct FStructModelNeedsNoObjectPerRow : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTableTestStructModel * Model = NewObject< USmartTableTestStructModel >();

            TArray< FSmartTableTestRow > Rows;
            for ( int32 Index = 0; Index < 3; ++Index )
            {
                FSmartTableTestRow & Row = Rows.AddDefaulted_GetRef();
                Row.Callsign             = FString::Printf( TEXT( "Asteroid #%d" ), ( Index + 1 ) * 9 );
                Row.MassTonnes           = 1000.0 * ( 3 - Index );
            }

            Model->SetRows( MoveTemp( Rows ) );
            AITEST_EQUAL( "The model counts its structs", Model->GetNumRows(), 3 );
            AITEST_EQUAL( "...and they all draw", Model->GetNumPresentedRows(), 3 );

            Model->SortRows( SortedBy( TEXT( "MassTonnes" ), ESmartTableSortMode::Ascending ) );
            AITEST_EQUAL( "The lightest is first", RowTextAt( Model, 0, TEXT( "Callsign" ) ), FString( TEXT( "Asteroid #27" ) ) );

            Model->SortRows( SortedBy( TEXT( "Callsign" ), ESmartTableSortMode::Ascending ) );
            AITEST_EQUAL( "#9 before #18", RowTextAt( Model, 0, TEXT( "Callsign" ) ), FString( TEXT( "Asteroid #9" ) ) );
            AITEST_EQUAL( "...and #18 before #27", RowTextAt( Model, 1, TEXT( "Callsign" ) ), FString( TEXT( "Asteroid #18" ) ) );

            AITEST_EQUAL( "A row is identified by what the model says", Model->GetRowId( 0 ).ToString(), FString( TEXT( "Asteroid #9" ) ) );

            AITEST_TRUE( "A struct model has no item to hand a custom cell", Model->GetRowItem( 0 ) == nullptr );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FStructModelNeedsNoObjectPerRow, "SmartTables.Model.StructModelNeedsNoObjectPerRow" );

    struct FAnEditedValueSurvivesEverySort : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTableTestStructModel * Model = NewObject< USmartTableTestStructModel >();

            TArray< FSmartTableTestRow > Rows;
            for ( int32 Index = 0; Index < 3; ++Index )
            {
                FSmartTableTestRow & Row = Rows.AddDefaulted_GetRef();
                Row.Callsign             = FString::Printf( TEXT( "Rock-%d" ), Index );
                Row.MassTonnes           = 100.0 * ( Index + 1 );
            }

            Model->SetRows( MoveTemp( Rows ) );

            Model->SetMass( 0, 12345.0 );
            AITEST_EQUAL( "The edit reaches the model", Model->GetRows()[ 0 ].MassTonnes, 12345.0 );
            AITEST_EQUAL( "...and the row is still row 0", RowTextAt( Model, 0, TEXT( "Callsign" ) ), FString( TEXT( "Rock-0" ) ) );

            Model->SortRows( SortedBy( TEXT( "MassTonnes" ), ESmartTableSortMode::Ascending ) );
            Model->SortRows( SortedBy( TEXT( "MassTonnes" ), ESmartTableSortMode::Descending ) );
            Model->SortRows( FSmartTableSortSpec() );

            AITEST_EQUAL( "Back in arrival order", RowTextAt( Model, 0, TEXT( "Callsign" ) ), FString( TEXT( "Rock-0" ) ) );
            AITEST_EQUAL( "...and the edit survived all three", Model->GetRows()[ 0 ].MassTonnes, 12345.0 );
            AITEST_EQUAL( "...as the table would read it", Model->GetCellSortKey( 0, TEXT( "MassTonnes" ) ).Number, 12345.0 );

            Model->SortRows( SortedBy( TEXT( "MassTonnes" ), ESmartTableSortMode::Ascending ) );
            AITEST_EQUAL( "The edit decides the new order", RowTextAt( Model, 2, TEXT( "Callsign" ) ), FString( TEXT( "Rock-0" ) ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAnEditedValueSurvivesEverySort, "SmartTables.Model.AnEditedValueSurvivesEverySort" );

    struct FARowStructSwapResolvesTheBindingsAgain : FAITestBase
    {
        virtual bool InstantTest() override
        {
            UDataTable * Table                = TableOfThreeRows();
            USmartTableDataTableModel * Model = RowModelOver( Table, { TEXT( "Callsign" ) } );

            AITEST_EQUAL( "The column resolves against the row struct it was given", Model->GetCellEditor( 0, TEXT( "Callsign" ) ), ESmartTableCellEditor::Text );

            Table->RowStruct = FSmartTableColumnLayout::StaticStruct();
            Table->OnDataTableChanged().Broadcast();

            AITEST_EQUAL( "The swap throws the stale binding away", Model->GetCellEditor( 0, TEXT( "Callsign" ) ), ESmartTableCellEditor::None );

            USmartTableDataTableModel * Rebound = RowModelOver( Table, { TEXT( "ColumnId" ) } );
            AITEST_EQUAL( "A column that IS on the new struct resolves", Rebound->GetCellEditor( 0, TEXT( "ColumnId" ) ), ESmartTableCellEditor::Text );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FARowStructSwapResolvesTheBindingsAgain, "SmartTables.DataTable.ARowStructSwapResolvesTheBindingsAgain" );

    struct FASortKeyIsEmptyWhileNothingIsBound : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTableDataTableModel * Model = RowModelOver( TableOfThreeRows(), { TEXT( "Callsign" ) } );

            AITEST_FALSE( "A bound column has a key to sort by", Model->GetCellSortKey( 0, TEXT( "Callsign" ) ).IsEmpty() );

            Model->SetDataTable( nullptr );

            AITEST_TRUE( "A cleared table sorts by nothing", Model->GetCellSortKey( 0, TEXT( "Callsign" ) ).IsEmpty() );
            AITEST_TRUE( "...and draws nothing, as it always did", Model->GetCellText( 0, TEXT( "Callsign" ) ).IsEmpty() );
            AITEST_TRUE( "An id that names no column is empty too", Model->GetCellSortKey( 0, TEXT( "Nonsense" ) ).IsEmpty() );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FASortKeyIsEmptyWhileNothingIsBound, "SmartTables.DataTable.ASortKeyIsEmptyWhileNothingIsBound" );

    static int32 ProxyCount( UObject * Outer )
    {
        int32 Count = 0;
        ForEachObjectWithOuter( Outer, [ &Count ]( UObject * Object )
        {
            Count += Object->IsA< USmartTableDataTableRow >() ? 1 : 0;
        } );

        return Count;
    }

    struct FFindingARowForAStrangerBuildsNoProxy : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTableDataTableModel * Model = RowModelOver( TableOfThreeRows(), { TEXT( "Callsign" ) } );

            USmartTableDataTableRow * Stranger = NewObject< USmartTableDataTableRow >();

            AITEST_EQUAL( "A stranger is in no row", Model->NaturalRowOfItem( Stranger ), int32( INDEX_NONE ) );
            AITEST_EQUAL( "...and asking built nothing", ProxyCount( Model ), 0 );

            AITEST_EQUAL( "Null is in no row either", Model->NaturalRowOfItem( nullptr ), int32( INDEX_NONE ) );
            AITEST_EQUAL( "...also for free", ProxyCount( Model ), 0 );

            UObject * Real = Model->GetRowItem( 1 );
            AITEST_NOT_NULL( "A row hands out a stand-in when something asks for one", Real );
            AITEST_EQUAL( "...exactly one", ProxyCount( Model ), 1 );

            AITEST_EQUAL( "...which is found at its own row", Model->NaturalRowOfItem( Real ), 1 );
            AITEST_EQUAL( "...without building another", ProxyCount( Model ), 1 );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FFindingARowForAStrangerBuildsNoProxy, "SmartTables.DataTable.FindingARowForAStrangerBuildsNoProxy" );
}
UE_ENABLE_OPTIMIZATION_SHIP

#undef LOCTEXT_NAMESPACE
#endif
