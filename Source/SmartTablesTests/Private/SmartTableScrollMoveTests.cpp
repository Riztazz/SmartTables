// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTablesTestsMacros.h"

#if SMARTTABLES_WITH_TESTS

#include "AITestsCommon.h"

#include "Containers/Array.h"
#include "Containers/UnrealString.h"
#include "Math/UnrealMathUtility.h"
#include "SmartTableScrollMove.h"
#include "SmartTableTestTypes.h"

#define LOCTEXT_NAMESPACE "SmartTablesTest"

UE_DISABLE_OPTIMIZATION_SHIP

namespace SmartTable::Test
{
    static const float RowsOnScreen = 10.0f;
    static const float Seconds      = 0.25f;

    struct FAZeroLengthMoveIsAJump : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FSmartTableScrollMove Scroll;

            AITEST_FALSE( "No move time means nothing to aim", Scroll.Aim( 40, 100, KeptRow( TEXT( "Row40" ), 40 ), RowsOnScreen, 0.0f, 0.0f ) );
            AITEST_FALSE( "...and nothing is left aimed", Scroll.Move.IsSet() );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAZeroLengthMoveIsAJump, "SmartTables.ScrollMove.AZeroLengthMoveIsAJump" );

    struct FTheTargetIsTheLeastMoveThatShowsTheRow : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FSmartTableScrollMove Scroll;

            AITEST_TRUE( "A row below the view is a move", Scroll.Aim( 20, 100, KeptRow( TEXT( "Row73" ), 73 ), RowsOnScreen, 0.0f, Seconds ) );

            AITEST_TRUE( "The target puts the row at the bottom of the view", FMath::IsNearlyEqual( Scroll.Move->Offset, 11.0f ) );

            AITEST_EQUAL( "The natural row is carried, not used as the offset", Scroll.Move->Row.NaturalRow, 73 );

            AITEST_TRUE( "A row already at the top of the view needs no move", !Scroll.Aim( 4, 100, KeptRow( TEXT( "Row4" ), 4 ), RowsOnScreen, 0.0f, Seconds ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FTheTargetIsTheLeastMoveThatShowsTheRow, "SmartTables.ScrollMove.TheTargetIsTheLeastMoveThatShowsTheRow" );

    struct FALastRowAlreadyBelowTheViewIsPlacedNotMoved : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FSmartTableScrollMove Scroll;

            AITEST_FALSE( "The last row from below is placed", Scroll.Aim( 99, 100, KeptRow( TEXT( "Row99" ), 99 ), RowsOnScreen, 95.0f, Seconds ) );
            AITEST_FALSE( "...and nothing is left aimed", Scroll.Move.IsSet() );

            AITEST_TRUE( "The same last row from the top is a move", Scroll.Aim( 99, 100, KeptRow( TEXT( "Row99" ), 99 ), RowsOnScreen, 0.0f, Seconds ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FALastRowAlreadyBelowTheViewIsPlacedNotMoved, "SmartTables.ScrollMove.ALastRowAlreadyBelowTheViewIsPlacedNotMoved" );

    struct FAStepNeverOvershootsAndArrivalEndsTheMove : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FSmartTableScrollMove Scroll;
            Scroll.Aim( 20, 100, KeptRow( TEXT( "Row73" ), 73 ), RowsOnScreen, 0.0f, Seconds );

            float Offset = 0.0f;

            AITEST_EQUAL( "A step short of the target arrives at nothing", Scroll.Step( 0.0f, 0.1f, Offset ), INDEX_NONE );
            AITEST_TRUE( "...and moves at the worked out speed", FMath::IsNearlyEqual( Offset, 4.4f ) );
            AITEST_TRUE( "...and the move is still going", Scroll.Move.IsSet() );

            AITEST_EQUAL( "A step that reaches the target gives back the row", Scroll.Step( 10.9f, 0.1f, Offset ), 73 );
            AITEST_TRUE( "...lands exactly on it rather than past it", FMath::IsNearlyEqual( Offset, 11.0f ) );
            AITEST_FALSE( "...and the move is over", Scroll.Move.IsSet() );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAStepNeverOvershootsAndArrivalEndsTheMove, "SmartTables.ScrollMove.AStepNeverOvershootsAndArrivalEndsTheMove" );

    struct FTheSpeedIsSetOnceFromTheDistance : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FSmartTableScrollMove Scroll;
            Scroll.Aim( 20, 100, KeptRow( TEXT( "Row73" ), 73 ), RowsOnScreen, 0.0f, Seconds );

            const float Speed = Scroll.Move->ItemsPerSecond;

            AITEST_TRUE( "The whole distance over the whole time", FMath::IsNearlyEqual( Speed, 44.0f ) );

            float First = 0.0f;
            float Then  = 0.0f;

            Scroll.Step( 0.0f, 0.05f, First );
            Scroll.Step( First, 0.05f, Then );

            AITEST_TRUE( "Two equal steps cover equal ground", FMath::IsNearlyEqual( First, Then - First ) );
            AITEST_TRUE( "...and the speed never moved", FMath::IsNearlyEqual( Scroll.Move->ItemsPerSecond, Speed ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FTheSpeedIsSetOnceFromTheDistance, "SmartTables.ScrollMove.TheSpeedIsSetOnceFromTheDistance" );

    struct FWithinAQuarterRowOfTheBottomCountsAsTheEnd : FAITestBase
    {
        virtual bool InstantTest() override
        {
            AITEST_TRUE( "The very bottom is the end", FSmartTableScrollMove::IsViewAtEnd( 0.0f, 1000 ) );
            AITEST_TRUE( "A fifth of a row short is still the end", FSmartTableScrollMove::IsViewAtEnd( 0.0002f, 1000 ) );
            AITEST_FALSE( "A third of a row up is not, since a reader scrolling up starts there", FSmartTableScrollMove::IsViewAtEnd( 0.00033f, 1000 ) );
            AITEST_TRUE( "A table showing nothing is at its end", FSmartTableScrollMove::IsViewAtEnd( 1.0f, 0 ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FWithinAQuarterRowOfTheBottomCountsAsTheEnd, "SmartTables.ScrollMove.WithinAQuarterRowOfTheBottomCountsAsTheEnd" );

    struct FAMoveKnowsTheRowItIsHeadingTo : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FSmartTableScrollMove Scroll;
            AITEST_FALSE( "Nothing moving heads nowhere", Scroll.IsHeadingTo( 39 ) );

            AITEST_TRUE( "A move to the last row is aimed", Scroll.Aim( 39, 40, KeptRow( TEXT( "Line40" ), 39 ), RowsOnScreen, 20.0f, Seconds ) );
            AITEST_TRUE( "...and heads to that row", Scroll.IsHeadingTo( 39 ) );
            AITEST_FALSE( "...and to no other", Scroll.IsHeadingTo( 38 ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAMoveKnowsTheRowItIsHeadingTo, "SmartTables.ScrollMove.AMoveKnowsTheRowItIsHeadingTo" );

    struct FResetLeavesNothingAimed : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FSmartTableScrollMove Scroll;
            Scroll.Aim( 20, 100, KeptRow( TEXT( "Row73" ), 73 ), RowsOnScreen, 0.0f, Seconds );
            Scroll.bViewAtEnd   = false;
            Scroll.bOpenedAtEnd = true;

            Scroll.Stop();

            AITEST_FALSE( "Stopping ends the move", Scroll.Move.IsSet() );
            AITEST_FALSE( "...and leaves where the view sits alone", Scroll.bViewAtEnd );
            AITEST_TRUE( "...and leaves the opening scroll paid", Scroll.bOpenedAtEnd );

            Scroll.Aim( 20, 100, KeptRow( TEXT( "Row73" ), 73 ), RowsOnScreen, 0.0f, Seconds );
            Scroll.Reset();

            AITEST_FALSE( "A reset ends the move", Scroll.Move.IsSet() );
            AITEST_TRUE( "...and a table nothing has scrolled follows its first row", Scroll.bViewAtEnd );
            AITEST_FALSE( "...and owes its opening scroll again", Scroll.bOpenedAtEnd );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FResetLeavesNothingAimed, "SmartTables.ScrollMove.ResetLeavesNothingAimed" );

    static FSmartTableRowSetFacts LogOfFortyRows( bool bViewAtEnd, float OffsetBefore )
    {
        FSmartTableRowSetFacts Facts;
        Facts.bViewAtEnd      = bViewAtEnd;
        Facts.PresentedBefore = 40;
        Facts.PresentedAfter  = 41;
        Facts.OffsetBefore    = OffsetBefore;

        return Facts;
    }

    static FSmartTableRowSetDiff OneLineArrived()
    {
        FSmartTableRowSetDiff Diff;
        Diff.Arrived.Add( { TEXT( "Line41" ), 40 } );

        return Diff;
    }

    static FSmartTableRowSetDiff OneLineLeft()
    {
        FSmartTableRowSetDiff Diff;
        Diff.Left.Add( TEXT( "Line1" ) );

        return Diff;
    }

    struct FAViewAtTheEndStaysAtTheEnd : FAITestBase
    {
        virtual bool InstantTest() override
        {
            int32 Asked          = 0;
            const auto NoArrival = [ &Asked ]()
            {
                ++Asked;
                return INDEX_NONE;
            };

            const FSmartTableRowSetScroll Grew = FSmartTableScrollMove::ScrollAfterRowSet( LogOfFortyRows( true, 30.0f ), OneLineArrived(), NoArrival );
            AITEST_TRUE( "A view at the end moves to the new last row", Grew.Kind == ESmartTableRowSetScroll::ToRow );
            AITEST_EQUAL( "...which is the last drawn row", Grew.PresentedRow, 40 );

            FSmartTableRowSetFacts SameCount = LogOfFortyRows( true, 30.0f );
            SameCount.PresentedAfter         = 40;
            AITEST_TRUE( "A row in and a row out leave a view at the end where it is", FSmartTableScrollMove::ScrollAfterRowSet( SameCount, OneLineArrived(), NoArrival ).Kind == ESmartTableRowSetScroll::Stay );

            AITEST_EQUAL( "The arrived rows are never looked for, since the end is the answer", Asked, 0 );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAViewAtTheEndStaysAtTheEnd, "SmartTables.ScrollMove.AViewAtTheEndStaysAtTheEnd" );

    struct FAViewScrolledUpHoldsItsTopRow : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const auto NoArrival = []()
            {
                return INDEX_NONE;
            };

            AITEST_TRUE( "A view scrolled up holds its top row while rows leave", FSmartTableScrollMove::ScrollAfterRowSet( LogOfFortyRows( false, 12.5f ), OneLineLeft(), NoArrival ).Kind == ESmartTableRowSetScroll::Hold );
            AITEST_TRUE( "...and while rows arrive", FSmartTableScrollMove::ScrollAfterRowSet( LogOfFortyRows( false, 12.5f ), OneLineArrived(), NoArrival ).Kind == ESmartTableRowSetScroll::Hold );

            AITEST_TRUE( "A view at the top holds nothing, so new rows above it show", FSmartTableScrollMove::ScrollAfterRowSet( LogOfFortyRows( false, 0.0f ), OneLineLeft(), NoArrival ).Kind == ESmartTableRowSetScroll::Stay );

            FSmartTableRowSetFacts Moving = LogOfFortyRows( false, 12.5f );
            Moving.bMoving                = true;
            AITEST_TRUE( "A move going owns the offset, so nothing is held", FSmartTableScrollMove::ScrollAfterRowSet( Moving, OneLineLeft(), NoArrival ).Kind == ESmartTableRowSetScroll::Stay );

            AITEST_TRUE( "A change that names no row holds nothing", FSmartTableScrollMove::ScrollAfterRowSet( LogOfFortyRows( false, 12.5f ), FSmartTableRowSetDiff(), NoArrival ).Kind == ESmartTableRowSetScroll::Stay );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAViewScrolledUpHoldsItsTopRow, "SmartTables.ScrollMove.AViewScrolledUpHoldsItsTopRow" );

    struct FWithoutStickToEndTheViewGoesToTheArrival : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FSmartTableRowSetFacts Facts = LogOfFortyRows( false, 12.5f );
            Facts.bStickToEnd            = false;

            const FSmartTableRowSetScroll Drawn = FSmartTableScrollMove::ScrollAfterRowSet( Facts, OneLineArrived(), []()
            {
                return 7;
            } );
            AITEST_TRUE( "The view moves to the arrived row", Drawn.Kind == ESmartTableRowSetScroll::ToRow );
            AITEST_EQUAL( "...wherever the sort drew it", Drawn.PresentedRow, 7 );

            const FSmartTableRowSetScroll Hidden = FSmartTableScrollMove::ScrollAfterRowSet( Facts, OneLineArrived(), []()
            {
                return INDEX_NONE;
            } );
            AITEST_TRUE( "An arrival the filter hides leaves the view held", Hidden.Kind == ESmartTableRowSetScroll::Hold );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FWithoutStickToEndTheViewGoesToTheArrival, "SmartTables.ScrollMove.WithoutStickToEndTheViewGoesToTheArrival" );

    struct FANewSetOrAFirstFillScrollsNothing : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const auto NoArrival = []()
            {
                return INDEX_NONE;
            };

            FSmartTableRowSetDiff Replaced = FSmartTableRowSetDiff::Replaced();
            Replaced.Arrived.Add( { TEXT( "Line41" ), 40 } );
            AITEST_TRUE( "A new set scrolls nothing", FSmartTableScrollMove::ScrollAfterRowSet( LogOfFortyRows( true, 30.0f ), Replaced, NoArrival ).Kind == ESmartTableRowSetScroll::Stay );

            FSmartTableRowSetFacts FirstFill = LogOfFortyRows( true, 0.0f );
            FirstFill.PresentedBefore        = 0;
            AITEST_TRUE( "Nor do the first rows a table draws", FSmartTableScrollMove::ScrollAfterRowSet( FirstFill, OneLineArrived(), NoArrival ).Kind == ESmartTableRowSetScroll::Stay );

            FSmartTableRowSetFacts Off = LogOfFortyRows( true, 30.0f );
            Off.bScrollToAddedRow      = false;
            AITEST_TRUE( "With Scroll To Added Row off a view at the end stays", FSmartTableScrollMove::ScrollAfterRowSet( Off, OneLineArrived(), NoArrival ).Kind == ESmartTableRowSetScroll::Stay );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FANewSetOrAFirstFillScrollsNothing, "SmartTables.ScrollMove.ANewSetOrAFirstFillScrollsNothing" );

    struct FAHeldRowKeepsItsFraction : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FSmartTableHeldRow Held;
            Held.RowId        = TEXT( "Line13" );
            Held.PresentedRow = 12;
            Held.Offset       = 12.25f;

            AITEST_TRUE( "Three rows gone above it put it three places up, as far into the row as before", FMath::IsNearlyEqual( Held.OffsetAt( 9 ), 9.25f ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAHeldRowKeepsItsFraction, "SmartTables.ScrollMove.AHeldRowKeepsItsFraction" );
}
UE_ENABLE_OPTIMIZATION_SHIP

#undef LOCTEXT_NAMESPACE

#endif
