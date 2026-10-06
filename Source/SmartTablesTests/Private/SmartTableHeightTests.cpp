// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTablesTestsMacros.h"

#if SMARTTABLES_WITH_TESTS

#include "AITestsCommon.h"

#include "Table/SmartTableHeight.h"

#define LOCTEXT_NAMESPACE "SmartTablesTest"

UE_DISABLE_OPTIMIZATION_SHIP

namespace SmartTable::Test
{
    struct FFitRowsCountsTheHeaderAndCapsTheRows : FAITestBase
    {
        virtual bool InstantTest() override
        {
            AITEST_EQUAL( "The header, then the rows up to the cap", Height::FitRows( 10, 4, 20.0f, 30.0f ), 110.0f );
            AITEST_EQUAL( "Fewer rows than the cap asks for only those rows", Height::FitRows( 3, 4, 20.0f, 30.0f ), 90.0f );
            AITEST_EQUAL( "No header adds nothing", Height::FitRows( 3, 4, 20.0f, 0.0f ), 60.0f );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FFitRowsCountsTheHeaderAndCapsTheRows, "SmartTables.Height.FitRowsCountsTheHeaderAndCapsTheRows" );

    struct FZeroMaxVisibleRowsAsksForEveryRow : FAITestBase
    {
        virtual bool InstantTest() override
        {
            AITEST_EQUAL( "A cap of zero asks for every row", Height::FitRows( 10, 0, 20.0f, 0.0f ), 200.0f );
            AITEST_EQUAL( "So does a cap below zero", Height::FitRows( 10, -1, 20.0f, 0.0f ), 200.0f );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FZeroMaxVisibleRowsAsksForEveryRow, "SmartTables.Height.ZeroMaxVisibleRowsAsksForEveryRow" );

    struct FAListNoTallerThanItsWindowIsNotSizedByContent : FAITestBase
    {
        virtual bool InstantTest() override
        {
            AITEST_FALSE( "A list as tall as its window is laid out right", Height::LooksSizedByContent( 800.0f, 500, 800.0f, 20.0f ) );
            AITEST_FALSE( "A list shorter than its window is laid out right", Height::LooksSizedByContent( 600.0f, 500, 800.0f, 20.0f ) );
            AITEST_FALSE( "With no window height there is nothing to judge by", Height::LooksSizedByContent( 10000.0f, 500, 0.0f, 20.0f ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAListNoTallerThanItsWindowIsNotSizedByContent, "SmartTables.Height.AListNoTallerThanItsWindowIsNotSizedByContent" );

    struct FAListThatBuiltTwiceWhatFitsIsSizedByContent : FAITestBase
    {
        virtual bool InstantTest() override
        {
            AITEST_EQUAL( "A window of 800 shows 40 rows of 20", Height::RowsWindowFits( 800.0f, 20.0f ), 40 );
            AITEST_EQUAL( "A part row counts as one", Height::RowsWindowFits( 810.0f, 20.0f ), 41 );

            AITEST_TRUE( "A tall list that built more than twice what fits is sized by content", Height::LooksSizedByContent( 10000.0f, 81, 800.0f, 20.0f ) );
            AITEST_FALSE( "Exactly twice is not more than twice", Height::LooksSizedByContent( 10000.0f, 80, 800.0f, 20.0f ) );
            AITEST_FALSE( "With no row height there is nothing to count", Height::LooksSizedByContent( 10000.0f, 81, 800.0f, 0.0f ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAListThatBuiltTwiceWhatFitsIsSizedByContent, "SmartTables.Height.AListThatBuiltTwiceWhatFitsIsSizedByContent" );

    struct FAShortListThatOverhangsIsNotSizedByContent : FAITestBase
    {
        virtual bool InstantTest() override
        {
            AITEST_FALSE( "A list a little taller than its window that built few rows proves nothing", Height::LooksSizedByContent( 900.0f, 45, 800.0f, 20.0f ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAShortListThatOverhangsIsNotSizedByContent, "SmartTables.Height.AShortListThatOverhangsIsNotSizedByContent" );
}

UE_ENABLE_OPTIMIZATION_SHIP

#undef LOCTEXT_NAMESPACE

#endif
