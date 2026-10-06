// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTablesTestsMacros.h"

#if SMARTTABLES_WITH_TESTS

#include "AITestsCommon.h"

#include "SmartTableColumnSelection.h"

#define LOCTEXT_NAMESPACE "SmartTablesTest"

UE_DISABLE_OPTIMIZATION_SHIP

namespace SmartTable::Test
{
    static const TArray< FName > ThreeColumns = { TEXT( "Callsign" ), TEXT( "Class" ), TEXT( "Owner" ) };

    struct FSelectingAColumnReportsWhetherTheAimMoved : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FSmartTableColumnSelection Selection;

            AITEST_TRUE( "The first aim is a change", Selection.Select( TEXT( "Class" ), ThreeColumns ) );
            AITEST_EQUAL( "...and it stuck", Selection.Selected, FName( TEXT( "Class" ) ) );

            AITEST_FALSE( "Aiming where it already points is not a change", Selection.Select( TEXT( "Class" ), ThreeColumns ) );

            AITEST_FALSE( "A column that is not shown is refused", Selection.Select( TEXT( "Notes" ), ThreeColumns ) );
            AITEST_EQUAL( "...and the aim did not move", Selection.Selected, FName( TEXT( "Class" ) ) );

            AITEST_TRUE( "Clearing is a change", Selection.Select( NAME_None, ThreeColumns ) );
            AITEST_EQUAL( "...and nothing is aimed at", Selection.Selected, FName( NAME_None ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FSelectingAColumnReportsWhetherTheAimMoved, "SmartTables.ColumnSelection.SelectingAColumnReportsWhetherTheAimMoved" );

    struct FTheAimWrapsAtBothEnds : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FSmartTableColumnSelection Selection;

            Selection.Move( 1, ThreeColumns );
            AITEST_EQUAL( "From nowhere, forwards lands on the first", Selection.Selected, FName( TEXT( "Callsign" ) ) );

            Selection.Move( 1, ThreeColumns );
            Selection.Move( 1, ThreeColumns );
            AITEST_EQUAL( "Two more reaches the last", Selection.Selected, FName( TEXT( "Owner" ) ) );

            Selection.Move( 1, ThreeColumns );
            AITEST_EQUAL( "Off the right end wraps to the first", Selection.Selected, FName( TEXT( "Callsign" ) ) );

            Selection.Move( -1, ThreeColumns );
            AITEST_EQUAL( "Off the left end wraps to the last", Selection.Selected, FName( TEXT( "Owner" ) ) );

            FSmartTableColumnSelection Backwards;
            Backwards.Move( -1, ThreeColumns );
            AITEST_EQUAL( "From nowhere, backwards lands on the last", Backwards.Selected, FName( TEXT( "Owner" ) ) );

            FSmartTableColumnSelection Empty;
            AITEST_FALSE( "A table with no shown columns has nothing to aim at", Empty.Move( 1, {} ) );
            AITEST_EQUAL( "...and says so", Empty.Selected, FName( NAME_None ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FTheAimWrapsAtBothEnds, "SmartTables.ColumnSelection.TheAimWrapsAtBothEnds" );

    struct FAnIntentThatNamesNoColumnActsOnTheAim : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FSmartTableColumnSelection Selection;

            AITEST_EQUAL( "Nothing aimed resolves to the first shown column", Selection.Resolve( NAME_None, ThreeColumns ), FName( TEXT( "Callsign" ) ) );
            AITEST_EQUAL( "...and that IS the aim now", Selection.Selected, FName( TEXT( "Callsign" ) ) );

            AITEST_EQUAL( "A named column is used", Selection.Resolve( TEXT( "Owner" ), ThreeColumns ), FName( TEXT( "Owner" ) ) );
            AITEST_EQUAL( "...and aimed at", Selection.Selected, FName( TEXT( "Owner" ) ) );
            AITEST_EQUAL( "...so the next unnamed intent follows it", Selection.Resolve( NAME_None, ThreeColumns ), FName( TEXT( "Owner" ) ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAnIntentThatNamesNoColumnActsOnTheAim, "SmartTables.ColumnSelection.AnIntentThatNamesNoColumnActsOnTheAim" );

    struct FHidingTheAimedColumnHandsTheAimToItsPlace : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FSmartTableColumnSelection Selection;
            Selection.Select( TEXT( "Class" ), ThreeColumns );

            const TArray< FName > WithoutClass = { TEXT( "Callsign" ), TEXT( "Owner" ) };
            AITEST_TRUE( "Losing the aimed column moves the aim", Selection.Revalidate( WithoutClass ) );
            AITEST_EQUAL( "...onto whatever took its place", Selection.Selected, FName( TEXT( "Owner" ) ) );

            FSmartTableColumnSelection AtTheEnd;
            AtTheEnd.Select( TEXT( "Owner" ), ThreeColumns );
            const TArray< FName > WithoutOwner = { TEXT( "Callsign" ), TEXT( "Class" ) };
            AtTheEnd.Revalidate( WithoutOwner );
            AITEST_EQUAL( "The new last column takes the aim", AtTheEnd.Selected, FName( TEXT( "Class" ) ) );

            FSmartTableColumnSelection Untouched;
            Untouched.Select( TEXT( "Callsign" ), ThreeColumns );
            AITEST_FALSE( "A still-shown aim is not disturbed", Untouched.Revalidate( ThreeColumns ) );
            AITEST_EQUAL( "...at all", Untouched.Selected, FName( TEXT( "Callsign" ) ) );

            FSmartTableColumnSelection Gone;
            Gone.Select( TEXT( "Class" ), ThreeColumns );
            Gone.Revalidate( {} );
            AITEST_EQUAL( "No shown columns means no aim", Gone.Selected, FName( NAME_None ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FHidingTheAimedColumnHandsTheAimToItsPlace, "SmartTables.ColumnSelection.HidingTheAimedColumnHandsTheAimToItsPlace" );

    struct FAReorderMovesTheAimWithItsColumn : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FSmartTableColumnSelection Selection;
            Selection.Select( TEXT( "Class" ), ThreeColumns );

            const TArray< FName > Reordered = { TEXT( "Owner" ), TEXT( "Callsign" ), TEXT( "Class" ) };
            AITEST_FALSE( "A reorder does not move the aim", Selection.Revalidate( Reordered ) );
            AITEST_EQUAL( "...it is still on the same COLUMN", Selection.Selected, FName( TEXT( "Class" ) ) );

            Selection.Move( 1, Reordered );
            AITEST_EQUAL( "The walk uses the order in force now", Selection.Selected, FName( TEXT( "Owner" ) ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAReorderMovesTheAimWithItsColumn, "SmartTables.ColumnSelection.AReorderMovesTheAimWithItsColumn" );
}
UE_ENABLE_OPTIMIZATION_SHIP

#undef LOCTEXT_NAMESPACE

#endif
