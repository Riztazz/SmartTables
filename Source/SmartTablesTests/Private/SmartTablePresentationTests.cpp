// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTablesTestsMacros.h"

#if SMARTTABLES_WITH_TESTS

#include "AITestsCommon.h"

#include "SmartTableDataTableModel.h"
#include "SmartTableDispatcher.h"
#include "SmartTableObjectModel.h"

#define LOCTEXT_NAMESPACE "SmartTablesTest"

UE_DISABLE_OPTIMIZATION_SHIP

namespace SmartTable::Test
{
    static USmartTableObjectModel * ModelOverNames( const TArray< FString > & Names )
    {
        USmartTableObjectModel * Model = NewObject< USmartTableObjectModel >();

        FSmartTableColumn Column;
        Column.ColumnId = TEXT( "RowName" );
        Model->SetColumns( { Column } );

        TArray< UObject * > Items;
        for ( const FString & Name : Names )
        {
            USmartTableDataTableRow * Row = NewObject< USmartTableDataTableRow >();
            Row->RowName                  = FName( *Name );
            Items.Add( Row );
        }

        Model->SetItems( Items );

        return Model;
    }

    static FString PresentedName( USmartTableObjectModel * Model, int32 PresentedRow )
    {
        return Model->GetCellText( Model->PresentedToNaturalRow( PresentedRow ), TEXT( "RowName" ) ).ToString();
    }

    struct FSortingLeavesNaturalSpaceWhereItWas : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTableObjectModel * Model = ModelOverNames( { TEXT( "Vanta" ), TEXT( "Asteroid #10" ), TEXT( "Asteroid #2" ) } );

            FSmartTableSortSpec Spec;
            FSmartTableSortColumn & Level = Spec.Columns.AddDefaulted_GetRef();
            Level.ColumnId                = TEXT( "RowName" );
            Level.Mode                    = ESmartTableSortMode::Ascending;

            AITEST_TRUE( "The model takes the sort", Model->SortRows( Spec ) );

            AITEST_EQUAL( "Natural row 0 has not moved", Model->GetCellText( 0, TEXT( "RowName" ) ).ToString(), FString( TEXT( "Vanta" ) ) );
            AITEST_EQUAL( "Nor has natural row 2", Model->GetCellText( 2, TEXT( "RowName" ) ).ToString(), FString( TEXT( "Asteroid #2" ) ) );

            AITEST_EQUAL( "Presented row 0", PresentedName( Model, 0 ), FString( TEXT( "Asteroid #2" ) ) );
            AITEST_EQUAL( "Presented row 1", PresentedName( Model, 1 ), FString( TEXT( "Asteroid #10" ) ) );
            AITEST_EQUAL( "Presented row 2", PresentedName( Model, 2 ), FString( TEXT( "Vanta" ) ) );

