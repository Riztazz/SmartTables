// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTablesTestsMacros.h"

#if SMARTTABLES_WITH_TESTS

#include "AITestsCommon.h"

#include "SmartTableArrivals.h"

#define LOCTEXT_NAMESPACE "SmartTablesTest"

UE_DISABLE_OPTIMIZATION_SHIP

namespace SmartTable::Test
{
    static const float Window   = 2.0f;
    static const float Fraction = 0.75f;

    struct FAMarkedRowIsNewUntilTheWindowCloses : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FSmartTableArrivals Arrivals;

            AITEST_TRUE( "A table nothing added owes nothing", Arrivals.IsEmpty() );

            Arrivals.Mark( TEXT( "Row7" ), 100.0 );

            AITEST_FALSE( "A marked row makes the sweep worth running", Arrivals.IsEmpty() );
            AITEST_TRUE( "...and the row is new", Arrivals.IsNew( TEXT( "Row7" ) ) );
            AITEST_TRUE( "...and owes its entrance", Arrivals.OwesEntrance( TEXT( "Row7" ) ) );
            AITEST_FALSE( "A row nobody marked is not new", Arrivals.IsNew( TEXT( "Row8" ) ) );

            Arrivals.Expire( 101.9, Window );
            AITEST_TRUE( "Inside the window the mark stands", Arrivals.IsNew( TEXT( "Row7" ) ) );

