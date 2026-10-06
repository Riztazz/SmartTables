// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTablesTestsMacros.h"

#if SMARTTABLES_WITH_TESTS

#include "AITestsCommon.h"

#include "SmartTableColumnLayout.h"
#include "SmartTableLayoutStore.h"
#include "SmartTableTestTypes.h"

#define LOCTEXT_NAMESPACE "SmartTablesTest"

UE_DISABLE_OPTIMIZATION_SHIP

namespace SmartTable::Test
{
    using namespace SmartTable::ColumnLayout;

    static FColumn Fill( const TCHAR * ColumnId, float Coefficient )
    {
        FColumn Column;
        Column.ColumnId      = ColumnId;
        Column.Sizing        = ESmartTableColumnSizing::Fill;
        Column.AuthoredWidth = Coefficient;

        return Column;
    }

    static FColumn Fixed( const TCHAR * ColumnId, float Pixels )
    {
        FColumn Column;
        Column.ColumnId      = ColumnId;
        Column.Sizing        = ESmartTableColumnSizing::Fixed;
        Column.AuthoredWidth = Pixels;

        return Column;
    }

    static FColumn Dragged( const TCHAR * ColumnId, float Pixels )
    {
        FColumn Column   = Fill( ColumnId, 1.0f );
        Column.UserWidth = Pixels;

        return Column;
    }

    static float WidthOf( const TArray< FResolvedWidth > & Resolved, const TCHAR * ColumnId )
    {
        const FResolvedWidth * Found = Resolved.FindByPredicate( [ ColumnId ]( const FResolvedWidth & Candidate )
        {
            return Candidate.ColumnId == ColumnId;
        } );

        return Found ? Found->Width : -1.0f;
    }

    static float TotalOf( const TArray< FResolvedWidth > & Resolved )
    {
        float Total = 0.0f;
        for ( const FResolvedWidth & Entry : Resolved )
        {
            Total += Entry.Width;
        }

        return Total;
    }

    struct FResolvingTwiceAtTheSameWidthChangesNothing : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const TArray< FColumn > Columns = { Fixed( TEXT( "Icon" ), 100.0f ), Fill( TEXT( "Name" ), 1.0f ), Dragged( TEXT( "Notes" ), 220.0f ) };

            const TArray< FResolvedWidth > First = ResolveWidths( Columns, 800.0f, 24.0f, false );
            const TArray< FResolvedWidth > Again = ResolveWidths( Columns, 800.0f, 24.0f, false );

