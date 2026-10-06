// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTablesTestsMacros.h"

#if SMARTTABLES_WITH_TESTS

#include "AITestsCommon.h"

#include "SmartTable.h"

#define LOCTEXT_NAMESPACE "SmartTablesTest"

UE_DISABLE_OPTIMIZATION_SHIP

namespace SmartTable::Test
{
    static const FText EmptyStateNoRows    = FText::FromString( TEXT( "Nothing to show" ) );
    static const FText EmptyStateNoMatches = FText::FromString( TEXT( "No rows match \"{0}\"" ) );

    struct FEmptyStateNamesTheRightProblem : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const FText NoData = USmartTable::MakeEmptyStateText( 0, FText::GetEmpty(), EmptyStateNoRows, EmptyStateNoMatches );
            AITEST_TRUE( "An empty table says it is empty", NoData.EqualTo( EmptyStateNoRows ) );

            const FText Filtered = USmartTable::MakeEmptyStateText( 12, FText::FromString( TEXT( "vanta" ) ), EmptyStateNoRows, EmptyStateNoMatches );
            AITEST_TRUE( "Rows that exist but do not show blame the filter", Filtered.ToString().Contains( TEXT( "vanta" ) ) );

            const FText FilteredEmpty = USmartTable::MakeEmptyStateText( 0, FText::FromString( TEXT( "vanta" ) ), EmptyStateNoRows, EmptyStateNoMatches );
            AITEST_TRUE( "A filter over no data still says there is no data", FilteredEmpty.EqualTo( EmptyStateNoRows ) );

            const FText Unfiltered = USmartTable::MakeEmptyStateText( 12, FText::GetEmpty(), EmptyStateNoRows, EmptyStateNoMatches );
            AITEST_TRUE( "Rows with no filter cannot blame one", Unfiltered.EqualTo( EmptyStateNoRows ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FEmptyStateNamesTheRightProblem, "SmartTables.EmptyState.NamesTheRightProblem" );
}
UE_ENABLE_OPTIMIZATION_SHIP

#undef LOCTEXT_NAMESPACE
#endif