            AITEST_EQUAL( "Natural 2 draws first", Model->NaturalToPresentedRow( 2 ), 0 );
            AITEST_EQUAL( "Natural 0 draws last", Model->NaturalToPresentedRow( 0 ), 2 );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FSortingLeavesNaturalSpaceWhereItWas, "SmartTables.Presentation.SortingLeavesNaturalSpaceWhereItWas" );

    struct FUnsortingIsTheIdentityMap : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTableObjectModel * Model = ModelOverNames( { TEXT( "Charlie" ), TEXT( "Alpha" ), TEXT( "Bravo" ) } );

            FSmartTableSortSpec Spec;
            FSmartTableSortColumn & Level = Spec.Columns.AddDefaulted_GetRef();
            Level.ColumnId                = TEXT( "RowName" );
            Level.Mode                    = ESmartTableSortMode::Ascending;
            Model->SortRows( Spec );

            AITEST_EQUAL( "Sorted", PresentedName( Model, 0 ), FString( TEXT( "Alpha" ) ) );

            Model->SortRows( FSmartTableSortSpec() );

            AITEST_EQUAL( "Back to arrival order", PresentedName( Model, 0 ), FString( TEXT( "Charlie" ) ) );
            AITEST_EQUAL( "...", PresentedName( Model, 1 ), FString( TEXT( "Alpha" ) ) );
            AITEST_EQUAL( "...", PresentedName( Model, 2 ), FString( TEXT( "Bravo" ) ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FUnsortingIsTheIdentityMap, "SmartTables.Presentation.UnsortingIsTheIdentityMap" );

    struct FFilterHidesRowsWithoutLosingThem : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTableObjectModel * Model = ModelOverNames( { TEXT( "Vanta" ), TEXT( "Garris" ), TEXT( "Vanta Two" ) } );

            AITEST_TRUE( "The model takes the filter", Model->ApplyTextFilter( FText::FromString( TEXT( "Vanta" ) ), { TEXT( "RowName" ) } ) );
            AITEST_EQUAL( "Two rows match", Model->GetNumPresentedRows(), 2 );

            AITEST_EQUAL( "Natural space is untouched", Model->GetNumRows(), 3 );
            AITEST_EQUAL( "The filtered row draws nowhere", Model->NaturalToPresentedRow( 1 ), INDEX_NONE );

            Model->ApplyTextFilter( FText::GetEmpty(), { TEXT( "RowName" ) } );
            AITEST_EQUAL( "Clearing brings them all back", Model->GetNumPresentedRows(), 3 );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FFilterHidesRowsWithoutLosingThem, "SmartTables.Presentation.FilterHidesRowsWithoutLosingThem" );

    struct FSynchronousDispatcherIsImmediate : FAITestBase
    {
        virtual bool InstantTest() override
        {
            bool bRan = false;
            SmartTable::Dispatchers::Synchronous()( [ &bRan ]()
            {
                bRan = true;
            } );

            AITEST_TRUE( "Work ran before the dispatch returned", bRan );

            bool bMarshalled = false;
            SmartTable::Dispatchers::RunOnGameThread( [ &bMarshalled ]()
            {
                bMarshalled = true;
            } );

            AITEST_TRUE( "Already on the game thread, so it ran here", bMarshalled );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FSynchronousDispatcherIsImmediate, "SmartTables.Async.SynchronousDispatcherIsImmediate" );

    struct FAsyncSortAppliesThroughTheDispatcher : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTableObjectModel * Model = ModelOverNames( { TEXT( "Charlie" ), TEXT( "Alpha" ), TEXT( "Bravo" ) } );

            Model->SetWorkDispatcher( SmartTable::Dispatchers::Synchronous() );

            FSmartTableSortSpec Spec;
            FSmartTableSortColumn & Level = Spec.Columns.AddDefaulted_GetRef();
            Level.ColumnId                = TEXT( "RowName" );
            Level.Mode                    = ESmartTableSortMode::Ascending;
            Model->SortRows( Spec );

            AITEST_EQUAL( "The async result is already applied", PresentedName( Model, 0 ), FString( TEXT( "Alpha" ) ) );
            AITEST_FALSE( "...and the table was told the work finished", Model->IsBusy() );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAsyncSortAppliesThroughTheDispatcher, "SmartTables.Async.AsyncSortAppliesThroughTheDispatcher" );

    static FSmartTableSortSpec ByName()
    {
        FSmartTableSortSpec Spec;
        FSmartTableSortColumn & Level = Spec.Columns.AddDefaulted_GetRef();
        Level.ColumnId                = TEXT( "RowName" );
        Level.Mode                    = ESmartTableSortMode::Ascending;

        return Spec;
    }

    struct FHeard
    {
        FSmartTablePresentationChange Change;
        int32 Presented = 0;
        int32 Rows      = 0;
    };

    static void RecordEvery( USmartTableModel * Model, TArray< FHeard > & Into )
    {
        Model->OnPresentationChanged().AddLambda( [ Model, &Into ]( const FSmartTablePresentationChange & Change )
        {
            Into.Add( { Change, Model->GetNumPresentedRows(), Model->GetNumRows() } );
        } );
    }

    struct FAValuesSweepWaitsForAPendingRowSetChange : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTableObjectModel * Model = ModelOverNames( { TEXT( "Charlie" ), TEXT( "Alpha" ), TEXT( "Bravo" ), TEXT( "Delta" ) } );

            TArray< TUniqueFunction< void() > > Pending;
            Model->SetWorkDispatcher( [ &Pending ]( TUniqueFunction< void() > Work )
            {
                Pending.Add( MoveTemp( Work ) );
            } );

            Model->SortRows( ByName() );

            Model->RemoveItem( Model->GetItems()[ 0 ] );

            TArray< FHeard > Heard;
            RecordEvery( Model, Heard );

            Model->NotifyRowsChanged();

            AITEST_TRUE( "The sweep announces nothing while the rebuild is still out", Heard.IsEmpty() );

            for ( TUniqueFunction< void() > & Work : Pending )
            {
                Work();
            }

            AITEST_FALSE( "It is announced once the rows land", Heard.IsEmpty() );

            AITEST_TRUE( "...carrying the row-set fact", Heard.Last().Change.bRowSetMoved );
            AITEST_TRUE( "...and the stale-cells fact with it", Heard.Last().Change.bCellsStale );

            for ( const FHeard & One : Heard )
            {
                AITEST_TRUE( "No announcement ever claimed more presented rows than the model has", One.Presented <= One.Rows );
            }

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAValuesSweepWaitsForAPendingRowSetChange, "SmartTables.Presentation.AValuesSweepWaitsForAPendingRowSetChange" );

    struct FASingleRowTickWaitsForAPendingRebuild : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTableObjectModel * Model = ModelOverNames( { TEXT( "Charlie" ), TEXT( "Alpha" ), TEXT( "Bravo" ), TEXT( "Delta" ) } );

            TArray< TUniqueFunction< void() > > Pending;
            Model->SetWorkDispatcher( [ &Pending ]( TUniqueFunction< void() > Work )
            {
                Pending.Add( MoveTemp( Work ) );
            } );

            Model->SortRows( ByName() );
            Model->RemoveItem( Model->GetItems()[ 0 ] );

            TArray< FHeard > Heard;
            RecordEvery( Model, Heard );

            Model->NotifyRowChanged( 1 );

            AITEST_TRUE( "The tick announces nothing while the rebuild is out", Heard.IsEmpty() );

            for ( TUniqueFunction< void() > & Work : Pending )
            {
                Work();
            }

            AITEST_FALSE( "It lands with the rebuild", Heard.IsEmpty() );

            AITEST_TRUE( "...as stale cells", Heard.Last().Change.bCellsStale );
            AITEST_EQUAL( "...naming no row", Heard.Last().Change.NaturalRow, INDEX_NONE );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FASingleRowTickWaitsForAPendingRebuild, "SmartTables.Presentation.ASingleRowTickWaitsForAPendingRebuild" );

    struct FASingleCellTickWaitsForAPendingRebuildToo : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTableObjectModel * Model = ModelOverNames( { TEXT( "Charlie" ), TEXT( "Alpha" ), TEXT( "Bravo" ), TEXT( "Delta" ) } );

            TArray< TUniqueFunction< void() > > Pending;
            Model->SetWorkDispatcher( [ &Pending ]( TUniqueFunction< void() > Work )
            {
                Pending.Add( MoveTemp( Work ) );
            } );

            Model->SortRows( ByName() );
            Model->RemoveItem( Model->GetItems()[ 0 ] );

            TArray< FHeard > Heard;
            RecordEvery( Model, Heard );

            Model->NotifyCellChanged( 1, TEXT( "RowName" ) );

            AITEST_TRUE( "A cell tick waits on the rebuild the same way a row tick does", Heard.IsEmpty() );

            for ( TUniqueFunction< void() > & Work : Pending )
            {
                Work();
            }

            AITEST_FALSE( "It lands with the rebuild", Heard.IsEmpty() );
            AITEST_EQUAL( "...naming no row", Heard.Last().Change.NaturalRow, INDEX_NONE );
            AITEST_EQUAL( "...and no column, which went with it", Heard.Last().Change.ColumnId, FName( NAME_None ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FASingleCellTickWaitsForAPendingRebuildToo, "SmartTables.Presentation.ASingleCellTickWaitsForAPendingRebuildToo" );

    struct FACellTickNamesItsColumn : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTableObjectModel * Model = ModelOverNames( { TEXT( "Charlie" ), TEXT( "Alpha" ) } );

            TArray< FHeard > Heard;
            RecordEvery( Model, Heard );

            Model->NotifyCellChanged( 1, TEXT( "RowName" ) );

            AITEST_FALSE( "With nothing in flight it goes out at once", Heard.IsEmpty() );
            AITEST_TRUE( "...as stale cells", Heard.Last().Change.bCellsStale );
            AITEST_EQUAL( "...naming its row", Heard.Last().Change.NaturalRow, 1 );
            AITEST_EQUAL( "...and its column", Heard.Last().Change.ColumnId, FName( TEXT( "RowName" ) ) );

            Heard.Reset();
            Model->NotifyCellChanged( 1, NAME_None );

            AITEST_FALSE( "A cell tick naming no column still goes out", Heard.IsEmpty() );
            AITEST_EQUAL( "...as the whole row, which is what it meant", Heard.Last().Change.NaturalRow, 1 );
            AITEST_EQUAL( "...naming no column", Heard.Last().Change.ColumnId, FName( NAME_None ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FACellTickNamesItsColumn, "SmartTables.Presentation.ACellTickNamesItsColumn" );

    struct FARowTickIsNeverFoldedBehindASortThatWillNotLand : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTableObjectModel * Model = ModelOverNames( { TEXT( "Charlie" ), TEXT( "Alpha" ), TEXT( "Bravo" ) } );

            TArray< TUniqueFunction< void() > > Pending;
            Model->SetWorkDispatcher( [ &Pending ]( TUniqueFunction< void() > Work )
            {
                Pending.Add( MoveTemp( Work ) );
            } );

            Model->SortRows( ByName() );

            Model->SortRows( FSmartTableSortSpec() );

            TArray< FHeard > Heard;
            RecordEvery( Model, Heard );

            Model->NotifyRowChanged( 1 );

            AITEST_FALSE( "The tick is announced rather than folded away", Heard.IsEmpty() );
            AITEST_TRUE( "...as stale cells", Heard.Last().Change.bCellsStale );
            AITEST_EQUAL( "...naming its row", Heard.Last().Change.NaturalRow, 1 );

            const int32 HeardBefore = Heard.Num();
            for ( TUniqueFunction< void() > & Work : Pending )
            {
                Work();
            }

            AITEST_EQUAL( "A superseded sort landing announces nothing", Heard.Num(), HeardBefore );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FARowTickIsNeverFoldedBehindASortThatWillNotLand, "SmartTables.Presentation.ARowTickIsNeverFoldedBehindASortThatWillNotLand" );

    struct FAnInlineApplyDoesNotSpendADispatchedSortsClaim : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTableObjectModel * Model = ModelOverNames( { TEXT( "Charlie" ), TEXT( "Alpha" ), TEXT( "Bravo" ) } );

            TArray< TUniqueFunction< void() > > Pending;
            Model->SetWorkDispatcher( [ &Pending ]( TUniqueFunction< void() > Work )
            {
                Pending.Add( MoveTemp( Work ) );
            } );

            Model->SortRows( ByName() );
            AITEST_TRUE( "A dispatched sort makes the model busy", Model->IsBusy() );

            Model->SortRows( FSmartTableSortSpec() );
            AITEST_TRUE( "An inline apply leaves the dispatched sort still counted", Model->IsBusy() );

            for ( TUniqueFunction< void() > & Work : Pending )
            {
                Work();
            }

            AITEST_FALSE( "Only the dispatched sort landing clears busy", Model->IsBusy() );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAnInlineApplyDoesNotSpendADispatchedSortsClaim, "SmartTables.Async.AnInlineApplyDoesNotSpendADispatchedSortsClaim" );

    struct FAccumulatingChangesLosesNoFact : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FSmartTablePresentationChange Change;

            AITEST_FALSE( "A change nobody asserted says nothing", Change.IsAnything() );

            Change.Add( FSmartTablePresentationChange::RowSet() );
            Change.Add( FSmartTablePresentationChange::EveryRowsValues() );

            AITEST_TRUE( "The row set moved", Change.bRowSetMoved );
            AITEST_TRUE( "...and the cells are stale", Change.bCellsStale );
            AITEST_TRUE( "...and the order moved, which both of them imply", Change.bOrderMoved );

            FSmartTablePresentationChange OneRow = FSmartTablePresentationChange::OneRowsValues( 7 );
            AITEST_EQUAL( "One row's tick names its row", OneRow.NaturalRow, 7 );

            OneRow.Add( FSmartTablePresentationChange::OneRowsValues( 7 ) );
            AITEST_EQUAL( "The same row again is still about that row", OneRow.NaturalRow, 7 );

            OneRow.Add( FSmartTablePresentationChange::OneRowsValues( 9 ) );
            AITEST_EQUAL( "A second row makes it about no single row", OneRow.NaturalRow, INDEX_NONE );

            FSmartTablePresentationChange Widened = FSmartTablePresentationChange::OneRowsValues( 3 );
            Widened.Add( FSmartTablePresentationChange::Order() );
            AITEST_EQUAL( "So does anything wider", Widened.NaturalRow, INDEX_NONE );

            FSmartTablePresentationChange Untouched = FSmartTablePresentationChange::OneRowsValues( 3 );
            Untouched.Add( FSmartTablePresentationChange() );
            AITEST_EQUAL( "Folding in nothing changes nothing", Untouched.NaturalRow, 3 );
            AITEST_TRUE( "...and leaves what was there", Untouched.bCellsStale );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAccumulatingChangesLosesNoFact, "SmartTables.Presentation.AccumulatingChangesLosesNoFact" );

    struct FAChangeSaysHowMuchItReaches : FAITestBase
    {
        virtual bool InstantTest() override
        {
            AITEST_TRUE( "A change nobody asserted reaches nothing", FSmartTablePresentationChange().Scope() == ESmartTablePresentationScope::Nothing );
            AITEST_TRUE( "One row's values reach that row", FSmartTablePresentationChange::OneRowsValues( 4 ).Scope() == ESmartTablePresentationScope::Row );
            AITEST_TRUE( "One cell's value reaches that cell", FSmartTablePresentationChange::OneCellsValue( 4, TEXT( "Mass" ) ).Scope() == ESmartTablePresentationScope::Cell );
            AITEST_TRUE( "Every row's values reach the screen", FSmartTablePresentationChange::EveryRowsValues().Scope() == ESmartTablePresentationScope::Screen );
            AITEST_TRUE( "A new order reaches the screen", FSmartTablePresentationChange::Order().Scope() == ESmartTablePresentationScope::Screen );
            AITEST_TRUE( "A new row set reaches the screen", FSmartTablePresentationChange::RowSet().Scope() == ESmartTablePresentationScope::Screen );

            FSmartTablePresentationChange Impossible = FSmartTablePresentationChange::OneCellsValue( 4, TEXT( "Mass" ) );
            Impossible.bOrderMoved                   = true;
            AITEST_TRUE( "A named cell beside a moved order reaches the screen, whatever it names", Impossible.Scope() == ESmartTablePresentationScope::Screen );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAChangeSaysHowMuchItReaches, "SmartTables.Presentation.AChangeSaysHowMuchItReaches" );

    struct FAFoldedColumnSurvivesOnlyWhileItsRowDoes : FAITestBase
    {
        static FSmartTablePresentationChange Cell( int32 Row, const TCHAR * Column )
        {
            return FSmartTablePresentationChange::OneCellsValue( Row, Column );
        }

        virtual bool InstantTest() override
        {
            FSmartTablePresentationChange Same = Cell( 7, TEXT( "Mass" ) );
            Same.Add( Cell( 7, TEXT( "Mass" ) ) );
            AITEST_TRUE( "The same cell twice is still that cell", Same.Scope() == ESmartTablePresentationScope::Cell );
            AITEST_EQUAL( "...naming its column", Same.ColumnId, FName( TEXT( "Mass" ) ) );

            FSmartTablePresentationChange TwoColumns = Cell( 7, TEXT( "Mass" ) );
            TwoColumns.Add( Cell( 7, TEXT( "Speed" ) ) );
            AITEST_TRUE( "Two columns of one row widen to the row", TwoColumns.Scope() == ESmartTablePresentationScope::Row );
            AITEST_EQUAL( "...which names no column", TwoColumns.ColumnId, FName( NAME_None ) );
            AITEST_EQUAL( "...and keeps the row", TwoColumns.NaturalRow, 7 );

            FSmartTablePresentationChange CellThenRow = Cell( 7, TEXT( "Mass" ) );
            CellThenRow.Add( FSmartTablePresentationChange::OneRowsValues( 7 ) );
            AITEST_TRUE( "A cell then its whole row is the row", CellThenRow.Scope() == ESmartTablePresentationScope::Row );

            FSmartTablePresentationChange RowThenCell = FSmartTablePresentationChange::OneRowsValues( 7 );
            RowThenCell.Add( Cell( 7, TEXT( "Mass" ) ) );
            AITEST_TRUE( "And the other way round is the row too", RowThenCell.Scope() == ESmartTablePresentationScope::Row );

            FSmartTablePresentationChange OneColumnTwoRows = Cell( 7, TEXT( "Mass" ) );
            OneColumnTwoRows.Add( Cell( 9, TEXT( "Mass" ) ) );
            AITEST_TRUE( "One column in two rows widens to the screen", OneColumnTwoRows.Scope() == ESmartTablePresentationScope::Screen );
            AITEST_EQUAL( "...dropping the column with the row it belonged to", OneColumnTwoRows.ColumnId, FName( NAME_None ) );
            AITEST_EQUAL( "...so no reader is left a column with no row", OneColumnTwoRows.NaturalRow, INDEX_NONE );

            FSmartTablePresentationChange Sorted = Cell( 7, TEXT( "Mass" ) );
            Sorted.Add( FSmartTablePresentationChange::Order() );
            AITEST_TRUE( "Anything wider widens it", Sorted.Scope() == ESmartTablePresentationScope::Screen );
            AITEST_EQUAL( "...and takes the column with it", Sorted.ColumnId, FName( NAME_None ) );

            FSmartTablePresentationChange Fresh;
            Fresh.Add( Cell( 7, TEXT( "Mass" ) ) );
            AITEST_TRUE( "Folding a cell into nothing keeps the cell", Fresh.Scope() == ESmartTablePresentationScope::Cell );
            AITEST_EQUAL( "...both halves of it", Fresh.NaturalRow, 7 );
            AITEST_EQUAL( "...column included", Fresh.ColumnId, FName( TEXT( "Mass" ) ) );

            FSmartTablePresentationChange Untouched = Cell( 7, TEXT( "Mass" ) );
            Untouched.Add( FSmartTablePresentationChange() );
            AITEST_TRUE( "Folding nothing into a cell changes nothing", Untouched.Scope() == ESmartTablePresentationScope::Cell );
            AITEST_EQUAL( "...column included", Untouched.ColumnId, FName( TEXT( "Mass" ) ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAFoldedColumnSurvivesOnlyWhileItsRowDoes, "SmartTables.Presentation.AFoldedColumnSurvivesOnlyWhileItsRowDoes" );

    struct FARowSetChangeIsAnnouncedOnlyWhenTheRowsLand : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTableObjectModel * Model = ModelOverNames( { TEXT( "Charlie" ), TEXT( "Alpha" ), TEXT( "Bravo" ), TEXT( "Delta" ) } );

            TArray< TUniqueFunction< void() > > Pending;
            Model->SetWorkDispatcher( [ &Pending ]( TUniqueFunction< void() > Work )
            {
                Pending.Add( MoveTemp( Work ) );
            } );

            Model->SortRows( ByName() );

            TArray< FHeard > Heard;
            RecordEvery( Model, Heard );

            Model->RemoveItem( Model->GetItems()[ 0 ] );

            AITEST_TRUE( "Nothing is announced while the rebuild is still out", Heard.IsEmpty() );

            for ( TUniqueFunction< void() > & Work : Pending )
            {
                Work();
            }

            AITEST_FALSE( "The change is announced once the rows land", Heard.IsEmpty() );
            AITEST_TRUE( "...and it says the row SET moved, so selection is remapped", Heard.Last().Change.bRowSetMoved );

            for ( const FHeard & One : Heard )
            {
                AITEST_TRUE( "No announcement ever claimed more presented rows than the model has", One.Presented <= One.Rows );
            }

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FARowSetChangeIsAnnouncedOnlyWhenTheRowsLand, "SmartTables.Presentation.ARowSetChangeIsAnnouncedOnlyWhenTheRowsLand" );

    struct FARowSetChangeSurvivesASortThatOvertakesIt : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTableObjectModel * Model = ModelOverNames( { TEXT( "Charlie" ), TEXT( "Alpha" ), TEXT( "Bravo" ), TEXT( "Delta" ) } );

            TArray< TUniqueFunction< void() > > Pending;
            Model->SetWorkDispatcher( [ &Pending ]( TUniqueFunction< void() > Work )
            {
                Pending.Add( MoveTemp( Work ) );
            } );

            Model->SortRows( ByName() );

            TArray< FHeard > Heard;
            RecordEvery( Model, Heard );

            Model->RemoveItem( Model->GetItems()[ 0 ] );
            Model->SortRows( ByName() );

            for ( TUniqueFunction< void() > & Work : Pending )
            {
                Work();
            }

            AITEST_TRUE( "The later re-sort does not drop the row-set fact", Heard.Last().Change.bRowSetMoved );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FARowSetChangeSurvivesASortThatOvertakesIt, "SmartTables.Presentation.ARowSetChangeSurvivesASortThatOvertakesIt" );
}
UE_ENABLE_OPTIMIZATION_SHIP

#undef LOCTEXT_NAMESPACE
#endif