            AITEST_EQUAL( "The same columns come back", First.Num(), Again.Num() );
            for ( int32 Index = 0; Index < First.Num(); ++Index )
            {
                AITEST_TRUE( "...naming the same column", First[ Index ].ColumnId == Again[ Index ].ColumnId );
                AITEST_TRUE( "...at the same width", FMath::IsNearlyEqual( First[ Index ].Width, Again[ Index ].Width ) );
            }

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FResolvingTwiceAtTheSameWidthChangesNothing, "SmartTables.ColumnLayout.ResolvingTwiceAtTheSameWidthChangesNothing" );

    struct FResolvingSplitsTheLeftoverByFillWeight : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const TArray< FColumn > Columns         = { Fixed( TEXT( "Icon" ), 100.0f ), Fill( TEXT( "Name" ), 1.0f ), Fill( TEXT( "Notes" ), 2.0f ) };
            const TArray< FResolvedWidth > Resolved = ResolveWidths( Columns, 400.0f, 24.0f, false );

            AITEST_EQUAL( "Every column was resolved", Resolved.Num(), 3 );
            AITEST_TRUE( "The fixed column takes its authored pixels", FMath::IsNearlyEqual( WidthOf( Resolved, TEXT( "Icon" ) ), 100.0f ) );
            AITEST_TRUE( "One share of the leftover", FMath::IsNearlyEqual( WidthOf( Resolved, TEXT( "Name" ) ), 100.0f ) );
            AITEST_TRUE( "Two shares of the leftover", FMath::IsNearlyEqual( WidthOf( Resolved, TEXT( "Notes" ) ), 200.0f ) );

            AITEST_TRUE( "An unmeasured table resolves nothing", ResolveWidths( Columns, 0.0f, 24.0f, false ).IsEmpty() );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FResolvingSplitsTheLeftoverByFillWeight, "SmartTables.ColumnLayout.ResolvingSplitsTheLeftoverByFillWeight" );

    struct FAResizeMovesOnlyTheFillColumns : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const TArray< FColumn > Columns = {
                Fixed( TEXT( "Icon" ), 100.0f ),
                Dragged( TEXT( "Callsign" ), 220.0f ),
                Fill( TEXT( "Name" ), 1.0f ),
                Fill( TEXT( "Notes" ), 2.0f ),
            };

            const TArray< FResolvedWidth > Wide   = ResolveWidths( Columns, 2413.0f, 24.0f, false );
            const TArray< FResolvedWidth > Narrow = ResolveWidths( Columns, 2395.0f, 24.0f, false );

            AITEST_TRUE( "The fixed column did not move", FMath::IsNearlyEqual( WidthOf( Narrow, TEXT( "Icon" ) ), 100.0f ) );
            AITEST_TRUE( "The dragged column did not move", FMath::IsNearlyEqual( WidthOf( Narrow, TEXT( "Callsign" ) ), 220.0f ) );

            AITEST_TRUE( "One share of the loss", FMath::IsNearlyEqual( WidthOf( Wide, TEXT( "Name" ) ) - WidthOf( Narrow, TEXT( "Name" ) ), 6.0f ) );
            AITEST_TRUE( "Two shares of the loss", FMath::IsNearlyEqual( WidthOf( Wide, TEXT( "Notes" ) ) - WidthOf( Narrow, TEXT( "Notes" ) ), 12.0f ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAResizeMovesOnlyTheFillColumns, "SmartTables.ColumnLayout.AResizeMovesOnlyTheFillColumns" );

    struct FAUserWidthSurvivesEveryResize : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const TArray< FColumn > Columns = { Dragged( TEXT( "Notes" ), 317.5f ), Fill( TEXT( "Name" ), 1.0f ) };

            AITEST_TRUE( "Cramped", FMath::IsNearlyEqual( WidthOf( ResolveWidths( Columns, 300.0f, 24.0f, false ), TEXT( "Notes" ) ), 317.5f ) );
            AITEST_TRUE( "Ordinary", FMath::IsNearlyEqual( WidthOf( ResolveWidths( Columns, 2413.0f, 24.0f, false ), TEXT( "Notes" ) ), 317.5f ) );
            AITEST_TRUE( "Enormous", FMath::IsNearlyEqual( WidthOf( ResolveWidths( Columns, 10000.0f, 24.0f, false ), TEXT( "Notes" ) ), 317.5f ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAUserWidthSurvivesEveryResize, "SmartTables.ColumnLayout.AUserWidthSurvivesEveryResize" );

    struct FShrinkingOverflowsInsteadOfRewritingUserWidths : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const TArray< FColumn > Columns         = { Dragged( TEXT( "Name" ), 400.0f ), Dragged( TEXT( "Notes" ), 400.0f ), Fill( TEXT( "Tail" ), 1.0f ) };
            const TArray< FResolvedWidth > Resolved = ResolveWidths( Columns, 500.0f, 24.0f, false );

            AITEST_TRUE( "The first user width is untouched", FMath::IsNearlyEqual( WidthOf( Resolved, TEXT( "Name" ) ), 400.0f ) );
            AITEST_TRUE( "...and so is the second", FMath::IsNearlyEqual( WidthOf( Resolved, TEXT( "Notes" ) ), 400.0f ) );
            AITEST_TRUE( "The fill is at the floor", FMath::IsNearlyEqual( WidthOf( Resolved, TEXT( "Tail" ) ), 24.0f ) );
            AITEST_TRUE( "...and the table overflows, honestly", TotalOf( Resolved ) > 500.0f );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FShrinkingOverflowsInsteadOfRewritingUserWidths, "SmartTables.ColumnLayout.ShrinkingOverflowsInsteadOfRewritingUserWidths" );

    struct FEveryColumnIsAtLeastTheMinimumWidth : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const TArray< FColumn > Columns         = { Dragged( TEXT( "Sliver" ), 3.0f ), Fixed( TEXT( "Tiny" ), 1.0f ), Fill( TEXT( "Name" ), 1.0f ) };
            const TArray< FResolvedWidth > Resolved = ResolveWidths( Columns, 60.0f, 24.0f, false );

            for ( const FResolvedWidth & Entry : Resolved )
            {
                AITEST_TRUE( "Nothing resolves below the floor", Entry.Width >= 24.0f );
            }

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FEveryColumnIsAtLeastTheMinimumWidth, "SmartTables.ColumnLayout.EveryColumnIsAtLeastTheMinimumWidth" );

    struct FHidingAColumnHandsItsSpaceToTheFills : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const TArray< FColumn > Shown = { Fixed( TEXT( "Icon" ), 100.0f ), Fill( TEXT( "Name" ), 1.0f ), Fill( TEXT( "Notes" ), 1.0f ) };
            const TArray< FColumn > Fewer = { Fill( TEXT( "Name" ), 1.0f ), Fill( TEXT( "Notes" ), 1.0f ) };

            const TArray< FResolvedWidth > All    = ResolveWidths( Shown, 500.0f, 24.0f, false );
            const TArray< FResolvedWidth > Hidden = ResolveWidths( Fewer, 500.0f, 24.0f, false );

            AITEST_EQUAL( "The hidden column has no entry", Hidden.Num(), 2 );
            AITEST_TRUE( "Its pixels went to the fills", FMath::IsNearlyEqual( WidthOf( Hidden, TEXT( "Name" ) ), 250.0f ) );
            AITEST_TRUE( "...evenly", FMath::IsNearlyEqual( WidthOf( Hidden, TEXT( "Notes" ) ), 250.0f ) );

            const TArray< FResolvedWidth > Back = ResolveWidths( Shown, 500.0f, 24.0f, false );
            AITEST_TRUE( "Showing it again restores the original split", FMath::IsNearlyEqual( WidthOf( Back, TEXT( "Name" ) ), WidthOf( All, TEXT( "Name" ) ) ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FHidingAColumnHandsItsSpaceToTheFills, "SmartTables.ColumnLayout.HidingAColumnHandsItsSpaceToTheFills" );

    struct FTheLastColumnStretchAbsorbsExactlyTheLeftover : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const TArray< FColumn > Columns         = { Fixed( TEXT( "Icon" ), 100.0f ), Fixed( TEXT( "Name" ), 100.0f ), Fixed( TEXT( "Notes" ), 50.0f ) };
            const TArray< FResolvedWidth > Resolved = ResolveWidths( Columns, 400.0f, 24.0f, true );

            AITEST_TRUE( "The last column closes the gap", FMath::IsNearlyEqual( WidthOf( Resolved, TEXT( "Notes" ) ), 200.0f ) );
            AITEST_TRUE( "...and the table fills its space exactly", FMath::IsNearlyEqual( TotalOf( Resolved ), 400.0f ) );

            const TArray< FColumn > Wide = { Fixed( TEXT( "Icon" ), 100.0f ), Fixed( TEXT( "Name" ), 100.0f ), Dragged( TEXT( "Notes" ), 400.0f ) };
            AITEST_TRUE( "A wider user width is left alone", FMath::IsNearlyEqual( WidthOf( ResolveWidths( Wide, 400.0f, 24.0f, true ), TEXT( "Notes" ) ), 400.0f ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FTheLastColumnStretchAbsorbsExactlyTheLeftover, "SmartTables.ColumnLayout.TheLastColumnStretchAbsorbsExactlyTheLeftover" );

    struct FANonResizableColumnResolvesByItsSizing : FAITestBase
    {
        virtual bool InstantTest() override
        {
            TArray< FColumn > Columns = { Fill( TEXT( "Locked" ), 1.0f ), Fill( TEXT( "Name" ), 1.0f ) };
            Columns[ 0 ].bResizable   = false;

            const TArray< FResolvedWidth > Resolved = ResolveWidths( Columns, 300.0f, 24.0f, false );

            AITEST_EQUAL( "It is answered for like any other", Resolved.Num(), 2 );
            AITEST_TRUE( "...by its coefficient, not as pixels", FMath::IsNearlyEqual( WidthOf( Resolved, TEXT( "Locked" ) ), 150.0f ) );
            AITEST_TRUE( "...and its neighbour is unaffected", FMath::IsNearlyEqual( WidthOf( Resolved, TEXT( "Name" ) ), 150.0f ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FANonResizableColumnResolvesByItsSizing, "SmartTables.ColumnLayout.ANonResizableColumnResolvesByItsSizing" );

    static FColumn Measured( const TCHAR * ColumnId, float Width, bool bResizable = true )
    {
        FColumn Column;
        Column.ColumnId     = ColumnId;
        Column.CurrentWidth = Width;
        Column.bResizable   = bResizable;

        return Column;
    }

    struct FEdgeHitTestFindsTheEdgeUnderThePointer : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const TArray< FColumn > Columns = { Measured( TEXT( "Name" ), 100.0f ), Measured( TEXT( "Notes" ), 150.0f ) };

            AITEST_TRUE( "Dead on the first edge", FindEdgeAt( Columns, 100.0f, 8.0f, 0.0f ).ColumnId == TEXT( "Name" ) );
            AITEST_TRUE( "Just inside the grip", FindEdgeAt( Columns, 106.0f, 8.0f, 0.0f ).ColumnId == TEXT( "Name" ) );
            AITEST_TRUE( "Just outside it", FindEdgeAt( Columns, 120.0f, 8.0f, 0.0f ).ColumnId.IsNone() );
            AITEST_TRUE( "The second edge", FindEdgeAt( Columns, 250.0f, 8.0f, 0.0f ).ColumnId == TEXT( "Notes" ) );

            AITEST_TRUE( "The column's left edge rides along", FMath::IsNearlyEqual( FindEdgeAt( Columns, 250.0f, 8.0f, 0.0f ).Left, 100.0f ) );

            AITEST_TRUE( "A leading offset moves every edge", FindEdgeAt( Columns, 140.0f, 8.0f, 40.0f ).ColumnId == TEXT( "Name" ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FEdgeHitTestFindsTheEdgeUnderThePointer, "SmartTables.ColumnLayout.EdgeHitTestFindsTheEdgeUnderThePointer" );

    struct FEdgeHitTestSkipsColumnsTheAuthorLockedAgainstResize : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const TArray< FColumn > Columns = { Measured( TEXT( "Locked" ), 100.0f, false ), Measured( TEXT( "Notes" ), 150.0f ) };

            AITEST_TRUE( "No grip on a locked column's edge", FindEdgeAt( Columns, 100.0f, 8.0f, 0.0f ).ColumnId.IsNone() );

            AITEST_TRUE( "It still occupies its pixels", FindEdgeAt( Columns, 250.0f, 8.0f, 0.0f ).ColumnId == TEXT( "Notes" ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FEdgeHitTestSkipsColumnsTheAuthorLockedAgainstResize, "SmartTables.ColumnLayout.EdgeHitTestSkipsColumnsTheAuthorLockedAgainstResize" );

    struct FEdgeHitTestGivesUpPastAnUnmeasuredColumn : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const TArray< FColumn > Columns = { Measured( TEXT( "Name" ), 0.0f ), Measured( TEXT( "Notes" ), 150.0f ) };

            AITEST_TRUE( "Nothing is offered past the unknown", FindEdgeAt( Columns, 150.0f, 8.0f, 0.0f ).ColumnId.IsNone() );
            AITEST_TRUE( "Not even at its own nominal edge", FindEdgeAt( Columns, 0.0f, 8.0f, 0.0f ).ColumnId.IsNone() );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FEdgeHitTestGivesUpPastAnUnmeasuredColumn, "SmartTables.ColumnLayout.EdgeHitTestGivesUpPastAnUnmeasuredColumn" );

    struct FGrowDragTakesFromTheNearestRightNeighbourFirst : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const TArray< float > Right = { 100.0f, 100.0f };
            const FGrantedDrag Drag     = ResolveGrowDrag( 100.0f, 150.0f, Right, 24.0f );

            AITEST_TRUE( "The drag got what it asked for", FMath::IsNearlyEqual( Drag.Granted, 150.0f ) );
            AITEST_TRUE( "The nearest neighbour paid", FMath::IsNearlyEqual( Drag.RightWidths[ 0 ], 50.0f ) );
            AITEST_TRUE( "The far one did not", FMath::IsNearlyEqual( Drag.RightWidths[ 1 ], 100.0f ) );
            AITEST_TRUE( "The drag was resolved instead of refused", Drag.bNegotiated );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FGrowDragTakesFromTheNearestRightNeighbourFirst, "SmartTables.ColumnLayout.GrowDragTakesFromTheNearestRightNeighbourFirst" );

    struct FGrowDragStopsWhenEveryRightColumnIsAtTheFloor : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const TArray< float > Right = { 100.0f, 100.0f };
            const FGrantedDrag Drag     = ResolveGrowDrag( 100.0f, 10000.0f, Right, 24.0f );

            AITEST_TRUE( "Growth stops at the floor, it does not push columns off screen", FMath::IsNearlyEqual( Drag.Granted, 252.0f ) );
            AITEST_TRUE( "Both neighbours are at the floor", FMath::IsNearlyEqual( Drag.RightWidths[ 0 ], 24.0f ) );
            AITEST_TRUE( "...both of them", FMath::IsNearlyEqual( Drag.RightWidths[ 1 ], 24.0f ) );

            AITEST_TRUE( "The drag was resolved instead of refused", Drag.bNegotiated );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FGrowDragStopsWhenEveryRightColumnIsAtTheFloor, "SmartTables.ColumnLayout.GrowDragStopsWhenEveryRightColumnIsAtTheFloor" );

    struct FShrinkingGivesTheSpaceBackToTheSameNeighbour : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const TArray< float > Start = { 100.0f, 100.0f };

            const FGrantedDrag Out = ResolveGrowDrag( 100.0f, 150.0f, Start, 24.0f );
            AITEST_TRUE( "On the way out the neighbour pays", FMath::IsNearlyEqual( Out.RightWidths[ 0 ], 50.0f ) );

            const FGrantedDrag Back = ResolveGrowDrag( 100.0f, 100.0f, Start, 24.0f );

            AITEST_TRUE( "The drag shrank as asked", FMath::IsNearlyEqual( Back.Granted, 100.0f ) );
            AITEST_TRUE( "The neighbour got its pixels back", FMath::IsNearlyEqual( Back.RightWidths[ 0 ], 100.0f ) );
            AITEST_TRUE( "The far column never moved", FMath::IsNearlyEqual( Back.RightWidths[ 1 ], 100.0f ) );

            const FGrantedDrag Past = ResolveGrowDrag( 100.0f, 60.0f, Start, 24.0f );
            AITEST_TRUE( "Giving space up widens the same neighbour", FMath::IsNearlyEqual( Past.RightWidths[ 0 ], 140.0f ) );
            AITEST_TRUE( "...and still nothing beyond it", FMath::IsNearlyEqual( Past.RightWidths[ 1 ], 100.0f ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FShrinkingGivesTheSpaceBackToTheSameNeighbour, "SmartTables.ColumnLayout.ShrinkingGivesTheSpaceBackToTheSameNeighbour" );

    struct FAGrowDragRefusesToNegotiateAgainstAnUnmeasuredColumn : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const TArray< float > WithUnknown = { 0.0f, 150.0f, 100.0f };
            const FGrantedDrag Drag           = ResolveGrowDrag( 108.0f, 116.0f, WithUnknown, 24.0f );

            AITEST_TRUE( "The unmeasured neighbour is left alone", FMath::IsNearlyEqual( Drag.RightWidths[ 0 ], 0.0f ) );
            AITEST_TRUE( "...and so is every other neighbour", FMath::IsNearlyEqual( Drag.RightWidths[ 1 ], 150.0f ) );
            AITEST_TRUE( "...all of them", FMath::IsNearlyEqual( Drag.RightWidths[ 2 ], 100.0f ) );
            AITEST_FALSE( "The drag was refused, so none of these may be written", Drag.bNegotiated );

            AITEST_TRUE( "The dragged column still gets its width", FMath::IsNearlyEqual( Drag.Granted, 116.0f ) );

            const TArray< float > AllKnown = { 100.0f, 100.0f };
            const FGrantedDrag Normal      = ResolveGrowDrag( 100.0f, 150.0f, AllKnown, 24.0f );
            AITEST_TRUE( "A measured table still negotiates", FMath::IsNearlyEqual( Normal.RightWidths[ 0 ], 50.0f ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAGrowDragRefusesToNegotiateAgainstAnUnmeasuredColumn, "SmartTables.ColumnLayout.AGrowDragRefusesToNegotiateAgainstAnUnmeasuredColumn" );

    struct FADragMovesANeighbourByTheDragAndNotByTheEmptySpace : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const TArray< float > Right = { 130.0f, 110.0f, 130.0f, 110.0f, 110.0f, 110.0f, 90.0f, 140.0f, 100.0f, 320.0f };
            const FGrantedDrag Drag     = ResolveGrowDrag( 150.0f, 158.0f, Right, 24.0f );

            AITEST_TRUE( "The neighbour paid the drag, not the empty space", FMath::IsNearlyEqual( Drag.RightWidths[ 0 ], 122.0f ) );
            AITEST_TRUE( "The drag got what it asked for", FMath::IsNearlyEqual( Drag.Granted, 158.0f ) );
            AITEST_TRUE( "The drag was resolved instead of refused", Drag.bNegotiated );

            const FGrantedDrag Still = ResolveGrowDrag( 150.0f, 150.0f, Right, 24.0f );
            AITEST_TRUE( "A drag of nothing moves nothing", FMath::IsNearlyEqual( Still.RightWidths[ 0 ], 130.0f ) );
            AITEST_TRUE( "...and the far columns are equally still", FMath::IsNearlyEqual( Still.RightWidths[ 9 ], 320.0f ) );

            AITEST_TRUE( "...and still answers for every column", Still.bNegotiated );
            AITEST_EQUAL( "...all ten of them", Still.RightWidths.Num(), 10 );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FADragMovesANeighbourByTheDragAndNotByTheEmptySpace, "SmartTables.ColumnLayout.ADragMovesANeighbourByTheDragAndNotByTheEmptySpace" );

    struct FAColumnHitTestNamesTheColumnUnderThePointer : FAITestBase
    {
        virtual bool InstantTest() override
        {
            TArray< FColumn > Shown = { Fixed( TEXT( "Callsign" ), 100.0f ), Fixed( TEXT( "Class" ), 60.0f ), Fixed( TEXT( "Owner" ), 80.0f ) };
            for ( FColumn & Column : Shown )
            {
                Column.CurrentWidth = Column.AuthoredWidth;
            }

            AITEST_EQUAL( "Inside the first column", ColumnAt( Shown, 25.0f, 20.0f ), FName( TEXT( "Callsign" ) ) );
            AITEST_EQUAL( "Its right edge belongs to the NEXT column", ColumnAt( Shown, 120.0f, 20.0f ), FName( TEXT( "Class" ) ) );
            AITEST_EQUAL( "Inside the last", ColumnAt( Shown, 200.0f, 20.0f ), FName( TEXT( "Owner" ) ) );

            AITEST_EQUAL( "The row-number gutter selects nothing", ColumnAt( Shown, 5.0f, 20.0f ), FName( NAME_None ) );
            AITEST_EQUAL( "Past the end selects nothing", ColumnAt( Shown, 5000.0f, 20.0f ), FName( NAME_None ) );

            Shown[ 1 ].bResizable = false;
            AITEST_EQUAL( "A locked column is still selectable", ColumnAt( Shown, 130.0f, 20.0f ), FName( TEXT( "Class" ) ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAColumnHitTestNamesTheColumnUnderThePointer, "SmartTables.ColumnLayout.AColumnHitTestNamesTheColumnUnderThePointer" );

    struct FStretchedLastColumnIsAFloorNotAnOverride : FAITestBase
    {
        virtual bool InstantTest() override
        {
            AITEST_TRUE( "It fills the leftover", FMath::IsNearlyEqual( StretchLast( 50.0f, 400.0f, 300.0f ), 100.0f ) );

            AITEST_TRUE( "A wider column is left alone", FMath::IsNearlyEqual( StretchLast( 250.0f, 400.0f, 300.0f ), 250.0f ) );

            AITEST_TRUE( "No leftover means no change", FMath::IsNearlyEqual( StretchLast( 80.0f, 400.0f, 500.0f ), 80.0f ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FStretchedLastColumnIsAFloorNotAnOverride, "SmartTables.ColumnLayout.StretchedLastColumnIsAFloorNotAnOverride" );

    struct FAPartialOrderKeepsUnnamedColumnsInAuthoredOrder : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const TArray< FName > Authored = { TEXT( "A" ), TEXT( "B" ), TEXT( "C" ), TEXT( "D" ) };
            const TArray< FName > Merged   = MergeOrder( Authored, { TEXT( "C" ), TEXT( "A" ) } );

            AITEST_EQUAL( "Every column is still present", Merged.Num(), 4 );
            AITEST_TRUE( "The named ones lead, in the order asked for", Merged[ 0 ] == TEXT( "C" ) && Merged[ 1 ] == TEXT( "A" ) );
            AITEST_TRUE( "The rest keep their authored order", Merged[ 2 ] == TEXT( "B" ) && Merged[ 3 ] == TEXT( "D" ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAPartialOrderKeepsUnnamedColumnsInAuthoredOrder, "SmartTables.ColumnLayout.APartialOrderKeepsUnnamedColumnsInAuthoredOrder" );

    struct FAnOrderNamesEachColumnAtMostOnce : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const TArray< FName > Authored = { TEXT( "A" ), TEXT( "B" ) };

            const TArray< FName > WithGhost = MergeOrder( Authored, { TEXT( "Ghost" ), TEXT( "B" ) } );
            AITEST_EQUAL( "The unknown name is dropped", WithGhost.Num(), 2 );
            AITEST_TRUE( "...and the real one still leads", WithGhost[ 0 ] == TEXT( "B" ) );

            const TArray< FName > WithRepeat = MergeOrder( Authored, { TEXT( "B" ), TEXT( "B" ), TEXT( "A" ) } );
            AITEST_EQUAL( "A repeat does not duplicate the column", WithRepeat.Num(), 2 );
            AITEST_TRUE( "...and the order still holds", WithRepeat[ 0 ] == TEXT( "B" ) && WithRepeat[ 1 ] == TEXT( "A" ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAnOrderNamesEachColumnAtMostOnce, "SmartTables.ColumnLayout.AnOrderNamesEachColumnAtMostOnce" );
    struct FAnOlderFileIsBroughtForwardWithNoOrderInvented : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FSmartTableLayout Old;
            Old.Version                                = 1;
            Old.Columns.AddDefaulted_GetRef().ColumnId = TEXT( "Mass" );
            Old.Columns.AddDefaulted_GetRef().ColumnId = TEXT( "Status" );
            Old.Columns.AddDefaulted_GetRef().ColumnId = TEXT( "Crew" );

            AITEST_TRUE( "An older file is brought forward", SmartTable::MigrateStoredLayout( Old ) );
            AITEST_EQUAL( "And says so", Old.Version, FSmartTableLayout::CurrentVersion );
            AITEST_EQUAL( "The overrides are left alone", Old.Columns.Num(), 3 );

            AITEST_EQUAL( "And the order is left for the table, which alone knows how many columns there are", Old.Order.Num(), 0 );

            AITEST_FALSE( "A file already forward is left alone", SmartTable::MigrateStoredLayout( Old ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAnOlderFileIsBroughtForwardWithNoOrderInvented, "SmartTables.ColumnLayout.AnOlderFileIsBroughtForwardWithNoOrderInvented" );

    struct FARenamedColumnIsStillDrawn : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const TArray< FName > Authored = { TEXT( "Mass" ), TEXT( "Renamed" ), TEXT( "Crew" ) };
            const TArray< FName > Stored   = { TEXT( "Mass" ), TEXT( "WasCalledThis" ), TEXT( "Crew" ) };

            const TArray< FName > Merged = SmartTable::ColumnLayout::MergeOrder( Authored, Stored );

            AITEST_EQUAL( "Every column is placed", Merged.Num(), 3 );
            AITEST_TRUE( "The renamed one among them", Merged.Contains( FName( TEXT( "Renamed" ) ) ) );
            AITEST_FALSE( "And the id naming nothing is gone", Merged.Contains( FName( TEXT( "WasCalledThis" ) ) ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FARenamedColumnIsStillDrawn, "SmartTables.ColumnLayout.ARenamedColumnIsStillDrawn" );

    struct FAPartOldLayoutDoesNotBecomeAnOrder : FAITestBase
    {
        static FSmartTableColumn Named( const TCHAR * Id )
        {
            FSmartTableColumn Column;
            Column.ColumnId = Id;

            return Column;
        }

        virtual bool InstantTest() override
        {
            USmartTableTestHarness * Table = NewObject< USmartTableTestHarness >();
            AITEST_NOT_NULL( "A table builds", Table );

            Table->Author( { Named( TEXT( "A" ) ), Named( TEXT( "B" ) ), Named( TEXT( "C" ) ), Named( TEXT( "D" ) ) } );

            USmartTableTestLayoutStore * Store = NewObject< USmartTableTestLayoutStore >();
            AITEST_NOT_NULL( "A store builds", Store );

            Store->Stored.Version                                = 1;
            Store->Stored.Columns.AddDefaulted_GetRef().ColumnId = TEXT( "D" );

            Table->SetTableId( TEXT( "PartOld" ) );
            Table->SetLayoutStore( Store );

            const TArray< FName > Order = Table->GetColumnOrder();

            AITEST_EQUAL( "Every column is placed", Order.Num(), 4 );
            AITEST_EQUAL( "And the first is the one authored first, not the one with an override", Order[ 0 ], FName( TEXT( "A" ) ) );
            AITEST_EQUAL( "...second", Order[ 1 ], FName( TEXT( "B" ) ) );
            AITEST_EQUAL( "...third", Order[ 2 ], FName( TEXT( "C" ) ) );
            AITEST_EQUAL( "...fourth", Order[ 3 ], FName( TEXT( "D" ) ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAPartOldLayoutDoesNotBecomeAnOrder, "SmartTables.ColumnLayout.APartOldLayoutDoesNotBecomeAnOrder" );

    struct FAWholeOldLayoutStillCarriesItsOrder : FAITestBase
    {
        static FSmartTableColumn Named( const TCHAR * Id )
        {
            FSmartTableColumn Column;
            Column.ColumnId = Id;

            return Column;
        }

        virtual bool InstantTest() override
        {
            USmartTableTestHarness * Table     = NewObject< USmartTableTestHarness >();
            USmartTableTestLayoutStore * Store = NewObject< USmartTableTestLayoutStore >();
            AITEST_NOT_NULL( "A table and a store build", Store );

            Table->Author( { Named( TEXT( "A" ) ), Named( TEXT( "B" ) ), Named( TEXT( "C" ) ) } );

            Store->Stored.Version = 1;
            for ( const TCHAR * Id : { TEXT( "C" ), TEXT( "A" ), TEXT( "B" ) } )
            {
                Store->Stored.Columns.AddDefaulted_GetRef().ColumnId = Id;
            }

            Table->SetTableId( TEXT( "WholeOld" ) );
            Table->SetLayoutStore( Store );

            const TArray< FName > Order = Table->GetColumnOrder();

            AITEST_EQUAL( "The stored order comes forward", Order.Num(), 3 );
            AITEST_EQUAL( "In the order it was stored in", Order[ 0 ], FName( TEXT( "C" ) ) );
            AITEST_EQUAL( "...second", Order[ 1 ], FName( TEXT( "A" ) ) );
            AITEST_EQUAL( "...third", Order[ 2 ], FName( TEXT( "B" ) ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAWholeOldLayoutStillCarriesItsOrder, "SmartTables.ColumnLayout.AWholeOldLayoutStillCarriesItsOrder" );

    struct FMovingAColumnCountsPlacesOnScreen : FAITestBase
    {
        static FSmartTableColumn Named( const TCHAR * Id )
        {
            FSmartTableColumn Column;
            Column.ColumnId = Id;

            return Column;
        }

        virtual bool InstantTest() override
        {
            USmartTableTestHarness * Table = NewObject< USmartTableTestHarness >();
            Table->Author( { Named( TEXT( "A" ) ), Named( TEXT( "B" ) ), Named( TEXT( "C" ) ), Named( TEXT( "D" ) ) } );

            Table->SetColumnVisible( TEXT( "B" ), false );

            Table->MoveColumn( TEXT( "A" ), 1 );

            const TArray< FName > After = Table->GetColumnOrder();
            AITEST_EQUAL( "Every column is still placed", After.Num(), 4 );
            AITEST_EQUAL( "The hidden one keeps its place", After[ 0 ], FName( TEXT( "B" ) ) );
            AITEST_EQUAL( "...and A landed PAST the next shown column", After[ 1 ], FName( TEXT( "C" ) ) );
            AITEST_EQUAL( "...which is A one place further right on screen", After[ 2 ], FName( TEXT( "A" ) ) );
            AITEST_EQUAL( "...with D where it was", After[ 3 ], FName( TEXT( "D" ) ) );

            Table->MoveColumn( TEXT( "C" ), -1 );

            const TArray< FName > Refused = Table->GetColumnOrder();
            AITEST_EQUAL( "A column with no shown neighbour that way does not move", Refused[ 0 ], FName( TEXT( "B" ) ) );
            AITEST_EQUAL( "...and nothing else moved either", Refused[ 1 ], FName( TEXT( "C" ) ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FMovingAColumnCountsPlacesOnScreen, "SmartTables.ColumnLayout.MovingAColumnCountsPlacesOnScreen" );

    struct FOnlyTheColumnsADragReallyMovedAreWritten : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const TArray< float > Targets = { 120.0f, 90.0f, 200.0f, 150.0f };
            const TArray< float > Drawn   = { 100.0f, 90.0f, 200.0f, 150.0f };

            const TArray< int32 > Moved = WidthsToWrite( Targets, Drawn, 0.5f );

            AITEST_EQUAL( "One of four columns moved", Moved.Num(), 1 );
            AITEST_EQUAL( "...and it is the one beside the drag", Moved[ 0 ], 0 );

            const TArray< float > NoneDrawn;
            const TArray< int32 > All = WidthsToWrite( Targets, NoneDrawn, 0.5f );

            AITEST_EQUAL( "A first drag has no drawn width to match, so every answer is written", All.Num(), 4 );

            const TArray< int32 > Nothing = WidthsToWrite( Targets, Targets, 0.5f );

            AITEST_TRUE( "A drag that changed nothing marks nothing as chosen", Nothing.IsEmpty() );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FOnlyTheColumnsADragReallyMovedAreWritten, "SmartTables.ColumnLayout.OnlyTheColumnsADragReallyMovedAreWritten" );

    struct FAStepCountsOnlyShownColumns : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const TArray< FName > Order = { TEXT( "A" ), TEXT( "B" ), TEXT( "C" ), TEXT( "D" ), TEXT( "E" ) };
            const TSet< FName > Hidden  = { TEXT( "B" ), TEXT( "D" ) };

            const auto IsShown = [ &Hidden ]( FName ColumnId )
            {
                return !Hidden.Contains( ColumnId );
            };

            AITEST_EQUAL( "One step right from A passes hidden B and lands on C", LandingIndex( Order, 0, 1, IsShown ), 2 );
            AITEST_EQUAL( "Two steps right from A land on E", LandingIndex( Order, 0, 2, IsShown ), 4 );
            AITEST_EQUAL( "One step left from E passes hidden D and lands on C", LandingIndex( Order, 4, -1, IsShown ), 2 );
            AITEST_EQUAL( "A step past the last shown column lands on that last one", LandingIndex( Order, 0, 5, IsShown ), 4 );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAStepCountsOnlyShownColumns, "SmartTables.ColumnLayout.AStepCountsOnlyShownColumns" );

    struct FAStepWithNoShownColumnThatWayLandsNowhere : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const TArray< FName > Order = { TEXT( "A" ), TEXT( "B" ), TEXT( "C" ) };
            const TSet< FName > Hidden  = { TEXT( "B" ), TEXT( "C" ) };

            const auto IsShown = [ &Hidden ]( FName ColumnId )
            {
                return !Hidden.Contains( ColumnId );
            };

            AITEST_EQUAL( "Only hidden columns lie to the right", LandingIndex( Order, 0, 1, IsShown ), INDEX_NONE );
            AITEST_EQUAL( "Nothing lies to the left of the first column", LandingIndex( Order, 0, -1, IsShown ), INDEX_NONE );
            AITEST_EQUAL( "A step of zero lands nowhere", LandingIndex( Order, 0, 0, IsShown ), INDEX_NONE );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAStepWithNoShownColumnThatWayLandsNowhere, "SmartTables.ColumnLayout.AStepWithNoShownColumnThatWayLandsNowhere" );

    static FSmartTableColumn HeaderColumn( ESmartTableColumnSizing Sizing, bool bResizable )
    {
        FSmartTableColumn Column;
        Column.Sizing     = Sizing;
        Column.bResizable = bResizable;

        return Column;
    }

    struct FAResizableColumnGoesManualOnceWidthsAreLive : FAITestBase
    {
        virtual bool InstantTest() override
        {
            AITEST_TRUE( "A resizable Fill column goes Manual once the widths are live", HeaderWidthModeFor( HeaderColumn( ESmartTableColumnSizing::Fill, true ), true ) == EHeaderWidthMode::Manual );
            AITEST_TRUE( "A resizable Fixed column goes Manual too", HeaderWidthModeFor( HeaderColumn( ESmartTableColumnSizing::Fixed, true ), true ) == EHeaderWidthMode::Manual );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAResizableColumnGoesManualOnceWidthsAreLive, "SmartTables.ColumnLayout.AResizableColumnGoesManualOnceWidthsAreLive" );

    struct FBeforeWidthsAreLiveAColumnKeepsItsAuthoredMode : FAITestBase
    {
        virtual bool InstantTest() override
        {
            AITEST_TRUE( "Before the widths are live, a resizable Fill column stays Fill", HeaderWidthModeFor( HeaderColumn( ESmartTableColumnSizing::Fill, true ), false ) == EHeaderWidthMode::Fill );
            AITEST_TRUE( "Before the widths are live, a resizable Fixed column stays Fixed", HeaderWidthModeFor( HeaderColumn( ESmartTableColumnSizing::Fixed, true ), false ) == EHeaderWidthMode::Fixed );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FBeforeWidthsAreLiveAColumnKeepsItsAuthoredMode, "SmartTables.ColumnLayout.BeforeWidthsAreLiveAColumnKeepsItsAuthoredMode" );

    struct FALockedColumnNeverGoesManual : FAITestBase
    {
        virtual bool InstantTest() override
        {
            AITEST_TRUE( "A Fill column that cannot be resized stays Fill with the widths live", HeaderWidthModeFor( HeaderColumn( ESmartTableColumnSizing::Fill, false ), true ) == EHeaderWidthMode::Fill );
            AITEST_TRUE( "A Fixed column that cannot be resized stays Fixed with the widths live", HeaderWidthModeFor( HeaderColumn( ESmartTableColumnSizing::Fixed, false ), true ) == EHeaderWidthMode::Fixed );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FALockedColumnNeverGoesManual, "SmartTables.ColumnLayout.ALockedColumnNeverGoesManual" );

    struct FOnlyColumnsWhoseWidthMovedAreNamed : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const TMap< FName, float > Before    = { { TEXT( "Name" ), 100.0f }, { TEXT( "Mass" ), 50.0f }, { TEXT( "Notes" ), 80.0f } };
            const TArray< FResolvedWidth > After = { { TEXT( "Name" ), 100.0f }, { TEXT( "Mass" ), 60.0f }, { TEXT( "Notes" ), 80.2f } };

            const TArray< FName > Moved = MovedColumns( Before, After, 0.5f );

            AITEST_EQUAL( "Only one column moved past the epsilon", Moved.Num(), 1 );
            AITEST_TRUE( "...and it is the one whose width changed", Moved[ 0 ] == TEXT( "Mass" ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FOnlyColumnsWhoseWidthMovedAreNamed, "SmartTables.ColumnLayout.OnlyColumnsWhoseWidthMovedAreNamed" );

    struct FAColumnNewToThePassCountsAsMoved : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const TMap< FName, float > Before    = { { TEXT( "Name" ), 100.0f } };
            const TArray< FResolvedWidth > After = { { TEXT( "Name" ), 100.0f }, { TEXT( "Mass" ), 40.0f } };

            const TArray< FName > Moved = MovedColumns( Before, After, 0.5f );

            AITEST_EQUAL( "A column the last pass never sized counts as moved", Moved.Num(), 1 );
            AITEST_TRUE( "...and it is the new one", Moved[ 0 ] == TEXT( "Mass" ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAColumnNewToThePassCountsAsMoved, "SmartTables.ColumnLayout.AColumnNewToThePassCountsAsMoved" );

    struct FACursorNearAnEdgeScrollsTowardIt : FAITestBase
    {
        virtual bool InstantTest() override
        {
            AITEST_EQUAL( "Near the right edge the table pans right", AutoScrollStep( 490.0f, 500.0f, 48.0f, 24.0f ), 24.0f );
            AITEST_EQUAL( "Near the left edge it pans left", AutoScrollStep( 10.0f, 500.0f, 48.0f, 24.0f ), -24.0f );
            AITEST_EQUAL( "Past the right edge it still pans right", AutoScrollStep( 600.0f, 500.0f, 48.0f, 24.0f ), 24.0f );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FACursorNearAnEdgeScrollsTowardIt, "SmartTables.ColumnLayout.ACursorNearAnEdgeScrollsTowardIt" );

    struct FACursorAwayFromBothEdgesScrollsNothing : FAITestBase
    {
        virtual bool InstantTest() override
        {
            AITEST_EQUAL( "In the middle the table does not pan", AutoScrollStep( 250.0f, 500.0f, 48.0f, 24.0f ), 0.0f );
            AITEST_EQUAL( "On the inner line of the right zone it does not pan", AutoScrollStep( 452.0f, 500.0f, 48.0f, 24.0f ), 0.0f );
            AITEST_EQUAL( "On the inner line of the left zone it does not pan", AutoScrollStep( 48.0f, 500.0f, 48.0f, 24.0f ), 0.0f );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FACursorAwayFromBothEdgesScrollsNothing, "SmartTables.ColumnLayout.ACursorAwayFromBothEdgesScrollsNothing" );
}
UE_ENABLE_OPTIMIZATION_SHIP

#undef LOCTEXT_NAMESPACE
#endif