            Arrivals.Expire( 102.1, Window );
            AITEST_FALSE( "Past the window it is gone", Arrivals.IsNew( TEXT( "Row7" ) ) );
            AITEST_TRUE( "...and there is nothing left to sweep", Arrivals.IsEmpty() );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAMarkedRowIsNewUntilTheWindowCloses, "SmartTables.Arrivals.AMarkedRowIsNewUntilTheWindowCloses" );

    struct FAPlayedEntranceLeavesTheRowNewAndOwedNothing : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FSmartTableArrivals Arrivals;
            Arrivals.Mark( TEXT( "Row7" ), 100.0 );
            Arrivals.MarkPlayed( TEXT( "Row7" ), true );

            AITEST_TRUE( "The row is still new, which is what a cell asks", Arrivals.IsNew( TEXT( "Row7" ) ) );
            AITEST_FALSE( "...and owes no second entrance", Arrivals.OwesEntrance( TEXT( "Row7" ) ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAPlayedEntranceLeavesTheRowNewAndOwedNothing, "SmartTables.Arrivals.APlayedEntranceLeavesTheRowNewAndOwedNothing" );

    struct FARefusedPlayLeavesTheEntranceOwed : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FSmartTableArrivals Arrivals;
            Arrivals.Mark( TEXT( "Row7" ), 100.0 );
            Arrivals.MarkPlayed( TEXT( "Row7" ), false );

            AITEST_TRUE( "A wrapper that refused leaves the entrance owed", Arrivals.OwesEntrance( TEXT( "Row7" ) ) );

            Arrivals.MarkPlayed( TEXT( "Row9" ), true );
            AITEST_FALSE( "An answer for a row nothing marked adds nothing", Arrivals.IsNew( TEXT( "Row9" ) ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FARefusedPlayLeavesTheEntranceOwed, "SmartTables.Arrivals.ARefusedPlayLeavesTheEntranceOwed" );

    struct FRemovingARowTakesItsMarkWithIt : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FSmartTableArrivals Arrivals;
            Arrivals.Mark( TEXT( "Row7" ), 100.0 );
            Arrivals.Mark( TEXT( "Row8" ), 100.0 );

            Arrivals.Unmark( TEXT( "Row7" ) );

            AITEST_FALSE( "The row on its way out is owed nothing", Arrivals.IsNew( TEXT( "Row7" ) ) );
            AITEST_TRUE( "...and the row beside it is untouched", Arrivals.OwesEntrance( TEXT( "Row8" ) ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FRemovingARowTakesItsMarkWithIt, "SmartTables.Arrivals.RemovingARowTakesItsMarkWithIt" );

    struct FMarkingAgainReArmsAPlayedRow : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FSmartTableArrivals Arrivals;
            Arrivals.Mark( TEXT( "Row7" ), 100.0 );
            Arrivals.MarkPlayed( TEXT( "Row7" ), true );

            Arrivals.Mark( TEXT( "Row7" ), 400.0 );

            AITEST_TRUE( "A row marked again owes its entrance again", Arrivals.OwesEntrance( TEXT( "Row7" ) ) );

            Arrivals.Expire( 401.0, Window );
            AITEST_TRUE( "...and the window runs from the second mark", Arrivals.IsNew( TEXT( "Row7" ) ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FMarkingAgainReArmsAPlayedRow, "SmartTables.Arrivals.MarkingAgainReArmsAPlayedRow" );

    struct FAnEdgeOfARowIsNotASighting : FAITestBase
    {
        virtual bool InstantTest() override
        {
            AITEST_FALSE( "Two pixels of a forty pixel row at the bottom is not seen", FSmartTableArrivals::IsRevealed( 398.0f, 40.0f, 0.0f, 400.0f, Fraction ) );
            AITEST_FALSE( "Half of it is not seen either", FSmartTableArrivals::IsRevealed( 380.0f, 40.0f, 0.0f, 400.0f, Fraction ) );
            AITEST_TRUE( "Three quarters is", FSmartTableArrivals::IsRevealed( 370.0f, 40.0f, 0.0f, 400.0f, Fraction ) );
            AITEST_TRUE( "A row wholly on screen is", FSmartTableArrivals::IsRevealed( 100.0f, 40.0f, 0.0f, 400.0f, Fraction ) );
            AITEST_FALSE( "A row above the top is not", FSmartTableArrivals::IsRevealed( -38.0f, 40.0f, 0.0f, 400.0f, Fraction ) );
            AITEST_FALSE( "A row nothing has laid out is not", FSmartTableArrivals::IsRevealed( 100.0f, 0.0f, 0.0f, 400.0f, Fraction ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAnEdgeOfARowIsNotASighting, "SmartTables.Arrivals.AnEdgeOfARowIsNotASighting" );

    struct FResetForgetsEveryRow : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FSmartTableArrivals Arrivals;
            Arrivals.Mark( TEXT( "Row7" ), 100.0 );
            Arrivals.Mark( TEXT( "Row8" ), 100.0 );
            Arrivals.MarkPlayed( TEXT( "Row8" ), true );

            Arrivals.Reset();

            AITEST_TRUE( "Nothing is left to sweep", Arrivals.IsEmpty() );
            AITEST_FALSE( "...and no id from the old model is still new", Arrivals.IsNew( TEXT( "Row7" ) ) );
            AITEST_FALSE( "...whether its entrance had played or not", Arrivals.IsNew( TEXT( "Row8" ) ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FResetForgetsEveryRow, "SmartTables.Arrivals.ResetForgetsEveryRow" );

    struct FAppliedDiffMarksArrivalsAndClearsDepartures : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FSmartTableArrivals Arrivals;
            Arrivals.Mark( TEXT( "Row7" ), 100.0 );

            FSmartTableRowSetDiff Diff;
            Diff.Arrived.Add( { TEXT( "Row8" ), 8 } );
            Diff.Left.Add( TEXT( "Row7" ) );
            Arrivals.Apply( Diff, 101.0 );

            AITEST_TRUE( "The row that arrived is new", Arrivals.OwesEntrance( TEXT( "Row8" ) ) );
            AITEST_FALSE( "...and the row that left is not", Arrivals.IsNew( TEXT( "Row7" ) ) );

            FSmartTableRowSetDiff Replaced = FSmartTableRowSetDiff::Replaced();
            Replaced.Arrived.Add( { TEXT( "Row9" ), 0 } );
            Arrivals.Apply( Replaced, 102.0 );

            AITEST_FALSE( "A new set drops the marks from before it", Arrivals.IsNew( TEXT( "Row8" ) ) );
            AITEST_TRUE( "...and keeps a row that arrived after it", Arrivals.IsNew( TEXT( "Row9" ) ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAppliedDiffMarksArrivalsAndClearsDepartures, "SmartTables.Arrivals.AppliedDiffMarksArrivalsAndClearsDepartures" );
}
UE_ENABLE_OPTIMIZATION_SHIP

#undef LOCTEXT_NAMESPACE

#endif
