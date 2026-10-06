// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTablesTestsMacros.h"

#if SMARTTABLES_WITH_TESTS

#include "AITestsCommon.h"

#include "SmartTable.h"
#include "SmartTableTypes.h"

#define LOCTEXT_NAMESPACE "SmartTablesTest"

UE_DISABLE_OPTIMIZATION_SHIP

namespace SmartTable::Test
{
    struct FALockedColumnHasNoResizeGrip : FAITestBase
    {
        virtual bool InstantTest() override
        {
            USmartTable * Table = NewObject< USmartTable >();

            FSmartTableColumn Free;
            Free.ColumnId   = TEXT( "Callsign" );
            Free.bResizable = true;

            FSmartTableColumn Locked;
            Locked.ColumnId   = TEXT( "Owner" );
            Locked.bResizable = false;

            Table->SetColumns( { Free, Locked } );

            AITEST_TRUE( "A resizable column has a grip", Table->CanResizeColumn( TEXT( "Callsign" ) ) );
            AITEST_FALSE( "A column the author locked does not", Table->CanResizeColumn( TEXT( "Owner" ) ) );
            AITEST_FALSE( "Neither does a column that does not exist", Table->CanResizeColumn( TEXT( "Nothing" ) ) );

            Table->SetColumnVisible( TEXT( "Callsign" ), false );

            AITEST_FALSE( "A hidden column has no grip either", Table->CanResizeColumn( TEXT( "Callsign" ) ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FALockedColumnHasNoResizeGrip, "SmartTables.ColumnLayout.ALockedColumnHasNoResizeGrip" );
}
UE_ENABLE_OPTIMIZATION_SHIP

#undef LOCTEXT_NAMESPACE

#endif
