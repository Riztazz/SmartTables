// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTablesTestsMacros.h"

#if SMARTTABLES_WITH_TESTS

#include "AITestsCommon.h"

#include "SmartTable.h"
#include "SmartTableDataTableModel.h"
#include "SmartTableObjectModel.h"
#include "SmartTableTestTypes.h"

#define LOCTEXT_NAMESPACE "SmartTablesTest"

UE_DISABLE_OPTIMIZATION_SHIP

namespace SmartTable::Test
{
    static FSmartTableSortSpec SortedByName( ESmartTableSortMode Mode )
    {
        FSmartTableSortSpec Spec;
        FSmartTableSortColumn & Level = Spec.Columns.AddDefaulted_GetRef();
        Level.ColumnId                = TEXT( "RowName" );
        Level.Mode                    = Mode;

        return Spec;
    }

    static FString ItemModelNameAt( USmartTableObjectModel * Model, int32 PresentedRow )
    {
        return Model->GetCellText( Model->PresentedToNaturalRow( PresentedRow ), TEXT( "RowName" ) ).ToString();
    }

    struct FItemsAreOwnedAndNullsNeverDraw : FAITestBase
    {
        virtual bool InstantTest() override
        {
            UObject * First  = NamedRow( TEXT( "Vanta" ) );
            UObject * Second = NamedRow( TEXT( "Garris" ) );

            USmartTableObjectModel * Model = NamedRowModel( { First, nullptr, Second, nullptr } );

            AITEST_EQUAL( "Nulls are dropped, not drawn", Model->GetNumRows(), 2 );
            AITEST_EQUAL( "...and the survivors keep their order", Model->GetItems().Num(), 2 );
            AITEST_EQUAL( "The first item is where it was put", Model->IndexOfItem( First ), 0 );
            AITEST_EQUAL( "...and so is the second", Model->IndexOfItem( Second ), 1 );
            AITEST_EQUAL( "An item that was never added is nowhere", Model->IndexOfItem( NamedRow( TEXT( "Keller" ) ) ), INDEX_NONE );

            AITEST_TRUE( "The model holds its items", Model->GetItems()[ 0 ] == First );

            AITEST_TRUE( "A row hands back its item", Model->GetRowItem( 0 ) == First );
            AITEST_TRUE( "Out of range hands back nothing", Model->GetRowItem( 7 ) == nullptr );

            AITEST_TRUE( "A row is identified by its item", Model->GetRowId( 1 ) == USmartTableObjectModel::RowIdOf( Second ) );
            AITEST_TRUE( "...and out of range identifies nothing", Model->GetRowId( 7 ).IsNone() );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FItemsAreOwnedAndNullsNeverDraw, "SmartTables.ObjectModel.ItemsAreOwnedAndNullsNeverDraw" );

    struct FAddRemoveClearMoveThePresentationWithThem : FAITestBase
    {
        virtual bool InstantTest() override
        {
            UObject * First                = NamedRow( TEXT( "Vanta" ) );
            USmartTableObjectModel * Model = NamedRowModel( { First } );

            UObject * Second = NamedRow( TEXT( "Garris" ) );
            Model->AddItem( Second );
            AITEST_EQUAL( "An added item is a row", Model->GetNumRows(), 2 );
            AITEST_EQUAL( "...and it draws", Model->GetNumPresentedRows(), 2 );

            Model->AddItem( nullptr );
            AITEST_EQUAL( "A null add changes nothing", Model->GetNumRows(), 2 );

            Model->RemoveItem( First );
            AITEST_EQUAL( "A removed item stops being a row", Model->GetNumRows(), 1 );
            AITEST_EQUAL( "...and stops drawing", Model->GetNumPresentedRows(), 1 );
            AITEST_EQUAL( "...leaving the other one", ItemModelNameAt( Model, 0 ), FString( TEXT( "Garris" ) ) );

            Model->RemoveItem( NamedRow( TEXT( "NeverAdded" ) ) );
            AITEST_EQUAL( "Removing something absent changes nothing", Model->GetNumRows(), 1 );

            Model->ClearItems();
            AITEST_EQUAL( "Clearing empties natural space", Model->GetNumRows(), 0 );
            AITEST_EQUAL( "...and presented space with it", Model->GetNumPresentedRows(), 0 );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAddRemoveClearMoveThePresentationWithThem, "SmartTables.ObjectModel.AddRemoveClearMoveThePresentationWithThem" );

