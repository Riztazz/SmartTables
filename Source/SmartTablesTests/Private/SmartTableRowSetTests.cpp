// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTablesTestsMacros.h"

#if SMARTTABLES_WITH_TESTS

#include "AITestsCommon.h"

#include "Engine/DataTable.h"
#include "SmartTableDataTableModel.h"
#include "SmartTableObjectModel.h"
#include "SmartTableRowSetDiff.h"
#include "SmartTableTestTypes.h"

#define LOCTEXT_NAMESPACE "SmartTablesTest"

UE_DISABLE_OPTIMIZATION_SHIP

namespace SmartTable::Test
{
    static TArray< FName > RowSetIds( std::initializer_list< const TCHAR * > Names )
    {
        TArray< FName > Ids;
        for ( const TCHAR * Name : Names )
        {
            Ids.Add( Name );
        }

        return Ids;
    }

    static FString RowSetArrivedAsText( const FSmartTableRowSetDiff & Diff )
    {
        return FString::JoinBy( Diff.Arrived, TEXT( "," ), []( const FSmartTableKeptRow & Row )
        {
            return FString::Printf( TEXT( "%s@%d" ), *Row.RowId.ToString(), Row.NaturalRow );
        } );
    }

    static FString ItemRowId( const UObject * Item )
    {
        return USmartTableObjectModel::RowIdOf( Item ).ToString();
    }

    static FString RowSetLeftAsText( const FSmartTableRowSetDiff & Diff )
    {
        TArray< FString > Names;
        for ( const FName RowId : Diff.Left )
        {
            Names.Add( RowId.ToString() );
        }
        Names.Sort();

        return FString::Join( Names, TEXT( "," ) );
    }

    struct FRowSetListener
    {
        TArray< FSmartTablePresentationChange > Heard;

        FRowSetListener( USmartTableModel & InModel, FAutomationTestBase & InRunner )
            : Model( &InModel )
            , Runner( InRunner )
        {
            Handle = InModel.OnPresentationChanged().AddLambda( [ this ]( const FSmartTablePresentationChange & Change )
            {
                Heard.Add( Change );
            } );
        }

        ~FRowSetListener()
        {
            if ( USmartTableModel * Listened = Model.Get() )
            {
                Listened->OnPresentationChanged().Remove( Handle );
            }
        }

        FSmartTablePresentationChange TakeOnly( const TCHAR * What )
        {
            Runner.TestEqual( *FString::Printf( TEXT( "%s broadcasts once" ), What ), Heard.Num(), 1 );

            FSmartTablePresentationChange Change = Heard.IsEmpty() ? FSmartTablePresentationChange() : Heard.Last();
            Heard.Reset();

            return Change;
        }

    private:
        TWeakObjectPtr< USmartTableModel > Model;
        FAutomationTestBase & Runner;
        FDelegateHandle Handle;
    };

    struct FAGrowingSetArrivesAtItsEnd : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const FSmartTableRowSetDiff Diff = FSmartTableRowSetDiff::Between( RowSetIds( { TEXT( "A" ), TEXT( "B" ), TEXT( "C" ) } ), RowSetIds( { TEXT( "A" ), TEXT( "B" ), TEXT( "C" ), TEXT( "D" ), TEXT( "E" ) } ) );

