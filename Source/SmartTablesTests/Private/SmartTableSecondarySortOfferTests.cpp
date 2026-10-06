// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTablesTestsMacros.h"

#if SMARTTABLES_WITH_TESTS

#include "AITestsCommon.h"

#include "SmartTableSorting.h"
#include "SmartTableTypes.h"

#define LOCTEXT_NAMESPACE "SmartTablesTest"

UE_DISABLE_OPTIMIZATION_SHIP

namespace SmartTable::Test
{
    struct FThenSortByThisNeedsSomethingToBeSecondTo : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FSmartTableSortSpec Spec;

            AITEST_FALSE( "An unsorted table has no level to be second to", SmartTable::Sorting::HasLevelOtherThan( Spec, TEXT( "Owner" ) ) );

            Spec = SmartTable::Sorting::SpecWithPrimary( TEXT( "Callsign" ), ESmartTableSortMode::Ascending );

            AITEST_TRUE( "A primary elsewhere is something to be second to", SmartTable::Sorting::HasLevelOtherThan( Spec, TEXT( "Owner" ) ) );

            AITEST_FALSE( "The only sorted column is not second to itself", SmartTable::Sorting::HasLevelOtherThan( Spec, TEXT( "Callsign" ) ) );

            Spec = SmartTable::Sorting::SpecWithSecondary( Spec, TEXT( "Owner" ), ESmartTableSortMode::Descending );

            AITEST_TRUE( "With two levels the primary still has a partner", SmartTable::Sorting::HasLevelOtherThan( Spec, TEXT( "Callsign" ) ) );
            AITEST_TRUE( "...and so does the secondary", SmartTable::Sorting::HasLevelOtherThan( Spec, TEXT( "Owner" ) ) );

            AITEST_FALSE( "An emptied spec offers nothing", SmartTable::Sorting::HasLevelOtherThan( FSmartTableSortSpec(), TEXT( "Callsign" ) ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FThenSortByThisNeedsSomethingToBeSecondTo, "SmartTables.Sorting.ThenSortByThisNeedsSomethingToBeSecondTo" );
}
UE_ENABLE_OPTIMIZATION_SHIP

#undef LOCTEXT_NAMESPACE

#endif