    struct FNotificationsSayWhichKindOfChange : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTableObjectModel * Model = NamedRowModel( { NamedRow( TEXT( "Vanta" ) ), NamedRow( TEXT( "Garris" ) ) } );

            FSmartTablePresentationChange LastChange;
            int32 LastRow = -2;

            Model->OnPresentationChanged().AddLambda( [ &LastChange, &LastRow ]( const FSmartTablePresentationChange & Change )
            {
                LastChange = Change;
                LastRow    = Change.NaturalRow;
            } );

            Model->SortRows( SortedByName( ESmartTableSortMode::Ascending ) );
            AITEST_EQUAL( "Garris sorted to the top", ItemModelNameAt( Model, 0 ), FString( TEXT( "Garris" ) ) );

            Model->NotifyRowChanged( 1 );
            AITEST_TRUE( "A value tick says the cells are stale", LastChange.bCellsStale );
            AITEST_FALSE( "...and says nothing else moved", LastChange.bOrderMoved || LastChange.bRowSetMoved );
            AITEST_EQUAL( "...and it names its row", LastRow, 1 );
            AITEST_EQUAL( "...and nothing moved", ItemModelNameAt( Model, 0 ), FString( TEXT( "Garris" ) ) );

            Model->NotifyRowsChanged();

            AITEST_TRUE( "A bulk value change says the cells are stale", LastChange.bCellsStale );
            AITEST_TRUE( "...and that the order was re-applied over them", LastChange.bOrderMoved );
            AITEST_EQUAL( "...and names no row", LastRow, INDEX_NONE );

            Model->NotifyNumRowsChanged();
            AITEST_TRUE( "Rows appearing or leaving moves the row set", LastChange.bRowSetMoved );

            Model->AddItem( NamedRow( TEXT( "Asteroid #2" ) ) );
            AITEST_EQUAL( "A new row lands in the active order", ItemModelNameAt( Model, 0 ), FString( TEXT( "Asteroid #2" ) ) );
            AITEST_EQUAL( "...ahead of the ones already there", ItemModelNameAt( Model, 1 ), FString( TEXT( "Garris" ) ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FNotificationsSayWhichKindOfChange, "SmartTables.ObjectModel.NotificationsSayWhichKindOfChange" );

    struct FChangingColumnsReresolvesTheBindings : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTableObjectModel * Model = NamedRowModel( { NamedRow( TEXT( "Vanta" ) ) } );
            AITEST_EQUAL( "The declared column reads", ItemModelNameAt( Model, 0 ), FString( TEXT( "Vanta" ) ) );

            FSmartTableColumn Renamed;
            Renamed.ColumnId    = TEXT( "Whatever" );
            Renamed.BindingName = TEXT( "RowName" );
            Model->SetColumns( { Renamed } );

            AITEST_EQUAL( "A re-declared column resolves afresh", Model->GetCellText( 0, TEXT( "Whatever" ) ).ToString(), FString( TEXT( "Vanta" ) ) );
            AITEST_TRUE( "...and the old id resolves to nothing", Model->GetCellText( 0, TEXT( "RowName" ) ).IsEmpty() );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FChangingColumnsReresolvesTheBindings, "SmartTables.ObjectModel.ChangingColumnsReresolvesTheBindings" );

    struct FSupersededSortResultIsDropped : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTableObjectModel * Model = NamedRowModel( { NamedRow( TEXT( "Charlie" ) ), NamedRow( TEXT( "Alpha" ) ), NamedRow( TEXT( "Bravo" ) ) } );

            TArray< TUniqueFunction< void() > > Pending;
            Model->SetWorkDispatcher( [ &Pending ]( TUniqueFunction< void() > Work )
            {
                Pending.Add( MoveTemp( Work ) );
            } );

            Model->SortRows( SortedByName( ESmartTableSortMode::Ascending ) );
            Model->SortRows( SortedByName( ESmartTableSortMode::Descending ) );