            AITEST_EQUAL( "The two new rows arrived, at the rows they now hold", RowSetArrivedAsText( Diff ), FString( TEXT( "D@3,E@4" ) ) );
            AITEST_TRUE( "...and nothing left", Diff.Left.IsEmpty() );
            AITEST_FALSE( "A set that kept rows is no new set", Diff.bReplaced );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAGrowingSetArrivesAtItsEnd, "SmartTables.RowSet.AGrowingSetArrivesAtItsEnd" );

    struct FATrimFromTheFrontIsRowsLeaving : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const FSmartTableRowSetDiff Diff = FSmartTableRowSetDiff::Between( RowSetIds( { TEXT( "A" ), TEXT( "B" ), TEXT( "C" ), TEXT( "D" ) } ), RowSetIds( { TEXT( "C" ), TEXT( "D" ), TEXT( "E" ) } ) );

            AITEST_EQUAL( "The rows trimmed from the front left", RowSetLeftAsText( Diff ), FString( TEXT( "A,B" ) ) );
            AITEST_EQUAL( "...and the one added at the end arrived", RowSetArrivedAsText( Diff ), FString( TEXT( "E@2" ) ) );
            AITEST_TRUE( "It names rows one by one", Diff.NamesRows() );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FATrimFromTheFrontIsRowsLeaving, "SmartTables.RowSet.ATrimFromTheFrontIsRowsLeaving" );

    struct FASetSharingNothingIsANewSet : FAITestBase
    {
        virtual bool InstantTest() override
        {
            AITEST_TRUE( "Two sets with no row in common are a new set", FSmartTableRowSetDiff::Between( RowSetIds( { TEXT( "A" ), TEXT( "B" ) } ), RowSetIds( { TEXT( "C" ), TEXT( "D" ) } ) ).bReplaced );
            AITEST_TRUE( "The first fill is a new set", FSmartTableRowSetDiff::Between( {}, RowSetIds( { TEXT( "A" ) } ) ).bReplaced );
            AITEST_TRUE( "A set emptied out is a new set", FSmartTableRowSetDiff::Between( RowSetIds( { TEXT( "A" ) } ), {} ).bReplaced );
            AITEST_TRUE( "...and a new set names no row", !FSmartTableRowSetDiff::Between( {}, RowSetIds( { TEXT( "A" ) } ) ).NamesRows() );

            const FSmartTableRowSetDiff Same = FSmartTableRowSetDiff::Between( RowSetIds( { TEXT( "A" ), TEXT( "B" ) } ), RowSetIds( { TEXT( "B" ), TEXT( "A" ) } ) );
            AITEST_FALSE( "The same rows in another order are no new set", Same.bReplaced );
            AITEST_FALSE( "...and name no row", Same.NamesRows() );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FASetSharingNothingIsANewSet, "SmartTables.RowSet.ASetSharingNothingIsANewSet" );

    struct FARepeatedIdArrivesAsTwoRows : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const FSmartTableRowSetDiff Diff = FSmartTableRowSetDiff::Between( RowSetIds( { TEXT( "A" ) } ), RowSetIds( { TEXT( "A" ), TEXT( "B" ), TEXT( "B" ) } ) );

            AITEST_EQUAL( "One id twice is two arrived rows", RowSetArrivedAsText( Diff ), FString( TEXT( "B@1,B@2" ) ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FARepeatedIdArrivesAsTwoRows, "SmartTables.RowSet.ARepeatedIdArrivesAsTwoRows" );

    struct FAFoldKeepsTheOrderRowsWereSaidIn : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FSmartTableRowSetDiff Arrived;
            Arrived.Arrived.Add( { TEXT( "X" ), 7 } );

            FSmartTableRowSetDiff Left;
            Left.Left.Add( TEXT( "X" ) );

            FSmartTableRowSetDiff CameAndWent = Arrived;
            CameAndWent.Add( Left );
            AITEST_TRUE( "A row that arrived and then left did not arrive", CameAndWent.Arrived.IsEmpty() );
            AITEST_EQUAL( "...and it left", RowSetLeftAsText( CameAndWent ), FString( TEXT( "X" ) ) );

            FSmartTableRowSetDiff WentAndCame = Left;
            WentAndCame.Add( Arrived );
            AITEST_EQUAL( "A row that left and then arrived arrived", RowSetArrivedAsText( WentAndCame ), FString( TEXT( "X@7" ) ) );
            AITEST_TRUE( "...and did not leave", WentAndCame.Left.IsEmpty() );

            FSmartTableRowSetDiff ReplacedThenArrived = FSmartTableRowSetDiff::Replaced();
            ReplacedThenArrived.Add( Arrived );
            AITEST_TRUE( "A row after a new set keeps the new set", ReplacedThenArrived.bReplaced );
            AITEST_EQUAL( "...and still arrives", RowSetArrivedAsText( ReplacedThenArrived ), FString( TEXT( "X@7" ) ) );

            FSmartTableRowSetDiff ArrivedThenReplaced = Arrived;
            ArrivedThenReplaced.Add( FSmartTableRowSetDiff::Replaced() );
            AITEST_TRUE( "A new set drops the rows that arrived before it", ArrivedThenReplaced.bReplaced && ArrivedThenReplaced.Arrived.IsEmpty() );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAFoldKeepsTheOrderRowsWereSaidIn, "SmartTables.RowSet.AFoldKeepsTheOrderRowsWereSaidIn" );

    struct FTheItemsModelSaysWhichItemsCameAndWent : FAITestBase
    {
        virtual bool InstantTest() override
        {
            UObject * Alpha   = NamedRow( TEXT( "Alpha" ) );
            UObject * Bravo   = NamedRow( TEXT( "Bravo" ) );
            UObject * Charlie = NamedRow( TEXT( "Charlie" ) );
            UObject * Delta   = NamedRow( TEXT( "Delta" ) );
            UObject * Echo    = NamedRow( TEXT( "Echo" ) );

            USmartTableObjectModel * Model = NamedRowModel( { Alpha, Bravo, Charlie } );
            FRowSetListener Listener( *Model, GetTestRunner() );

            Model->SetItems( { Bravo, Charlie, Delta } );
            const FSmartTableRowSetDiff Given = Listener.TakeOnly( TEXT( "SetItems" ) ).RowSetDiff;
            AITEST_EQUAL( "SetItems says the new item arrived", RowSetArrivedAsText( Given ), ItemRowId( Delta ) + TEXT( "@2" ) );
            AITEST_EQUAL( "...and the missing one left", RowSetLeftAsText( Given ), ItemRowId( Alpha ) );

            Model->AddItem( Echo );
            AITEST_EQUAL( "AddItem says its row arrived at the end", RowSetArrivedAsText( Listener.TakeOnly( TEXT( "AddItem" ) ).RowSetDiff ), ItemRowId( Echo ) + TEXT( "@3" ) );

            Model->RemoveItem( Bravo );
            AITEST_EQUAL( "RemoveItem says its row left", RowSetLeftAsText( Listener.TakeOnly( TEXT( "RemoveItem" ) ).RowSetDiff ), ItemRowId( Bravo ) );

            Model->SetItems( { NamedRow( TEXT( "Foxtrot" ) ) } );
            AITEST_TRUE( "A set sharing no item is a new set", Listener.TakeOnly( TEXT( "SetItems of new items" ) ).RowSetDiff.bReplaced );

            Model->ClearItems();
            AITEST_TRUE( "ClearItems is a new set", Listener.TakeOnly( TEXT( "ClearItems" ) ).RowSetDiff.bReplaced );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FTheItemsModelSaysWhichItemsCameAndWent, "SmartTables.RowSet.TheItemsModelSaysWhichItemsCameAndWent" );

    struct FADataTableEditSaysWhichRowsCameAndWent : FAITestBase
    {
        virtual bool InstantTest() override
        {
            UDataTable * Table = NewObject< UDataTable >();
            Table->RowStruct   = FSmartTableTestRow::StaticStruct();
            Table->AddRow( TEXT( "Row_1" ), FSmartTableTestRow() );
            Table->AddRow( TEXT( "Row_2" ), FSmartTableTestRow() );

            USmartTableDataTableModel * Model = NewObject< USmartTableDataTableModel >();
            FRowSetListener Listener( *Model, GetTestRunner() );

            Model->SetDataTable( Table );
            AITEST_TRUE( "A new DataTable is a new set", Listener.TakeOnly( TEXT( "SetDataTable" ) ).RowSetDiff.bReplaced );

            Table->AddRow( TEXT( "Row_3" ), FSmartTableTestRow() );
            AITEST_EQUAL( "A row added to the asset arrived", RowSetArrivedAsText( Listener.TakeOnly( TEXT( "AddRow" ) ).RowSetDiff ), FString( TEXT( "Row_3@2" ) ) );

            Table->RemoveRow( TEXT( "Row_1" ) );
            AITEST_EQUAL( "A row taken out of the asset left", RowSetLeftAsText( Listener.TakeOnly( TEXT( "RemoveRow" ) ).RowSetDiff ), FString( TEXT( "Row_1" ) ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FADataTableEditSaysWhichRowsCameAndWent, "SmartTables.RowSet.ADataTableEditSaysWhichRowsCameAndWent" );

    struct FChangesHeldByADispatchedSortLandOnce : FAITestBase
    {
        virtual bool InstantTest() override
        {
            UObject * Alpha = NamedRow( TEXT( "Alpha" ) );
            UObject * Bravo = NamedRow( TEXT( "Bravo" ) );

            USmartTableObjectModel * Model = NamedRowModel( { NamedRow( TEXT( "Charlie" ) ), Alpha } );

            TArray< TUniqueFunction< void() > > Pending;
            Model->SetWorkDispatcher( [ &Pending ]( TUniqueFunction< void() > Work )
            {
                Pending.Add( MoveTemp( Work ) );
            } );

            FSmartTableSortSpec Spec;
            FSmartTableSortColumn & Level = Spec.Columns.AddDefaulted_GetRef();
            Level.ColumnId                = TEXT( "RowName" );
            Level.Mode                    = ESmartTableSortMode::Ascending;
            Model->SortRows( Spec );

            FRowSetListener Listener( *Model, GetTestRunner() );

            Model->AddItem( Bravo );
            Model->RemoveItem( Alpha );

            AITEST_TRUE( "Nothing lands while the sorts are out", Listener.Heard.IsEmpty() );

            for ( TUniqueFunction< void() > & Work : Pending )
            {
                Work();
            }

            const FSmartTablePresentationChange Landed = Listener.TakeOnly( TEXT( "The newest sort" ) );
            AITEST_EQUAL( "It carries one arrived row", Landed.RowSetDiff.Arrived.Num(), 1 );
            AITEST_EQUAL( "...which is the one AddItem put in", Landed.RowSetDiff.Arrived[ 0 ].RowId.ToString(), ItemRowId( Bravo ) );
            AITEST_EQUAL( "...and the row that left", RowSetLeftAsText( Landed.RowSetDiff ), ItemRowId( Alpha ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FChangesHeldByADispatchedSortLandOnce, "SmartTables.RowSet.ChangesHeldByADispatchedSortLandOnce" );

    struct FTwoItemsOfOneNameAreTwoRows : FAITestBase
    {
        virtual bool InstantTest() override
        {
            UObject * Here  = NewObject< USmartTableDataTableRow >( GetTransientPackage(), MakeUniqueObjectName( GetTransientPackage(), USmartTableDataTableRow::StaticClass(), TEXT( "Twin" ) ) );
            UObject * There = NewObject< USmartTableDataTableRow >( NamedRow( TEXT( "Elsewhere" ) ), Here->GetFName() );

            AITEST_TRUE( "The two items share an object name", Here->GetFName() == There->GetFName() );
            AITEST_TRUE( "...and not a row id", USmartTableObjectModel::RowIdOf( Here ) != USmartTableObjectModel::RowIdOf( There ) );

            USmartTableObjectModel * Model = NamedRowModel( { Here } );
            FRowSetListener Listener( *Model, GetTestRunner() );

            Model->SetItems( { Here, There } );
            AITEST_EQUAL( "The second one arrives as a row of its own", RowSetArrivedAsText( Listener.TakeOnly( TEXT( "SetItems" ) ).RowSetDiff ), ItemRowId( There ) + TEXT( "@1" ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FTwoItemsOfOneNameAreTwoRows, "SmartTables.RowSet.TwoItemsOfOneNameAreTwoRows" );
}

UE_ENABLE_OPTIMIZATION_SHIP

#undef LOCTEXT_NAMESPACE

#endif
