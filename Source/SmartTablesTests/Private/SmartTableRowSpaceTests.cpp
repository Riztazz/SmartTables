// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTablesTestsMacros.h"

#if SMARTTABLES_WITH_TESTS

#include "AITestsCommon.h"

#include "Table/SmartTableRowSpace.h"

#define LOCTEXT_NAMESPACE "SmartTablesTest"

UE_DISABLE_OPTIMIZATION_SHIP

namespace SmartTable::Test
{
    static FString RowSpaceRowsAsText( TConstArrayView< int32 > Rows )
    {
        return FString::JoinBy( Rows, TEXT( "," ), []( int32 Row )
        {
            return FString::FromInt( Row );
        } );
    }

    struct FACaretStepClampsAtBothEnds : FAITestBase
    {
        virtual bool InstantTest() override
        {
            AITEST_EQUAL( "A step inside the table lands where it points", RowSpace::StepCaret( 1, 2, 5 ), 3 );
            AITEST_EQUAL( "A step past the last row stops on it", RowSpace::StepCaret( 3, 10, 5 ), 4 );
            AITEST_EQUAL( "A step past the first row stops on it", RowSpace::StepCaret( 1, -10, 5 ), 0 );
            AITEST_EQUAL( "A table with no rows has nowhere to land", RowSpace::StepCaret( INDEX_NONE, 1, 0 ), INDEX_NONE );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FACaretStepClampsAtBothEnds, "SmartTables.RowSpace.ACaretStepClampsAtBothEnds" );

    struct FWithNoCaretForwardStartsAtTheTopAndBackAtTheBottom : FAITestBase
    {
        virtual bool InstantTest() override
        {
            AITEST_EQUAL( "With no caret, a step forward lands on the first row", RowSpace::StepCaret( INDEX_NONE, 1, 5 ), 0 );
            AITEST_EQUAL( "...and so does a whole page forward", RowSpace::StepCaret( INDEX_NONE, 20, 5 ), 0 );
            AITEST_EQUAL( "With no caret, a step back lands on the last row", RowSpace::StepCaret( INDEX_NONE, -1, 5 ), 4 );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FWithNoCaretForwardStartsAtTheTopAndBackAtTheBottom, "SmartTables.RowSpace.WithNoCaretForwardStartsAtTheTopAndBackAtTheBottom" );

    struct FAStepAsLargeAsAnInt32HoldsStopsOnTheLastRow : FAITestBase
    {
        virtual bool InstantTest() override
        {
            AITEST_EQUAL( "A step as large as an int32 holds stops on the last row", RowSpace::StepCaret( 1, MAX_int32, 5 ), 4 );
            AITEST_EQUAL( "...and one as small stops on the first", RowSpace::StepCaret( 3, MIN_int32, 5 ), 0 );
            AITEST_EQUAL( "A step past what an int32 holds stops on the last row too", RowSpace::StepCaret( 1, static_cast< int64 >( MAX_int32 ) * 4, 5 ), 4 );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAStepAsLargeAsAnInt32HoldsStopsOnTheLastRow, "SmartTables.RowSpace.AStepAsLargeAsAnInt32HoldsStopsOnTheLastRow" );

    struct FAPageStepPastWhatAnInt32HoldsStopsAtTheEnd : FAITestBase
    {
        virtual bool InstantTest() override
        {
            AITEST_EQUAL( "A page step lands a whole page on", RowSpace::StepCaretByPage( 0, 2, 1, 5 ), 2 );
            AITEST_EQUAL( "As many pages of twenty rows as an int32 holds stops on the last row", RowSpace::StepCaretByPage( 1, 20, MAX_int32, 5 ), 4 );
            AITEST_EQUAL( "...and as many pages back stops on the first", RowSpace::StepCaretByPage( 3, 20, MIN_int32, 5 ), 0 );
            AITEST_EQUAL( "With no caret, a page forward lands on the first row", RowSpace::StepCaretByPage( INDEX_NONE, 20, 1, 5 ), 0 );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAPageStepPastWhatAnInt32HoldsStopsAtTheEnd, "SmartTables.RowSpace.APageStepPastWhatAnInt32HoldsStopsAtTheEnd" );

    struct FMovingDownNeedsTheGapBelowTheRowSteppedOver : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const TArray< int32 > Block    = { 1, 2 };
            const TArray< int32 > Unsorted = { 2, 1 };