            AITEST_EQUAL( "Both questions were dispatched", Pending.Num(), 2 );
            AITEST_TRUE( "...and the table is told it is waiting", Model->IsBusy() );

            Pending[ 1 ]();
            AITEST_EQUAL( "The current question is answered", ItemModelNameAt( Model, 0 ), FString( TEXT( "Charlie" ) ) );

            AITEST_TRUE( "...but the older question is still outstanding", Model->IsBusy() );

            Pending[ 0 ]();
            AITEST_EQUAL( "The superseded answer is dropped", ItemModelNameAt( Model, 0 ), FString( TEXT( "Charlie" ) ) );
            AITEST_FALSE( "...and the last arrival ends the wait", Model->IsBusy() );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FSupersededSortResultIsDropped, "SmartTables.ObjectModel.SupersededSortResultIsDropped" );

    struct FBusyIsRaisedAndClearedAroundTheWork : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTableObjectModel * Model = NamedRowModel( { NamedRow( TEXT( "Charlie" ) ), NamedRow( TEXT( "Alpha" ) ) } );

            TArray< bool > BusyStates;
            Model->OnBusyChanged().AddLambda( [ &BusyStates ]( bool bBusy )
            {
                BusyStates.Add( bBusy );
            } );

            TArray< TUniqueFunction< void() > > Pending;
            Model->SetWorkDispatcher( [ &Pending ]( TUniqueFunction< void() > Work )
            {
                Pending.Add( MoveTemp( Work ) );
            } );

            AITEST_FALSE( "A model with nothing in flight is not busy", Model->IsBusy() );

            Model->SortRows( SortedByName( ESmartTableSortMode::Ascending ) );
            AITEST_TRUE( "Dispatching raises busy", Model->IsBusy() );
            AITEST_EQUAL( "...and says so once", BusyStates.Num(), 1 );
            AITEST_TRUE( "...as true", BusyStates[ 0 ] );

            Pending[ 0 ]();
            AITEST_FALSE( "The answer clears it", Model->IsBusy() );
            AITEST_EQUAL( "...and says so once more", BusyStates.Num(), 2 );
            AITEST_FALSE( "...as false", BusyStates[ 1 ] );

