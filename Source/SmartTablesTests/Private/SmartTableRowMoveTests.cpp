// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTablesTestsMacros.h"

#if SMARTTABLES_WITH_TESTS

#include "AITestsCommon.h"

#include "Framework/Application/SlateApplication.h"
#include "SmartTableDataTableModel.h"
#include "SmartTableObjectModel.h"
#include "SmartTableTestTypes.h"

#define LOCTEXT_NAMESPACE "SmartTablesTest"

UE_DISABLE_OPTIMIZATION_SHIP

namespace SmartTable::Test
{
    static FString OrderAsText( const TArray< int32 > & Order )
    {
        return FString::JoinBy( Order, TEXT( "" ), []( int32 Row )
        {
            return FString::FromInt( Row );
        } );
    }

    static FString SelectionAsText( TArray< int32 > Rows )
    {
        Rows.Sort();

        return OrderAsText( Rows );
    }

    struct FAMovedRowLandsAfterTheRowAboveTheGap : FAITestBase
    {
        virtual bool InstantTest() override
        {
            AITEST_EQUAL( "Down one lands after the row it stepped over", OrderAsText( USmartTableModel::PlanRowMove( 4, { 1 }, 3 ) ), FString( TEXT( "0213" ) ) );
            AITEST_EQUAL( "Up one lands in front of the row it stepped over", OrderAsText( USmartTableModel::PlanRowMove( 4, { 3 }, 1 ) ), FString( TEXT( "0312" ) ) );
            AITEST_EQUAL( "Gap zero is in front of everything", OrderAsText( USmartTableModel::PlanRowMove( 3, { 2 }, 0 ) ), FString( TEXT( "201" ) ) );
            AITEST_EQUAL( "Gap NumRows is past the end", OrderAsText( USmartTableModel::PlanRowMove( 3, { 0 }, 3 ) ), FString( TEXT( "120" ) ) );

            AITEST_EQUAL( "Dropping a row on its own top edge changes nothing", OrderAsText( USmartTableModel::PlanRowMove( 3, { 1 }, 1 ) ), FString( TEXT( "012" ) ) );
            AITEST_EQUAL( "...and on its own bottom edge too", OrderAsText( USmartTableModel::PlanRowMove( 3, { 1 }, 2 ) ), FString( TEXT( "012" ) ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAMovedRowLandsAfterTheRowAboveTheGap, "SmartTables.RowMove.AMovedRowLandsAfterTheRowAboveTheGap" );

    struct FTheGapCountsRowsThatAreAboutToLeave : FAITestBase
    {
        virtual bool InstantTest() override
        {
            AITEST_EQUAL( "A gap below the moving row does not drift", OrderAsText( USmartTableModel::PlanRowMove( 5, { 1 }, 3 ) ), FString( TEXT( "02134" ) ) );

            AITEST_EQUAL( "...and it does not drift by the number of them either", OrderAsText( USmartTableModel::PlanRowMove( 6, { 0, 1 }, 4 ) ), FString( TEXT( "230145" ) ) );

            AITEST_EQUAL( "A gap above the moving rows needs no correction", OrderAsText( USmartTableModel::PlanRowMove( 6, { 4, 5 }, 1 ) ), FString( TEXT( "045123" ) ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FTheGapCountsRowsThatAreAboutToLeave, "SmartTables.RowMove.TheGapCountsRowsThatAreAboutToLeave" );

    struct FScatteredRowsMoveAsOneBlockInTheirOwnOrder : FAITestBase
    {
        virtual bool InstantTest() override
        {
            AITEST_EQUAL( "Scattered rows arrive together", OrderAsText( USmartTableModel::PlanRowMove( 6, { 0, 2, 4 }, 6 ) ), FString( TEXT( "135024" ) ) );

            AITEST_EQUAL( "...in their own order and not the order they were named", OrderAsText( USmartTableModel::PlanRowMove( 6, { 4, 0, 2 }, 6 ) ), FString( TEXT( "135024" ) ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FScatteredRowsMoveAsOneBlockInTheirOwnOrder, "SmartTables.RowMove.ScatteredRowsMoveAsOneBlockInTheirOwnOrder" );

    struct FAMoveThatCannotMeanAnythingIsRefused : FAITestBase
    {
        virtual bool InstantTest() override
        {
            AITEST_TRUE( "No rows named is no move", USmartTableModel::PlanRowMove( 4, {}, 2 ).IsEmpty() );
            AITEST_TRUE( "A row past the end is refused", USmartTableModel::PlanRowMove( 4, { 4 }, 2 ).IsEmpty() );
            AITEST_TRUE( "A negative row is refused", USmartTableModel::PlanRowMove( 4, { -1 }, 2 ).IsEmpty() );
            AITEST_TRUE( "A gap past the end is refused", USmartTableModel::PlanRowMove( 4, { 0 }, 5 ).IsEmpty() );
            AITEST_TRUE( "A negative gap is refused", USmartTableModel::PlanRowMove( 4, { 0 }, -1 ).IsEmpty() );
            AITEST_TRUE( "An empty table is refused", USmartTableModel::PlanRowMove( 0, { 0 }, 0 ).IsEmpty() );

            AITEST_TRUE( "The same row named twice is refused", USmartTableModel::PlanRowMove( 4, { 1, 1 }, 3 ).IsEmpty() );

            AITEST_TRUE( "Moving every row is refused", USmartTableModel::PlanRowMove( 3, { 0, 1, 2 }, 0 ).IsEmpty() );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAMoveThatCannotMeanAnythingIsRefused, "SmartTables.RowMove.AMoveThatCannotMeanAnythingIsRefused" );

    struct FOnlyAModelThatOwnsItsRowsMovesThem : FAITestBase
    {
        virtual bool InstantTest() override
        {
            UObject * First  = NamedRow( TEXT( "Vanta" ) );
            UObject * Second = NamedRow( TEXT( "Garris" ) );
            UObject * Third  = NamedRow( TEXT( "Keller" ) );

            USmartTableObjectModel * Model = NamedRowModel( { First, Second, Third } );

            AITEST_TRUE( "The items model moves its rows", Model->MoveRows( { 2 }, 0 ) );
            AITEST_TRUE( "...and the array really moved", Model->GetItems()[ 0 ] == Third );
            AITEST_TRUE( "...taking the rest down with it", Model->GetItems()[ 1 ] == First );
            AITEST_EQUAL( "...so the moved row answers to its new number", Model->IndexOfItem( Third ), 0 );

            AITEST_FALSE( "A refused move gives back false", Model->MoveRows( { 9 }, 0 ) );
            AITEST_TRUE( "...and leaves the array alone", Model->GetItems()[ 0 ] == Third );

            USmartTableDataTableModel * FromAsset = NewObject< USmartTableDataTableModel >();
            AITEST_FALSE( "A model that does not own its order does not move it", FromAsset->MoveRows( { 0 }, 1 ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FOnlyAModelThatOwnsItsRowsMovesThem, "SmartTables.RowMove.OnlyAModelThatOwnsItsRowsMovesThem" );

    struct FADropReportsTheSelectionOnce : FAITestBase
    {
        virtual bool InstantTest() override
        {
            AITEST_TRUE( "Slate is up, so a table builds its widget", FSlateApplication::IsInitialized() );

            UObject * Garris = NamedRow( TEXT( "Garris" ) );
            UObject * Keller = NamedRow( TEXT( "Keller" ) );

            USmartTableTestHarness * Table    = NamedRowTable( { NamedRow( TEXT( "Vanta" ) ), Garris, Keller, NamedRow( TEXT( "Osei" ) ), NamedRow( TEXT( "Brand" ) ) } );
            const TSharedRef< SWidget > Built = Table->TakeWidget();

            Table->SetRowsSelected( { 1, 2 }, true );
            AITEST_EQUAL( "Two rows are picked up", SelectionAsText( Table->GetSelectedRows() ), FString( TEXT( "12" ) ) );

            Table->OnSelectionChanged.AddDynamic( Table, &USmartTableTestHarness::HearSelection );

            AITEST_TRUE( "The drop lands", Table->MoveRowsTo( { 1, 2 }, 4 ) );

            AITEST_EQUAL( "One drop, one report", Table->SelectionsHeard.Num(), 1 );
            AITEST_EQUAL( "...naming the rows where they landed", SelectionAsText( Table->SelectionsHeard[ 0 ] ), FString( TEXT( "23" ) ) );

            const TArray< UObject * > Picked = Table->GetSelectedItems();
            AITEST_TRUE( "...which hold the two rows picked up", Picked.Num() == 2 && Picked.Contains( Garris ) && Picked.Contains( Keller ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FADropReportsTheSelectionOnce, "SmartTables.RowMove.ADropReportsTheSelectionOnce" );

    struct FADropThatClearsTheSortReportsTheSelectionOnce : FAITestBase
    {
        virtual bool InstantTest() override
        {
            AITEST_TRUE( "Slate is up, so a table builds its widget", FSlateApplication::IsInitialized() );

            UObject * Garris = NamedRow( TEXT( "Garris" ) );
            UObject * Keller = NamedRow( TEXT( "Keller" ) );

            USmartTableTestHarness * Table    = NamedRowTable( { NamedRow( TEXT( "Vanta" ) ), Garris, Keller, NamedRow( TEXT( "Osei" ) ), NamedRow( TEXT( "Brand" ) ) } );
            const TSharedRef< SWidget > Built = Table->TakeWidget();

            Table->SortByColumn( TEXT( "RowName" ), ESmartTableSortMode::Ascending );
            AITEST_FALSE( "The rows are sorted", Table->GetSortSpec().IsEmpty() );

            Table->SetRowsSelected( { 1, 2 }, true );
            AITEST_EQUAL( "Two rows are picked up", SelectionAsText( Table->GetSelectedRows() ), FString( TEXT( "12" ) ) );

            Table->OnSelectionChanged.AddDynamic( Table, &USmartTableTestHarness::HearSelection );

            AITEST_TRUE( "The drop lands", Table->MoveRowsTo( { 1, 2 }, 4 ) );
            AITEST_TRUE( "...and takes the sort off", Table->GetSortSpec().IsEmpty() );

            AITEST_EQUAL( "One drop, one report, with the sort cleared on the way", Table->SelectionsHeard.Num(), 1 );
            AITEST_EQUAL( "...naming the rows where they landed", SelectionAsText( Table->SelectionsHeard[ 0 ] ), FString( TEXT( "23" ) ) );

            const TArray< UObject * > Picked = Table->GetSelectedItems();
            AITEST_TRUE( "...which hold the two rows picked up", Picked.Num() == 2 && Picked.Contains( Garris ) && Picked.Contains( Keller ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FADropThatClearsTheSortReportsTheSelectionOnce, "SmartTables.RowMove.ADropThatClearsTheSortReportsTheSelectionOnce" );

    struct FADropThatClearsTheSortSavesTheLayout : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTableTestLayoutStore * Store = NewObject< USmartTableTestLayoutStore >();

            USmartTableTestHarness * Table = NamedRowTable( { NamedRow( TEXT( "Vanta" ) ), NamedRow( TEXT( "Garris" ) ), NamedRow( TEXT( "Keller" ) ) } );
            Table->SetTableId( TEXT( "DropTestTable" ) );
            Table->SetLayoutStore( Store );

            Table->CycleColumnSort( TEXT( "RowName" ) );
            AITEST_FALSE( "A header click sorts and files the sort", Store->Stored.SortSpec.IsEmpty() );

            AITEST_TRUE( "A move from code lands", Table->MoveRowsTo( { 0 }, 3 ) );
            AITEST_TRUE( "...takes the sort off", Table->GetSortSpec().IsEmpty() );
            AITEST_FALSE( "...and files nothing", Store->Stored.SortSpec.IsEmpty() );

            Table->CycleColumnSort( TEXT( "RowName" ) );
            AITEST_FALSE( "A second header click sorts again", Table->GetSortSpec().IsEmpty() );

            Table->MoveRowsFromDrop( { Table->GetModel()->KeepRow( 0 ) }, 3 );
            AITEST_TRUE( "A drop takes the sort off", Table->GetSortSpec().IsEmpty() );
            AITEST_TRUE( "...and files the layout without it", Store->Stored.SortSpec.IsEmpty() );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FADropThatClearsTheSortSavesTheLayout, "SmartTables.RowMove.ADropThatClearsTheSortSavesTheLayout" );

    struct FTheCaretFollowsItsRowThroughADrop : FAITestBase
    {
        virtual bool InstantTest() override
        {
            AITEST_TRUE( "Slate is up, so a table builds its widget", FSlateApplication::IsInitialized() );

            USmartTableTestHarness * Table    = NamedRowTable( { NamedRow( TEXT( "Vanta" ) ), NamedRow( TEXT( "Garris" ) ), NamedRow( TEXT( "Keller" ) ), NamedRow( TEXT( "Osei" ) ), NamedRow( TEXT( "Brand" ) ) } );
            const TSharedRef< SWidget > Built = Table->TakeWidget();

            Table->MoveSelection( 1 );
            Table->MoveSelection( 1 );
            Table->MoveSelection( 1 );
            Table->SetRowsSelected( { 1 }, true );
            AITEST_EQUAL( "The caret walks to Keller", Table->GetFocusedRow(), 2 );
            AITEST_EQUAL( "...with Garris picked up beside it", SelectionAsText( Table->GetSelectedRows() ), FString( TEXT( "12" ) ) );

            AITEST_TRUE( "The drop lands", Table->MoveRowsTo( { 1, 2 }, 4 ) );

            AITEST_EQUAL( "Both rows land in front of Brand", SelectionAsText( Table->GetSelectedRows() ), FString( TEXT( "23" ) ) );
            AITEST_EQUAL( "...and the caret stays on Keller", Table->GetFocusedRow(), 3 );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FTheCaretFollowsItsRowThroughADrop, "SmartTables.RowMove.TheCaretFollowsItsRowThroughADrop" );
}

UE_ENABLE_OPTIMIZATION_SHIP

#undef LOCTEXT_NAMESPACE

#endif