            AITEST_EQUAL( "One down, the gap is the one below the row stepped over", RowSpace::GapForBlockMove( Block, 1, 5 ), 4 );
            AITEST_EQUAL( "One up, the gap is the one above the row stepped over", RowSpace::GapForBlockMove( Block, -1, 5 ), 0 );
            AITEST_EQUAL( "The rows of the block can come in any order", RowSpace::GapForBlockMove( Unsorted, 1, 5 ), 4 );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FMovingDownNeedsTheGapBelowTheRowSteppedOver, "SmartTables.RowSpace.MovingDownNeedsTheGapBelowTheRowSteppedOver" );

    struct FABlockMovePastEitherEndIsRefused : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const TArray< int32 > OnFirst      = { 0 };
            const TArray< int32 > OnLast       = { 4 };
            const TArray< int32 > AboveTheLast = { 3 };
            const TArray< int32 > Empty;

            AITEST_EQUAL( "A block on the first row has no gap above it", RowSpace::GapForBlockMove( OnFirst, -1, 5 ), INDEX_NONE );
            AITEST_EQUAL( "A block on the last row has no gap below it", RowSpace::GapForBlockMove( OnLast, 1, 5 ), INDEX_NONE );
            AITEST_EQUAL( "A block one above the last row moves into the last gap", RowSpace::GapForBlockMove( AboveTheLast, 1, 5 ), 5 );
            AITEST_EQUAL( "An empty block moves nowhere", RowSpace::GapForBlockMove( Empty, 1, 5 ), INDEX_NONE );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FABlockMovePastEitherEndIsRefused, "SmartTables.RowSpace.ABlockMovePastEitherEndIsRefused" );

    struct FABlockMoveFarPastEitherEndIsRefused : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const TArray< int32 > OnRowThree = { 3 };
            const TArray< int32 > OnRowOne   = { 1 };

            AITEST_EQUAL( "A block moved as far down as an int32 holds is refused", RowSpace::GapForBlockMove( OnRowThree, MAX_int32, 5 ), INDEX_NONE );
            AITEST_EQUAL( "...and one moved as far up is refused too", RowSpace::GapForBlockMove( OnRowOne, MIN_int32, 5 ), INDEX_NONE );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FABlockMoveFarPastEitherEndIsRefused, "SmartTables.RowSpace.ABlockMoveFarPastEitherEndIsRefused" );

    struct FAGapLandsAfterTheDrawnRowAbove : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const TArray< int32 > Drawn = { 0, 3, 4 };

            const auto PresentedToNatural = [ &Drawn ]( int32 PresentedRow )
            {
                return Drawn.IsValidIndex( PresentedRow ) ? Drawn[ PresentedRow ] : INDEX_NONE;
            };

            AITEST_EQUAL( "The first gap is in front of every row", RowSpace::NaturalGapFor( 0, 6, PresentedToNatural ), 0 );
            AITEST_EQUAL( "A gap below zero is the first gap", RowSpace::NaturalGapFor( -1, 6, PresentedToNatural ), 0 );
            AITEST_EQUAL( "A gap lands right after the drawn row above it, in front of the rows the filter hides", RowSpace::NaturalGapFor( 1, 6, PresentedToNatural ), 1 );
            AITEST_EQUAL( "One step on screen is three rows on in natural space", RowSpace::NaturalGapFor( 2, 6, PresentedToNatural ), 4 );
            AITEST_EQUAL( "The gap below the last drawn row lands in front of the hidden row after it", RowSpace::NaturalGapFor( 3, 6, PresentedToNatural ), 5 );
            AITEST_EQUAL( "A gap with no drawn row above it lands past the last row", RowSpace::NaturalGapFor( 4, 6, PresentedToNatural ), 6 );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAGapLandsAfterTheDrawnRowAbove, "SmartTables.RowSpace.AGapLandsAfterTheDrawnRowAbove" );

    struct FRowsAreFoundByIdWhereverTheirNumbersWent : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const TArray< FName > Before   = { TEXT( "A" ), TEXT( "B" ), TEXT( "C" ), TEXT( "D" ), TEXT( "E" ) };
            const TArray< int32 > Selected = { 1, 3 };

            const TSet< FName > Ids = RowSpace::IdsOf( Selected, [ &Before ]( int32 Row )
            {
                return Before[ Row ];
            } );

            AITEST_EQUAL( "Each selected row gives its id", Ids.Num(), 2 );
            AITEST_TRUE( "...the one on row 1", Ids.Contains( FName( TEXT( "B" ) ) ) );
            AITEST_TRUE( "...and the one on row 3", Ids.Contains( FName( TEXT( "D" ) ) ) );

            const TArray< FName > After = { TEXT( "D" ), TEXT( "A" ), TEXT( "B" ), TEXT( "C" ), TEXT( "E" ) };

            const auto IdAfter = [ &After ]( int32 Row )
            {
                return After[ Row ];
            };

