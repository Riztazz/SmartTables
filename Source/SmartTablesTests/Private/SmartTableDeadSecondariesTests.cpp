// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTablesTestsMacros.h"

#if SMARTTABLES_WITH_TESTS

#include "AITestsCommon.h"

#include "SmartTableDeadSecondaries.h"
#include "SmartTableSorting.h"

#define LOCTEXT_NAMESPACE "SmartTablesTest"

UE_DISABLE_OPTIMIZATION_SHIP

namespace SmartTable::Test
{
    struct FDeadSecondariesRows
    {
        TArray< FName > ColumnIds = { TEXT( "Kind" ), TEXT( "Mass" ), TEXT( "Name" ) };

        TArray< TArray< FString > > Rows = { { TEXT( "A" ), TEXT( "1" ), TEXT( "x" ) }, { TEXT( "A" ), TEXT( "2" ), TEXT( "x" ) }, { TEXT( "B" ), TEXT( "3" ), TEXT( "y" ) } };

        void Recompute( FSmartTableDeadSecondaries & Dead, const FSmartTableSortSpec & Spec ) const
        {
            Dead.Recompute( Spec, Rows.Num(), ColumnIds, [ this ]( int32 PresentedRow, FName ColumnId )
            {
                return FSmartTableSortKey::MakeText( Rows[ PresentedRow ][ ColumnIds.IndexOfByKey( ColumnId ) ] );
            } );
        }
    };

    struct FAFreshValueIsStale : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const FSmartTableDeadSecondaries Dead;

            AITEST_TRUE( "A value nothing has worked out yet is stale", Dead.IsStale() );
            AITEST_FALSE( "...and holds no dead column", Dead.Contains( TEXT( "Name" ) ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAFreshValueIsStale, "SmartTables.DeadSecondaries.AFreshValueIsStale" );

    struct FWorkingItOutClearsTheStaleMark : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const FDeadSecondariesRows Table;
            FSmartTableDeadSecondaries Dead;

            Table.Recompute( Dead, Sorting::SpecWithPrimary( TEXT( "Kind" ), ESmartTableSortMode::Ascending ) );

            AITEST_FALSE( "Working the answer out clears the stale mark", Dead.IsStale() );
            AITEST_TRUE( "A column holding one value through every tie is dead", Dead.Contains( TEXT( "Name" ) ) );
            AITEST_FALSE( "A column that differs inside a tie is not", Dead.Contains( TEXT( "Mass" ) ) );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FWorkingItOutClearsTheStaleMark, "SmartTables.DeadSecondaries.WorkingItOutClearsTheStaleMark" );

    struct FNothingSortedLeavesNoColumnDead : FAITestBase
    {
        virtual bool InstantTest() override
        {
            const FDeadSecondariesRows Table;
            FSmartTableDeadSecondaries Dead;

            Table.Recompute( Dead, FSmartTableSortSpec() );

            AITEST_FALSE( "With nothing sorted, no column is dead", Dead.Contains( TEXT( "Name" ) ) );
            AITEST_FALSE( "...and the answer is not stale", Dead.IsStale() );

            Table.Recompute( Dead, Sorting::SpecWithPrimary( TEXT( "Kind" ), ESmartTableSortMode::Ascending ) );
            Dead.Clear();

            AITEST_FALSE( "Clearing leaves no column dead", Dead.Contains( TEXT( "Name" ) ) );
            AITEST_FALSE( "...and nothing stale", Dead.IsStale() );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FNothingSortedLeavesNoColumnDead, "SmartTables.DeadSecondaries.NothingSortedLeavesNoColumnDead" );

    struct FAStaleMarkKeepsTheLastAnswerUntilItIsWorkedOutAgain : FAITestBase
    {
        virtual bool InstantTest() override
        {
            FDeadSecondariesRows Table;
            FSmartTableDeadSecondaries Dead;

            const FSmartTableSortSpec ByKind = Sorting::SpecWithPrimary( TEXT( "Kind" ), ESmartTableSortMode::Ascending );
            Table.Recompute( Dead, ByKind );

            Table.Rows[ 1 ][ 2 ] = TEXT( "z" );
            Dead.MarkStale();

            AITEST_TRUE( "A stale mark says the answer may not fit", Dead.IsStale() );
            AITEST_TRUE( "...and keeps the last answer until it is worked out again", Dead.Contains( TEXT( "Name" ) ) );

            Table.Recompute( Dead, ByKind );

            AITEST_FALSE( "Worked out again, the column that now breaks a tie is not dead", Dead.Contains( TEXT( "Name" ) ) );
            AITEST_FALSE( "...and the answer is fresh", Dead.IsStale() );

            return true;
        }
    };
    IMPLEMENT_AI_INSTANT_TEST( FAStaleMarkKeepsTheLastAnswerUntilItIsWorkedOutAgain, "SmartTables.DeadSecondaries.AStaleMarkKeepsTheLastAnswerUntilItIsWorkedOutAgain" );
}

UE_ENABLE_OPTIMIZATION_SHIP

#undef LOCTEXT_NAMESPACE

#endif
