// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTablesTestsMacros.h"

#if SMARTTABLES_WITH_TESTS

#include "AITestsCommon.h"

#include "SmartTableCellDrag.h"

#define LOCTEXT_NAMESPACE "SmartTablesTest"

UE_DISABLE_OPTIMIZATION_SHIP

namespace SmartTable::Test
{
    struct FTheSourceWinsATie : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FSmartTableCellDrag Drag;
            Drag.Begin( TEXT( "Keller" ), TEXT( "Name" ) );

            AITEST_TRUE( "The pointer can come back over the cell it started on", Drag.SetTarget( TEXT( "Keller" ), TEXT( "Name" ) ) );
            AITEST_EQUAL( "...and that cell is still the source", Drag.RoleOf( TEXT( "Keller" ), TEXT( "Name" ) ), ESmartTableCellDragRole::Source );
            AITEST_EQUAL( "The cell beside it is nothing to the drag", Drag.RoleOf( TEXT( "Keller" ), TEXT( "Mass" ) ), ESmartTableCellDragRole::None );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FTheSourceWinsATie, "SmartTables.CellDrag.TheSourceWinsATie" );

    struct FATargetIsSetOnlyWhileADragIsOut : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FSmartTableCellDrag Drag;

            AITEST_FALSE( "With no drag out, no cell becomes a target", Drag.SetTarget( TEXT( "Garris" ), TEXT( "Name" ) ) );
            AITEST_EQUAL( "...and the cell has no role", Drag.RoleOf( TEXT( "Garris" ), TEXT( "Name" ) ), ESmartTableCellDragRole::None );
            AITEST_FALSE( "...and no drag is out", Drag.IsActive() );

            Drag.Begin( TEXT( "Vanta" ), TEXT( "Name" ) );

            AITEST_TRUE( "Once a drag is out, the cell under the pointer becomes the target", Drag.SetTarget( TEXT( "Garris" ), TEXT( "Name" ) ) );
            AITEST_EQUAL( "...and draws as one", Drag.RoleOf( TEXT( "Garris" ), TEXT( "Name" ) ), ESmartTableCellDragRole::Target );
            AITEST_TRUE( "...while the drag is out", Drag.IsActive() );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FATargetIsSetOnlyWhileADragIsOut, "SmartTables.CellDrag.ATargetIsSetOnlyWhileADragIsOut" );

    struct FMovingOntoTheSameCellChangesNothing : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FSmartTableCellDrag Drag;
            Drag.Begin( TEXT( "Vanta" ), TEXT( "Name" ) );

            AITEST_TRUE( "The first cell the pointer reaches becomes the target", Drag.SetTarget( TEXT( "Garris" ), TEXT( "Name" ) ) );
            AITEST_FALSE( "The same cell again changes nothing", Drag.SetTarget( TEXT( "Garris" ), TEXT( "Name" ) ) );
            AITEST_TRUE( "Another column of the same row is another cell", Drag.SetTarget( TEXT( "Garris" ), TEXT( "Mass" ) ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FMovingOntoTheSameCellChangesNothing, "SmartTables.CellDrag.MovingOntoTheSameCellChangesNothing" );

    struct FALeaveClearsOnlyTheCellItNames : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FSmartTableCellDrag Drag;
            Drag.Begin( TEXT( "Vanta" ), TEXT( "Name" ) );
            Drag.SetTarget( TEXT( "Garris" ), TEXT( "Name" ) );

            AITEST_TRUE( "The enter of the new cell comes first", Drag.SetTarget( TEXT( "Keller" ), TEXT( "Name" ) ) );
            AITEST_FALSE( "The leave of the old cell then changes nothing", Drag.ClearTarget( TEXT( "Garris" ), TEXT( "Name" ) ) );
            AITEST_EQUAL( "...and the new cell is still the target", Drag.RoleOf( TEXT( "Keller" ), TEXT( "Name" ) ), ESmartTableCellDragRole::Target );

            AITEST_TRUE( "The leave of the target clears it", Drag.ClearTarget( TEXT( "Keller" ), TEXT( "Name" ) ) );
            AITEST_EQUAL( "...so that cell has no role", Drag.RoleOf( TEXT( "Keller" ), TEXT( "Name" ) ), ESmartTableCellDragRole::None );
            AITEST_TRUE( "...and the drag is still out", Drag.IsActive() );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FALeaveClearsOnlyTheCellItNames, "SmartTables.CellDrag.ALeaveClearsOnlyTheCellItNames" );

    struct FEndingWhatNeverStartedChangesNothing : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FSmartTableCellDrag Drag;

            AITEST_FALSE( "Ending with no drag out changes nothing", Drag.End() );

            Drag.Begin( TEXT( "Vanta" ), TEXT( "Name" ) );

            AITEST_TRUE( "Ending a drag that is out changes the marking", Drag.End() );
            AITEST_FALSE( "...and leaves no drag out", Drag.IsActive() );
            AITEST_FALSE( "Ending it again changes nothing", Drag.End() );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FEndingWhatNeverStartedChangesNothing, "SmartTables.CellDrag.EndingWhatNeverStartedChangesNothing" );

    struct FARowWithNoIdHasNoRole : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FSmartTableCellDrag Drag;

            AITEST_EQUAL( "With no drag out, a cell with no row is not the source", Drag.RoleOf( NAME_None, NAME_None ), ESmartTableCellDragRole::None );

            Drag.Begin( TEXT( "Vanta" ), TEXT( "Name" ) );

            AITEST_EQUAL( "With a drag out, a cell with no row has no role", Drag.RoleOf( NAME_None, TEXT( "Name" ) ), ESmartTableCellDragRole::None );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FARowWithNoIdHasNoRole, "SmartTables.CellDrag.ARowWithNoIdHasNoRole" );
}

UE_ENABLE_OPTIMIZATION_SHIP

#undef LOCTEXT_NAMESPACE

#endif
