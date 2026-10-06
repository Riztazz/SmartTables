// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTablesTestsMacros.h"

#if SMARTTABLES_WITH_TESTS

#include "AITestsCommon.h"

#include "Containers/Array.h"
#include "SmartTable.h"
#include "SmartTableCell.h"
#include "SmartTableCellDragDropOp.h"
#include "SmartTableDataTableModel.h"
#include "SmartTableKeptRow.h"
#include "SmartTableModel.h"
#include "SmartTableTestTypes.h"

#define LOCTEXT_NAMESPACE "SmartTablesTest"

UE_DISABLE_OPTIMIZATION_SHIP

namespace SmartTable::Test
{

    static int32 KeptRowFoundIn( const TArray< FName > & Ids, const FSmartTableKeptRow & Row )
    {
        return Row.FindNow( Ids.Num(), [ &Ids ]( int32 NaturalRow )
        {
            return Ids[ NaturalRow ];
        } );
    }

    struct FARowNothingMovedIsWhereItWas : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const TArray< FName > Ids = { TEXT( "Vanta" ), TEXT( "Garris" ), TEXT( "Keller" ) };

            AITEST_EQUAL( "A row nothing moved answers its own number", KeptRowFoundIn( Ids, KeptRow( TEXT( "Keller" ), 2 ) ), 2 );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FARowNothingMovedIsWhereItWas, "SmartTables.KeptRow.ARowNothingMovedIsWhereItWas" );

    struct FARowIsFoundAfterRowsArriveOrLeaveAboveIt : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const TArray< FName > Trimmed = { TEXT( "Keller" ), TEXT( "Osei" ) };

            AITEST_EQUAL( "Two rows left above Keller, so Keller is row 0", KeptRowFoundIn( Trimmed, KeptRow( TEXT( "Keller" ), 2 ) ), 0 );
            AITEST_EQUAL( "A number past the end still starts the search", KeptRowFoundIn( Trimmed, KeptRow( TEXT( "Osei" ), 3 ) ), 1 );

            const TArray< FName > Grown = { TEXT( "Brand" ), TEXT( "Vanta" ), TEXT( "Garris" ), TEXT( "Keller" ) };

            AITEST_EQUAL( "One row arrived above Keller, so Keller is row 3", KeptRowFoundIn( Grown, KeptRow( TEXT( "Keller" ), 2 ) ), 3 );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FARowIsFoundAfterRowsArriveOrLeaveAboveIt, "SmartTables.KeptRow.ARowIsFoundAfterRowsArriveOrLeaveAboveIt" );

    struct FARowThatLeftIsFoundNowhere : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const TArray< FName > Ids = { TEXT( "Vanta" ), TEXT( "Garris" ), TEXT( "Osei" ) };

            AITEST_EQUAL( "Keller left, and no row answers for it", KeptRowFoundIn( Ids, KeptRow( TEXT( "Keller" ), 2 ) ), INDEX_NONE );
            AITEST_EQUAL( "A row kept with no id is found nowhere", KeptRowFoundIn( Ids, FSmartTableKeptRow() ), INDEX_NONE );

            const TArray< FName > Empty;

            AITEST_EQUAL( "An empty model holds no row", KeptRowFoundIn( Empty, KeptRow( TEXT( "Vanta" ), 0 ) ), INDEX_NONE );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FARowThatLeftIsFoundNowhere, "SmartTables.KeptRow.ARowThatLeftIsFoundNowhere" );

    struct FWithOneIdOnTwoRowsTheNearerWins : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const TArray< FName > Ids = { TEXT( "Vanta" ), TEXT( "Garris" ), TEXT( "Keller" ), TEXT( "Osei" ), TEXT( "Vanta" ) };

            AITEST_EQUAL( "Kept at row 3, the Vanta one row below wins", KeptRowFoundIn( Ids, KeptRow( TEXT( "Vanta" ), 3 ) ), 4 );
            AITEST_EQUAL( "Kept at row 1, the Vanta one row above wins", KeptRowFoundIn( Ids, KeptRow( TEXT( "Vanta" ), 1 ) ), 0 );
            AITEST_EQUAL( "Kept halfway between, the one above wins", KeptRowFoundIn( Ids, KeptRow( TEXT( "Vanta" ), 2 ) ), 0 );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FWithOneIdOnTwoRowsTheNearerWins, "SmartTables.KeptRow.WithOneIdOnTwoRowsTheNearerWins" );

    struct FADragMarksItsRowsAfterRowsLeaveAbove : FAITestBase
    {
        virtual bool InstantTest() override
        {
            UObject * Vanta                = NamedRow( TEXT( "Vanta" ) );
            UObject * Garris               = NamedRow( TEXT( "Garris" ) );
            USmartTableTestHarness * Table = NamedRowTable( { Vanta, Garris, NamedRow( TEXT( "Keller" ) ), NamedRow( TEXT( "Osei" ) ) } );
            const FName Column             = TEXT( "RowName" );

            Table->BeginCellDrag( 1, Column );
            Table->SetCellDragTarget( 2, Column );

            Table->RemoveItem( Vanta );

            AITEST_EQUAL( "Garris began the drag and is row 0 now, still the source", Table->GetCellDragRole( 0, Column ), ESmartTableCellDragRole::Source );
            AITEST_EQUAL( "Keller is row 1 now, still the target", Table->GetCellDragRole( 1, Column ), ESmartTableCellDragRole::Target );
            AITEST_EQUAL( "Row 2 is Osei now, and nothing to the drag", Table->GetCellDragRole( 2, Column ), ESmartTableCellDragRole::None );

            Table->RemoveItem( Garris );

            AITEST_EQUAL( "With the source gone, Keller at row 0 is still the target", Table->GetCellDragRole( 0, Column ), ESmartTableCellDragRole::Target );
            AITEST_EQUAL( "...and Osei at row 1 is nothing", Table->GetCellDragRole( 1, Column ), ESmartTableCellDragRole::None );
            AITEST_TRUE( "...and the drag is out until it ends", Table->IsCellDragActive() );

            Table->EndCellDrag();

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FADragMarksItsRowsAfterRowsLeaveAbove, "SmartTables.KeptRow.ADragMarksItsRowsAfterRowsLeaveAbove" );

    struct FADropReportsItsSourceRowWhereItIsNow : FAITestBase
    {
        virtual bool InstantTest() override
        {
            UObject * Vanta                = NamedRow( TEXT( "Vanta" ) );
            UObject * Keller               = NamedRow( TEXT( "Keller" ) );
            USmartTableTestHarness * Table = NamedRowTable( { Vanta, NamedRow( TEXT( "Garris" ) ), Keller } );
            const FName Column             = TEXT( "RowName" );

            Table->OnCellDropped.AddDynamic( Table, &USmartTableTestHarness::HearCellDrop );

            USmartTableCell * Cell = NewObject< USmartTableTextCell >();
            Cell->AssignCell( Table, Table->GetModel(), 1, Column, ESmartTableAssignReason::Scrolled );

            USmartTableCellDragDropOp * FromGarris = Cell->MakeCellDragPayload();
            AITEST_NOT_NULL( "A drag from Garris gives a payload", FromGarris );
            AITEST_EQUAL( "...naming Garris by id", FromGarris->SourceRowId.ToString(), Table->GetModel()->GetRowId( 1 ).ToString() );

            Table->RemoveItem( Vanta );
            Table->NotifyCellDropped( FromGarris, 1, Column );

            AITEST_EQUAL( "The drop is reported", Table->DropSourcesHeard.Num(), 1 );
            AITEST_EQUAL( "...from Garris, row 0 now that Vanta left", Table->DropSourcesHeard[ 0 ], 0 );

            Cell->AssignCell( Table, Table->GetModel(), 1, Column, ESmartTableAssignReason::Scrolled );
            USmartTableCellDragDropOp * FromKeller = Cell->MakeCellDragPayload();

            Table->RemoveItem( Keller );
            Table->NotifyCellDropped( FromKeller, 0, Column );

            AITEST_EQUAL( "A drop whose source row left is reported to nobody", Table->DropSourcesHeard.Num(), 1 );

            Cell->ReleaseCell();

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FADropReportsItsSourceRowWhereItIsNow, "SmartTables.KeptRow.ADropReportsItsSourceRowWhereItIsNow" );

    struct FARowDragMovesTheRowsItPickedUp : FAITestBase
    {
        virtual bool InstantTest() override
        {
            UObject * Vanta                = NamedRow( TEXT( "Vanta" ) );
            UObject * Garris               = NamedRow( TEXT( "Garris" ) );
            UObject * Keller               = NamedRow( TEXT( "Keller" ) );
            UObject * Osei                 = NamedRow( TEXT( "Osei" ) );
            UObject * Brand                = NamedRow( TEXT( "Brand" ) );
            USmartTableTestHarness * Table = NamedRowTable( { Vanta, Garris, Keller, Osei, Brand } );
            USmartTableModel * Model       = Table->GetModel();

            const TArray< FSmartTableKeptRow > Picked = { Model->KeepRow( 1 ), Model->KeepRow( 2 ) };

            Table->RemoveItem( Vanta );
            Table->MoveRowsFromDrop( Picked, 3 );

            const TConstArrayView< TObjectPtr< UObject > > Moved = Table->GetItems();
            AITEST_TRUE( "Garris and Keller land in front of Brand, which leaves Osei first", Moved.Num() == 4 && Moved[ 0 ] == Osei && Moved[ 1 ] == Garris && Moved[ 2 ] == Keller && Moved[ 3 ] == Brand );

            const TArray< FSmartTableKeptRow > PickedOsei = { Model->KeepRow( 0 ) };

            Table->RemoveItem( Osei );
            Table->MoveRowsFromDrop( PickedOsei, 3 );

            const TConstArrayView< TObjectPtr< UObject > > Kept = Table->GetItems();
            AITEST_TRUE( "A drag whose rows all left moves nothing", Kept.Num() == 3 && Kept[ 0 ] == Garris && Kept[ 1 ] == Keller && Kept[ 2 ] == Brand );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FARowDragMovesTheRowsItPickedUp, "SmartTables.KeptRow.ARowDragMovesTheRowsItPickedUp" );

    struct FARowMenuPickNamesTheRowItOpenedOn : FAITestBase
    {
        virtual bool InstantTest() override
        {
            UObject * Vanta                = NamedRow( TEXT( "Vanta" ) );
            UObject * Keller               = NamedRow( TEXT( "Keller" ) );
            USmartTableTestHarness * Table = NamedRowTable( { Vanta, NamedRow( TEXT( "Garris" ) ), Keller } );

            Table->OnRowMenuEntryChosen.AddDynamic( Table, &USmartTableTestHarness::HearMenuPick );

            const FSmartTableKeptRow OpenedOnKeller = Table->GetModel()->KeepRow( 2 );

            Table->RemoveItem( Vanta );
            Table->PickRowMenuEntry( TEXT( "Inspect" ), OpenedOnKeller );

            AITEST_EQUAL( "The pick is reported", Table->MenuPicksHeard.Num(), 1 );
            AITEST_TRUE( "...with Keller as its item", Table->MenuPicksHeard[ 0 ].Key == Keller );
            AITEST_EQUAL( "...at the number Keller has now", Table->MenuPicksHeard[ 0 ].Value, 1 );

            Table->RemoveItem( Keller );
            Table->PickRowMenuEntry( TEXT( "Inspect" ), OpenedOnKeller );

            AITEST_EQUAL( "A pick on a row that left is reported to nobody", Table->MenuPicksHeard.Num(), 1 );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FARowMenuPickNamesTheRowItOpenedOn, "SmartTables.KeptRow.ARowMenuPickNamesTheRowItOpenedOn" );
}

UE_ENABLE_OPTIMIZATION_SHIP

#undef LOCTEXT_NAMESPACE

#endif