            Model->SetWorkDispatcher( FSmartTableWorkDispatcher() );
            Model->SortRows( SortedByName( ESmartTableSortMode::Descending ) );
            AITEST_EQUAL( "A synchronous sort announces no waiting", BusyStates.Num(), 2 );
            AITEST_EQUAL( "...and is already applied", ItemModelNameAt( Model, 0 ), FString( TEXT( "Charlie" ) ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FBusyIsRaisedAndClearedAroundTheWork, "SmartTables.ObjectModel.BusyIsRaisedAndClearedAroundTheWork" );

    struct FAnAbandonedSortStillGivesBusyBack : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTableObjectModel * Model = NamedRowModel( { NamedRow( TEXT( "Charlie" ) ), NamedRow( TEXT( "Alpha" ) ) } );

            TArray< TUniqueFunction< void() > > Pending;
            Model->SetWorkDispatcher( [ &Pending ]( TUniqueFunction< void() > Work )
            {
                Pending.Add( MoveTemp( Work ) );
            } );

            Model->SortRows( SortedByName( ESmartTableSortMode::Ascending ) );
            Model->SortRows( SortedByName( ESmartTableSortMode::Descending ) );
            AITEST_EQUAL( "Both were dispatched", Pending.Num(), 2 );
            AITEST_TRUE( "...and the model is waiting", Model->IsBusy() );

            Pending[ 0 ]();
            AITEST_TRUE( "A dropped result does not clear a sort still in flight", Model->IsBusy() );

            Pending[ 1 ]();
            AITEST_FALSE( "The last arrival clears it", Model->IsBusy() );
            AITEST_EQUAL( "...and the winning order is the one that was asked for last", ItemModelNameAt( Model, 0 ), FString( TEXT( "Charlie" ) ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAnAbandonedSortStillGivesBusyBack, "SmartTables.ObjectModel.AnAbandonedSortStillGivesBusyBack" );

    struct FASortAbandonedEntirelyDoesNotStrandBusy : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTableObjectModel * Model = NamedRowModel( { NamedRow( TEXT( "Charlie" ) ), NamedRow( TEXT( "Alpha" ) ) } );

            TArray< TUniqueFunction< void() > > Pending;
            Model->SetWorkDispatcher( [ &Pending ]( TUniqueFunction< void() > Work )
            {
                Pending.Add( MoveTemp( Work ) );
            } );

            Model->SortRows( SortedByName( ESmartTableSortMode::Ascending ) );
            AITEST_TRUE( "Waiting on the sort", Model->IsBusy() );

            Model->SetWorkDispatcher( FSmartTableWorkDispatcher() );
            Model->SortRows( SortedByName( ESmartTableSortMode::Descending ) );

            Pending[ 0 ]();
            AITEST_FALSE( "The stale result leaves nothing held", Model->IsBusy() );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FASortAbandonedEntirelyDoesNotStrandBusy, "SmartTables.ObjectModel.ASortAbandonedEntirelyDoesNotStrandBusy" );

    struct FEveryWriteToTheItemsMovesTheRowsWithThem : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTableDataTableRow * Alpha   = NamedRow( TEXT( "Alpha" ) );
            USmartTableDataTableRow * Bravo   = NamedRow( TEXT( "Bravo" ) );
            USmartTableDataTableRow * Charlie = NamedRow( TEXT( "Charlie" ) );

            USmartTableObjectModel * Model = NamedRowModel( { Alpha, Bravo, Charlie } );

            AITEST_EQUAL( "SetItems files each item at its row", Model->IndexOfItem( Charlie ), 2 );

            Model->RemoveItem( Alpha );
            AITEST_EQUAL( "Taking one out moves the rest up", Model->IndexOfItem( Charlie ), 1 );
            AITEST_EQUAL( "...and the one taken out is gone", Model->IndexOfItem( Alpha ), INDEX_NONE );

            Model->AddItem( Alpha );
            AITEST_EQUAL( "Appending files it at the end", Model->IndexOfItem( Alpha ), 2 );

            Model->ClearItems();
            AITEST_EQUAL( "Clearing leaves nothing filed", Model->IndexOfItem( Charlie ), INDEX_NONE );

            Model->AddItem( Charlie );
            Model->AddItem( Bravo );

            AITEST_EQUAL( "The item put back first holds row 0", Model->IndexOfItem( Charlie ), 0 );
            AITEST_EQUAL( "...and the next holds row 1", Model->IndexOfItem( Bravo ), 1 );
            AITEST_EQUAL( "...and one never put back is gone", Model->IndexOfItem( Alpha ), INDEX_NONE );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FEveryWriteToTheItemsMovesTheRowsWithThem, "SmartTables.ObjectModel.EveryWriteToTheItemsMovesTheRowsWithThem" );

    struct FRemoveItemLeavesTheModelInForceAlone : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTableObjectModel * Own = NewObject< USmartTableObjectModel >();
            Own->SetItems( { NamedRow( TEXT( "Vanta" ) ) } );

            USmartTable * Table = NewObject< USmartTable >();
            Table->SetModel( Own );
            Table->RemoveItem( NamedRow( TEXT( "Garris" ) ) );

            AITEST_TRUE( "A removal leaves a model given to SetModel in force", Table->GetModel() == Own );
            AITEST_EQUAL( "...with every row it holds", Own->GetNumRows(), 1 );

            USmartTable * Bare = NewObject< USmartTable >();
            Bare->RemoveItem( NamedRow( TEXT( "Keller" ) ) );
            AITEST_NULL( "A table with no model is given none by a removal", Bare->GetModel() );

            UObject * Kept = NamedRow( TEXT( "Osei" ) );
            UObject * Gone = NamedRow( TEXT( "Brand" ) );

            USmartTable * Fed = NewObject< USmartTable >();
            Fed->SetItems( { Kept, Gone } );
            Fed->RemoveItem( Gone );
            AITEST_EQUAL( "A row from SetItems still comes out", Fed->GetModel()->GetNumRows(), 1 );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FRemoveItemLeavesTheModelInForceAlone, "SmartTables.Table.RemoveItemLeavesTheModelInForceAlone" );
}
UE_ENABLE_OPTIMIZATION_SHIP

#undef LOCTEXT_NAMESPACE
#endif