            const RowSpace::FFoundRows InEveryRow = RowSpace::FindByIdsInEveryRow( After.Num(), Ids, TEXT( "D" ), IdAfter );
            AITEST_EQUAL( "Both rows are found at their new numbers", RowSpaceRowsAsText( InEveryRow.Rows ), FString( TEXT( "0,2" ) ) );
            AITEST_EQUAL( "...and the caret with them", InEveryRow.Caret, 0 );

            const TArray< int32 > Drawn = { 4, 2, 0 };

            const RowSpace::FFoundRows InDrawn = RowSpace::FindByIds( Drawn, Ids, TEXT( "B" ), IdAfter );
            AITEST_EQUAL( "Over the drawn rows, they come back in the order drawn", RowSpaceRowsAsText( InDrawn.Rows ), FString( TEXT( "2,0" ) ) );
            AITEST_EQUAL( "...and the caret is found among them", InDrawn.Caret, 2 );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FRowsAreFoundByIdWhereverTheirNumbersWent, "SmartTables.RowSpace.RowsAreFoundByIdWhereverTheirNumbersWent" );

    struct FACaretWhoseRowIsGoneIsDropped : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const TArray< FName > RowIds = { TEXT( "A" ), TEXT( "B" ), TEXT( "C" ), NAME_None };
            const TArray< int32 > Drawn  = { 0, 2, 3 };
            const TSet< FName > NoIds;

            const auto IdOf = [ &RowIds ]( int32 Row )
            {
                return RowIds[ Row ];
            };

            AITEST_EQUAL( "A caret whose id no row has is dropped", RowSpace::FindByIdsInEveryRow( RowIds.Num(), NoIds, TEXT( "Z" ), IdOf ).Caret, INDEX_NONE );
            AITEST_EQUAL( "A caret on a row the drawn rows leave out is dropped", RowSpace::FindByIds( Drawn, NoIds, TEXT( "B" ), IdOf ).Caret, INDEX_NONE );
            AITEST_EQUAL( "A caret on a drawn row is kept", RowSpace::FindByIds( Drawn, NoIds, TEXT( "C" ), IdOf ).Caret, 2 );
            AITEST_EQUAL( "No caret finds no row, even one with no id", RowSpace::FindByIds( Drawn, NoIds, NAME_None, IdOf ).Caret, INDEX_NONE );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FACaretWhoseRowIsGoneIsDropped, "SmartTables.RowSpace.ACaretWhoseRowIsGoneIsDropped" );

    struct FARowIsFoundNearWhereItWas : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const TArray< int32 > Rows = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

            int32 Reads        = 0;
            const auto RowIdOf = [ &Reads ]( int32 NaturalRow )
            {
                ++Reads;
                return FName( *FString::Printf( TEXT( "Line%d" ), NaturalRow ) );
            };

            AITEST_EQUAL( "A row still where it was is found there", SmartTable::RowSpace::FindIdNear( Rows, TEXT( "Line6" ), 6, RowIdOf ), 6 );
            AITEST_EQUAL( "...in one read", Reads, 1 );

            Reads = 0;
            AITEST_EQUAL( "A row three places up is found", SmartTable::RowSpace::FindIdNear( Rows, TEXT( "Line3" ), 6, RowIdOf ), 3 );
            AITEST_TRUE( "...in a few reads and not a walk of every row", Reads < 7 );

            AITEST_EQUAL( "A row further down is found too", SmartTable::RowSpace::FindIdNear( Rows, TEXT( "Line9" ), 6, RowIdOf ), 9 );
            AITEST_EQUAL( "A place past the end starts at the last row", SmartTable::RowSpace::FindIdNear( Rows, TEXT( "Line8" ), 40, RowIdOf ), 8 );

            Reads = 0;
            AITEST_EQUAL( "A row that is gone is found nowhere", SmartTable::RowSpace::FindIdNear( Rows, TEXT( "Line42" ), 6, RowIdOf ), INDEX_NONE );
            AITEST_EQUAL( "...after a read of every row", Reads, Rows.Num() );

            AITEST_EQUAL( "No rows find nothing", SmartTable::RowSpace::FindIdNear( {}, TEXT( "Line1" ), 0, RowIdOf ), INDEX_NONE );
            AITEST_EQUAL( "An id of None finds nothing", SmartTable::RowSpace::FindIdNear( Rows, NAME_None, 0, RowIdOf ), INDEX_NONE );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FARowIsFoundNearWhereItWas, "SmartTables.RowSpace.ARowIsFoundNearWhereItWas" );
}

UE_ENABLE_OPTIMIZATION_SHIP

#undef LOCTEXT_NAMESPACE

#endif
