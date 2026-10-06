// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTablesTestsMacros.h"

#if SMARTTABLES_WITH_TESTS

#include "AITestsCommon.h"

#include "SmartTableResizeGesture.h"

#define LOCTEXT_NAMESPACE "SmartTablesTest"

UE_DISABLE_OPTIMIZATION_SHIP

namespace SmartTable::Test
{

    static constexpr int32 PressUser     = 7;
    static constexpr uint32 PressPointer = 3;

    static FSmartTablePointerResize PressOn( const TCHAR * ColumnId )
    {
        FSmartTablePointerResize Dragging;
        Dragging.ColumnId     = ColumnId;
        Dragging.PressLocalX  = 412.0f;
        Dragging.StartWidth   = 180.0f;
        Dragging.UserIndex    = PressUser;
        Dragging.PointerIndex = PressPointer;

        return Dragging;
    }

    static FSmartTableIntentResize HoldOn( const TCHAR * ColumnId )
    {
        FSmartTableIntentResize Holding;
        Holding.ColumnId = ColumnId;

        return Holding;
    }

    struct FResetLeavesNothingBehind : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FSmartTableResizeGesture Gesture;

            AITEST_FALSE( "A gesture nobody began is not active", Gesture.IsActive() );
            AITEST_TRUE( "...and is at rest", Gesture.IsAtRest() );

            Gesture.Kind.Set< FSmartTablePointerResize >( PressOn( TEXT( "Callsign" ) ) );
            Gesture.StartWidths.Add( TEXT( "Callsign" ), 180.0f );
            Gesture.StartWidths.Add( TEXT( "Owner" ), 90.0f );

            AITEST_TRUE( "Taking a shape is what makes it active", Gesture.IsActive() );

            Gesture.Reset();

            AITEST_FALSE( "Reset ends the gesture", Gesture.IsActive() );
            AITEST_TRUE( "...and puts it back at rest", Gesture.IsAtRest() );
            AITEST_EQUAL( "...the column is let go", Gesture.GetColumnId(), FName( NAME_None ) );
            AITEST_NULL( "...the press behind it is gone", Gesture.AsPointer() );
            AITEST_FALSE( "...and no user drives it any more", Gesture.IsDrivenBy( PressUser, PressPointer ) );
            AITEST_EQUAL( "...and the width snapshot is empty", Gesture.StartWidths.Num(), 0 );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FResetLeavesNothingBehind, "SmartTables.ResizeGesture.ResetLeavesNothingBehind" );

    struct FASecondGestureInheritsNoBaseline : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FSmartTableResizeGesture Gesture;

            Gesture.Kind.Set< FSmartTablePointerResize >( PressOn( TEXT( "Callsign" ) ) );
            Gesture.StartWidths.Add( TEXT( "Owner" ), 90.0f );
            Gesture.Reset();

            Gesture.Kind.Set< FSmartTablePointerResize >( PressOn( TEXT( "Owner" ) ) );

            AITEST_NULL( "The previous gesture's snapshot is gone", Gesture.StartWidths.Find( TEXT( "Owner" ) ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FASecondGestureInheritsNoBaseline, "SmartTables.ResizeGesture.ASecondGestureInheritsNoBaseline" );

    struct FAnIntentCarriesNoPressToBeReadOff : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FSmartTableResizeGesture Gesture;
            Gesture.Kind.Set< FSmartTableIntentResize >( HoldOn( TEXT( "Callsign" ) ) );

            AITEST_TRUE( "An intent hold is a live resize", Gesture.IsActive() );
            AITEST_TRUE( "...and says which way it came in", Gesture.IsFromIntent() );
            AITEST_EQUAL( "...naming its column", Gesture.GetColumnId(), FName( TEXT( "Callsign" ) ) );

            AITEST_NULL( "There is no press to read off it", Gesture.AsPointer() );
            AITEST_FALSE( "...so no release can end it", Gesture.IsDrivenBy( PressUser, PressPointer ) );

            Gesture.Kind.Set< FSmartTablePointerResize >( PressOn( TEXT( "Callsign" ) ) );

            AITEST_FALSE( "A pointer taking over stops it being an intent", Gesture.IsFromIntent() );
            AITEST_NOT_NULL( "...and brings a press with it", Gesture.AsPointer() );
            AITEST_TRUE( "...and a user whose release ends it", Gesture.IsDrivenBy( PressUser, PressPointer ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAnIntentCarriesNoPressToBeReadOff, "SmartTables.ResizeGesture.AnIntentCarriesNoPressToBeReadOff" );

    struct FOnlyThePressingUserCanEndTheDrag : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FSmartTableResizeGesture Gesture;
            Gesture.Kind.Set< FSmartTablePointerResize >( PressOn( TEXT( "Callsign" ) ) );

            AITEST_TRUE( "The user that pressed drives it", Gesture.IsDrivenBy( PressUser, PressPointer ) );
            AITEST_FALSE( "Another user's release does not", Gesture.IsDrivenBy( PressUser + 1, PressPointer ) );
            AITEST_FALSE( "Nor another pointer on the same user", Gesture.IsDrivenBy( PressUser, PressPointer + 1 ) );

            AITEST_FALSE( "Nor the hardware cursor, when it was not the one that pressed", Gesture.IsDrivenBy( 0, 0 ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FOnlyThePressingUserCanEndTheDrag, "SmartTables.ResizeGesture.OnlyThePressingUserCanEndTheDrag" );

    struct FTheGripFillsASnapshotWithNoGestureBehindIt : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FSmartTableResizeGesture Gesture;
            Gesture.StartWidths.Add( TEXT( "Callsign" ), 180.0f );

            AITEST_FALSE( "Nothing is being resized", Gesture.IsActive() );
            AITEST_FALSE( "...but the gesture is not at rest, so FinishResize still has work", Gesture.IsAtRest() );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FTheGripFillsASnapshotWithNoGestureBehindIt, "SmartTables.ResizeGesture.TheGripFillsASnapshotWithNoGestureBehindIt" );
}
UE_ENABLE_OPTIMIZATION_SHIP

#undef LOCTEXT_NAMESPACE

#endif
